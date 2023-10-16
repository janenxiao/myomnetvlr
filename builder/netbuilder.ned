//
// This file is part of an OMNeT++/OMNEST simulation example.
//
// Copyright (C) 1992-2015 Andras Varga
//
// This file is distributed WITHOUT ANY WARRANTY. See the file
// `license' for details on this and other legal matters.
//

package builder;

//
// Dynamically sets up a network, with the topology coming from text files
//
simple NetBuilder
{
    parameters:
        @display("i=block/cogwheel_s");
        string nodesFile;
        string connectionsFile;
        string nodeType;
        string routingConfiguratorType = default("");    // if provided, create "routingConfigurator" module in network
        
        double channelDelay @unit(s) = default(0s);	// propagation delay
        double channelBitErrorRate = default(0);	//  ratio between the number of bits incorrectly received and the total number of bits transmitted
        double channelDatarate @unit(bps) = default(2Mbps);	// bits per second; 0=infinite
}
