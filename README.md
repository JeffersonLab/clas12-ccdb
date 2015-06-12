# CLAS12 CCDB C++ API #

To configure, build and install you may run the following:

    ./waf configure --prefix=/path/to/install
    ./waf build
    ./waf install

These can be combined into a single command:

    ./waf configure build install --prefix=/path/to/install

### Purpose of this project ###

This project provides a C++ library for interfacing with the CLAS12 Calibration and Constants Database. This includes header files for the CCDB backend, the CLAS12 wrapper headers and a shared (or static) library file to link against. The build system used is based on [waf](https://code.google.com/p/waf/) who's only dependency is python (2.4+). The interface also depends on the [boost libraries](http://www.boost.org) which are included, though used only if the host system does not already have them installed.

### How to use this library ###

There are some small test programs in the `test` directory where you may find simple examples of how to read from and write to the database. Included with this project is a script (in the `scripts`) directory to pull the CCDB database from the official MySQL server into a local file (SQLite3 format).

* The code for this project is kept on [github.com](https://github.com/JeffersonLab/clas12-ccdb.git) and the latest version may be checked out using [git](https://git-scm.com):

    git clone https://github.com/JeffersonLab/clas12-ccdb.git

* If you are having trouble using this library or have found a bug, please file an issue on the [CLAS12 CCDB issues page](https://github.com/JeffersonLab/clas12-ccdb/issues). As always, please be as detailed as possible and try to provide a *complete minimal example* that reveals the bug.
* Hint about filing bug reports: if your example is longer than 20 lines, it is probably not "minimal" and if it is missing header include statements, it is probably not "complete."
