# -*- Mode: python; py-indent-offset: 4; indent-tabs-mode: nil; coding: utf-8; -*-

def build(bld):
    module = bld.create_ns3_module('traffic', ['core',
                                               'applications',
                                               'flow-monitor',
                                               'point-to-point'])
    module.source = [
        'helper/http-client-trace-plot.cc',
        'helper/http-helper.cc',
        'helper/http-kpi-helper.cc',
        'helper/nrtv-client-trace-plot.cc',
        'helper/nrtv-helper.cc',
        'helper/nrtv-kpi-helper.cc',
        'model/http-client.cc',
        'model/http-entity-header.cc',
        'model/http-server.cc',
        'model/http-variables.cc',
        'model/nrtv-client.cc',
        'model/nrtv-header.cc',
        'model/nrtv-server.cc',
        'model/nrtv-variables.cc',
        'model/traffic-bounded-log-normal-variable.cc',
        'model/traffic-bounded-pareto-variable.cc',
        ]

    module_test = bld.create_ns3_module_test_library('traffic')
    module_test.source = [
        'test/nrtv-test.cc',
        ]

    headers = bld.new_task_gen(features=['ns3header'])
    headers.module = 'traffic'
    headers.source = [
        'helper/histogram-plot-helper.h',
        'helper/http-client-trace-plot.h',
        'helper/http-helper.h',
        'helper/http-kpi-helper.h',
        'helper/nrtv-client-trace-plot.h',
        'helper/nrtv-helper.h',
        'helper/nrtv-kpi-helper.h',
        'model/http-client.h',
        'model/http-entity-header.h',
        'model/http-server.h',
        'model/http-variables.h',
        'model/nrtv-client.h',
        'model/nrtv-header.h',
        'model/nrtv-server.h',
        'model/nrtv-variables.h',
        'model/traffic-bounded-log-normal-variable.h',
        'model/traffic-bounded-pareto-variable.h',
        ]

    if bld.env.ENABLE_EXAMPLES:
        bld.add_subdirs('examples')
