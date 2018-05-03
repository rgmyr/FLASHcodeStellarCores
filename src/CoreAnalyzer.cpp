/*
 *  CoreAnalyzer class implementation
 *  Author: Ross Meyer
 *
 *  Source Outline:
 *      - Local consts and funcs
 *      - Constructor
 *      - Public funcs
 *      - Private funcs
 */

#include "CoreAnalyzer.hpp"
#include "RossGlobals.h"
#include "SinkRecord.hpp"
#include "Sink.hpp"
#include "Mesh.h"
#include "Data.h"

//#include <fstream>
//#include <cstring>
#include <algorithm>    // std::count, std::equal, std::max/min_element
#include <iterator>     // std::distance
#include <set>
#include <limits>
#include <math.h>       // sqrt()
//#include <time.h>
//#include <unistd.h>

extern "C" 
{
#include <tourtre.h>
// need this to access ctx info directly
#include "ctContext.h"
}

using std::cout;
using std::cerr;
using std::endl;
using std::vector;

// LOCAL constants and functions

const float G = 6.674e-8;   // in cgs
float min_distance = 1.0e19;

float vec_distance(const vector<float>& pos1, const vector<float>& pos2)
{
    float xdist = pos1[0] - pos2[0];
    float ydist = pos1[1] - pos2[1];
    float zdist = pos1[2] - pos2[2];
    return sqrt(xdist*xdist + ydist*ydist + zdist*zdist);
}

float sink_grav(const vector<float>& pos, const Sink& sink)
{
    return G*sink.getMass() / vec_distance(pos, sink.getPosition());
}

double value ( size_t v, void * d ) {
	Mesh * mesh = reinterpret_cast<Mesh*>(d);
	return mesh->data[v];
}

size_t neighbors ( size_t v, size_t * nbrs, void * d ) {
	Mesh * mesh = static_cast<Mesh*>(d);
	static vector<size_t> nbrsBuf;
	
	nbrsBuf.clear();
	mesh->getNeighbors(v,nbrsBuf);
	
	for (uint i = 0; i < nbrsBuf.size(); i++) {
		nbrs[i] = nbrsBuf[i]; 
	}
	return nbrsBuf.size();
}

// CONSTRUCTOR
// NOTE: pass base_dir without trailing "/" for proper sink id setting
CoreAnalyzer::CoreAnalyzer(const std::string base_dir, 
                           const SinkRecord & sink_rec,
                           const int id):
    data_directory(base_dir),
    sink_id(id),
    sinks_mapped(false)
{
    sinks = sink_rec.getMySinks();   // does this work?
    setVariableNames();
}


/*
 *      PUBLIC FUNCTIONS
 */

void CoreAnalyzer::loadAllData()
{
    cout << "Called CoreAnalyzer::loadAllData()" << endl << endl;

    // LOAD MINMAX_XYZ 

    vector< std::string >::const_iterator it;

    n_elems = 512*512*512; // should be more general in the future
    pixel_size = 512;

    for (it = var_names.begin(); it != var_names.end(); ++it)
    {
        std::string file_var_name = (*it).substr(0,4);
        std::string filename = data_directory + "extracted_" + file_var_name;

        vector<float> temp_var(n_elems, 0.0);
        vector<float> temp_bounds(6, 0.0);
        // pass the vector as C-style array
        loadArrayFromHDF(&temp_var[0], filename, *it);
        data_map[*it] = temp_var;

        loadArrayFromHDF(&temp_bounds[0], filename, "minmax_xyz");
        bounds_map[*it] = temp_bounds;

        cout << "CoreAnalyzer::loadAllData --> inserted " << *it << " vector in data_map";
        cout << endl << "  MinMax_xyz = ";
        print_container(temp_bounds);
        cout << endl << endl;
    }

    check_bounds_map();
    check_data_minmax();
}

