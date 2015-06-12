#include <iomanip>
#include <iostream>
#include <chrono>
#include <stdexcept>

#include "clas12/ccdb/constants_table.hpp"

using namespace std;

using std::chrono::system_clock;

using clas12::ccdb::ConstantsDB;
using clas12::ccdb::ConnectionInfoSQLite;
using clas12::ccdb::ConstantSetInfo;
using clas12::ccdb::get_constants_db;

using clas12::ccdb::ConstantsTable;

int main(int argc, char** argv)
{
    string ccdb_sqlite_file = "clas12.sqlite";
    auto cinfo = ConnectionInfoSQLite(ccdb_sqlite_file);

    int run = 0;
    string variation = "default";
    time_t timestamp = system_clock::to_time_t(system_clock::now());
    auto csinfo = ConstantSetInfo(run, variation, timestamp);

    auto db = get_constants_db(cinfo, csinfo);

    auto ftof_status = ConstantsTable(db,"/calibration/ftof/status");

    auto ones = vector<int>(ftof_status.nrows(), 1);
    ftof_status.elem("left",0,1);
    ftof_status.col("right",ones);

    ftof_status.add_to_database(db,"default",10,100);

    auto sector = ftof_status.col<int>("sector");
    auto panel = ftof_status.col<string>("panel");
    auto paddle = ftof_status.col<int>("paddle");
    auto left = ftof_status.col<int>("left");
    auto right = ftof_status.col<int>("right");

    cout << "                      status\n"
            "sector panel paddle left right\n";
    for (int i=0; i<10; i++)
    {
        cout << setw(4) << sector[i]
             << setw(6) << panel[i]
             << setw(7) << paddle[i]
             << setw(6) << left[i]
             << setw(5) << right[i]
             << endl;
    }
}
