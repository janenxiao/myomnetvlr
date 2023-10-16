//
// This file is part of an OMNeT++/OMNEST simulation example.
//
// Copyright (C) 1992-2015 Andras Varga
//
// This file is distributed WITHOUT ANY WARRANTY. See the file
// `license' for details on this and other legal matters.
//

package routing;

//
// Models a router
//
simple RoutingBase like IRouting
{
    parameters:
        @class("omnetvlr::RoutingBase");
        @display("i=block/switch");
        @signal[drop](type="long");
        @statistic[drop](title="dropped packet byte length";unit=bytes;record=vector?,count,sum;interpolationmode=none);

        string routingConfigModule = default("^.^.routingConfigurator");    // parent module: Node, RoutingConfigurator is a sibling module of all Node modules

        string testDstAssignmentFile = default("");         // row format: my vid, testDst1, testDst2, ..
        
        string resultNodeCSVFile = default("results/nodeStats.csv");    // filename that records node statistics in finish(), e.g. pneis, vneis, routing table size
        string resultTestCSVFile = default("results/sendRecords.csv");    // filename that records all messages sent and received

        double recordReceivedMsgAggPeriod = default(0);	// if > 0, write only 1 record for all received messages of the same type in a period (e.g. every 1 second) with recordMessageRecord(/*action=*/2

        string routingTableVidCSVFile = default("");    // if provided, write vlrRoutingTable at nodes to file after some time
        string writeRoutingTableToFileTimes = default("");    // time after start to write vlrRoutingTable to file if routingTableVidCSVFile provided, e.g. "500, 1000"

        string writeNodeStatsTimes = default("");    // time after start to record nodeStats to allNodeRecords, e.g. "500, 1000", if not specified, will only record nodeStats in finish()

        double simulateBeaconLossRate = default(0);	  // value in range [0, 1), ratio at which I'll ignore received broadcast messages to simulate loss because of collision (because broadcast messages aren't retransmitted)

        
        

        // // // string failureSimulationOpTimes = default("");    // time to start each operation batch in node/link failureSimulationFile, e.g. "500, 1000"
        // // // string failureLinkSimulationFile = default("");    // simulate link failures after some time, row format: node1,node2
        // // // string failureNodeSimulationFile = default("");     // simulate node failures after some time, row format: nodeid
        // // // string failureRingPartitionFile = default("");     // each line contains vids in a connected component at a time in failureSimulationOpTimes

        
    gates:
        input in[];
        output out[];
}


