# -*- Mode: python; py-indent-offset: 4; indent-tabs-mode: nil; coding: utf-8; -*-

def build(bld):
    module = bld.create_ns3_module('traffic', ['core', 'internet'])
    module.source = [
        'helper/http-helper.cc',
        'model/http-client.cc',
        'model/http-entity-header.cc',
        'model/http-server.cc',
        'model/http-variables.cc',
        ]

    module_test = bld.create_ns3_module_test_library('traffic')
    module_test.source = []

    headers = bld.new_task_gen(features=['ns3header'])
    headers.module = 'traffic'
    headers.source = [
        'helper/http-helper.h',
        'model/http-client.h',
        'model/http-entity-header.h',
        'model/http-server.h',
        'model/http-variables.h',
        ]

    if bld.env.ENABLE_EXAMPLES:
        bld.add_subdirs('examples')
