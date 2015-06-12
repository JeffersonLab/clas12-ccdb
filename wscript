#! /usr/bin/env python
# encoding: utf-8

from waflib import Utils

top     = '.'
out     = 'build'

VERSION = '0.3.0'
APPNAME = 'clas12_ccdb'
BREIF = '''\
This is a C++ interface library for the CLAS12
Calibration and Constants Database (CCDB).
'''

def options(opt):
    opt.load('compiler_c compiler_cxx cutil boost')


def configure(conf):
    conf.load('compiler_c compiler_cxx cutil')
    conf.load('boost boost_util')
    conf.load('config_script')

    conf.env['VERSION'] = VERSION
    conf.env['APPNAME'] = APPNAME
    conf.env['BREIF']   = BREIF

    conf.check_cxx11()
    conf.check_mysql()

    conf.recurse('ext')
    conf.recurse('src')


def build(bld):

    bld.install_files(bld.options.bindir,
        'scripts/clas12-ccdb-mysql2sqlite',
        chmod=Utils.O755)

    bld.recurse('ext')
    bld.recurse('src')

    bld.config_script(
        target = 'clas12-ccdb-config',
        includes = [bld.options.incdir],
        stlibpath = [bld.options.libdir],
        use = '''\
            C++11
            CLAS12_CCDB
            CCDB
            BOOST
                boost_filesystem
                boost_system
            MYSQL
        '''.split(),
        install_path = bld.options.bindir)

    # ensure main project is built before test objects
    bld.add_group()

    bld.recurse('test')
