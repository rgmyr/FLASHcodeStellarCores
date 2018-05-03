/* 
 * The Sink Record class reads, prints, and writes information related to sink data
 * read directly from the checkpoint files. The ctor should be passed a filename.
 */

#ifndef SINK_RECORD_H
#define SINK_RECORD_H

#include "RossGlobals.h"
#include "Sink.hpp"
#include <vector>
#include <string>

class HDFIO;    // forward dec.

class SinkRecord
{
    private:     
        HDFIO InOut;
        std::string Filename;

        std::vector<int> data_dims;
        unsigned int num_sinks, num_attributes;
        long data_size;

        float * DataPointer;
        bool has_been_allocated;

        std::vector< Sink > mySinks;

        SinkRecord(); // don't use default ctor
    
    public:     // some of these should be const --> need to fix that

        SinkRecord(const std::string);  // pass filename to the ctor

        ~SinkRecord() { delete [] DataPointer; }

        void resetFromFile();

        float getAttribute(const unsigned int, const unsigned int) const;

        int getNumSinks(void) const { return num_sinks; }

        std::vector< Sink > getMySinks() const { return mySinks; }

        void WriteSinkRecord(const int, const std::string); // currently unused

        void writeAllScripts() const;

        void printAllData(void);

        void printAllSinks(void);
};

#endif
