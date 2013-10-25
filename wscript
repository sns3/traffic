# -*- Mode: python; py-indent-offset: 4; indent-tabs-mode: nil; coding: utf-8; -*-

def build(bld):
    module = bld.create_ns3_module('traffic', ['core'])
    module.source = [
        'model/http-variables.cc',
        ]

    module_test = bld.create_ns3_module_test_library('traffic')
    module_test.source = []

    headers = bld.new_task_gen(features=['ns3header'])
    headers.module = 'traffic'
    headers.source = [
        'model/http-variables.h',
        ]

    if bld.env.ENABLE_EXAMPLES:
        bld.add_subdirs('examples')
