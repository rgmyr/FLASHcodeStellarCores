/*
 * Sink class implementation
 */

#include "RossGlobals.h"
#include "Sink.hpp"
#include <sstream>
#include <fstream>
#include <iostream>

using std::cout;
using std::endl;

Sink::Sink(const float * data)
{
    has_ID = false;
    mass = data[mass_id];
    xpos = data[xpos_id];
    ypos = data[ypos_id];
    zpos = data[zpos_id];
    xvel = data[xvel_id];
    yvel = data[yvel_id];
    zvel = data[zvel_id];
    form_time = data[form_time_id];
    acc_rate = data[acc_rate_id];
    setVariableNames();
}

void Sink::setVariableNames()
{
    var_names.push_back("gpot");
    var_names.push_back("dens");
    var_names.push_back("eint");
    var_names.push_back("velx");
    var_names.push_back("vely");
    var_names.push_back("velz");
}

void Sink::setID(const int num)
{
    ID = num;
    has_ID = true;
}

std::vector<float> Sink::getPosition(void) const
{
    std::vector<float> XYZ;
    XYZ.push_back(xpos);
    XYZ.push_back(ypos);
    XYZ.push_back(zpos);
    return XYZ;
}

std::vector<float> Sink::getVelocity(void) const
{
    std::vector<float> XYZvel;
    XYZvel.push_back(xvel);
    XYZvel.push_back(yvel);
    XYZvel.push_back(zvel);
    return XYZvel;
}

float Sink::getFormationTime(void) const
{
    return form_time;
}

float Sink::getMass(void) const
{
    return mass;
}

void Sink::writeExtractorScript(const std::string base_dir, 
                                const int chk, 
                                const std::string data_file) const
{
    if (!has_ID)
    {
        cout <<"Attempting to write script for un-IDed sink... call ->setID(int) first."<< endl;
        return;
    }
    else
    {
    // Create and open the script file
    std::stringstream ss;
    ss << base_dir << "/extract_scripts" << "/chk_" << chk << "/extract_sink" << ID << ".sh";
    std::string script_fname( ss.str() );
    std::ofstream outfile(script_fname.c_str(), std::ofstream::out);

    cout << "Sink" << ID << " writing extractor script to:" << endl
              << "        " << script_fname << endl;
    
    std::vector< std::string >::const_iterator it; 
    for(it = var_names.begin(); it != var_names.end(); ++it)
    {
        ss.str("");     // clears the stringstream
        ss << "/data1/r900-1/rossm/QuickFlash-1.0.0/extractor/extractor --range=";

        // CHECK X COORDINATE BOUNDS
        float x_lower = xpos- 0.5*extract_size;
        float x_upper = xpos+ 0.5*extract_size;
        if (x_lower < 0.0)
        {
            ss << "0.0," << extract_size << ",";
        } 
        else if (x_upper > simulation_max_bound)
        {
            ss << simulation_max_bound-extract_size << "," << simulation_max_bound << ",";
        }
        else
        {
            ss << x_lower << "," << x_upper << ",";
        }

        // CHECK Y COORDINATE BOUDNS
        float y_lower = ypos- 0.5*extract_size;
        float y_upper = ypos+ 0.5*extract_size;
        if (y_lower < 0.0)
        {
            ss << "0.0," << extract_size << ",";
        }
        else if (y_upper > simulation_max_bound)
        {
            ss << simulation_max_bound-extract_size << "," << simulation_max_bound << ",";
        }
        else
        {
            ss << y_lower << "," << y_upper << ",";
        }

        // CHECK Z COORDINATE BOUNDS
        float z_lower = zpos- 0.5*extract_size;
        float z_upper = zpos+ 0.5*extract_size;
        if (z_lower < 0.0)
        {
            ss << "0.0," << extract_size << " ";
        }
        else if (z_upper > simulation_max_bound)
        {
            ss << simulation_max_bound-extract_size << "," << simulation_max_bound << " ";
        }
        else
        {
            ss << z_lower << "," << z_upper << " ";
        }

        if (*it == "dens" || it->substr(0,3) == "vel")  // can't use for any others
        {
            ss << "--add_sink_contribution ";
        }

        ss << "--override_amr_consistent_extraction ";  // unless you don't want to
        ss << "--pixel=" << extract_pixels << " ";
        ss << "--varname=" << *it << " ";
        ss << "--outfile=" << base_dir << "/data/chk_" << chk
            << "/sink" << ID << "/extracted_" << *it << " ";
        ss << data_file << endl;

        if (*it != "velz") { ss << endl; }  // just to be clean about it ;)
        
        outfile << ss.rdbuf();
    }
    outfile.close();
    cout << ".......... Done." << endl << endl;
    }
}
    

void Sink::printMe() const
{
    if (!has_ID)
    {
        cout << "You're trying to print an un-IDed sink... call ->setID(int) first." << endl;
        return;
    }
    else
    {
    cout << "ID: " << ID << endl;
    cout << "   mass:               " << mass << endl;
    cout << "   position (x, y, z): " << xpos << "  " << ypos << "  " << zpos << endl;
    cout << "   velocity (x, y, z): " << xvel << "  " << yvel << "  " << zvel << endl;
    cout << "   formation time:     " << form_time << endl;
    cout << "   accretion rate:     " << acc_rate << endl;
    cout << endl;
    }
}

// Compare Sinks based on formation times:
//  --> this way ID's should be the same for a given sink in all files
bool operator< (const Sink& lhs, const Sink& rhs)
{
    return lhs.getFormationTime() < rhs.getFormationTime();
}


