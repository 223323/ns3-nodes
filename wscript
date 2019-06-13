## -*- Mode: python; py-indent-offset: 4; indent-tabs-mode: nil; coding: utf-8; -*-

import os
import sys

def configure(conf):
    
    ns3_pkgs = [
        #'core',
        'point-to-point',
        'internet',
        'applications',
        'netanim',
        #'point-to-point-layout',
    ]

    if sys.platform.startswith('linux'):
        NS3_PKGCONFIG = os.path.join(conf.env.NS3_HOME, 'lib/pkgconfig')
    else:
        raise NotImplementedError("Works only on linux!")

    conf.check_cfg(
        package = ['libns3-dev-{}-debug'.format(ns3_pkg) for ns3_pkg in ns3_pkgs],
        pkg_config_path = NS3_PKGCONFIG,
        args = '--cflags --libs',
        mandatory = True, 
        uselib_store = 'NS3'
    )
    conf.env.RPATH_NS3 = conf.env.LIBPATH_NS3


def build(bld):
    
    bld.program(
        source = [
            'nodes.cc',
            'main.cc',
            'app.cc',
            'api.cc',
            'modified-default-simulator-impl.cc'
        ],
        use = 'NS3',
        target = 'ns3_test_app'
    )