void CoreAnalyzer::mapSinkGravity()
{
    cout << "CoreAnalyzer::mapSinkGravity() called..." << endl;

    unsigned int i(0);      // to keep track of flat index
    // iterate over gpot cells
    vector<float>::iterator gpot_it;
    for (gpot_it = data_map["gpot"].begin(); gpot_it != data_map["gpot"].end(); ++gpot_it)
    {
        vector<float> cell_pos = index_to_position(i);

        // iterate over sink particles
        vector<Sink>::const_iterator sink_it;
        int sink_counter = 0;
        for (sink_it = sinks.begin(); sink_it != sinks.end(); ++sink_it)
        {
            float dist_to_sink = vec_distance(cell_pos, sink_it->getPosition());

            if (dist_to_sink == 0.0){
                cout << "THERE WAS A DISTANCE = 0!!!" << endl;
                dist_to_sink = 0.01;
            }


            if ((sink_counter == sink_id) && (dist_to_sink < min_distance))
            {
                min_distance = dist_to_sink;
                sink_cell_index = i;
            }

            *gpot_it -= G*sink_it->getMass() / dist_to_sink;

            sink_counter++;
        }
        i++;
    }
    cout << "The closest cell to the sink particle is index: " << sink_cell_index << endl;
    cout << "       with distance: " << min_distance << endl << endl;
    sinks_mapped = true;
}


void CoreAnalyzer::findCoreRegion()
{
    cout << "CoreAnalyzer::findCoreRegion() called... " << endl;

    if (!sinks_mapped)
    {
        cerr << "PROBLEM!!! Attempting to find core region before mapping sink grav." << endl;
        return;
    }

    Data data;
    data.loadFromVector(data_map["gpot"], data_map["gpot"].size() ); 

    cout << "Number of data points: " << data.totalSize << endl;
    cout << endl;

    //Create mesh
    Mesh mesh(data);
    std::vector<size_t> totalOrder;
    mesh.createGraph( totalOrder ); //this just sorts the vertices according to data.less()

    //init libtourtre
    ctContext * ctx = ct_init(
        data.totalSize, //numVertices
        &(totalOrder.front()), // c array style
        &value, // callback funcs
        &neighbors,
        &mesh //data for callbacks.
    );
    cout << "Initialized ctContext" << endl;

    //create contour tree
    ct_sweepAndMerge( ctx );

    ctBranch * root = ct_decompose( ctx );

    // ARC MAPPING STUFF
    cout << "Getting arc_map..." << endl;
    ctArc ** arc_map = ct_arcMap( ctx );
    vector<ctArc*> arc_vec(arc_map, arc_map + data.totalSize);
    cout << "size of arc_vec = " << arc_vec.size() << endl;

    ctArc * sink_arcptr = arc_vec[sink_cell_index];

    vector<ctArc*>::const_iterator arc_it;
    unsigned int i(0);
    for (arc_it = arc_vec.begin(); arc_it != arc_vec.end(); ++arc_it)
    {
        if (*arc_it == sink_arcptr) // if cell is associated with same arc as sink
        {
            core_indices.push_back(i);
        }
        i++;
    }
    cout << "core_indices have been set... there are " << core_indices.size() << " cells" << endl;

    ct_cleanup( ctx );
}

