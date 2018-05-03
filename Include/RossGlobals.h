#ifndef ROSS_HEADER_H
#define ROSS_HEADER_H

#include "HDFIO.h"
#include <string>

extern hid_t HDFDataType;   // set by calling CheckMachineFor...() below
extern bool HDFDataType_is_set;     // ...only need to call it once

const float simulation_max_bound = 1.58967e+18;
const float extract_size =  7.5e16;     // set the desired extraction grid size (cm)
const int extract_pixels =  512;     // set the desired extraction resolution (pixels^3)

static const int mass_id = 5;
static const int xpos_id = 9;     
static const int ypos_id = 10;    
static const int zpos_id = 11;
static const int xvel_id = 14;    
static const int yvel_id = 15;    
static const int zvel_id = 16;
static const int form_time_id = 21;
static const int acc_rate_id = 22;


/*
 * Call this to set the global HDFDataType variable before reading data with HDFIO
 */
void CheckMachineForLittleBigEndianNumerics(void);

/*
 * Data from the specified filename and dataset name is loaded into *data array
 */
void loadArrayFromHDF(float* data, std::string filename, std::string dataset_name);

/*
 * Print the contents of an STL container
 */
template <typename T>
void print_container (T const & container)
{
    typename T::const_iterator pos;
    typename T::const_iterator end( container.end() );

    for (pos = container.begin(); pos != end; ++pos){
        std::cout << *pos << " ";
    }
    std::cout << std::endl;
}


#endif
