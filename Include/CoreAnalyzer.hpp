#ifndef CORE_ANALYZER_H
#define CORE_ANALYZER_H

#include <map>
#include <vector>
#include <string>

class SinkRecord;   // forward dec.
class Sink;

class CoreAnalyzer
{
    private:
        std::string data_directory;

        std::vector< Sink > sinks;
        int sink_id;    // the sink whose core we're interested in

        std::vector< std::string > var_names;
        std::map< std::string,  std::vector<float> > data_map;  // maps var names to data
        std::map< std::string,  std::vector<float> > bounds_map;  // bounds should all be equal
        std::vector< float > minmax_xyz;    // set by calling check_bounds_map()

        int pixel_size;
        double cell_size;    // in cm
        double cell_vol;
        double half_cell;
        std::vector<float>::size_type n_elems;

        float core_volume;
        double core_region_mass;
        double bound_core_region_mass;
        std::vector<double> core_cell_energies;

        bool sinks_mapped;

        std::vector<unsigned int> core_indices;  // set via ctArcmap() 
        unsigned int sink_cell_index;

        std::vector<float> index_to_position(const unsigned int index);
        void setVariableNames();
        void check_bounds_map();
        void check_data_minmax();

        CoreAnalyzer();   // private default ctor --> Don't use

    public:

        CoreAnalyzer(const std::string base_dir, 
                     const SinkRecord & sink_rec,
                     const int id);  // ctor

        void loadAllData ();

        void mapSinkGravity ();

        void findCoreRegion (); // using libtourtre
        void altFindCoreRegion(); // my hand written algorithm

        float calculateBoundMass ();

        float getRegionVolume ();

};

#endif
