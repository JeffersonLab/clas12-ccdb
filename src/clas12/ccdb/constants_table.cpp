#include "constants_table.hpp"

#include <algorithm>
#include <climits>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <iomanip>
#include <fstream>
#include <string>
#include <sstream>
#include <stdexcept>

#include <boost/filesystem.hpp>

#include "CCDB/CalibrationGenerator.h"
#include "CCDB/Calibration.h"

namespace clas12
{
namespace ccdb
{

namespace fs = boost::filesystem;

using std::string;
using std::stringstream;
using std::time;
using std::time_t;
using std::vector;
using std::setw;

using ::ccdb::CalibrationGenerator;
using ::ccdb::Assignment;

typedef ::ccdb::Calibration ConstantsDB;

typedef vector<string> ColumnData;
typedef vector<ColumnData> TableData;
typedef vector<string> ColumnNames;
typedef vector<string> ColumnTypes;

ConnectionInfoMySQL::ConnectionInfoMySQL(
    const string& user     ,
    const string& password ,
    const string& host     ,
          int     port     ,
    const string& database
)
: user(user)
, password(password)
, host(host)
, port(port)
, database(database)
{}

string ConnectionInfoMySQL::connection_string() const
{
    stringstream ss;
    ss << "mysql://";
    ss << user;
    if (password != "")
    {
        ss << ":" << password;
    }
    ss << "@" << host;
    ss << ":" << port;
    ss << "/" << database;
    return ss.str();
}

ConnectionInfoSQLite::ConnectionInfoSQLite(string filepath)
: filepath(filepath)
{}

string ConnectionInfoSQLite::connection_string() const
{
    fs::path path(filepath);
    if (! fs::exists(path))
    {
        throw std::invalid_argument( "Path: '" +
            path.string() + "' could not be found." );
    }
    else if (! fs::is_regular_file(path))
    {
        throw std::invalid_argument( "Path: '" +
            path.string() + "' is not a regular file." );
    }
    else
    {
        return "sqlite://" + path.string();
    }
}


ConstantSetInfo::ConstantSetInfo(
          int     run      ,
    const string& variation,
          time_t  timestamp)
: run(run)
, variation(variation)
, timestamp(timestamp)
{}

string ConstantSetInfo::constant_set_string(const string& table_path) const
{
    stringstream ss;

    ss << table_path;

    if (run != INT_MAX)
    {
        ss << ":" << run;
    }
    else if (variation != "default" || timestamp != 0)
    {
        ss << ":";
    }

    if (variation != "default")
    {
        ss << ":" << timestamp;
    }
    else if (timestamp != 0)
    {
        ss << ":";
    }

    if (timestamp != 0)
    {
        ss << ":" << timestamp;
    }

    return ss.str();
}

unique_ptr<ConstantsDB> get_constants_db()
{
    auto cinfo = ConnectionInfoMySQL();
    auto csinfo = ConstantSetInfo();
    return get_constants_db(cinfo,csinfo);
}

unsigned int ConstantsTable::find_column(const string& colname)
{
    return distance(
        columns.begin(),
        find(columns.begin(), columns.end(), colname) );
}

ConstantsTable::ConstantsTable(
    const unique_ptr<ConstantsDB>& db,
    const string& table_path )
: table_path(table_path)
{
    bool disconnect = false;
    if (!db->IsConnected())
    {
        disconnect = true;
        db->Connect(db->GetConnectionString());
    }

    unique_ptr<Assignment> assignment(
        db->GetAssignment(table_path, true) );
    values = assignment->GetData();
    auto* type_table = assignment->GetTypeTable();
    columns = type_table->GetColumnNames();
    column_types = type_table->GetColumnTypeStrings();
}

string ConstantsTable::write_to_file(const string& fname, bool header)
{
    fs::path filepath;
    if (fname == "")
    {
        // dump data into a temporary file
        filepath = std::tmpnam(nullptr);
    }
    else
    {
        filepath = fname;
        if (fs::exists(filepath))
        {
            stringstream err;
            err << "Output file already exists: " << filepath << endl;
            throw std::invalid_argument(err.str());
        }
    }

    std::ofstream fout(filepath.string().c_str());
    if (header)
    {
        int maxwidth = 0;
        for (auto c: this->columns)
        {
            if (c.size() > maxwidth)
            {
                maxwidth = c.size();
            }
        }
        for (auto t: this->column_types)
        {
            if (t.size() > maxwidth)
            {
                maxwidth = t.size();
            }
        }
        maxwidth += 1;

        fout << "#";
        for (auto c : this->columns)
        {
            fout << setw(maxwidth) << c;
        }
        fout << "#";
        for (auto t : this->column_types)
        {
            fout << setw(maxwidth) << t;
        }
    }

    for (int r=0; r<nrows(); r++)
    {
        for (int c=0; c<ncols(); c++)
        {
            if (c>0)
            {
                fout << " ";
            }
            fout << values[r][c];
        }
        fout << endl;
    }
    fout.close();

    return filepath.string();
}

void ConstantsTable::add_to_database(
    const unique_ptr<ConstantsDB>& db,
    const string& variation,
    const long int run_min ,
    const long int run_max )
{
    fs::path filepath(this->write_to_file());

    // fix connection string for python API
    string connstr = db->GetConnectionString();
    if (connstr.find("sqlite") != string::npos)
    {
        connstr.insert(7,"/");
    }

    // build system call
    stringstream cmd;
    cmd << "clas12-ccdb"
        << " -c " + connstr
        << " add"
        << " '" + table_path + "'"
        << " -v '" + variation + "'";
    if (run_min != 0)
    {
        cmd << " -r " << run_min;
        if (run_max != INT_MAX)
        {
            cmd << "-" << run_max;
        }
    }
    else if (run_max != INT_MAX)
    {
        cmd << " -r -" << run_max;
    }
    cmd << " " << filepath.string();

    // make system call
    std::system(cmd.str().c_str());

    // remove temporary file
    fs::remove(filepath);
}

unsigned int ConstantsTable::nrows() const
{
    return values.size();
}

unsigned int ConstantsTable::ncols() const
{
    if (this->nrows() > 0)
    {
        return values.at(0).size();
    }
    else
    {
        return 0;
    }
}

string ConstantsTable::colname(const unsigned int& i)
{
    return columns.at(i);
}

string ConstantsTable::coltype(const unsigned int& i)
{
    return column_types.at(i);
}

string ConstantsTable::coltype(const string& colname)
{
    return coltype(find_column(colname));
}

string ConstantsTable::coltype(const char* colname)
{
    return coltype(string(colname));
}


unsigned int ConstantsTable::row(const string& colname, const char* val)
{
    return row<string>(colname, string(val));
}

} // namespace clas12::ccdb
} // namespace clas12
