/* 
 * The Sink Record class reads, prints, and writes information related to sink data
 * read directly from the checkpoint files. The ctor should be passed a filename.
 */

#include "RossGlobals.h"
#include "HDFIO.h" 
#include "Sink.hpp"
#include "SinkRecord.hpp"
#include <iostream>

using std::cout;
using std::endl;

SinkRecord::SinkRecord(const std::string fname):
    Filename(fname),
    has_been_allocated(false)
{
    resetFromFile();
}

void SinkRecord::resetFromFile()
{
    InOut.open(Filename, 'r');

    data_dims = InOut.getDims("sink particle data");
    data_size = InOut.getSize("sink particle data");
    num_sinks = data_dims[0];
    num_attributes = data_dims[1];
    // Probably superfluous, but these should always be the same?
    assert( (num_sinks*num_attributes) == data_size );

    cout << "Sink Data dimensions: "; print_container( data_dims );
    cout << "Data size: " << data_size << endl;

    // Global function sets global var: HDFDataType
    CheckMachineForLittleBigEndianNumerics();

    if (has_been_allocated) { delete [] DataPointer; }

    DataPointer = new float[data_size];
    has_been_allocated = true;
    InOut.read(DataPointer, "sink particle data", ::HDFDataType);
    InOut.close();

    unsigned int i;
    for (i = 0; i < num_sinks; i++)
    {
        // call Sink ctor and push onto mySinks
        mySinks.push_back( Sink( &DataPointer[i*num_attributes] ) );
    }

    // sorts the sinks by formation times, so they always have the same ID
    std::sort( mySinks.begin(), mySinks.end() );

    // give the sinks a unique int ID from sorted order
    for (i = 0; i < num_sinks; i++)
    {
        mySinks[i].setID(i);
    }
}

float SinkRecord::getAttribute(const unsigned int sink_id, const unsigned int attr_id) const
{
    if (attr_id >= num_attributes)
    {
        cout
            << "Warning: attribute index must be less than the number of attributes"
            << " to avoid errors" << endl << "You passed: " << attr_id << endl
            << "Number of attributes: " << num_attributes << endl;
        return 1;
    } else if (sink_id >= num_sinks) {
        cout
            << "Error --> passed sink_id: " << sink_id << " with num_sinks: " << num_sinks
            << endl;
        return 1;
    } else {
        return this->DataPointer[sink_id * num_attributes + attr_id];
    }
}

// For writing meta info to file adjacent to extracted datasets
// -- might not need this, just run everything from one main and
// give reference to SinkRecord to CoreAnalyzer?
void WriteSinkRecord(const int sink_id, const std::string outfile_name)
{

}

void SinkRecord::writeAllScripts() const
{
    cout << "SinkRecord::writeAllScripts() called..." << endl << endl;

    std::string baseDir("/data1/r900-1/rossm/QuickFlash-1.0.0/core_code");

    std::vector< Sink >::const_iterator it;
    for (it = mySinks.begin(); it != mySinks.end(); ++it)
    {
        int chk_num = 66;   // needs to be more general
        it->writeExtractorScript(baseDir, chk_num, Filename);
    }
}

void SinkRecord::printAllData(void)
{
    for (int i = 0; i < data_size; i++)
    {
        cout << DataPointer[i] << endl;
        if (i % (num_attributes-1) == 0){ cout << endl; }
    }
}

void SinkRecord::printAllSinks(void)
{
    cout << endl; 

    std::vector< Sink >::iterator it;
    for (it = mySinks.begin(); it != mySinks.end(); ++it)
    {
        it->printMe();
    }
}

