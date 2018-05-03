/*
 * Sink class for interacting with individual sink particle data. The SinkRecord class
 * owns a vector of Sink instances for each particle in the input file.
 */

#ifndef SINK_H
#define SINK_H

#include <string>
#include <vector>

class Sink
{
    private:
        float mass, xpos, ypos, zpos, xvel, yvel, zvel, form_time, acc_rate;
        std::vector< std::string > var_names;

        int ID;
        bool has_ID;

        Sink();   // don't use default constructor

    public:
        // pass a data pointer to the ctor, at the desired first attribute in FLASH sink dataset
        Sink(const float*); 

        void setVariableNames();

        // getters and setters --> delete all unused ones later
        void setID(const int);

        std::vector<float> getPosition() const;
        std::vector<float> getVelocity() const;
        float getFormationTime() const;
        float getMass() const;

        void writeExtractorScript(std::string base_dir, 
                                  int chk, 
                                  std::string data_file) const;
        void printMe() const;

        // so that SinkRecord sorts by formation time
        friend bool operator< (const Sink&, const Sink&); 
};

#endif
