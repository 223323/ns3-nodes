## -*- Mode: python; py-indent-offset: 4; indent-tabs-mode: nil; coding: utf-8; -*-

def build(bld):
    obj = bld.create_ns3_program('rtrk-nodes', ['core', 'point-to-point', 'internet', 'applications', 
		'point-to-point-layout', 'netanim'])
		# 'point-to-point-layout', 'netanim', 'olsr','nix-vector-routing'])
    obj.source = ['nodes.cc', 'main.cc', 'app.cc', 'api.cc']
    
