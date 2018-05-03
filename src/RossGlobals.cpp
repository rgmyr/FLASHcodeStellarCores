
#include "RossGlobals.h"
#include <iostream>

hid_t HDFDataType;
bool HDFDataType_is_set = false;

/*
 * Call this to set the global HDFDataType variable for reading data
 */
void CheckMachineForLittleBigEndianNumerics(void)
{
    int TestInteger = 1;
    char *pc = (char*) &TestInteger;
    // lowest address contains the least significant byte
    if (pc[0] == 1)
    {
        std::cout << "CheckMachineForLittleBigEndianNumerics:  --> We have LITTLE ENDIAN on this machine." << std::endl << std::endl;
        HDFDataType = H5T_IEEE_F32LE;
    }
    else
    {
        std::cout << "CheckMachineForLittleBigEndianNumerics:  --> We have BIG ENDIAN on this machine." << std::endl << std::endl;
        HDFDataType = H5T_IEEE_F32BE;
    }
    HDFDataType_is_set = true;
}

/*
 * Data from the specified filename and dataset name is loaded into *data array
 */
void loadArrayFromHDF(float* data, std::string filename, std::string dataset_name)
{
    if (!HDFDataType_is_set)
    {
        CheckMachineForLittleBigEndianNumerics();
    }
    HDFIO HDFInput = HDFIO();
    HDFInput.open(filename, 'r');
    std::cout << "---> loadArrayFromHDF:  Reading dataset: " << dataset_name << std::endl;
    HDFInput.read(data, dataset_name, ::HDFDataType);
    HDFInput.close();
}