void CoreAnalyzer::altFindCoreRegion()
{
    cout << "CoreAnalyzer::altFindCoreRegion() called... " << endl;

    if (!sinks_mapped)
    {
        cerr << "PROBLEM!!! Attempting to find core region before mapping sink grav." << endl;
        return;
    }
    Data data;
    data.loadFromVector(data_map["gpot"], data_map["gpot"].size() ); 

    cout << "Number of data points: " << data.totalSize << endl;
    cout << endl;

    // Create mesh -- Should maybe smooth the data first? Could put code in Data or Mesh.
    Mesh mesh(data);
    std::vector<size_t> totalOrder;
    mesh.createGraph( totalOrder ); // sorts the vertices according to data.less()

    std::vector<size_t> holder (data.totalSize, 0);    // placeholder
    holder[sink_cell_index] = 2;    // "key" for sink region
    
    std::vector<size_t>::const_iterator it;
    for (it = totalOrder.begin(); it != totalOrder.end(); ++it)
    {
        std::vector<size_t> neighbs;
        mesh.getNeighbors18(*it, neighbs);
        std::vector<size_t> hold_nbs;
        std::vector<size_t>::const_iterator hit;
        for (hit = neighbs.begin(); hit != neighbs.end() ; ++hit)
        {
            hold_nbs.push_back(holder[*hit]);
        }
        
        // if not neighboring current sink region, mark as 'outside' = 1
        bool sink_neighbor = (std::find(hold_nbs.begin(), hold_nbs.end(), 2) != hold_nbs.end());
        if (!sink_neighbor){
            holder[*it] = 1;
            continue;
        }

        // if neighboring sink region but not the 'outside' region, mark as 'sink' = 2
        bool outside_neighbor = (std::find(hold_nbs.begin(), hold_nbs.end(), 1) != hold_nbs.end());
        if (!outside_neighbor){ // and sink_neighbor implicitly
            holder[*it] = 2;
            continue;
        }

        // if it neighbors both regions, mark as 'sink'(?) and we're done
        holder[*it] = 2;
        break;
    }
    
    for (it = holder.begin(); it != holder.end(); ++it)
    {
        if (*it == 2)
            core_indices.push_back(*it);
    }

}



float CoreAnalyzer::calculateBoundMass ()
{
    double core_CoM_velx(0.0), core_CoM_vely(0.0), core_CoM_velz(0.0);
    double core_region_mass(0.0), core_px(0.0), core_py(0.0), core_pz(0.0);
    double bound_core_mass(0.0);
    double reference_gpot(-std::numeric_limits<double>::max());
    cout << "minimum double value = " << reference_gpot << endl;
    cout << "cell_vol = " << cell_vol << endl;
    unsigned int num_bound_cells(0);

    vector<unsigned int>::const_iterator it;
    // find CoM
    for (it = core_indices.begin(); it != core_indices.end(); ++it)
    {
        double cell_mass = cell_vol*data_map["dens_pp"][*it];
        core_px += cell_mass*data_map["velx_pp"][*it];
        core_py += cell_mass*data_map["vely_pp"][*it];
        core_pz += cell_mass*data_map["velz_pp"][*it];
        core_region_mass += cell_mass;

        if (data_map["gpot"][*it] > reference_gpot)
        {
           // reference_gpot was initialized to smallest possible float
           reference_gpot = data_map["gpot"][*it]; 
        }
            
    }
    cout << "Reference (max in core region) gpot: " << reference_gpot << endl;

    core_CoM_velx = core_px / core_region_mass;
    core_CoM_vely = core_py / core_region_mass;
    core_CoM_velz = core_pz / core_region_mass;

    // Calculate cell energies and check for those < 0
    for (it = core_indices.begin(); it != core_indices.end(); ++it)
    {
        double vx_rel, vy_rel, vz_rel, Etherm, Ekin, Egrav, Etotal;

        double cell_mass = cell_vol*data_map["dens_pp"][*it];

        vx_rel = data_map["velx_pp"][*it] - core_CoM_velx;
        vy_rel = data_map["vely_pp"][*it] - core_CoM_vely;
        vz_rel = data_map["velz_pp"][*it] - core_CoM_velz; 
        Ekin = 0.5 * cell_mass * (vx_rel*vx_rel + vy_rel*vy_rel + vz_rel*vz_rel);

        Etherm = cell_mass * data_map["eint"][*it];

        Egrav = cell_mass * (data_map["gpot"][*it]-reference_gpot);

        Etotal = Ekin + Etherm + Egrav;
        if (Etotal < 0.0)
        {
            bound_core_mass += cell_mass;
            ++num_bound_cells;
        }
        if (*it == sink_cell_index)
        {
            cout << "   For cell nearest to sink:" << endl;
            cout << "       Ekin = " << Ekin << endl;
            cout << "       Etherm = " << Etherm << endl;
            cout << "       Egrav = " << Egrav << endl;
            cout << "       Total Energy = " << Etotal << endl << endl;
        }
    }
    cout << "There were " << core_indices.size() << "cells" << num_bound_cells << " bound)" << endl;
    cout << "--> Total core region mass (Msol): " << core_region_mass / 2.0e33 << endl;
    cout << "--> Bound core mass (Msol): " << bound_core_mass / 2.0e33 << endl << endl;

    return bound_core_mass;
}


