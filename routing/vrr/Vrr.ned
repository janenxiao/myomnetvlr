//
// This file is part of an OMNeT++/OMNEST simulation example.
//
// Copyright (C) 1992-2015 Andras Varga
//
// This file is distributed WITHOUT ANY WARRANTY. See the file
// `license' for details on this and other legal matters.
//

package routing.vrr;

import routing.RoutingBase;

//
// Models a router
//
// Remember to set @class(...) in derived modules to avoid using RoutingBase C++ abstract class directly
//      - NOTE specify namespace::Class, since C++ classes of simple modules are in the 'omnetpp' namespace otherwise
//      - if we specify "@namespace(omnetvlr);" in package.ned, the namespace will apply to all components in this package and below
//
simple Vrr extends RoutingBase
{
    parameters:
        @class("omnetvlr::Vrr");

        // VRR parameters
        double beaconInterval @unit(s) = default(2s);
        double maxJitter @unit(s) = default(0.1 * beaconInterval);     // 0.5 * beaconInterval
        double neighborValidityInterval @unit(s) = default(5 * beaconInterval);
        double repSeqValidityInterval @unit(s) = default(6 * beaconInterval);
        double repSeqPersistenceInterval @unit(s) = default(200 * beaconInterval);
        double inNetworkEmptyVsetWarmupTime @unit(s) = default(6 * beaconInterval);

        int setupReqRetryLimit = default(4);        // number of times to resend a setupReq

        int vsetHalfCardinality = default(1);		// number of CW vneis or number of CCW vneis a node should have

        bool sendRepairLocalNoTemp = default(true);  // whether to send repairLocalReply instead of tearing down broken vroutes directly
        double repairLinkReqWaitTime @unit(s) = default(neighborValidityInterval + 5s);     // wait time for repairLocalReply for vroute whose nexthop is lost before tearing it down
        bool removeBrokenVrouteEndpointInRepair = default(true);  // whether to remove brokenVroute endpoint from endpointToRoutesMap when prevhop/nexthop unavailable, if false and selected brokenVroute whose next hop is unavailable in findNextHop(), message can be dropped to prevent loop

        // statistics measurement
        bool sendTestPacket = default(true);	// send test messages for hop count measurement purpose
        bool recordReceivedMsg = default(true);	// record received message (may not be directed to me) with recordMessageRecord(/*action=*/2
        bool recordDroppedMsg = default(false);	// should be different than recordReceivedMsg, record message arrived and destined for me, or dropped at me with recordMessageRecord(/*action=*/1 or 4

}


