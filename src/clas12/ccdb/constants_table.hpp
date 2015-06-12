#ifndef CLAS12_CCDB_CONSTANTS_TABLE_HPP
#define CLAS12_CCDB_CONSTANTS_TABLE_HPP

#include <algorithm>
#include <climits>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <memory>
#include <string>
#include <sstream>
#include <stdexcept>

#include "CCDB/CalibrationGenerator.h"
#include "CCDB/Calibration.h"

namespace clas12
{
namespace ccdb
{

using ::ccdb::CalibrationGenerator;
typedef ::ccdb::Calibration ConstantsDB;

using std::string;
using std::stringstream;
using std::time;
using std::time_t;
using std::vector;
using std::unique_ptr;

typedef vector<string> ColumnData;
typedef vector<ColumnData> TableData;
typedef vector<string> ColumnNames;
typedef vector<string> ColumnTypes;

class ConnectionInfo
{
  public:
    virtual
    string connection_string() const =0;
};

/** \brief combines the user, host and such into the MySQL connection
 * string used by MySQLCalibration::Connect().
 *
 * forms the string:
 *     "mysql://clas12reader@clasdb.jlab.org:3306/clas12"
 * by default.
 *
 * \return the MySQL connection string
 **/
class ConnectionInfoMySQL : public ConnectionInfo
{
  public:
    string user;
    string password;
    string host;
    int port;
    string database;

    ConnectionInfoMySQL(
        const string& user     = "clas12reader",
        const string& password = "",
        const string& host     = "clasdb.jlab.org",
              int     port     = 3306,
        const string& database = "clas12");

    string connection_string() const;
};

/** \brief creates an SQLite connection string to be used by
 * SQLiteCalibration::Connect().
 *
 * forms the string:
 *     "sqlite:///clas12_ccdb.sqlite"
 * by default.
 *
 * \return the SQLite connection string
 **/
class ConnectionInfoSQLite : public ConnectionInfo
{
  public:
    string filepath;
    ConnectionInfoSQLite(string filepath = "clas12_ccdb.sqlite");
    string connection_string() const;
};


struct ConstantSetInfo
{
    int run;
    string variation;
    time_t timestamp;

    ConstantSetInfo(
              int     run       = INT_MAX,
        const string& variation = "default",
              time_t  timestamp = 0);

    string constant_set_string(const string& table_path) const;
};

template <class ConnectionInfoType>
unique_ptr<ConstantsDB> get_constants_db(
    const ConnectionInfoType& conn,
    const ConstantSetInfo& csinfo)
{
    auto cinfo = dynamic_cast<const ConnectionInfo*>(&conn);
    string connstr = cinfo->connection_string();
    CalibrationGenerator calibgen;
    return unique_ptr<ConstantsDB>(calibgen.MakeCalibration(
        connstr, csinfo.run, csinfo.variation, csinfo.timestamp ) );
}

unique_ptr<ConstantsDB> get_constants_db();

/** \brief ConstantsTable is a conatiner class for any constants
 *  set. It will connect to the database when load_constants()
 *  is called. Columns can be accessed (and converted to specific
 *  types) by the col(colname) method. Individual numbers can be
 *  obtained via the elem<typename>(colname, row_idx) method.
 **/
class ConstantsTable
{
  private:
    /// the table as filled by Calibration*
    TableData values;

    /// the names of the columns
    ColumnNames columns;

    /// the types of the columns in string form
    ColumnTypes column_types;

    /// table path in database
    string table_path;

    /** \brief find the index of the column associated with the name
     *  colname.
     *
     * \return column index of the column identified by colname
     **/
    unsigned int find_column(const string& colname);

    /** \brief generic function to convert a string to any type (T)
     *
     **/
    template <typename T>
    T lexical_cast(const string& str)
    {
        T ret;
        if (!(stringstream(str) >> ret))
        {
            stringstream ss;
            ss << "Could not convert: '"
               << str
               << "' to a numeric type.";
            throw std::invalid_argument(ss.str());
        }
        return ret;
    }

    template <typename T>
    string to_string(const T& val)
    {
        stringstream ss;
        ss << val;
        return ss.str();
    }