/*
 *      PRIVATE FUNCTIONS
 */

void CoreAnalyzer::setVariableNames()
{
    var_names.push_back("gpot");
    var_names.push_back("dens_pp");     // I think the "_pp" just means sink contrib. added
    var_names.push_back("eint");
    var_names.push_back("velx_pp");
    var_names.push_back("vely_pp");
    var_names.push_back("velz_pp");
}

vector<float> CoreAnalyzer::index_to_position(const unsigned int id)
{
    vector<float> position;

    if (id >= n_elems) {
        std::cerr << "Error: trying to convert index of a point outside bounds" << endl;
    }

    int slab_size = pixel_size * pixel_size;    // make it more general?
    int x, y, z;

    z = id / slab_size;
    y = (id - z*slab_size) / pixel_size;
    x = id - z*slab_size - y*pixel_size;

    position.push_back(minmax_xyz[0] + x*cell_size + half_cell);
    position.push_back(minmax_xyz[2] + y*cell_size + half_cell);
    position.push_back(minmax_xyz[4] + z*cell_size + half_cell);

    if (id == 100) // value is arbitrary -- just checking
    {
        cout << "For id = 100... 3D index = : " << x << " " << y << " " << z << endl;
        cout << "              position = : ";
        print_container(position);
        cout << "              (where cell size = " << cell_size << " )" << endl;
    }

    return position;
}

void CoreAnalyzer::check_bounds_map()   // make sure bounds are okay
{
    vector< std::string >::const_iterator it;
    for (it = var_names.begin()+1; it != var_names.end(); ++it)
    {
        if (! std::equal(bounds_map[*it].begin(),
                        bounds_map[*it].end(),
                        bounds_map[*(it-1)].begin()))
        {
            cerr << "PROBLEM!!! --> unequal bounds for: ";
            cerr << *(it-1) << "and" << *it << endl;
        }
        else
        {
            cout << "Data bounds matched: " << *(it-1) << " and " << *it << endl;
        }
    }
    cout << endl;

    minmax_xyz = bounds_map["gpot"];
    cell_size = (minmax_xyz[1] - minmax_xyz[0])/pixel_size;
    half_cell = cell_size * 0.5;
    cell_vol = cell_size * cell_size * cell_size;
}

void CoreAnalyzer::check_data_minmax()
{
    cout << "Data minimum and maximum check:" << endl;
    vector< std::string >::const_iterator it;
    for (it = var_names.begin(); it != var_names.end(); ++it)
    {
        vector<float>::iterator biggest;
        biggest = std::max_element(data_map[*it].begin(), data_map[*it].end());
        vector<float>::iterator smallest;
        smallest = std::min_element(data_map[*it].begin(), data_map[*it].end());

        cout << "       " << *it << ":" << endl;
        cout << "           Max: " << *biggest; 
        cout << "       at index: " << std::distance(data_map[*it].begin(), biggest) << endl;
        cout << "           Min: " << *smallest;
        cout << "       at index: " << std::distance(data_map[*it].begin(), smallest) << endl;
        cout << endl;
    }
}

