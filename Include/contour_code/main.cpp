#include <iostream>
#include <fstream>
#include <cstring>
#include <set>
#include <vector>
#include <algorithm> // std::count

extern "C" 
{
#include <tourtre.h>
// need this to access ctx info directly
#include "ctContext.h"
}

#include "Data.h"
#include "Mesh.h"

#include <unistd.h>

using std::cin;
using std::cout;
using std::clog;
using std::cerr;
using std::endl;


double value ( size_t v, void * d ) {
	Mesh * mesh = reinterpret_cast<Mesh*>(d);
	return mesh->data[v];
}


size_t neighbors ( size_t v, size_t * nbrs, void * d ) {
	Mesh * mesh = static_cast<Mesh*>(d);
	static std::vector<size_t> nbrsBuf;
	
	nbrsBuf.clear();
	mesh->getNeighbors(v,nbrsBuf);
	
	for (uint i = 0; i < nbrsBuf.size(); i++) {
		nbrs[i] = nbrsBuf[i]; 
	}
	return nbrsBuf.size();
}


void outputTree( std::ofstream & out, ctBranch * b ) {
	out << "(" << b->extremum << ' ' << b->saddle ;
	
	for ( ctBranch * c = b->children.head; c != NULL; c = c->nextChild ){
		out << " ";
		outputTree( out, c );
	}
	
	out << ")";
} 


int main( int argc, char ** argv ) {


	int errflg = 0;
	int c;
	
	//command line parameters
	char filename[1024] = "";
	char outfile[1024] = "";
	
	#if USE_ZLIB
	char switches[256] = "i:o:";
	#else
	char switches[256] = "i:o:";
	#endif

    cout << endl;
	while ( ( c = getopt( argc, argv, switches ) ) != EOF ) {
		switch ( c ) {
				case 'i': {
					strcpy(filename,optarg);
                    cout << "Input File: " << filename << endl;
					break;
				}
				case 'o': {
					strcpy(outfile,optarg);
                    cout << "Output file: " << outfile << endl;
                    cout << endl;
					break;
				}
				
				case '?':
					errflg++;
		}
	}
		
	if ( errflg || filename[0] == '\0') {
		clog << "usage: " << argv[0] << " <flags> " << endl << endl;
		
		clog << "flags" << endl;
		clog << "\t -i < filename >  :  filename" << endl;
		clog << "\t -o < filename >  :  filename" << endl;
		clog << endl;

		clog << "Filename must be of the form <name>.<i>x<j>x<k>.<type>" << endl;
		clog << "i,j,k are the dimensions. type is one of uint8, uint16, float or double." << endl << endl;
		clog << "eg, \" turtle.128x256x512.float \" is a file with 128 x 256 x 512 floats." << endl;
		clog << "i dimension varies the FASTEST (in C that looks like \"array[k][j][i]\")" << endl;
		clog << "Data is read directly into memory -- ENDIANESS IS YOUR RESPONSIBILITY." << endl;

		return(1);
	}


	char prefix[1024];

	//Load data
	Data data;
	bool compress;
	if (!data.load( filename, prefix, &compress )) {
		cerr << "Failed to load data" << std::endl;
		exit(1);
	}
    cout << "Number of data points: " << data.totalSize << endl;
    cout << endl;

	//Create mesh
	Mesh mesh(data);
	std::vector<size_t> totalOrder;
	mesh.createGraph( totalOrder ); //this just sorts the vertices according to data.less()
	
	//init libtourtre
	ctContext * ctx = ct_init(
		data.totalSize, //numVertices
		&(totalOrder.front()), //totalOrder. Take the address of the front of an stl vector, which is the same as a C array
		&value,
		&neighbors,
		&mesh //data for callbacks. The global functions less, value and neighbors are just wrappers which call mesh->getNeighbors, etc
	);
	
	//create contour tree
	ct_sweepAndMerge( ctx );
	ctBranch * root = ct_decompose( ctx );

    cout << "Number of vertices: " << (*ctx).numVerts << endl;
    cout << endl;

    // ARC MAPPING STUFF
    cout << "Getting arc_map..." << endl;
    ctArc ** arc_map = ct_arcMap( ctx );

    std::set<ctArc*> arcset(arc_map, arc_map + data.totalSize);
    cout << "--> Size of set(arc_map): " <<  arcset.size() << endl;
    std::vector<ctArc*> arc_vec(arc_map, arc_map + data.totalSize);
    cout << "--> Size of vector(arc_map): " << arc_vec.size() << endl;

    ctArc * mid_value = arc_map[data.totalSize/2-1];
    cout << "--> ctArc pointer near arc_map[] midpoint: " << mid_value << endl;
    cout << "--> Vertices in associated ctArc: " << std::count(arc_vec.begin(), arc_vec.end(), mid_value) << endl;
    cout << "... but are vertices sorted by location, or by value now? Need to check ..." << endl;
    cout << endl;
    
    //create branch decomposition
    cout << "Getting branch_map..." << endl;
    ctBranch ** map = ct_branchMap(ctx);

    std::set<ctBranch*> branch_set(map, map + data.totalSize);
    cout << "--> Size of set(branch_map): " << branch_set.size() << endl;
    cout << endl;

	ct_cleanup( ctx );
	
	//output tree
	std::ofstream out(outfile,std::ios::out);
	if (out) {
		outputTree( out, root);
	} else {
		cerr << "couldn't open output file " << outfile << endl;
	}
	
}



	
