#include <iomanip>
#include <iostream>

#include "clas12/ccdb/constants_table.hpp"
#include "clas12/ccdb/parse_timestamp.hpp"

using namespace std;
using namespace clas12::ccdb;

int main(int argc, char** argv)
{
    auto cinfo = ConnectionInfoMySQL();
    cinfo.user     = "clas12reader";
    cinfo.password = "";
    cinfo.host     = "clasdb.jlab.org";
    cinfo.port     = 3306;
    cinfo.database = "clas12";

    auto csinfo = ConstantSetInfo();
    csinfo.run       = 0;
    csinfo.variation = "default";
    csinfo.timestamp = parse_timestamp("2015-03-20/00:00:00");

    auto db = get_constants_db(cinfo, csinfo);

    auto hv = ConstantsTable(db,"/test/cc_calib/htcc/htcchv");

    for (int c=0; c<hv.ncols(); c++)
    {
        cout << setw(10) << hv.colname(c);
    }
    cout << endl;
    for (int r=0; r<hv.nrows(); r++)
    {
        for (int c=0; c<hv.ncols(); c++)
        {
            cout << setw(10) << hv.elem<string>(c,r);
        }
        cout << endl;
    }
}
