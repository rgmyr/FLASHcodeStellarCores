/*
 * Demonstrates/tests some of the basic Sink/SinkRecord/CoreAnalyzer functionality.
 */

#include "SinkRecord.hpp"
#include "CoreAnalyzer.hpp"
#include "ezOptionParser.hpp"

using std::cout;
using std::endl;

int main(int argc, const char * argv[])
{
    ez::ezOptionParser opt;
    std::string infile;

    // flag for passing a different checkpoint file
    opt.add(
        "/scratch/cerberus/d5/ctss/cosmoCutout_mn2_hdf5_chk_0066",  //default file
        0,      // not required
        1,      // single arg expected
        0,      // ... so, no delimiter
        "Specify a checkpoint file to read from.",   // help info
        "-f",   // allowed option flags
        "-file",
        "-infile",
        "-input_file"
    );

    opt.parse(argc, argv);

    /* set infile to -f argument (or default) */
    opt.get("-f")->getString(infile);

    SinkRecord TestRecord(infile);  // give ctor a filename

    int num_sinks = TestRecord.getNumSinks();
    int i;

    TestRecord.printAllSinks();

    cout << "Masses : (index = " << mass_id << ")" << endl;
    for (i=0; i<num_sinks; ++i) { cout << TestRecord.getAttribute(i, mass_id) << "   "; }
    cout << endl << endl;

    cout << "Y position : (index = " << ypos_id << ")" << endl;
    for (i=0; i<num_sinks; ++i) { cout << TestRecord.getAttribute(i, ypos_id) << "   "; }
    cout << endl << endl;

    cout << "Y velocity : (index = " << yvel_id << ")" << endl;
    for (i=0; i<num_sinks; ++i) { cout << TestRecord.getAttribute(i, yvel_id) << "   "; }
    cout << endl << endl;

    cout << "Formation Times : (index = " << form_time_id << ")" <<  endl;
    for (i=0; i<num_sinks; ++i) { cout << TestRecord.getAttribute(i, form_time_id) << "   "; }
    cout << endl << endl;

    cout << "Accretion Rates : (index = " << acc_rate_id << ")" << endl;
    for (i=0; i<num_sinks; ++i) { cout << TestRecord.getAttribute(i, acc_rate_id) << "   "; }
    cout << endl << endl;

    TestRecord.writeAllScripts();

    std::string data_dir("/data1/r900-1/rossm/QuickFlash-1.0.0/core_code/data/chk_66/sink0/");

    CoreAnalyzer TestAnalyzer(data_dir, TestRecord, 0);

    TestAnalyzer.loadAllData();
    TestAnalyzer.mapSinkGravity();
    TestAnalyzer.altFindCoreRegion();   // using seeded region-growing
    cout << "...calculateBoundMass() returned: " << TestAnalyzer.calculateBoundMass() << endl;

    return 0;
}
