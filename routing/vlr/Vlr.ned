//
// This file is part of an OMNeT++/OMNEST simulation example.
//
// Copyright (C) 1992-2015 Andras Varga
//
// This file is distributed WITHOUT ANY WARRANTY. See the file
// `license' for details on this and other legal matters.
//

package routing.vlr;

import routing.RoutingBase;

//
// Models a router
//
// Remember to set @class(...) in derived modules to avoid using RoutingBase C++ abstract class directly
//      - NOTE specify namespace::Class, since C++ classes of simple modules are in the 'omnetpp' namespace otherwise
//      - if we specify "@namespace(omnetvlr);" in package.ned, the namespace will apply to all components in this package and below
//
simple Vlr extends RoutingBase
{
    parameters:
        @class("omnetvlr::Vlr");

        // VLR parameters
        double beaconInterval @unit(s) = default(2s);
        double maxJitter @unit(s) = default(0.1 * beaconInterval);     // 0.5 * beaconInterval
        double neighborValidityInterval @unit(s) = default(5 * beaconInterval);
        // // double pneiDiscoveryTime @unit(s) = default(3 * beaconInterval);
        double repSeqValidityInterval @unit(s) = default(6 * beaconInterval);
        double inNetworkEmptyVsetWarmupTime @unit(s) = default(6 * beaconInterval);
        
        int setupReqRetryLimit = default(4);        // number of times to resend a setupReq

        int vsetHalfCardinality = default(1);		// number of CW vneis or number of CCW vneis a node should have
        int backupVsetHalfCardinality = default(2);     // number of close nodes to build nonEss vroute
        int pendingVsetHalfCardinality = default(0);     // will be used only if larger than default -- max(vsetHalfCardinality + backupVsetHalfCardinality, 2*vsetHalfCardinality), 0 means use default

        bool representativeFixed = default(true);	// whether to use predefined representativeVid to start the overlay and ensure network is connected
        int representativeVid = default(0);         // vid of the root of the network
        bool startingRootFixed = default(false);	// whether to use representativeVid to start the overlay
        string repPosNodeIdCSVFile = default("");    // if provided, representativeVid will be selected from file
        string repPosNodeIdChoice = default("");     // select a representativeVid from repPosNodeIdCSVFile, e.g. "nodeAtCentre, 0" selects the first nodeId on row starting with "nodeAtCentre"
        
        int repairLinkReqFloodTTL = default(8);       // max number of hops a repairLinkReqFlood will propagate
        int routePrevhopVidsSize = default(4);       // max number of prevhopVids recorded for a route in routing table
        double repairLinkReqWaitTime @unit(s) = default(neighborValidityInterval + 5s);     // timeout for reply to a repairLinkReq sent

        bool setupTempRoute = default(true);	// whether to send repairLinkReq to build temporary route to repair broken vroutes
        bool keepDismantledRoute = default(true);	// whether to keep dismantled route (Teardown has been sent/received) in vlrRoutingTable for some time (dismantledRouteExpiration) to help with rebuild of broken vroutes
        bool checkOverHeardTraces = default(true);  // whether to record trace to pendingVnei if I receive a setupReq/repairLinkReply with trace (not destined for me)
        bool sendPeriodicNotifyVset = default(true);  // whether to send NotifyVset periodically
        bool sendNotifyVsetToReplacedVnei = default(true);  // whether to send NotifyVset to vneis removed immediately after adding a new vnei
        bool sendRepairLocalNoTemp = default(false);  // whether to send repairLocalReq instead of building temporary route with repairLinkReq, if true, set setupTempRoute to true

        // statistics measurement
        bool sendTestPacket = default(true);	// send test messages for hop count measurement purpose

        int simulationSeed = default(0);
        string firstRepSeqTimeoutCSVFile = default("");    // if provided, first repSeq timeout will be written to file
        string totalNumBeacomSentCSVFile = default("");    // if provided, totalNumBeaconSent will be written to file at end of simulation

        // visualization parameters
        bool displayBubbles = default(false);	// display bubble messages about routing state for packets
}