  public:
    ConstantsTable(
        const unique_ptr<ConstantsDB>& db,
        const string& table_path );

    string write_to_file(const string& fname = "", bool header = true);

    void add_to_database(
        const unique_ptr<ConstantsDB>& db,
        const string& variation = "default",
        const long int run_min  = 0,
        const long int run_max  = INT_MAX);

    /** \return number of rows in this data set.
     *
     **/
    unsigned int nrows() const;

    /** \return number of columns in this data set.
     *
     **/
    unsigned int ncols() const;

    /** \return the column name of the ith column
     *
     **/
    string colname(const unsigned int& i);

    /** \return the column type of the ith column
     *
     **/
    string coltype(const unsigned int& i);

    /** \return the column type of the column identified by colname
     *
     **/
    string coltype(const string& colname);

    /** \brief specialization of coltype() for const char*
     *
     * \return the column type of the column identified by colname
     **/
    string coltype(const char* colname);

    /** \return vector<T=double> of a column identified by the
     *  colname.
     **/
    template <typename T=double>
    vector<T> col(const string& colname)
    {
        vector<T> ret;

        unsigned int col_idx = find_column(colname);

        for (int row_idx=0; row_idx<nrows(); row_idx++)
        {
            ret.push_back(
                lexical_cast<T>(
                    values.at(row_idx).at(col_idx) ) );
        }

        return ret;
    }

    /** \brief finds the element in the table associated with column
     * identified by column index and row index specified
     * (default row: 0)
     **/
    template <typename T=double>
    T elem(const unsigned int& col, const unsigned int& row=0)
    {
        return lexical_cast<T>(
            values.at(row).at(col) );
    }

    /** \brief finds the element in the table associated with column
     * identified by colname and row specified (default row: 0)
     *
     *  typical usage:
     * for example, to get the xdist for region 2, we could get the
     * row index from the method row() and use that as in a call
     * elem() to get the element "xdist" at that row.
     *
     * ConstantsTable table;
     * table.load_constants("/geometry/dc/region");
     * table.elem( "xdist", table.row("region",2) );
     *
     * Equivalently, if we already know that the table is ordered
     * properly and the index associated with region 2 is 2-1 = 1,
     * then we could do this to get the same number as above:
     *
     * table.elem("xdist", 1);
     *
     *  \return element of the table cast to type T (default: double)
     **/
    template <typename T=double>
    T elem(const string& colname, const unsigned int& row=0)
    {
        return lexical_cast<T>(
            values.at(row).at(find_column(colname)) );
    }

    /** \brief find the first row of a specified column that has a
     * value that equals val.
     *
     * NOTE: getting the row index works for integers but
     * not for floats!!! Use only integers or strings. The
     * "string for floats" usage is OK, but not recommended.
     *
     * example:
     *   table.row("dist2tgt", "348.09")
     *
     * \return row index associated with the value val
     **/
    template <typename T>
    unsigned int row(const string& colname, const T& val)
    {
        for (unsigned int i=0; i<nrows(); i++)
        {
            if (elem<T>(colname,i) == val)
            {
                return i;
            }
        }
        stringstream ss;
        ss << "No such value: " << val << " in column " << colname;
        throw std::invalid_argument(ss.str());
    }

    /** \brief specialization for char* (converting to string)
     *
     **/
    unsigned int row(const string& colname, const char* val);

    /** Overwrite values in a specific column with
     **/
    template <typename T>
    ConstantsTable& col(const string& colname, const vector<T>& coldata)
    {
        unsigned int col_idx = find_column(colname);
        for (int row_idx=0; row_idx<nrows(); row_idx++)
        {
            values[row_idx][col_idx] = to_string(coldata[row_idx]);
        }
        return *this;
    }

    /** Overwrite value in specific spot in table
     **/
    template <typename T>
    ConstantsTable& elem(const string& colname,
                         const unsigned int& row,
                         T val)
    {
        values[row][find_column(colname)] = to_string(val);
        return *this;
    }

};

} // namespace clas12::ccdb
} // namespace clas12

#endif // CLAS12_CCDB_CONSTANTS_TABLE_HPP
