//
// This file is part of an OMNeT++/OMNEST simulation example.
//
// Copyright (C) 1992-2015 Andras Varga
//
// This file is distributed WITHOUT ANY WARRANTY. See the file
// `license' for details on this and other legal matters.
//

package routing.configurator;

//
// Models a router
//
simple RoutingConfigurator
{
    parameters:
        @class("omnetvlr::RoutingConfigurator");
        @display("i=block/cogwheel");

        string failureSimulationOpTimes = default("");    // time to start each operation batch in node/link failureSimulationFile, e.g. "500, 1000"
        string failureLinkSimulationFile = default("");    // simulate link failures after some time, row format: node1,node2
        string failureNodeSimulationFile = default("");     // simulate node failures after some time, row format: nodeid
        string failureRingPartitionFile = default("");     // each line contains vids in a connected component at a time in failureSimulationOpTimes
}


