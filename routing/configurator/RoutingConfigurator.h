//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __OMNETVLR_ROUTINGCONFIGURATOR_H
#define __OMNETVLR_ROUTINGCONFIGURATOR_H

#include <map>
#include <set>
#include <tuple>
#include <omnetpp.h>

// #include "../vlr/Vlr_m.h"

using namespace omnetpp;

namespace omnetvlr {

class RoutingConfigurator : public cSimpleModule
{
  public:
    // statistics collection
    std::vector<std::pair<double, std::vector<std::set<unsigned int>>>> vidRingRegistry;      // vidRingRegistry[0] (network partition stage 0): (time, [set contains vids in a connected component after time])
    
    // failure simulation
    std::map<unsigned int, std::vector<std::tuple<double, std::string, std::set<unsigned int>>>> failureSimulationMap;  // {node1, [(100, "stop", {node2, node3})]} means node1 stops processing messages from node2 and node3 after 100s; {node1, [(100, "stop", {})]} means node1 stops processing messages (node failure) after 100s

  public:
    virtual ~RoutingConfigurator();

    // static function    NOTE static methods don't have an entry in the vtable, thus can't be overridden by subclasses
    // NOTE provide inline keyword if the definition of the function is in a header file that is included across different files.

  protected:
    virtual void initialize() override;

    void initializeFailureSimulationMap();

  public:
};


}; // namespace omnetvlr

#endif

