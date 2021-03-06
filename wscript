#! /usr/bin/env python
# encoding: utf-8

APPNAME = 'CPP_PROJECT'
VERSION = "0.1"

target_name = 'ptw'
lib_fname = 'ptw.h'
test_cmd = 'my_test'
main_lib = ['pthread']


import sys
import os
import platform
import subprocess
import time
import re

top = '.'
out = 'build'
test_fname = os.path.join (out, test_cmd)

def options(opt):
    opt.load ('compiler_cxx')
    opt.add_option ('--enable-debug',
                    action='store_true', dest='debug',
                    help='enable debug options')
    opt.add_option ('--enable-profile',
                    action='store_true', dest='profile',
                    help='enable profiling options')
    opt.add_option ('--enable-test',
                    action='store_true', dest='test',
                    help='enable test suite')


def configure(conf):
    global main_lib
    lib_list = main_lib

    # ----------------------------------
    # check clang
    try:
        has_clang = True
        conf.find_program ('clang++')
    except Exception, e:
        has_clang = False

    if ('Darwin' == platform.system() and 
        os.environ.get('CXX') is None and 
        has_clang):
        conf.env.append_value ('CXX', 'clang++')

    # ----------------------------------
    # c++ flags setting
    cxxflags = ['-Wall', '-std=c++0x']
    linkflags = []

    if conf.options.debug:
        cxxflags.extend (['-O0', '-g', '-pg'])
        linkflags.extend (['-g', '-pg'])
    else:
        cxxflags.extend (['-O2'])    

    conf.env.append_value('CXXFLAGS', cxxflags) 
    conf.env.append_value('INCLUDES', ['.', '%s/include' % conf.env.PREFIX])
    conf.env.append_value('LINKFLAGS', linkflags)
        
    # ----------------------------------
    # compiler and libraries
    conf.load('compiler_cxx')
    for libname in lib_list: conf.check_cxx(lib = libname)

    if conf.options.test:
        p = subprocess.Popen('gtest-config --libdir', shell=True, stdout=subprocess.PIPE)
        gtest_libpath = p.stdout.readline().strip ()
        p.wait ()
        conf.env.append_value('LIBDIR', gtest_libpath)
        conf.check_cxx(lib = 'gtest', args = ['-lpthread'])

    conf.env.store('config.log')    
    conf.env.test = True if conf.options.test else False

def build(bld):
    def get_src_list(d, regex):
        src_re = re.compile (regex)
        a = []
        for f in os.listdir (d): 
            if src_re.search (f): a.append (os.path.join(d, f))
        return a

    src_list = []

    # main library 
    global main_lib
    global lib_fname
    src_dir = 'src'
    cc_file = '^[_A-Za-z0-9].*\.cc'

    src_list.extend (get_src_list (src_dir, cc_file))
    bld.shlib(
        source = src_list,
        libpath = os.path.join (bld.env.PREFIX, 'lib'),
        lib = main_lib,
        target = target_name)

    bld.install_files('${PREFIX}/include', lib_fname) 

    # test code
    inc_dir = os.path.join (bld.path.abspath(), 'src')
    rpath_list = [os.path.join (bld.env.PREFIX, 'lib'),
                  os.path.join (bld.path.abspath(), 'build', '..', 'src'),
                  os.path.join (bld.path.abspath(), 'build')]
    if bld.env.test:
        src_list = ['test/main.cc', 'test/opt_parse.cc']
        libs = ['gtest', 'pthread']
        bld.program(features = 'cxxprogram',
                    source = src_list,
                    target = test_cmd,
                    use = [target_name],
                    lib = libs,
                    includes = [inc_dir],
                    LIBDIR = [os.path.join (bld.env.PREFIX, 'lib')],
                    rpath = rpath_list)

    src_list = ['test/perf.cc', 'test/opt_parse.cc']
    bld.program(features = 'cxxprogram',
                source = src_list,
                target = 'ptw_perf',
                use = [target_name],
                lib = main_lib,
                includes = [inc_dir],
                LIBDIR = [os.path.join (bld.env.PREFIX, 'lib')],
                rpath = rpath_list)


def shutdown(ctx):
    pass

def test(ctx):
    global test_fname
    p = subprocess.call (test_fname)

    
def ci(ctx):
    while True:
        next = 300
        if (0 == subprocess.call ('./waf') and
            0 == subprocess.call (['nice', test_fname])):  
            pass
        else:
            next = 150

        print 
        for i in range(0, next):
            print '\tnext build after %4d sec..\r' % (next - i),
            sys.stdout.flush ()
            time.sleep (1)


