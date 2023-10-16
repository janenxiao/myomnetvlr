//
// This file is part of an OMNeT++/OMNEST simulation example.
//
// Copyright (C) 1992-2015 Andras Varga
//
// This file is distributed WITHOUT ANY WARRANTY. See the file
// `license' for details on this and other legal matters.
//

#include "Vrr.h"

#include <fstream>      // for reading/writing to file
#include <sstream>      // for std::stringstream, std::ostringstream, std::istringstream
#include <algorithm>      // for std::copy(..), std::find(..), std::set_intersection(..)

namespace omnetvlr {

Define_Module(Vrr);

Vrr::~Vrr()
{
    cancelAndDelete(beaconTimer);
    cancelAndDelete(purgeNeighborsTimer);
    cancelAndDelete(fillVsetTimer);
    cancelAndDelete(inNetworkWarmupTimer);
    // cancelAndDelete(repSeqExpirationTimer);
    cancelAndDelete(nonEssRouteTeardownTimer);

    cancelAndDelete(testPacketTimer);

    cancelPendingVsetTimers();
    cancelRepairLinkTimers();
}

// static VRR parameters -- unit: seconds
const double Vrr::nonEssRouteExpiration = 0.1;

// time to start sending TestPacket
const double Vrr::sendTestPacketOverlayWaitTime = 100;
double Vrr::lastTimeNodeJoinedInNetwork = 100;
bool Vrr::sendTestPacketStart = false;

void Vrr::initialize()
{
    //
    // Brute force approach -- every node does topology discovery on its own,
    // and finds routes to all other nodes independently, at the beginning
    // of the simulation. This could be improved: (1) central routing database,
    // (2) on-demand route calculation
    //
    // cTopology *topo = new cTopology("topo");

    // std::vector<std::string> nedTypes;
    // nedTypes.push_back(getParentModule()->getNedTypeName());
    // topo->extractByNedTypeName(nedTypes);
    // EV << "cTopology found " << topo->getNumNodes() << " nodes\n";

    // // cTopology::Node *thisNode = topo->getNodeFor(getParentModule());

    // // find and store next hops
    // for (int i = 0; i < topo->getNumNodes(); i++) {
    //     if (topo->getNode(i) == thisNode)
    //         continue;  // skip ourselves
    //     topo->calculateUnweightedSingleShortestPathsTo(topo->getNode(i));

    //     if (thisNode->getNumPaths() == 0)
    //         continue;  // not connected

    //     cGate *parentModuleGate = thisNode->getPath(0)->getLocalGate();
    //     int gateIndex = parentModuleGate->getIndex();
    //     int address = topo->getNode(i)->getModule()->par("address");
    //     rtable[address] = gateIndex;
    //     EV << "  towards address " << address << " gateIndex is " << gateIndex << endl;
    // }
    // delete topo;

    RoutingBase::initialize();

    // Vrr
    beaconInterval = par("beaconInterval");
    maxJitter = par("maxJitter");
    neighborValidityInterval = par("neighborValidityInterval");
    repSeqValidityInterval = par("repSeqValidityInterval");
    repSeqPersistenceInterval = par("repSeqPersistenceInterval");
    inNetworkEmptyVsetWarmupTime = par("inNetworkEmptyVsetWarmupTime");
    
    setupReqRetryLimit = par("setupReqRetryLimit");

    sendRepairLocalNoTemp = par("sendRepairLocalNoTemp");
    repairLinkReqWaitTime = par("repairLinkReqWaitTime");
    removeBrokenVrouteEndpointInRepair = par("removeBrokenVrouteEndpointInRepair");

    // context
    host = getParentModule();

    // internal
    beaconTimer = new cMessage("BeaconTimer");
    purgeNeighborsTimer = new cMessage("PurgeNeighborsTimer");
    fillVsetTimer = new cMessage("fillVsetTimer");
    inNetworkWarmupTimer = new cMessage("inNetworkWarmupTimer");
    // repSeqExpirationTimer = new cMessage("repSeqExpirationTimer");
    nonEssRouteTeardownTimer = new cMessage("nonEssRouteTeardownTimer");

    // statistics measurement
    sendTestPacket = par("sendTestPacket");
    testPacketTimer = new cMessage("testPacketTimer");
    recordReceivedMsg = par("recordReceivedMsg");
    recordDroppedMsg = par("recordDroppedMsg");

    if (sendTestPacket) {
        initializeSelfTestDstList();

        allSendRecords.reserve(allSendRecordsCapacity);     // request the minimum vector capacity at the start to avoid reallocation
    }

    vsetHalfCardinality = par("vsetHalfCardinality");

    // initialize static non-const variables for multiple ${repetition} or ${runnumber} bc cmdenv runs simulations in the same process, this means that e.g. if one simulation run writes a global variable, subsequent runs will also see the change
    lastTimeNodeJoinedInNetwork = sendTestPacketOverlayWaitTime;
    sendTestPacketStart = false;
}

void Vrr::handleMessage(cMessage *message)
{
    if (message->isSelfMessage())
        processSelfMessage(message);
    else
        processMessage(message);
}

void Vrr::processSelfMessage(cMessage *message)
{
    // self-message defined in RoutingBase
    if (message == startTimer)
        handleStartOperation();
    else if (message == failureSimulationTimer)
        processFailureSimulationTimer();
    else if (message == vidRingRegVsetTimer)
        processVidRingRegVsetTimer(/*numHalf=*/vsetHalfCardinality);
    else if (message == writeRoutingTableTimer)
        processWriteRoutingTableTimer();
    else if (message == writeNodeStatsTimer)
        processWriteNodeStatsTimer();
    else if (message == writeReceivedMsgAggTimer)
        processWriteReceivedMsgAggTimer();
    else if (auto failedpktTimer = dynamic_cast<FailedPacketDelayTimer *>(message))
        processFailedPacketDelayTimer(failedpktTimer);
    // self-message defined in Vrr
    else if (message == beaconTimer)
        processBeaconTimer();
    else if (message == purgeNeighborsTimer)
        processPurgeNeighborsTimer();
    else if (message == fillVsetTimer)
        processFillVsetTimer();
    else if (message == inNetworkWarmupTimer)
        processInNetworkWarmupTimer();
    // else if (message == repSeqExpirationTimer)
    //     processRepSeqExpirationTimer();
    else if (message == nonEssRouteTeardownTimer)
        processNonEssRouteTeardownTimer();
    else if (message == testPacketTimer)
        processTestPacketTimer();
    
    else if (auto waitsetupReqTimer = dynamic_cast<WaitSetupReqIntTimer *>(message))
        processWaitSetupReqTimer(waitsetupReqTimer);
    else if (auto waitrepairLinkTimer = dynamic_cast<WaitRepairLinkIntTimer *>(message))
        processWaitRepairLinkTimer(waitrepairLinkTimer);
}

void Vrr::processMessage(cMessage *message)
{
    bool pktForwarded = false;
    int pktGateIndex = message->getArrivalGate()->getIndex();

    if (!failureSimulationUnaddedPneiVids.empty()) {    // check if sender of message should be in failureSimulationPneiVidMap
        VlrRingVID msgPrevHopVid = getMessagePrevhopVid(message);
        if (msgPrevHopVid != VLRRINGVID_NULL && failureSimulationUnaddedPneiVids.find(msgPrevHopVid) != failureSimulationUnaddedPneiVids.end()) {   // msgPrevHopVid is a pnei that should be added to failureSimulationPneiVidMap
            std::set<VlrRingVID> failedPneis {msgPrevHopVid};
            handleFailureLinkSimulation(failedPneis, /*failedGateIndex=*/pktGateIndex);     // add msgPrevHopVid to failureSimulationPneiVidMap
        }
    }

    if (auto msg = dynamic_cast<NotifyLinkFailureInt *>(message))
        processNotifyLinkFailure(msg, pktForwarded);
    else if ((selfNodeFailure || failureSimulationPneiGates.find(pktGateIndex) != failureSimulationPneiGates.end())) {   // I've failed or message from a failed link
        if (auto unicastPacket = dynamic_cast<VlrIntUniPacket *>(message)) {
            EV_WARN << "Received VlrIntUniPacket from failed link, me=" << vid << ", selfNodeFailure=" << selfNodeFailure << ", sending NotifyLinkFailure(linkDown) back" << endl;
            sendNotifyLinkFailure(pktGateIndex, /*simLinkUp=*/false);

            if (recordStatsToFile && recordDroppedMsg) {   // record dropped message
                recordMessageRecord(/*action=*/4, /*src=*/VLRRINGVID_NULL, /*dst=*/VLRRINGVID_NULL, unicastPacket->getName(), /*msgId=*/unicastPacket->getMessageId(), /*hopcount=*/unicastPacket->getHopcount(), /*chunkByteLength=*/unicastPacket->getByteLength(), /*infoStr=*/"linkfailure");
            }
        } else
            EV_WARN << "Received packet from failed link, me=" << vid << ", selfNodeFailure=" << selfNodeFailure << ", ignoring packet " << message->getName() << endl;
    } else {
        bool processPkt = true;
        if (auto unicastPacket = dynamic_cast<VlrIntUniPacket *>(message)) {
            VlrRingVID msgPrevHopVid = unicastPacket->getVlrOption().getPrevHopVid();
            if (msgPrevHopVid != VLRRINGVID_NULL)
                psetTable.setRecvNeiGateIndex(msgPrevHopVid, pktGateIndex);
        } else if (simulateBeaconLossRate > 0 && uniform(0, 1) < simulateBeaconLossRate) {  // broadcast message
            EV_WARN << "Received broadcast packet and simulating packet loss due to collision, me=" << vid << ", ignoring packet " << message->getName() << endl;
            processPkt = false;
        }

        if (processPkt) {
            if (auto beacon = dynamic_cast<VlrIntBeacon *>(message))
                processBeacon(beacon, pktForwarded);
            else if (auto setupReq = dynamic_cast<SetupReqInt *>(message))
                processSetupReq(setupReq, pktForwarded);
            else if (auto setupReply = dynamic_cast<SetupReplyInt *>(message))
                processSetupReply(setupReply, pktForwarded);
            else if (auto setupFail = dynamic_cast<SetupFailInt *>(message))
                processSetupFail(setupFail, pktForwarded);
            else if (auto teardown = dynamic_cast<TeardownInt *>(message))
                processTeardown(teardown, pktForwarded);
            else if (auto testpacket = dynamic_cast<VlrIntTestPacket *>(message))
                processTestPacket(testpacket, pktForwarded);
            else if (auto repairLocalReply = dynamic_cast<RepairLocalReplyInt *>(message))
                processRepairLocalReply(repairLocalReply, pktForwarded);
        }
    }

    if (!pktForwarded)
        delete message;
}

VlrRingVID Vrr::getMessagePrevhopVid(cMessage *message) const
{
    VlrRingVID msgPrevHopVid = VLRRINGVID_NULL;
    if (auto unicastPacket = dynamic_cast<VlrIntUniPacket *>(message))
        msgPrevHopVid = unicastPacket->getVlrOption().getPrevHopVid();
    else if (auto beacon = dynamic_cast<VlrIntBeacon *>(message))
        msgPrevHopVid = beacon->getVid();
    
    return msgPrevHopVid;
}

void Vrr::cancelPendingVsetTimers()
{
    for (auto& elem : pendingVset)
        cancelAndDelete(elem.second);
}

void Vrr::cancelRepairLinkTimers()
{
    for (auto& elem : lostPneis)
        cancelAndDelete(elem.second.timer);
}

void Vrr::handleStartOperation()
{
    RoutingBase::handleStartOperation();
    
    scheduleBeaconTimer(/*firstTimer=*/true);
    scheduleFillVsetTimer(/*firstTimer=*/true);
    if (sendTestPacket)
        scheduleTestPacketTimer(/*firstTimer=*/true);

    selfRepSeqnum = 0;
    // representative.heardfromvid = VLRRINGVID_NULL;
    // representative.sequencenumber = 0;
    // representative.vid = vid;
    // representative.hopcount = 0;
    // if (representative.vid == vid) {    // I'm the rep of the network
    //     selfInNetwork = true;           // initially only rep.selfInNetwork = true
    //     representative.heardfromvid = vid;
    // }

    selfNodeFailure = false;    // stop simulating node failure at me
}

void Vrr::handleStopOperation()
{
    EV_DEBUG << "handleStopOperation() at node " << vid << endl;
    if (recordStatsToFile) { // write node status update
        recordNodeStatsRecord(/*infoStr=*/"beforeNodeFailure");
    }
    clearState();
    selfNodeFailure = true;

    nodesVsetCorrect.erase(vid);
}

void Vrr::handleFailureNodeRestart()
{
    EV_INFO << "Handling failure node Restart at node " << vid << ", broadcasting NotifyLinkFailure(linkUp) to pneis" << endl;
    // send NotifyLinkFailure(linkUp) to affected pneis to remove my vid in their failureSimulationPneiVidMap
    NotifyLinkFailureInt *msg = createNotifyLinkFailure(/*simLinkUp=*/true);
    sendCreatedPacket(msg, /*unicast=*/false, /*outGateIndex=*/-1, /*delay=*/0, /*checkFail=*/false);

    if (recordStatsToFile) { // write node status update
        recordNodeStatsRecord(/*infoStr=*/"beforeNodeRestart");
    }
}

void Vrr::recordCurrentNodeStats(const char *stage)
{
    // write node statistics to file
    std::ostringstream s;

    std::vector<VlrRingVID> linkedPneis = psetTable.getPneisLinked();
    std::vector<VlrRingVID> simulatedPneis;    // contains LINKED pneis that are NOT in failureSimulationPneiVids
    // ASSERT(std::is_sorted(linkedPneis.begin(), linkedPneis.end())); // linkedPneis should be sorted bc it's obtained by iterating over psetTable (std::map)
    std::vector<VlrRingVID> failureSimulationPneiVids;
    for (const auto& failedPnei: failureSimulationPneiVidMap)
        failureSimulationPneiVids.push_back(failedPnei.first);
    std::set_difference(linkedPneis.begin(), linkedPneis.end(), failureSimulationPneiVids.begin(), failureSimulationPneiVids.end(), std::inserter(simulatedPneis, simulatedPneis.end()));
    VlrRingVID smallestRep = vid;
    if (!representativeMap.empty())
        if (representativeMap.begin()->first < smallestRep)
            smallestRep = representativeMap.begin()->first;            

    s << "nodeStats" << ',' << simulatedPneis << ',' << selfInNetwork << ',' << selfNodeFailure << ',' 
            << smallestRep << ',' << printVsetToString() << ',' << vidRingRegVset << ',' << printRoutesToMeToString() << ','
            << simulatedPneis.size() << ',' << vset.size() << ',' << pendingVset.size() << ',' << vlrRoutingTable.vlrRoutesMap.size() << ',' << totalNumBeaconSent << ','
            << representativeMap.size() << ',' << representativeMapActualMaxSize << ',' << stage;  //  << ',' << pendingVset

    recordNodeStatsRecord(/*infoStr=*/s.str().c_str());
}

void Vrr::finish()
{
    if (recordStatsToFile) {
        // write node statistics to file
        recordCurrentNodeStats(/*stage=*/"finish");
        writeToResultNodeFile();

        // write send records file
        if (!receivedMsgAggregate.empty())    // if there are received messages not recorded in allSendRecords yet
            writeReceivedMsgAggToRecords();
        if (!allSendRecords.empty())
            writeToResultMessageFile();
    }

    // cSimpleModule::finish();     // reference to 'finish' may be ambiguous
}

void Vrr::clearState()
{
    // cancel all setupReq timers in pendingVset
    cancelPendingVsetTimers();
    // clear vneis recorded in vset and pendingVset
    pendingVset.clear();
    vset.clear();
    // clear routing table
    vlrRoutingTable.clear();
    // clear records of non-essential vroutes and recently accepted setupReq
    nonEssRoutes.clear();
    // cancel all repairLink timers of lost pneis
    cancelRepairLinkTimers();
    lostPneis.clear();
    
    selfInNetwork = false;
    cancelEvent(inNetworkWarmupTimer);  // leaveOverlay won't be called on rep, rep.selfInNetwork will become true after asserting itself as rep after enough wait time
    
    // clear pset
    psetTable.clear();
    // reset representative
    representativeMap.clear();
    // testDstList.clear();         // don't clear testDstList bc it's only read and assigned in initialize()
    cancelEvent(beaconTimer);
    cancelEvent(purgeNeighborsTimer);
    cancelEvent(fillVsetTimer);
    cancelEvent(testPacketTimer);
    // cancelEvent(writeRoutingTableTimer);     // don't cancel writeRoutingTableTimer bc it's only scheduled in initialize()
    
    // clear temporary data
}

NotifyLinkFailureInt* Vrr::createNotifyLinkFailure(bool simLinkUp)
{
    EV_DEBUG << "Creating NotifyLinkFailure" << endl;
    NotifyLinkFailureInt *msg = new NotifyLinkFailureInt(/*name=*/"NotifyLinkFailure");
    msg->setSrc(vid);                       // vidByteLength
    msg->setSimLinkUp(simLinkUp);             // 1 bit, neglected
    int chunkByteLength = VLRRINGVID_BYTELEN;

    msg->setByteLength(chunkByteLength);
    return msg;
}

void Vrr::sendNotifyLinkFailure(const int& outGateIndex, bool simLinkUp)
{
    NotifyLinkFailureInt *msg = createNotifyLinkFailure(simLinkUp);

    EV_INFO << "Sending NotifyLinkFailure" << endl;
    sendCreatedPacket(msg, /*unicast=*/true, /*outGateIndex=*/outGateIndex, /*delay=*/0, /*checkFail=*/false);
}

void Vrr::processNotifyLinkFailure(NotifyLinkFailureInt *msgIncoming, bool& pktForwarded)
{
    int msgGateIndex = msgIncoming->getArrivalGate()->getIndex();
    VlrRingVID srcVid = msgIncoming->getSrc();
    bool simLinkUp = msgIncoming->getSimLinkUp();

    EV_INFO << "Processing NotifyLinkFailure: src = " << srcVid << ", simLinkUp = " << simLinkUp << endl;

    if (!simLinkUp) {
        if (failureSimulationPneiVidMap.find(srcVid) == failureSimulationPneiVidMap.end()) {    // src not already in failureSimulationPneiVidMap
            // drop future messages from src to simulate link failure
            failureSimulationPneiGates.insert(msgGateIndex);
            failureSimulationPneiVidMap.insert({srcVid, msgGateIndex});
            failureSimulationUnaddedPneiVids.erase(srcVid);

            auto pneiItr = psetTable.vidToStateMap.find(srcVid);
            if (pneiItr != psetTable.vidToStateMap.end()) {  // src in pset, schedule to remove it from pset
                    
                // Commented out - modify lastHeard time of pnei to expire it asap (wait for 1 sec in case I receive more NotifyLinkFailure)
                simtime_t modifiedLastHeard = simTime() - neighborValidityInterval;
                if (pneiItr->second.lastHeard > modifiedLastHeard) { // don't modify lastHeard to a later time
                    pneiItr->second.lastHeard = modifiedLastHeard;
                    schedulePurgeNeighborsTimer();  // schedule purgeNeighborsTimer to expire failure simulating pneis
                }
                
            }   // if src not in pset yet, since it's added to failureSimulationPneiVidMap, we'll no longer receive any messages from src and it won't be added to pset
            if (recordStatsToFile) { // write node status update
                std::ostringstream s;
                s << "beforeLinkFailure" << ": " << srcVid;
                recordNodeStatsRecord(/*infoStr=*/s.str().c_str());
            }
        }
    } else {    // simLinkUp=true
        failureSimulationPneiGates.erase(msgGateIndex);
        failureSimulationPneiVidMap.erase(srcVid);
        failureSimulationUnaddedPneiVids.erase(srcVid);
        
        if (recordStatsToFile) { // write node status update
            std::ostringstream s;
            s << "beforeLinkRestart" << ": " << srcVid;
            recordNodeStatsRecord(/*infoStr=*/s.str().c_str());
        }
    }
}

//
// beacon timers
//

void Vrr::scheduleBeaconTimer(bool firstTimer/*=false*/)
{
    // if (VlrRing *myself = dynamic_cast<VlrRing *>(this)) {
    //     if (myself->vid == 1473 && simTime() > 140)
    //         EV_DEBUG << "ohno" << endl;
    // }
    if (firstTimer) {
        EV_DEBUG << "Scheduling 1st beacon timer" << endl;
        double random = uniform(0, 1);
        scheduleAt(simTime() + random * beaconInterval, beaconTimer);
        // calibratedBeaconTime = beaconTimer->getArrivalTime();
    } else {
        EV_DEBUG << "Scheduling beacon timer" << endl;
        // calibratedBeaconTime = calibratedBeaconTime + beaconInterval;
        double random = uniform(-1, 1);
        // scheduleAt(calibratedBeaconTime + random * maxJitter, beaconTimer);
        scheduleAt(simTime() + beaconInterval + random * maxJitter, beaconTimer);
    }
}

void Vrr::processBeaconTimer()
{
    EV_DEBUG << "Processing beacon timer" << endl;
    // const L3Address selfAddress = getSelfAddress();
    // if (!selfAddress.isUnspecified())
    sendBeacon();
    
    scheduleBeaconTimer();
    schedulePurgeNeighborsTimer();
}


//
// handling purge neighbors timers
//

void Vrr::schedulePurgeNeighborsTimer()
{
    EV_DEBUG << "Scheduling purge neighbors timer" << endl;
    simtime_t nextExpiration = getNextNeighborExpiration();
    if (nextExpiration == 0) {   // getNextNeighborExpiration() returns SIMTIME_ZERO, meaning no neighbours yet
        if (purgeNeighborsTimer->isScheduled())
            cancelEvent(purgeNeighborsTimer);
    }
    else {  // there are neighbours in pset
        if (!purgeNeighborsTimer->isScheduled())
            scheduleAt(nextExpiration, purgeNeighborsTimer);
        // scheduled purgeNeighborsTimer is earlier or later than earliest neighbour expiration time
        else if (purgeNeighborsTimer->getArrivalTime() != nextExpiration) {
            cancelEvent(purgeNeighborsTimer);
            scheduleAt(nextExpiration, purgeNeighborsTimer);
        }
    }
}

simtime_t Vrr::getNextNeighborExpiration() const
{
    if (psetTable.vidToStateMap.size() == 0)    // no pset neighbours detected
        return SIMTIME_ZERO;

    simtime_t oldestLastHeard = SimTime::getMaxTime();
    for (const auto & elem : psetTable.vidToStateMap) {
        const simtime_t& time = elem.second.lastHeard;
        if (time < oldestLastHeard)
            oldestLastHeard = time;
    }

    return oldestLastHeard + neighborValidityInterval;
}

void Vrr::purgeNeighbors()
{
    std::vector<VlrRingVID> expiredPneis = psetTable.getExpiredNeighbours(simTime() - neighborValidityInterval);
    std::vector<VlrRingVID> expiredLinkedPneis;
    if (!expiredPneis.empty()) {
        EV_WARN << "Purging pset of expired pneis: " << expiredPneis << ", me=" << vid << endl;
        for (const auto& oldNei : expiredPneis) {
            auto pneiItr = psetTable.vidToStateMap.find(oldNei);
            if (pneiItr->second.state == PNEI_LINKED) {
                expiredLinkedPneis.push_back(oldNei);
            }
        }
        if (!expiredLinkedPneis.empty())
            handleLostPneis(expiredLinkedPneis);

        for (const auto& oldNei : expiredPneis) {
            auto pneiItr = psetTable.vidToStateMap.find(oldNei);
            // delete expired pnei from pset after handleLostPneis() s.t. it can access psetTable.vidToStateMap[oldNei] properties
            psetTable.vidToStateMap.erase(pneiItr);
        }
    }
}

void Vrr::processPurgeNeighborsTimer()
{
    EV_DEBUG << "Processing purge neighbors timer" << endl;
    purgeNeighbors();
    schedulePurgeNeighborsTimer();
}

void Vrr::scheduleFillVsetTimer(bool firstTimer/*=false*/, double maxDelay/*=0*/)  // default parameter value should only be defined in function declaration
{
    if (firstTimer) {
        EV_DEBUG << "Scheduling 1st fillVsetTimer after at least pneiDiscoveryTime" << endl;
        scheduleAt(simTime() + uniform(1, 6) * beaconInterval, fillVsetTimer);
    } else if (maxDelay > 0) {
        EV_DEBUG << "Scheduling fillVsetTimer within maxDelay = " << maxDelay << endl;
        scheduleAt(simTime() + uniform(0, 1) * maxDelay, fillVsetTimer);
    } else {
        EV_DEBUG << "Scheduling fillVsetTimer" << endl;
        scheduleAt(simTime() + uniform(0.8, 1.2) * fillVsetInterval, fillVsetTimer);
    }
}

void Vrr::scheduleTestPacketTimer(bool firstTimer/*=false*/)
{
    if (firstTimer) {
        EV_DEBUG << "Scheduling 1st TestPacket timer after at least sendTestPacketOverlayWaitTime = " << sendTestPacketOverlayWaitTime << endl;
        scheduleAt(simTime() + sendTestPacketOverlayWaitTime + uniform(0.5, 1.5) * testSendInterval, testPacketTimer);
    } else {
        EV_DEBUG << "Scheduling TestPacket timer" << endl;
        scheduleAt(simTime() + uniform(0.5, 1.5) * testSendInterval, testPacketTimer);
    }
}

void Vrr::sendBeacon()
{
    // if (representative.vid == vid)
    //     representative.sequencenumber++;
    totalNumBeaconSent++;

    auto beacon = createBeacon();
    EV_INFO << "Sending beacon: vid = " << beacon->getVid() << ", inNetwork = " << beacon->getInNetwork() << ", number of pset neighbours = " << beacon->getPsetNeighbourArraySize() << ", rep = " << beacon->getRepstate().vid << endl;
    sendCreatedPacket(beacon, /*unicast=*/false, /*outGateIndex=*/-1, /*delay=*/0);
}

VlrIntBeacon* Vrr::createBeacon()
{
    VlrIntBeacon *beacon = new VlrIntBeacon(/*name=*/"Beacon");
    // unsigned int vid
    // bool inNetwork
    beacon->setVid(vid);                    // vidByteLength
    beacon->setInNetwork(selfInNetwork);    // 1 bit, neglected
    int chunkByteLength = VLRRINGVID_BYTELEN;
    
    // VlrIntRepState repstate
    // VlrIntRepState repstate2;
    VlrIntRepState& repstate = beacon->getRepstateForUpdate();
    repstate.vid = VLRRINGVID_NULL;
    VlrIntRepState& repstate2 = beacon->getRepstate2ForUpdate();
    repstate2.vid = VLRRINGVID_NULL;
    bool sendSelfRep = false;     // if I should broadcast myself as rep
    if (!vset.empty()) {
        if (vid < vset.begin()->first)    // if I'm smaller than the smallest vnei I have
            sendSelfRep = true;
    } else if (selfInNetwork) // vset empty
        sendSelfRep = true;
    if (sendSelfRep) {
        repstate.vid = vid;                             // vidByteLength
        repstate.sequencenumber = ++selfRepSeqnum;      // 4 byte
        // repstate.hopcount = representative.hopcount; // 2 byte
    }
    simtime_t expiredLastheard = simTime() - repSeqValidityInterval;
    for (auto repMapItr = representativeMap.begin(); repMapItr != representativeMap.end(); ++repMapItr) {
        if (repMapItr->second.heardfromvid != VLRRINGVID_NULL && repMapItr->second.lastHeard > expiredLastheard) {  // rep hasn't expired
            if (repstate2.vid == VLRRINGVID_NULL) {     // repstate2 hasn't been added
                repstate2.vid = repMapItr->first;
                repstate2.sequencenumber = repMapItr->second.sequencenumber;
                // repMapItr->second.sequencenumber won't be 0, if I didn't send rep in my last beacon, repMapItr->second.lastBeaconSeqnum would be 0
                repMapItr->second.lastBeaconSeqnumUnchanged = (repMapItr->second.sequencenumber == repMapItr->second.lastBeaconSeqnum);
                repMapItr->second.lastBeaconSeqnum = repMapItr->second.sequencenumber;
            } else if (repstate.vid == VLRRINGVID_NULL) {     // repstate hasn't been added
                repstate.vid = repMapItr->first;
                repstate.sequencenumber = repMapItr->second.sequencenumber;
                repMapItr->second.lastBeaconSeqnumUnchanged = (repMapItr->second.sequencenumber == repMapItr->second.lastBeaconSeqnum);
                repMapItr->second.lastBeaconSeqnum = repMapItr->second.sequencenumber;
            } else      // both repstate and repstate2 have been added
                break;
        }
    }
    // only keep lastBeaconSeqnum for rep included in this beacon
    for (auto repMapItr = representativeMap.begin(); repMapItr != representativeMap.end(); ++repMapItr)
        if (repMapItr->first != repstate2.vid && repMapItr->first != repstate.vid)
            repMapItr->second.lastBeaconSeqnum = 0;

    chunkByteLength += (VLRRINGVID_BYTELEN + 4) * 2;
    
    // unsigned int psetNeighbour[]
    // bool psetNeighbourIsInNetwork[]
    beacon->setPsetNeighbourArraySize(psetTable.vidToStateMap.size());
    beacon->setPsetNeighbourIsInNetworkArraySize(psetTable.vidToStateMap.size());
    beacon->setPsetNeighbourIsLinkedArraySize(psetTable.vidToStateMap.size());
    unsigned int k = 0;
    for (const auto & elem : psetTable.vidToStateMap) {
        beacon->setPsetNeighbourIsLinked(k, (elem.second.state == PNEI_LINKED));       // 1 bit
        beacon->setPsetNeighbourIsInNetwork(k,  elem.second.inNetwork);    // 1 bit
        beacon->setPsetNeighbour(k++, elem.first);  // vidByteLength
    }
    chunkByteLength += k * VLRRINGVID_BYTELEN + 2;      // assume a maximum of 8 pneis, i.e. 2 byte in total for LINKED and inNetwork status

    beacon->setByteLength(chunkByteLength);
    return beacon;
}

void Vrr::processBeacon(VlrIntBeacon *beacon, bool& pktForwarded)
{
    int pneiGateIndex = beacon->getArrivalGate()->getIndex();
    VlrRingVID pneiVid = beacon->getVid();
    bool pneiInNet = beacon->getInNetwork();
    unsigned int pneiPsetCount = beacon->getPsetNeighbourArraySize();
    const VlrIntRepState& pneiRepState = beacon->getRepstate();
    const VlrIntRepState& pneiRepState2 = beacon->getRepstate2();
    EV_INFO << "Processing beacon (me=" << vid << "): vid = " << pneiVid << ", gateIndex = " << pneiGateIndex << ", inNetwork = " << pneiInNet << ", number of pset neighbours = " << pneiPsetCount << ", rep = " << pneiRepState.vid << ", rep2 = " << pneiRepState2.vid << endl;

    std::set<VlrRingVID> linked2hopPneis;      // linked pneis of sender
    bool pneiCanHearMe = false;
    for (unsigned int i = 0; i < pneiPsetCount; i++) {
        if (beacon->getPsetNeighbour(i) == vid) {   // sender can hear me
            pneiCanHearMe = true;
            // break;
        }
        else if (beacon->getPsetNeighbourIsLinked(i))    // record linked pnei of sender
            linked2hopPneis.insert(beacon->getPsetNeighbour(i));
    }
    // only record linked2hopPneis if pnei is LINKED    // NOTE I wasn't in linked2hopPneis
    if (!pneiCanHearMe)
        linked2hopPneis.clear();

    PNeiState pneiState = pneiCanHearMe ? PNEI_LINKED : PNEI_PENDING;
    bool pneiNewlyHeard = false;    // pnei didn't exist in pset
    // if (pneiCanHearMe && !psetTable.pneiIsLinked(pneiVid)) {
    //     // not necessary since we find LINKED pnei directly from pset >>> TODO: add pnei to routing table
    // }
    auto pneiItr = psetTable.vidToStateMap.find(pneiVid);
    if (pneiItr != psetTable.vidToStateMap.end()) {     // pnei already in pset
        // if pnei was linked but is no longer linked
        if (!pneiCanHearMe && pneiItr->second.state == PNEI_LINKED) {
            std::vector<VlrRingVID> pneiVids {pneiVid};
            handleLostPneis(pneiVids);
        }
        // if pnei wasn't linked but is now linked
        // else if (pneiCanHearMe && pneiItr->second.state != PNEI_LINKED) {
        //     handleNewPnei(pneiVid);
        // }
    }
    else if (pneiCanHearMe) {     // pnei will be added in pset as LINKED
        // handleNewPnei(pneiVid);
        pneiNewlyHeard = true;
    }
    if (pneiCanHearMe) {     // this is a linked pnei, ensure it's not in lostPneis
        handleLinkedPnei(pneiVid);
        // if I haven't joined overlay and linked pnei is inNetwork, I'll schedule fillVsetTimer to send first setupReq soon    NOTE if I just heard pnei, I don't schedule setupReq to it bc it might not have added me as LINKED pnei, which means it can't build vroute to me
        if (!pneiNewlyHeard && pneiInNet && !selfInNetwork && pendingVset.empty() && vset.empty() /*&& !repSeqObserveTimer->isScheduled()*/) {
            double maxDelayToNextTimer = beaconInterval;     // or beaconInterval, or related to fillVsetInterval or inNetworkWarmupTime
            if (fillVsetTimer->isScheduled() && fillVsetTimer->getArrivalTime() - simTime() >= maxDelayToNextTimer) {    // only reschedule fillVsetTimer if it won't come within maxDelayToNextTimer
                cancelEvent(fillVsetTimer);
                scheduleFillVsetTimer(/*firstTimer=*/false, /*maxDelay=*/maxDelayToNextTimer);
            }
        }
    }

    psetTable.setNeighbour(pneiVid, pneiGateIndex, pneiState, pneiInNet, /*mpneis=*/linked2hopPneis);
    
    updateRepState(pneiVid, pneiRepState, pneiCanHearMe, /*pneiOtherRepVid=*/pneiRepState2.vid);   // update representative
    updateRepState(pneiVid, pneiRepState2, pneiCanHearMe, /*pneiOtherRepVid=*/pneiRepState.vid);   // update representative

    // if this is a linked pnei, add 2-hop neighbours heard in beacon to overheardMPneis
    // if (pneiCanHearMe && checkOverHeardMPneis) {
    //     for (const auto& mpnei : linked2hopPneis)
    //         overheardMPneis[mpnei] = std::make_pair(pneiVid, simTime() + neighborValidityInterval);   // initialize expireTime of overheard trace to 2-hop pnei
    // }

     // schedulePurgeNeighborsTimer();
}

void Vrr::updateRepState(VlrRingVID pnei, const VlrIntRepState& pneiRepState, bool pneiIsLinked, const VlrRingVID& pneiOtherRepVid) {
    EV_DETAIL << "Processing RepState of pnei " << pnei << ": rep = " << pneiRepState.vid << ", seqNo = " << pneiRepState.sequencenumber << ", isLinked = " << pneiIsLinked << endl;
    // if (representative.vid == vid && pneiRepState.vid > vid && pneiRepState.sequencenumber >= representative.sequencenumber) {     // my vid < received rep, ensure my seqNo is larger than received rep, so that the smallest rep in network will also have the largest seqNo
    //     // NOTE when rep expires at node A, it may not have expired at A's pnei B which heard rep from A, to avoid A adopting B's rep, node shouldn't decrement rep seqNo even after rep expires
    //         // then to ensure smallest rep can be accepted at all nodes, smallest rep should also have the largest rep seqNo
    //     representative.sequencenumber = pneiRepState.sequencenumber +1;
    // }
    const VlrRingVID& pneiRepVid = pneiRepState.vid;
    if (pneiIsLinked && pneiRepVid != VLRRINGVID_NULL && pneiRepVid != vid) {   // rep is valid and not myself
        bool repMapUpdated = false;
        auto repMapItr = representativeMap.find(pneiRepVid);
        if (repMapItr == representativeMap.end()) {
            if (representativeMap.size() < representativeMapMaxSize) {   // representativeMap not yet full
                auto itr_bool = representativeMap.insert({pneiRepVid, {}});
                repMapItr = itr_bool.first;
                repMapItr->second.vid = pneiRepVid;
                repMapItr->second.lastBeaconSeqnum = 0;
                repMapItr->second.lastBeaconSeqnumUnchanged = false;
                repMapUpdated = true;
            } else {      // representativeMap full
                repMapItr = --representativeMap.end();      // repMapItr points to the largest key in representativeMap
                if (pneiRepVid < repMapItr->first) {        // replace largest rep with pneiRepVid
                    representativeMap.erase(repMapItr);     // erase largest rep in representativeMap

                    auto itr_bool = representativeMap.insert({pneiRepVid, {}});     // std::pair<iterator, bool>: iterator (to the inserted element), bool (whether the element is inserted)
                    repMapItr = itr_bool.first;
                    repMapItr->second.vid = pneiRepVid;
                    repMapItr->second.lastBeaconSeqnum = 0;
                    repMapItr->second.lastBeaconSeqnumUnchanged = false;
                    repMapUpdated = true;
                }
            }
            if (repMapUpdated) {    // added pneiRepVid to representativeMap
                auto erasedRepItr = expiredErasedReps.find(pneiRepVid);
                if (erasedRepItr != expiredErasedReps.end() && erasedRepItr->second >= pneiRepState.sequencenumber) {     // pneiRepVid was removed from representativeMap but now added back with an old seqNo
                    std::ostringstream s;
                    s << "Error: added old rep=" << pneiRepVid << ", old seqNo=" << erasedRepItr->second << ", new seqNo=" << pneiRepState.sequencenumber << ", heard from pnei=" << pnei;
                    recordNodeStatsRecord(/*infoStr=*/s.str().c_str());
                    // search "Error" in all nodeStats files: grep --include=*nodeStats*.csv -irnwl "." -e "Error"
                }
                if (representativeMap.size() > representativeMapActualMaxSize)
                    representativeMapActualMaxSize = representativeMap.size();
            }
        } else {    // pneiRepVid already in representativeMap
            if (pneiRepState.sequencenumber > repMapItr->second.sequencenumber) {   // received a larger seqNo for pneiRepVid
                repMapUpdated = true;
            }
        }

        if (repMapUpdated) {
            repMapItr->second.sequencenumber = pneiRepState.sequencenumber;
            repMapItr->second.heardfromvid = pnei;
            repMapItr->second.lastHeard = simTime();

        // bool repSeqUpdated = false;
        // if (pneiRepState.vid < representative.vid && pneiRepState.sequencenumber > representative.sequencenumber) {
        //     representative.vid = pneiRepState.vid;
        //     representative.sequencenumber = pneiRepState.sequencenumber;
        //     representative.heardfromvid = pnei;
        //     repSeqUpdated = true;
        // } else if (pneiRepState.vid == representative.vid && pneiRepState.sequencenumber > representative.sequencenumber) {   // pnei has the same rep as me but larger seqNo, adopt larger seqNo, this can't happen if I'm rep
        //     representative.sequencenumber = pneiRepState.sequencenumber;
        //     representative.heardfromvid = pnei;
        //     repSeqUpdated = true;
        // }
        // if (repSeqUpdated) {
        //     // schedule a new rep seqNo expiration timer
        //     if (repSeqExpirationTimer->isScheduled())
        //         cancelEvent(repSeqExpirationTimer);
        //     scheduleAt(simTime() + repSeqValidityInterval, repSeqExpirationTimer);

            // add representative to pendingVset, always needed to repair partition
            if (selfInNetwork) {
                // only send setup to a rep if the beacon contains two reps and this is the larger rep (from VRR paper)
                if (pneiOtherRepVid != VLRRINGVID_NULL && pneiRepVid > pneiOtherRepVid) {   // other rep is valid and not myself
                    auto addResult = shouldAddVnei(pneiRepVid);
                    bool& shouldAdd = std::get<0>(addResult);  // access first element in tuple
                    if (shouldAdd)
                        pendingVsetAdd(pneiRepVid, /*numTrials=*/1, /*heardRep=*/true);
                }
            }

            EV_INFO << "Representative heard from pnei " << pnei << ": " << pneiRepVid << endl;
        }            
    }
}

void Vrr::handleLinkedPnei(const VlrRingVID& pneiVid)
{
    EV_DEBUG << "Received beacon from a LINKED pnei " << pneiVid << " in PsetTable at me=" << vid << endl;
    // if pnei in lostPneis (maybe was a lost pnei) but is now LINKED, set vroutes broken by pnei available
    auto lostPneiItr = lostPneis.find(pneiVid);
    if (lostPneiItr != lostPneis.end()) {    //  pneiVid in lostPneis
        EV_WARN << "LINKED pnei " << pneiVid << " was a lost pnei, removing it from lostPneis, setting nextHop isUnavailable = 0 for brokenVroutes = [";
        for (const auto& brokenVroute : lostPneiItr->second.brokenVroutes)
            EV_WARN << brokenVroute << " ";
        EV_WARN << "]" << endl;
        // set vroutes broken by lost pnei available
        for (const auto& brokenPathid : lostPneiItr->second.brokenVroutes) {
            auto vrouteItr = vlrRoutingTable.vlrRoutesMap.find(brokenPathid);
            if (vrouteItr != vlrRoutingTable.vlrRoutesMap.end()) {  // pathid still in vlrRoutingTable
                // NOTE temporary route or dismantled vroute shouldn't be in brokenVroutes
                // one of both of prevhop and nexthop is a lost pnei
                if (lostPneiItr->first == vrouteItr->second.prevhopVid) { // route prevhop == lost pnei
                    vlrRoutingTable.setRouteItrPrevNextIsUnavailable(vrouteItr, /*setPrev=*/true, /*value=*/0);     // set route prevhop available
                    vlrRoutingTable.addRouteEndInEndpointMap(brokenPathid, vrouteItr->second.fromVid);     // add back route fromVid in endpointToRoutesMap
                } else { // route nexthop == lost pnei
                    vlrRoutingTable.setRouteItrPrevNextIsUnavailable(vrouteItr, /*setPrev=*/false, /*value=*/0);     // set route nexthop available
                    vlrRoutingTable.addRouteEndInEndpointMap(brokenPathid, vrouteItr->second.toVid);     // add back route toVid in endpointToRoutesMap
                }
            }
        }
        // // put temporary vlinks to lost pnei in nonEssRoutes as they are no longer necessary for broken vroutes
        // delayLostPneiTempVlinksTeardown(lostPneiItr->second.tempVlinks, /*delay=*/nonEssRouteExpiration);   // wait some time before tearing down tempVlinks so otherEnd also detects me as pnei  
        
        cancelAndDelete(lostPneiItr->second.timer);
        lostPneis.erase(lostPneiItr);
    }
}

void Vrr::handleLostPneis(const std::vector<VlrRingVID>& pneiVids)    // const std::vector<VlrPathID> *rmVroutesPtr/*=nullptr*/
{
    EV_WARN << "handleLostPneis: Pneis " << pneiVids << " lost connection with me=" << vid << endl;

    // remove 2-hop pneis associated with lost pneis
    // ensure pnei state isn't LINKED so that pnei isn't qualified as next hop in processDelayedRepairRequest()
    for (const auto& pneiVid : pneiVids) {
        psetTable.removeMPneisOfPnei(pneiVid);

        auto pneiItr = psetTable.vidToStateMap.find(pneiVid);
        if (pneiItr != psetTable.vidToStateMap.end() && pneiItr->second.state == PNEI_LINKED) {
            pneiItr->second.state = PNEI_PENDING;
            pneiItr->second.hadBeenInNetwork = false;   // remove 1-hop route from routing table
        }

        // invalidate records in representativeMap with heardfromvid == pnei
        for (auto repMapItr = representativeMap.begin(); repMapItr != representativeMap.end(); ++repMapItr)
            if (repMapItr->second.heardfromvid == pneiVid)
                repMapItr->second.heardfromvid = VLRRINGVID_NULL;
    }

    std::map<VlrRingVID, std::vector<VlrPathID>> nextHopToPathidsMap;    // for Teardown to send, map next hop address to pathids (since these are temporary routes, next hop should always be available)
    for (size_t i = 0; i < pneiVids.size(); ++i) {
        const VlrRingVID& pneiVid = pneiVids[i];
        // const L3Address& pneiAddr = pneiAddrs[i];

        // find vroutes in vlrRoutingTable with prevhop/nexthop == pnei, set them unavailable
        ASSERT(pneiVid != VLRRINGVID_NULL);
        for (auto vrouteItr = vlrRoutingTable.vlrRoutesMap.begin(); vrouteItr != vlrRoutingTable.vlrRoutesMap.end(); ) {
            if (vrouteItr->second.prevhopVid == pneiVid || vrouteItr->second.nexthopVid == pneiVid) {   // route prevhop/nexthop == pnei
                const VlrPathID& oldPathid = vrouteItr->first;     // pathid broken by lost pnei

                // NOTE for vroutes in nonEssRoutes, I can't send Teardown bc next hop toward the other end is the lost pnei, but the lost pnei doesn't know this vroute is non-essential and will add it to brokenVroutes and schedule repairLinkReq for it
                // vroutes in nonEssRoutes will expire and be removed from vlrRoutingTable anyway
                // else if (nonEssRoutes.find(oldPathid) != nonEssRoutes.end()) {
                // }
                // for vroute, set it unavailable and add to lostPneis
                if (vrouteItr->second.prevhopVid == pneiVid) {   // route prevhop == lost pnei
                    // set route prevhop unavailable
                    vlrRoutingTable.setRouteItrPrevNextIsUnavailable(vrouteItr, /*setPrev=*/true, /*value=*/1);
                    if (removeBrokenVrouteEndpointInRepair)
                        // remove route fromVid in endpointToRoutesMap
                        vlrRoutingTable.removeRouteEndFromEndpointMap(oldPathid, vrouteItr->second.fromVid);
                    
                    // // if route fromVid is no longer an available endpoint, add it to recentUnavailableRouteEnd to leave time for repair
                    // if (!vlrRoutingTable.findAvailableRouteEndInEndpointMap(vrouteItr->second.fromVid))
                    //     recentUnavailableRouteEnd.insert({vrouteItr->second.fromVid, {oldPathid, simTime() + recentUnavailableRouteEndExpiration}});
                }
                if (vrouteItr->second.nexthopVid == pneiVid) {    // route nexthop == lost pnei
                    // set route nexthop unavailable
                    vlrRoutingTable.setRouteItrPrevNextIsUnavailable(vrouteItr, /*setPrev=*/false, /*value=*/1);
                    if (removeBrokenVrouteEndpointInRepair)
                        // remove route toVid in endpointToRoutesMap
                        vlrRoutingTable.removeRouteEndFromEndpointMap(oldPathid, vrouteItr->second.toVid);

                    // // if route toVid is no longer an available endpoint, add it to recentUnavailableRouteEnd to leave time for repair
                    // if (!vlrRoutingTable.findAvailableRouteEndInEndpointMap(vrouteItr->second.toVid))
                    //     recentUnavailableRouteEnd.insert({vrouteItr->second.toVid, {oldPathid, simTime() + recentUnavailableRouteEndExpiration}}); 
                }

                auto lostPneiItr = lostPneis.find(pneiVid);
                if (lostPneiItr == lostPneis.end())
                    lostPneis.insert({pneiVid, {{oldPathid}, nullptr}});
                else
                    lostPneiItr->second.brokenVroutes.insert(oldPathid);
                
                vrouteItr++;
                
                
            } else
                vrouteItr++;
        }
        auto lostPneiItr = lostPneis.find(pneiVid);
        if (lostPneiItr != lostPneis.end() && lostPneiItr->second.timer == nullptr) {   // some vroute was broken by lost pnei
            EV_WARN << "Lost pnei " << pneiVid << " caused broken vroutes = [";
            for (const auto& pathid : lostPneiItr->second.brokenVroutes) 
                EV_WARN << pathid << " ";
            EV_WARN << "]" << endl;

            // schedule WaitRepairLinkIntTimer to pneiVid
            char timerName[40] = "WaitRepairLinkIntTimer:";
            lostPneiItr->second.timer = new WaitRepairLinkIntTimer(strcat(timerName, std::to_string(pneiVid).c_str()));
            lostPneiItr->second.timer->setDst(pneiVid);
            int retryCount = (sendRepairLocalNoTemp) ? 0 : repairLinkReqRetryLimit;
            lostPneiItr->second.timer->setRetryCount(retryCount);
            // double delay = (failureSimulateLink && !failureSimulationMap.empty()) ? uniform(0, repairLinkReqfailureSimulateLinkMaxDelay) : 0.01;  // if failureSimulateLink, all link failures will be detected at the same time at both ends, thus add random delay to avoid many nodes flooding at the same time
            double delay = 0.01;    // add minimal delay to ensure I check delayedRepairLinkReq before sending repairLinkReq, bc even if I'm the larger link end, I may have detected link failure too late that the smaller end has already sent repairLinkReq to me
            // NOTE each end of the failed link may need to send RepairLinkReq if it has lost prevhop of some brokenVroute, but if both ends still alive and close, only one temporary route needs to be setup btw them, hence the smaller link end will send RepairLinkReq later
            
            // schedule a WaitRepairLinkIntTimer now
            scheduleAt(simTime() +delay, lostPneiItr->second.timer);
            EV_WARN << "Scheduling a RepairLinkReq timer (retryCount = " << lostPneiItr->second.timer->getRetryCount() << ") to lost pnei " << pneiVid << " right away at " << lostPneiItr->second.timer->getArrivalTime() << endl;
            
        }
    }
    // // check delayedRepairLinkReq to see if I can reply to brokenVroutes in received repairLinkReq after new link breakage detected
    // if (!delayedRepairReqTimer->isScheduled())
    //     scheduleAt(simTime(), delayedRepairReqTimer);
}

// void Vrr::processRepSeqExpirationTimer()
// {
//     // a valid rep seqNo timed out
//     EV_DEBUG << "Processing repExpirationTimer at node " << vid << ", representative " << representative << " expired" << endl;
    
//     if (recordStatsToFile) { // write node status update
//         recordNodeStatsRecord(/*infoStr=*/"rootLost");   // unused params (stage)
//     }

//     // // tear down all vroutes in routing table, inNetwork = false
//     // leaveOverlay();
//     representative.heardfromvid = VLRRINGVID_NULL;      // this means my rep seqNo is invalid
//     representative.vid = vid;      // reset rep to myself
//     // schedule observe period (don't send setupReq) after rep-timeout
//     // scheduleAt(simTime() + repSeqValidityInterval, repSeqObserveTimer);
// }

void Vrr::processInNetworkWarmupTimer()
{
    EV_DEBUG << "Processing inNetwork warmup timer at node " << vid << endl;
    if (!selfInNetwork) {
        bool allowInNetwork = true;
        if (vset.empty() && !pendingVset.empty()) {   // there are setupReq timers in pendingVset
            simtime_t_cref someArrivalTime = getASetupReqPendingArrivalTime();
            if (!someArrivalTime.isZero()) {
                allowInNetwork = false;
                scheduleAt(someArrivalTime + 1, inNetworkWarmupTimer);
                EV_INFO << "selfInNetwork warm-up time is over, but vset isn't full and I have scheduled setupReq timer, extending inNetwork warm-up" << endl;
            }
        }
        if (allowInNetwork && vset.empty()) {
            // assert I don't have LINKED && inNetwork pnei
            VlrRingVID proxy = getProxyForSetupReq();
            if (proxy != VLRRINGVID_NULL && fillVsetTimer->isScheduled()) {     // fillVsetTimer should always be scheduled after node start
                allowInNetwork = false;
                scheduleAt(fillVsetTimer->getArrivalTime() + 1, inNetworkWarmupTimer);
                EV_INFO << "selfInNetwork warm-up time is over, but I have LINKED && inNetwork pnei, extending inNetwork warm-up" << endl;
            }
        }
        if (allowInNetwork) {    // either vset isn't empty or I have no scheduled setupReq to send
            selfInNetwork = true;
            // vset likely unfull
            EV_WARN << "selfInNetwork warm-up time is over, setting inNetwork = " << selfInNetwork << " with vset = " << printVsetToString() << endl;

            if (recordStatsToFile) { // write node status update
                std::ostringstream s;
                s << "inNetwork=" << selfInNetwork << " vset=" << printVsetToString() << " vsetSize=" << vset.size();
                recordNodeStatsRecord(/*infoStr=*/s.str().c_str());   // unused params (stage)
            }
        }
    }
}

simtime_t_cref Vrr::getASetupReqPendingArrivalTime() const
{
    for (const auto& pair : pendingVset) {
        if (pair.second != nullptr && pair.second->isScheduled())
            return pair.second->getArrivalTime();
    }
    return SIMTIME_ZERO;
}

void Vrr::processNonEssRouteTeardownTimer()
{
    // send Teardown for every route in nonEssRoutes
    for (auto it = nonEssRoutes.begin(); it != nonEssRoutes.end(); ++it) {
        const VlrPathID& oldPathid = *it;

        auto vrouteItr = vlrRoutingTable.vlrRoutesMap.find(oldPathid);
        if (vrouteItr != vlrRoutingTable.vlrRoutesMap.end()) {  // path to tear down found in vlrRoutingTable
            bool isToNexthop = (vid == vrouteItr->second.fromVid);
            const VlrRingVID& otherEnd = (isToNexthop) ? vrouteItr->second.toVid : vrouteItr->second.fromVid;
            const VlrRingVID& nextHopVid = (isToNexthop) ?  vrouteItr->second.nexthopVid : vrouteItr->second.prevhopVid;
            char nextHopIsUnavailable = vlrRoutingTable.getPrevNextIsUnavailable(vrouteItr->second.isUnavailable, /*getPrev=*/!isToNexthop);     // if isToNexthop, next hop is nexthop, getPrev=false

            if (nextHopIsUnavailable != 1) {  // next hop isn't unavailable
                const auto& teardownOut = createTeardown(/*pathid=*/oldPathid, /*addSrcVset=*/true);
                sendCreatedTeardown(teardownOut, /*nextHopPnei=*/nextHopVid);

                if (recordStatsToFile) {   // record sent message
                    recordMessageRecord(/*action=*/0, /*src=*/vid, /*dst=*/otherEnd, "Teardown", /*msgId=*/0, /*hopcount=*/0, /*chunkByteLength=*/0, /*infoStr=*/"nonEssRoutes");   // unused params (msgId, hopcount, chunkByteLength) are just assigned to 0
                }
            } else {  // next hop is unavailable, remove this vroute from lostPneis.brokenVroutes
                removeRouteFromLostPneiBrokenVroutes(oldPathid, /*lostPneiVid=*/nextHopVid);
            }

            vlrRoutingTable.removeRouteByPathID(oldPathid);     // remove vroute in vlrRoutingTable
        }
    }
    nonEssRoutes.clear();
}

void Vrr::processFailedPacket(cPacket *packet, unsigned int pneiVid)
{
    VlrIntUniPacket *unicastPacket = check_and_cast<VlrIntUniPacket *>(packet);
    if (recordStatsToFile && recordDroppedMsg) {   // record dropped message
        recordMessageRecord(/*action=*/4, /*src=*/VLRRINGVID_NULL, /*dst=*/VLRRINGVID_NULL, unicastPacket->getName(), /*msgId=*/unicastPacket->getMessageId(), /*hopcount=*/unicastPacket->getHopcount(), /*chunkByteLength=*/unicastPacket->getByteLength(), /*infoStr=*/"linkfailure");
    }
    // VlrRingVID pneiVid = psetTable.getPneiFromGateIndex(failedGateIndex);

    if (pneiVid != VLRRINGVID_NULL) {
        auto pneiItr = psetTable.vidToStateMap.find(pneiVid);
        if (pneiItr != psetTable.vidToStateMap.end() && pneiItr->second.state == PNEI_LINKED) {
            EV_WARN << "Removing next hop (LINKED) of failed VLR packet: pneiVid = " << pneiVid << endl;
            std::vector<VlrRingVID> pneiVids {pneiItr->first};
            handleLostPneis(pneiVids);
            // delete lost pnei from pset
            psetTable.vidToStateMap.erase(pneiItr);
        }
    }
    delete packet;  // drop packet as it couldn't be sent
}

// if failedGateIndex > -1, failedPneis should only contain one pnei, and failedGateIndex is its gate index
void Vrr::handleFailureLinkSimulation(const std::set<unsigned int>& failedPneis, int failedGateIndex/*=-1*/)
{
    EV_INFO << "Handling failure link Simulation at node " << vid << "with failedPneis = " << failedPneis << endl;
    for (const auto& srcVid : failedPneis) {
        bool addedToFailurePneiVidMap = false;
        if (failureSimulationPneiVidMap.find(srcVid) == failureSimulationPneiVidMap.end()) {    // src not already in failureSimulationPneiVidMap
            auto pneiItr = psetTable.vidToStateMap.find(srcVid);
            if (pneiItr != psetTable.vidToStateMap.end()) {  // src in pset, i.e. I know its gateIndex
                // drop future messages from src to simulate link failure
                failureSimulationPneiGates.insert(pneiItr->second.gateIndex);
                failureSimulationPneiVidMap.insert({srcVid, pneiItr->second.gateIndex});
                addedToFailurePneiVidMap = true;
                
            } else if (failedGateIndex >= 0) {
                ASSERT(failedPneis.size() == 1);
                failureSimulationPneiGates.insert(failedGateIndex);
                failureSimulationPneiVidMap.insert({srcVid, failedGateIndex});
                addedToFailurePneiVidMap = true;
            } else      // src not in pset yet and failedGateIndex not known
                failureSimulationUnaddedPneiVids.insert(srcVid);
        }
        if (addedToFailurePneiVidMap) {
            failureSimulationUnaddedPneiVids.erase(srcVid);
            
            if (recordStatsToFile) { // write node status update
                std::ostringstream s;
                s << "beforeLinkFailure" << ": " << srcVid;
                recordNodeStatsRecord(/*infoStr=*/s.str().c_str());
            }
        }
    }
    
}

void Vrr::handleFailureLinkRestart(const std::set<unsigned int>& restartPneis)
{
    EV_INFO << "Handling failure link Restart at node " << vid << "with restartPneis = " << restartPneis << endl;
    // send NotifyLinkFailure(linkUp) to affected pneis to remove my vid in their failureSimulationPneiVidMap
    for (const auto& pnei : restartPneis) {
        auto failureSimPneiItr = failureSimulationPneiVidMap.find(pnei);
        if (failureSimPneiItr != failureSimulationPneiVidMap.end()) {
            EV_INFO << "Sending NotifyLinkFailure(linkUp) from me=" << vid << " to pnei=" << pnei << endl;
            int pneiGateIndex = failureSimPneiItr->second;
            sendNotifyLinkFailure(pneiGateIndex, /*simLinkUp=*/true);
            
            failureSimulationPneiGates.erase(pneiGateIndex);
            failureSimulationPneiVidMap.erase(failureSimPneiItr);
        } else  // pnei not in failureSimulationPneiVidMap
            failureSimulationUnaddedPneiVids.erase(pnei);
    }
    if (recordStatsToFile) { // write node status update
        std::ostringstream s;
        s << "beforeLinkRestart" << ": " << restartPneis;
        recordNodeStatsRecord(/*infoStr=*/s.str().c_str());
    }
}

// get a LINKED pnei as proxy for setupReq, return vid of proxy, or VLRRINGVID_NULL if no qualified proxy found
VlrRingVID Vrr::getProxyForSetupReq(bool checkInNetwork/*=true*/) const
{
    std::vector<VlrRingVID> activePneis;
    // use a random LINKED && inNetwork pnei
    if (checkInNetwork)
        activePneis = psetTable.getPneisInNetwork();
    else    // proxy doesn't have to be inNetwork for setupReqTrace, can use a random LINKED pnei
        activePneis = psetTable.getPneisLinked();
    
    size_t numActivePneis = activePneis.size();
    if (numActivePneis > 0)   // there is pnei that can be my proxy
        return activePneis[intuniform(0, numActivePneis-1)];
    
    // I'm not rep, use the pnei from which I heard the newest rep seqNo if it's LINKED and inNetwork
    // if (representative.heardfromvid != VLRRINGVID_NULL && psetTable.pneiIsLinkedInNetwork(representative.heardfromvid))
    //     return representative.heardfromvid;
    
    return VLRRINGVID_NULL;
}

// numTrials: [1, setupReqRetryLimit]
// if heardRep=true, this node is a representative I just heard from beacon, I can send setupReply to it instead of setupReq
bool Vrr::pendingVsetAdd(VlrRingVID node, int numTrials, bool heardRep/*=false*/)
{
    bool addedNode = false;
    if (node != vid && vset.find(node) == vset.end()) {  // node not myself and not a vnei in vset
        auto pendingItr = pendingVset.find(node);
        if (pendingItr == pendingVset.end()) {  // node not in pendingVset, see if it should be added
            addedNode = true;
            auto itr_bool = pendingVset.insert({node, nullptr});    // std::pair<iterator, bool>: iterator (to the inserted element), bool (whether the element is inserted)
            pendingItr = itr_bool.first;
            WaitSetupReqIntTimer*& setupReqTimer = pendingItr->second;
            char timerName[40] = "WaitSetupReqIntTimer:";
            setupReqTimer = new WaitSetupReqIntTimer(strcat(timerName, std::to_string(node).c_str()));
            setupReqTimer->setDst(node);
            // setupReqTimer->setTimerType(0);
            setupReqTimer->setRepairRoute(heardRep);   // NOTE using setupReqTimer.repairRoute as heardRep
            // setupReqTimer->setAlterPendingVnei(VLRRINGVID_NULL);
            setupReqTimer->setReqVnei(true);
            setupReqTimer->setRetryCount(setupReqRetryLimit - numTrials);

            double delay = 0 /*uniform(0, 0.1) * routeSetupReqWaitTime*/;  // add random delay for setupReq(reqVnei=false) to avoid sending multiple setupReq at same time when tryNonEssRoute becomes true
            scheduleAt(simTime() + delay, setupReqTimer);   // schedule setupReq to node now
        } else if (pendingItr->second != nullptr) {    // node already in pendingVset
            if (heardRep)   // only change heardRep from false to true
                pendingItr->second->setRepairRoute(heardRep);   // NOTE using setupReqTimer.repairRoute as heardRep

            int expectedRetryCount = setupReqRetryLimit - numTrials;
            if (pendingItr->second->getRetryCount() > expectedRetryCount)
                pendingItr->second->setRetryCount(expectedRetryCount);
            if (pendingItr->second->getRetryCount() < setupReqRetryLimit) {     // reschedule setupReq timer to node if it's not coming soon
                double maxDelayToNextTimer = 0.1;    // 1 or beaconInterval
                if (pendingItr->second->isScheduled() && pendingItr->second->getArrivalTime() - simTime() >= maxDelayToNextTimer) {    // only reschedule fillVsetTimer if it won't come within maxDelayToNextTimer
                    cancelEvent(pendingItr->second);
                    scheduleAt(simTime() + uniform(0, maxDelayToNextTimer), pendingItr->second);   // schedule setupReq to node now
                }
            }
        }
    }
    return addedNode;
}

// remove node if exists in pendingVset, cancel corresponding WaitSetupReqIntTimer (allocated or nullptr, i.e. not deleted), and adjust pendingVsetFarthest if relevant
void Vrr::pendingVsetErase(VlrRingVID node)
{
    auto itr = pendingVset.find(node);
    if (itr != pendingVset.end()) { // if node exists in pendingVset
        cancelAndDelete(itr->second);
        pendingVset.erase(itr);
    }
}

void Vrr::vsetInsertRmPending(VlrRingVID node)
{
    // vset.insert(node);
    // vset.insert({node, 0});
    // vset.insert({node, {{}, nullptr}});  // vnei: {std::set<VlrPathID>, WaitSetupReqIntTimer*}
    vset.insert({node, {}});              // vnei: {std::set<VlrPathID>}
    pendingVsetErase(node);

    // firstVneiReceived = true;
    // if (vsetHalfCardinality > 1)   // record two closest vneis to me
    // if (vset.size() >= 2 * vsetHalfCardinality) {
    //     // vsetClosest = getClosestVneisInVset();
    //     // calcVneiDistInVset();
    // }    
}

// if addToPending=true, send setupReq to try to add node back to vset
bool Vrr::vsetEraseAddPending(VlrRingVID node, bool addToPending)
{
    auto itr = vset.find(node);
    if (itr != vset.end()) { // if node exists in vset
        // cancelAndDelete(itr->second.setupReqTimer);
        vset.erase(itr);
        // node that was in vset shouldn't be in pendingVset
        if (addToPending)
            // vnei lost, retransmit this setupReq multiple times
            pendingVsetAdd(node, /*numTrials=*/setupReqRetryLimit);
        return true;
    }
    // if (vset.erase(node))

    return false;   // if node not in vset
}

void Vrr::processWaitSetupReqTimer(WaitSetupReqIntTimer *setupReqTimer)
{
    // WaitSetupReqIntTimer *setupReqTimer = check_and_cast<WaitSetupReqIntTimer *>(message);
    VlrRingVID targetVid = setupReqTimer->getDst();
    int retryCount = setupReqTimer->getRetryCount();
    // bool repairRoute = setupReqTimer->getRepairRoute();     // setupReq should be sent with repairRoute=true and traceVec to record trace toward dst
    bool heardRep = setupReqTimer->getRepairRoute();        // NOTE using setupReqTimer.repairRoute as heardRep
    // // bool allowSetupReqTrace = setupReqTimer->getAllowSetupReqTrace();     // setupReqTrace should be sent if setupReqRetryLimit has been reached
    // bool reqVnei = setupReqTimer->getReqVnei();     // setupReq should be sent with reqVnei=true and traceVec to record trace toward dst
    // char timerType = setupReqTimer->getTimerType();     // 0: pendingVset[dst], 1: vset[dst]

    EV_DETAIL << "(me=" << vid << ") Processing wait setupReq timer to node " << targetVid << ", WaitSetupReqIntTimer retryCount: " << retryCount << endl;

    bool resendSetupReq = false;
        
    // Commented out when setupReqRetryLimit = VLRSETUPREQ_THRESHOLD -- NOTE if retryCount > setupReqRetryLimit, it's not considered a proper retry, retryCount is probably set manually, manual retryCount range: [VLRSETUPREQ_THRESHOLD - setupReqRetryLimit, VLRSETUPREQ_THRESHOLD-1], we'll send a setupReq
    if (retryCount < setupReqRetryLimit /*|| (retryCount >= (VLRSETUPREQ_THRESHOLD - setupReqRetryLimit) && retryCount < VLRSETUPREQ_THRESHOLD)*/) {
        resendSetupReq = true;

        bool setupReqSent = false;
        // bool recordMessageAndReschedule = false;
        // // Commented out bc we want to add random delay only for the first setupReq(repairRoute=true) to targetVid to avoid jamming the failure area, that delay is added when we first schedule WaitSetupReqIntTimer to repair broken pathid in removeEndpointOnTeardown()
        // // double delay = (repairRoute) ? uniform(0, patchedRouteRepairSetupReqMaxDelay) : 0;  // if this setupReq is repairing a patched route, add a random delay to avoid jamming the failure area
        // double delay = 0;

        // resend setupReq to targetVid
        simtime_t expiredLastheard = simTime() - repSeqValidityInterval;

        auto repMapItr = representativeMap.find(targetVid);
        if (selfInNetwork && heardRep && repMapItr != representativeMap.end() && repMapItr->second.heardfromvid != VLRRINGVID_NULL && repMapItr->second.lastHeard > expiredLastheard) {
            VlrRingVID newnode = targetVid;
            // ensure it's possible for me send SetupReply to rep via rep path
            VlrIntOption vlrOptionOut;
            initializeVlrOption(vlrOptionOut, /*dstVid=*/newnode);
            VlrRingVID nextHopVid = findNextHopForSetupReply(vlrOptionOut, /*newnode=*/newnode);
            if (nextHopVid == VLRRINGVID_NULL) {
                EV_WARN << "No next hop found use rep path to send SetupReply at me = " << vid << " to rep = " << newnode << endl;
            } else {
                // try to add rep to my vset
                std::set<VlrRingVID> ovset = convertVsetToSet();
                // NOTE targetVid can't be in vset, otherwise this WaitSetupReqIntTimer would've been erased from pendingVset when it was added to vset
                bool addedSrc = vrrAdd(/*setupPacket=*/nullptr, /*srcVid=*/newnode);

                if (addedSrc) {     // targetVid already removed from pendingVset here!
                    // send SetupReply to rep and add it to vset
                    bool addedRoute = false;
                    VlrPathID newPathid;
                    do {    // loop in case generated newPathid isn't unique in my vlrRoutingTable
                        newPathid = genPathID(newnode);
                        addedRoute = vlrRoutingTable.addRoute(newPathid, vid, newnode, /*prevhopVid=*/VLRRINGVID_NULL, /*nexthopVid=*/nextHopVid, /*isVsetRoute=*/true).second;
                    } while (!addedRoute);
                    
                    SetupReplyInt* replyOutgoing = createSetupReply(/*newnode=*/newnode, /*proxy=*/newnode, newPathid, /*srcVsetPtr=*/&ovset);    // srcVset = oldVset (doesn't include newnode) in the created SetupReply
                    replyOutgoing->setVlrOption(vlrOptionOut);
                    EV_INFO << "Sending SetupReply to rep = " << newnode << " using rep path, src = " << vid << ", pathid = " << newPathid << ", proxy = " << replyOutgoing->getProxy() << ", nexthop: " << nextHopVid << endl;
                    sendCreatedSetupReply(replyOutgoing, /*outGateIndex=*/psetTable.getPneiGateIndex(nextHopVid));

                    if (recordStatsToFile) {   // record sent message
                        recordMessageRecord(/*action=*/0, /*src=*/vid, /*dst=*/newnode, "SetupReply", /*msgId=*/0, /*hopcount=*/0, /*chunkByteLength=*/0, /*infoStr=*/"setup to rep");
                    }

                    // vsetInsertRmPending(newnode) done in vrrAdd()
                    vset.at(newnode).insert(newPathid);

                    // set inNetwork = true bc I added newnode in vset, vset isn't empty
                    recordNewVnei(newnode);

                    return;
                }
            }                
        }

        // get random LINKED && inNetwork pnei
        VlrRingVID proxy = getProxyForSetupReq(/*checkInNetwork=*/true);
        if (proxy != VLRRINGVID_NULL) {
            EV_DETAIL << "Sending setupReq to dst = " << targetVid << ", proxy = " << proxy << endl;
            setupReqSent = true;
            // will reschedule this setupReqTimer and increment retryCount
            sendSetupReq(targetVid, proxy, setupReqTimer);
        }

        if (!setupReqSent) {     // if setupReq can't be sent, increment retryCount so we may send a setupReqTrace later
            int nextRetryCount = retryCount + 1;
            setupReqTimer->setRetryCount(nextRetryCount);
            EV_INFO << "No next hop found to send setupReq at me = " << vid << " on WaitSetupReqTimer retryCount = " << retryCount << ", rescheduling setupReq timer to dst = " << targetVid << ", new retryCount = " << nextRetryCount << endl;
            if (nextRetryCount >= setupReqRetryLimit)   // no more setupReq trials, can remove this WaitSetupReqTimer now
                scheduleAt(simTime(), setupReqTimer);
            else    // give some time before another setupReq trial
                scheduleAt(simTime() + uniform(0.4, 0.6) * routeSetupReqWaitTime, setupReqTimer);
        }
    }
    // else if (retryCount >= VLRSETUPREQ_THRESHOLD && allowSetupReqTrace) {
    //     retryCount -= VLRSETUPREQ_THRESHOLD;
    //     if (retryCount < setupReqTraceRetryLimit) {
    //         resendSetupReq = true;
    //         // resend setupReqTrace to targetVid, will reschedule this setupReqTimer and increment retryCount
    //         sendSetupReqTrace(targetVid, setupReqTimer);
    //     }
    // }
    if (!resendSetupReq) {
        EV_INFO << "SetupReq attempts for node " << targetVid << " reached setupReqRetryLimit=" << setupReqRetryLimit << " limit. Stop sending setupReq." << endl;
        // delete setupReqTimer;    // don't delete because we need to record number of setupReq sent to targetVid
        
        // delete targetVid from pendingVset as it's not reachable
        EV_INFO << "Deleting node " << targetVid << " from pendingVset" << endl;
        pendingVsetErase(targetVid);    // this setupReqTimer will be cancelAndDelete()
    }
}

// get the farthest ccw vnei and cw vnei (wrap-around search in vset) to me
std::pair<VlrRingVID, VlrRingVID> Vrr::getFarthestVneisInFullVset() const
{
    ASSERT(vset.size() == 2 * vsetHalfCardinality);     // assert vset is full
    auto itrup = vset.lower_bound(vid);  // itr type: std::set<...>::const_iterator bc this const function can't change vset
    for (int i = 0; i < vsetHalfCardinality-1; ++i) {   // increment itrup to get the vnei with largest cw distance to me
        if (itrup == vset.end())
            itrup = vset.begin();
        itrup++;
    }
    if (itrup == vset.end())
        itrup = vset.begin();
    auto itrlow = itrup;
    itrlow++;
    if (itrlow == vset.end())
        itrlow = vset.begin();
        
    // return std::make_pair(*itrlow, *itrup);
    return std::make_pair(itrlow->first, itrup->first);
}

// return shouldAdd, removedNeis
// if newnode already in vset, return shouldAdd=true
std::tuple<bool, std::vector<VlrRingVID>> Vrr::shouldAddVnei(const VlrRingVID& newnode) const
{
    // ASSERT(vset.find(newnode) == vset.end());   // assert newnode not in vset already
    std::tuple<bool, std::vector<VlrRingVID>> result;
    EV_DEBUG << "Deciding whether newnode=" << newnode << " can be added to vset" << endl;
    bool& shouldAdd = std::get<0>(result);  // access first element in tuple
    std::vector<VlrRingVID>& removedNeis = std::get<1>(result);
    // std::set<VlrRingVID>& neisToForward = std::get<2>(result);

    shouldAdd = (vset.size() < 2 * vsetHalfCardinality || vset.find(newnode) != vset.end());    // if vset not full, or newnode already in vset, shouldAdd = true
    if (shouldAdd) {
        // if (findNeisToForward) {
        //     // convert vset to a set of vids
        //     std::vector<VlrRingVID> vsetVec;
        //     for (const auto& vnei : vset)
        //         vsetVec.push_back(vnei.first);
        //     // select nodes in vset but not in knownSet, put them in neisToForward
        //     std::set_difference(vsetVec.begin(), vsetVec.end(), knownSet.begin(), knownSet.end(), std::inserter(neisToForward, neisToForward.begin()));
        //     // std::set_difference(vset.begin(), vset.end(), knownSet.begin(), knownSet.end(), std::inserter(neisToForward, neisToForward.begin()));
        // }
        // EV_DEBUG << "Vset not full, can add newnode=" << newnode << " to vset, neisToForward: " << neisToForward << endl;

    } else {
        // vset full, check if newnode is closer than the farthest ccw or cw vnei
        auto vneiPair = getFarthestVneisInFullVset();
        if (getVid_CCW_Distance(vid, newnode) < getVid_CCW_Distance(vid, vneiPair.first)) { // newnode closer than farthest ccw vnei
            shouldAdd = true;
            removedNeis.push_back(vneiPair.first);
            // if (findNeisToForward)
            //     getVneisForwardInFullVsetTo(newnode, knownSet, /*isNewnodeCCW=*/true, neisToForward);
            EV_DEBUG << "Vset full, can add newnode=" << newnode << " to vset, replaces farthest ccw vnei: " << removedNeis[0] << endl;
        }
        else if (getVid_CW_Distance(vid, newnode) < getVid_CW_Distance(vid, vneiPair.second)) {  // newnode closer than farthest cw vnei
            shouldAdd = true;
            removedNeis.push_back(vneiPair.second);
            // if (findNeisToForward)
            //     getVneisForwardInFullVsetTo(newnode, knownSet, /*isNewnodeCCW=*/false, neisToForward);
            EV_DEBUG << "Vset full, can add newnode=" << newnode << " to vset, replaces farthest cw vnei: " << removedNeis[0] << endl;
        }
        else
            EV_DEBUG << "Vset full, should not add newnode=" << newnode << " to vset: " << printVsetToString() << endl;
    }
    return result;
}

void Vrr::initializeVlrOption(VlrIntOption& vlrOption, const VlrRingVID& dstVid/*=VLRRINGVID_NULL*/) const
{
    vlrOption.setDstVid(dstVid);                  // vidByteLength
    vlrOption.setTowardVid(VLRRINGVID_NULL);           // vidByteLength
    vlrOption.setCurrentPathid(VLRPATHID_INVALID);    // VLRPATHID_BYTELEN

    vlrOption.setTempTowardVid(VLRRINGVID_NULL);     // vidByteLength
    vlrOption.setTempPathid(VLRPATHID_INVALID);      // VLRPATHID_BYTELEN

    vlrOption.setPrevHopVid(VLRRINGVID_NULL);      // vidByteLength
}

unsigned int Vrr::setupPacketSrcVset(VlrIntSetupPacket *setupPacket, const std::set<VlrRingVID> *srcVsetPtr) const
{
    // std::vector<VlrRingVID> availablePendingVset;
    // for (auto& elem : pendingVset) {
    //     if (IsLinkedPneiOrAvailableRouteEnd(elem.first))
    //         availablePendingVset.push_back(elem.first);
    // }
    // setupPacket->setSrcVsetArraySize(vset.size() + availablePendingVset.size());
    unsigned int k = 0;
    if (srcVsetPtr) {
        const std::set<VlrRingVID>& srcVset = *srcVsetPtr;
        setupPacket->setSrcVsetArraySize(srcVset.size());
        for (const auto& vnei : srcVset)     // audo& will be const reference bc this function is const
            setupPacket->setSrcVset(k++, vnei);
    } else {    // srcVset not specified, use current vset
        setupPacket->setSrcVsetArraySize(vset.size());
        for (const auto& vnei : vset)     // audo& will be const reference bc this function is const
            setupPacket->setSrcVset(k++, vnei.first);
    }

    // for (auto& vnei : availablePendingVset)
    //     setupPacket->setSrcVset(k++, vnei);
    
    return k;
}

int Vrr::getVlrUniPacketByteLength() const
{
    // unsigned int messageId;  // for statistics only, size ignored
    // VlrIntOption vlrOption;
        // unsigned int dstVid;
        // unsigned int towardVid;
        // VlrPathID currentPathid;
        // unsigned int tempTowardVid;
        // VlrPathID tempPathid;
        // unsigned int prevHopVid;
    // return 4 * VLRRINGVID_BYTELEN + 2 * VLRPATHID_BYTELEN;
    // return 0;   // no vlrOpion for VRR packets
    return VLRRINGVID_BYTELEN;   // only account for prevHopVid
}

int Vrr::computeSetupReqByteLength(SetupReqInt* setupReq) const
{
    // unsigned int dst, newnode, proxy;
    int chunkByteLength = VLRRINGVID_BYTELEN * 3;
    // unsigned int srcVset[]
    chunkByteLength += setupReq->getSrcVsetArraySize() * VLRRINGVID_BYTELEN;
    // // std::set<unsigned int> knownSet
    // chunkByteLength += setupReq->getKnownSet().size() * VLRRINGVID_BYTELEN;
    // // bool reqDispatch, repairRoute, recordTrace, reqVnei;
    // // unsigned int indexInTrace;
    // chunkByteLength += 1 + 3;
    // // std::vector<unsigned int> traceVec
    // chunkByteLength += setupReq->getTraceVec().size() * VLRRINGVID_BYTELEN;

    // for VlrIntUniPacket
    chunkByteLength += getVlrUniPacketByteLength();

    return chunkByteLength;
}

SetupReqInt* Vrr::createSetupReq(const VlrRingVID& dst, const VlrRingVID& proxy)
{
    SetupReqInt *setupReq = new SetupReqInt(/*name=*/"SetupReq");
    setupReq->setNewnode(vid);      // vidByteLength
    setupReq->setDst(dst);
    setupReq->setProxy(proxy);
    // // setupReq->setRemovedNei(VLRRINGVID_NULL);
    // setupReq->setTransferNode(VLRRINGVID_NULL);
    // setupReq->setReqDispatch(reqDispatch);
    // setupReq->setRepairRoute(false);
    // setupReq->setRecordTrace(false);
    // setupReq->setIndexInTrace(0);
    // setupReq->setReqVnei(true);

    setupPacketSrcVset(setupReq);
    setupReq->setMessageId(++allSendMessageId);
    setupReq->setHopcount(0);         // number of nodes this message traversed, including the starting and ending nodes, will be incremented to 1 in sendCreatedSetupReq()

    initializeVlrOption(setupReq->getVlrOptionForUpdate());

    return setupReq;
}

// if computeChunkLength = true, compute chunk length because chunk was just created with createSetupReq() or modified after dupShared() from another chunk
// else, no need to compute chunk length because chunk was dupShared() (not modified) from another chunk that has chunkLength set
void Vrr::sendCreatedSetupReq(SetupReqInt *setupReq, const int& outGateIndex, bool computeChunkLength/*=true*/, double delay/*=0*/)
{
    if (computeChunkLength)
        setupReq->setByteLength(computeSetupReqByteLength(setupReq));
    EV_DEBUG << "Sending setupReq: dst = " << setupReq->getDst() << ", newnode = " << setupReq->getNewnode() << ", proxy = " << setupReq->getProxy() << endl;

    setupReq->getVlrOptionForUpdate().setPrevHopVid(vid);    // set packet prevHopVid to myself
    setupReq->setHopcount(setupReq->getHopcount() +1);    // increment packet hopcount
    
    // NOTE addTag should be executed after chunkLength has been set, and chunkLength shouldn't be changed before findTag/getTag

    // all multihop VLR packets (setupReq, setupReply, etc) L3 dst are set to a pnei, greedy routing at L3 in routeDatagram() isn't needed, but we do greedy routing and deal with VlrOption at L4 (in processSetupReq() for example) 
    // udpPacket->addTagIfAbsent<VlrIntOptionReq>()->setVlrOption(vlrOption);      // VlrOption to be set in IP header in datagramLocalOutHook()
    
    sendCreatedPacket(setupReq, /*unicast=*/true, /*outGateIndex=*/outGateIndex, /*delay=*/delay, /*checkFail=*/true);
}

// if this is the first setupReq to dst, or dst is my own vid, setupReqTimer should be nullptr, i.e. no need to increment retryCount
void Vrr::sendSetupReq(const VlrRingVID& dst, const VlrRingVID& proxy, WaitSetupReqIntTimer *setupReqTimer)
{
    auto setupReq = createSetupReq(dst, proxy);
    int proxyGateIndex = psetTable.getPneiGateIndex(proxy);
    setupReq->getVlrOptionForUpdate().setDstVid(dst);

    sendCreatedSetupReq(setupReq, proxyGateIndex);
    
    if (recordStatsToFile) { // record sent message
        // std::ostringstream s;
        // s << "vset=" << printVsetToString() << " pendingVset=" << printPendingVsetToString();
        recordMessageRecord(/*action=*/0, /*src=*/vid, /*dst=*/dst, "SetupReq", /*msgId=*/0, /*hopcount=*/0, /*chunkByteLength=*/0);   // /*infoStr=*/s.str().c_str() unused params (msgId, hopcount, chunkByteLength) are just assigned to 0
    }

    if (setupReqTimer) {    // if setupReqTimer != nullptr
        int retryCount = setupReqTimer->getRetryCount() + 1;
        setupReqTimer->setRetryCount(retryCount);
        // reschedule the wait setupReq timer w/o backoff
        // int backoffCount = (retryCount <= setupReqRetryLimit) ? retryCount : 1;     // total backoff time: [1, setupReqRetryLimit] * routeSetupReqWaitTime, if retryCount not <= setupReqRetryLimit, it's not considered a proper retry (retryCount is probably set manually), thus wait time is constant w/o backoff
        scheduleAt(simTime() + /*uniform(1, 1.1)*/ routeSetupReqWaitTime, setupReqTimer);
        EV_DETAIL << "Rescheduling setupReq timer: dst = " << setupReq->getDst() << ", retryCount = " << retryCount << endl;
    }
}

void Vrr::processSetupReq(SetupReqInt* reqIncoming, bool& pktForwarded)
{
    EV_DEBUG << "Received SetupReq" << endl;

    VlrRingVID newnode = reqIncoming->getNewnode();
    VlrRingVID dstVid = reqIncoming->getDst();        // can also get dst using vlrOptionIn->getDstVid()
    VlrRingVID proxy = reqIncoming->getProxy();
    VlrIntOption& vlrOptionIn = reqIncoming->getVlrOptionForUpdate();

    EV_INFO << "Processing SetupReq: dst = " << dstVid << ", newnode = " << newnode << ", proxy = " << proxy << endl;
    
    if (recordStatsToFile && recordReceivedMsg) {   // record received message
        recordMessageRecord(/*action=*/2, /*src=*/newnode, /*dst=*/dstVid, "SetupReq", /*msgId=*/reqIncoming->getMessageId(), /*hopcount=*/reqIncoming->getHopcount()+1, /*chunkByteLength=*/reqIncoming->getByteLength());   // unimportant params (msgId, hopcount)
    }

    VlrRingVID nextHopVid = VLRRINGVID_NULL;
    // bool reqForMe = false;
    if (dstVid != vid) {
        nextHopVid = findNextHop(vlrOptionIn, /*excludeVid=*/newnode);
        if (nextHopVid != VLRRINGVID_NULL) {
            // forward setupReq to nextHopVid
            if (checkUniPacketHopcountLimit && reqIncoming->getHopcount()+1 >= uniPacketHopcountLimit) {    // packet reached max hopcount
                if (recordStatsToFile && recordDroppedMsg) {   // record dropped message
                    recordMessageRecord(/*action=*/4, /*src=*/newnode, /*dst=*/dstVid, "SetupReq", /*msgId=*/reqIncoming->getMessageId(), /*hopcount=*/reqIncoming->getHopcount()+1, /*chunkByteLength=*/reqIncoming->getByteLength(), /*infoStr=*/"hopcount limit reached");   // unimportant params (msgId, hopcount)
                }
                return;
            } else {
                sendCreatedSetupReq(reqIncoming, /*outGateIndex=*/psetTable.getPneiGateIndex(nextHopVid), /*computeChunkLength=*/false);   // no need to recompute chunk length when forwarding
                pktForwarded = true;
            }
        }
    }
    if (nextHopVid == VLRRINGVID_NULL) {    // no next hop to forward setupReq, check if newnode should be added to my vset
        if (recordStatsToFile && recordDroppedMsg) {   // record message for me
            recordMessageRecord(/*action=*/1, /*src=*/newnode, /*dst=*/dstVid, "SetupReq", /*msgId=*/reqIncoming->getMessageId(), /*hopcount=*/reqIncoming->getHopcount()+1, /*chunkByteLength=*/reqIncoming->getByteLength());
        }
        // nextHopVid = VLRRINGVID_NULL here
        // get next hop to newnode via proxy
        VlrIntOption vlrOptionOut;
        initializeVlrOption(vlrOptionOut, /*dstVid=*/proxy);
        if (newnode != vid) {
            nextHopVid = findNextHopForSetupReply(vlrOptionOut, /*newnode=*/newnode);
        } else {    // newnode == vid
            EV_WARN << "SetupReq sent by newnode==me: " << vid << ", dst = " << dstVid << ", proxy = " << proxy << ", has been routed back to me, dropping SetupReq" << endl;
        }
        if (nextHopVid == VLRRINGVID_NULL) {
            EV_WARN << "No next hop found to send SetupReply/SetupFail at me = " << vid << ": newnode = " << newnode << ", original dst = " << dstVid << ", proxy = " << proxy << ", dropping SetupReq" << endl;
        } else {
            std::set<VlrRingVID> ovset = convertVsetToSet();
            bool newnodeInVset = (vset.find(newnode) != vset.end());    // if true, addedSrc will be true
            bool addedSrc = vrrAdd(reqIncoming, /*srcVid=*/newnode);

            if (addedSrc) {
            
                // send SetupReply to newnode, add newnode to vset and vlrRoutingTable
                bool addedRoute = false;
                VlrPathID newPathid;
                do {    // loop in case generated newPathid isn't unique in my vlrRoutingTable
                    newPathid = genPathID(newnode);
                    addedRoute = vlrRoutingTable.addRoute(newPathid, vid, newnode, /*prevhopVid=*/VLRRINGVID_NULL, /*nexthopVid=*/nextHopVid, /*isVsetRoute=*/true).second;
                } while (!addedRoute);
                
                SetupReplyInt* replyOutgoing = createSetupReply(newnode, proxy, newPathid, /*srcVsetPtr=*/&ovset);    // srcVset = oldVset (doesn't include newnode) in the created SetupReply
                replyOutgoing->setVlrOption(vlrOptionOut);
                EV_INFO << "Sending SetupReply to newnode = " << newnode << ", src = " << vid << ", pathid = " << newPathid << ", proxy = " << replyOutgoing->getProxy() << ", nexthop: " << nextHopVid << endl;
                sendCreatedSetupReply(replyOutgoing, /*outGateIndex=*/psetTable.getPneiGateIndex(nextHopVid));

                if (recordStatsToFile) {   // record sent message
                    recordMessageRecord(/*action=*/0, /*src=*/vid, /*dst=*/newnode, "SetupReply", /*msgId=*/0, /*hopcount=*/0, /*chunkByteLength=*/0);   // unused params (msgId, hopcount, chunkByteLength) are just assigned to 0
                }

                // vsetInsertRmPending(newnode) done in vrrAdd()
                vset.at(newnode).insert(newPathid);

                // set inNetwork = true bc I added newnode in vset, vset isn't empty
                if (!newnodeInVset)     // if newnode wasn't in vset
                    recordNewVnei(newnode);
                
            } else {    // didn't add newnode, send setupFail
                // EV_WARN << "Didn't add newnode to vset = " << printVsetToString() << ", sending SetupFail to newnode = " << newnode << ", proxy = " << proxy << ", nexthop: " << nextHopVid << endl;
                auto replyOutgoing = createSetupFail(newnode, proxy);    // srcVset = vset, newnode isn't added to my vset
                replyOutgoing->setVlrOption(vlrOptionOut);
                EV_INFO << "Sending SetupFail to newnode = " << newnode << ", proxy = " << replyOutgoing->getProxy() << ", nexthop: " << nextHopVid << endl;
                sendCreatedSetupFail(replyOutgoing, /*outGateIndex=*/psetTable.getPneiGateIndex(nextHopVid), /*computeChunkLength=*/true);

                if (recordStatsToFile) {   // record sent message
                    // std::ostringstream s;
                    // s << "vset=" << printVsetToString() << " pendingVset=" << printPendingVsetToString();
                    recordMessageRecord(/*action=*/0, /*src=*/vid, /*dst=*/newnode, "SetupFail", /*msgId=*/0, /*hopcount=*/0, /*chunkByteLength=*/0);   // /*infoStr=*/s.str().c_str() unused params (msgId, hopcount, chunkByteLength) are just assigned to 0
                }
            }
        } 
    }
}

// if setupPacket != nullptr, if any nodes in packet's srcVset should be added to vset, send setupReq to it
// if srcVid != VLRRINGVID_NULL, return if it should be added to vset, if should, add it and tear down vset-routes to removed vneis
bool Vrr::vrrAdd(const VlrIntSetupPacket *setupPacket, const VlrRingVID& srcVid)
{
    if (setupPacket != nullptr) {
        int oVsetSize = setupPacket->getSrcVsetArraySize();
        for (int k = 0; k < oVsetSize; ++k) {
            VlrRingVID node = setupPacket->getSrcVset(k);
            auto addResult = shouldAddVnei(node);
            bool& shouldAdd = std::get<0>(addResult);  // access first element in tuple
            if (shouldAdd)
                pendingVsetAdd(node, /*numTrials=*/1);
        }
    }
    bool addedSrc = false;
    if (srcVid != VLRRINGVID_NULL) {
        if (vset.find(srcVid) == vset.end()) {      // srcVid not already in vset
            auto addResult = shouldAddVnei(srcVid);
            bool& shouldAdd = std::get<0>(addResult);  // access first element in tuple
            if (shouldAdd) {
                addedSrc = true;
                vsetInsertRmPending(srcVid);       // srcVid will be removed from pendingVset here!

                std::vector<VlrRingVID>& removedNeis = std::get<1>(addResult);
                // remove vneis that no longer belong to my vset bc newnode joining
                for (const VlrRingVID& oldNei : removedNeis) {
                    // delayRouteTeardown() must be called before oldNei is removed from vset
                    // get vset-routes btw me and oldVnei, add to nonEssRoutes to be torn down later
                    for (const VlrPathID& oldPathid : vset.at(oldNei)) {
                        vlrRoutingTable.vlrRoutesMap.at(oldPathid).isVsetRoute = false;
                        // put in nonEssRoutes to be torn down right after
                        nonEssRoutes.insert(oldPathid);
                        if (!nonEssRouteTeardownTimer->isScheduled())
                            scheduleAt(simTime() + nonEssRouteExpiration, nonEssRouteTeardownTimer);
                    }
                    vsetEraseAddPending(oldNei, /*addToPending=*/false);
                }
            }
        }
    }
    return addedSrc;
}

// set selfInNetwork=true and recordNodeStatsRecord() after adding a new vnei
void Vrr::recordNewVnei(const VlrRingVID& newVnei)
{
    // set inNetwork = true bc I added newnode in vset, vset isn't empty
    if (!selfInNetwork) {
        // if (inNetworkWarmupTimer->isScheduled()) {
        //     cancelEvent(inNetworkWarmupTimer);
        //     EV_DETAIL << "Canceling inNetworkWarmupTimer at me=" << vid << endl;
        // }
        // selfInNetwork = true;
        // lastTimeNodeJoinedInNetwork = simTime().dbl();

        if (!inNetworkWarmupTimer->isScheduled()) {
            scheduleAt(simTime() + inNetworkWarmupTime, inNetworkWarmupTimer);
            EV_DETAIL << "inNetwork scheduled to become true at " << inNetworkWarmupTimer->getArrivalTime() << endl;
        }
        else if (vset.size() >= 2 * vsetHalfCardinality) {  // vset is full
            cancelEvent(inNetworkWarmupTimer);
            selfInNetwork = true;

            lastTimeNodeJoinedInNetwork = simTime().dbl();
        }
        else if (vset.size() == 1) { // vset was empty but inNetworkWarmupTimer was scheduled, this means I was a rep that lost all my vneis, now that I've joined an existing overlay, cancel the original inNetworkWarmupTimer to schedule a new one to wait for other vneis
            cancelEvent(inNetworkWarmupTimer);
            scheduleAt(simTime() + inNetworkWarmupTime, inNetworkWarmupTimer);
            EV_DETAIL << "inNetwork re-scheduled to become true at " << inNetworkWarmupTimer->getArrivalTime() << endl;
        }
    }
    
    EV_INFO << "Adding newnode = " << newVnei << " to vset = " << printVsetToString() << ", inNetwork = " << selfInNetwork << endl;

    if (recordStatsToFile) { // write node status update
        if (vset.size() >= 2 * vsetHalfCardinality) {  // vset is full
            std::ostringstream s;
            s << "vsetFull: inNetwork=" << selfInNetwork << " newVnei=" << newVnei << " vset=" << printVsetToString();
            recordNodeStatsRecord(/*infoStr=*/s.str().c_str());   // unused params (stage)
            if (convertVsetToSet() == vidRingRegVset) {
                nodesVsetCorrect.insert(vid);

                std::ostringstream s;
                s << "vsetCorrect: inNetwork=" << selfInNetwork << " newVnei=" << newVnei << " vset=" << printVsetToString() << " numNodesVsetCorrect=" << nodesVsetCorrect.size();
                recordNodeStatsRecord(/*infoStr=*/s.str().c_str());                
            }
        }
    }
}


int Vrr::computeSetupReplyByteLength(SetupReplyInt* msg) const
{
    // unsigned int proxy, newnode, src;
    int chunkByteLength = VLRRINGVID_BYTELEN * 3;
    // VlrPathID pathid
    chunkByteLength += VLRPATHID_BYTELEN;
    // unsigned int srcVset[]
    chunkByteLength += msg->getSrcVsetArraySize() * VLRRINGVID_BYTELEN;
    // // std::vector<unsigned int> trace
    // chunkByteLength += msg->getTrace().size() * VLRRINGVID_BYTELEN;
    // std::vector<unsigned int> prevhopVids
    // unsigned int oldestPrevhopIndex
    chunkByteLength += (routePrevhopVidsSize-1) * VLRRINGVID_BYTELEN + 2;   // routePrevhopVidsSize-1 bc size of prevHopVid included in vlrOption
    // // bool reqVnei
    // chunkByteLength += 1;

    // for VlrIntUniPacket
    chunkByteLength += getVlrUniPacketByteLength();

    return chunkByteLength;
}

SetupReplyInt* Vrr::createSetupReply(const VlrRingVID& newnode, const VlrRingVID& proxy, const VlrPathID& pathid, const std::set<VlrRingVID> *srcVsetPtr/*=nullptr*/)
{
    SetupReplyInt *msg = new SetupReplyInt(/*name=*/"SetupReply");
    msg->setNewnode(newnode);      // vidByteLength
    msg->setProxy(proxy);
    msg->setSrc(vid);
    // setupReply->setReqVnei(true);

    msg->getPathidForUpdate() = pathid;

    msg->getPrevhopVidsForUpdate().push_back(vid);
    msg->setOldestPrevhopIndex(0);

    msg->setHopcount(0);         // number of nodes this message traversed, including the starting and ending nodes

    setupPacketSrcVset(msg, srcVsetPtr);
    msg->setMessageId(++allSendMessageId);

    initializeVlrOption(msg->getVlrOptionForUpdate());

    return msg;
}

// if computeChunkLength = true, compute chunk length because chunk was just created with createSetupReply() or modified after dupShared() from another chunk
// else, no need to compute chunk length because chunk was dupShared() (not modified) from another chunk that has chunkLength set
void Vrr::sendCreatedSetupReply(SetupReplyInt *setupReply, const int& outGateIndex, bool computeChunkLength/*=true*/, double delay/*=0*/)
{
    if (computeChunkLength)
        setupReply->setByteLength(computeSetupReplyByteLength(setupReply));
    EV_DEBUG << "Sending setupReply: src = " << setupReply->getSrc() << ", newnode = " << setupReply->getNewnode() << ", proxy = " << setupReply->getProxy() << endl;

    setupReply->getVlrOptionForUpdate().setPrevHopVid(vid);    // set packet prevHopVid to myself
    setupReply->setHopcount(setupReply->getHopcount() +1);    // increment packet hopcount
    
    // NOTE addTag should be executed after chunkLength has been set, and chunkLength shouldn't be changed before findTag/getTag

    // all multihop VLR packets (setupReq, setupReply, etc) L3 dst are set to a pnei, greedy routing at L3 in routeDatagram() isn't needed, but we do greedy routing and deal with VlrOption at L4 (in processSetupReq() for example) 
    // udpPacket->addTagIfAbsent<VlrIntOptionReq>()->setVlrOption(vlrOption);      // VlrOption to be set in IP header in datagramLocalOutHook()
    
    const VlrPathID& newPathid = setupReply->getPathid();

    sendCreatedPacket(setupReply, /*unicast=*/true, /*outGateIndex=*/outGateIndex, /*delay=*/delay, /*checkFail=*/true);
}

void Vrr::processSetupReply(SetupReplyInt *replyIncoming, bool& pktForwarded)
{
    EV_DEBUG << "Received SetupReply" << endl;

    VlrRingVID msgPrevHopVid = replyIncoming->getVlrOption().getPrevHopVid();
    if (msgPrevHopVid == VLRRINGVID_NULL)
        throw cRuntimeError("Received SetupReply with vlrOption.prevHopVid = null");

    VlrRingVID newnode = replyIncoming->getNewnode();
    VlrRingVID srcVid = replyIncoming->getSrc();        
    VlrRingVID proxy = replyIncoming->getProxy();       // can also get proxy using vlrOptionIn->getDstVid()
    const VlrPathID& newPathid = replyIncoming->getPathid();
    unsigned int msgHopCount = replyIncoming->getHopcount() +1;

    EV_INFO << "Processing SetupReply: proxy = " << proxy << ", newnode = " << newnode << ", src = " << srcVid << ", pathid: " << newPathid << ", hopcount: " << msgHopCount << endl;

    if (recordStatsToFile && recordReceivedMsg) {   // record received message
        recordMessageRecord(/*action=*/2, /*src=*/srcVid, /*dst=*/newnode, "SetupReply", /*msgId=*/replyIncoming->getMessageId(), /*hopcount=*/msgHopCount, /*chunkByteLength=*/replyIncoming->getByteLength());    // unimportant params (msgId)
    }
    bool pktForMe = false;      // set to true if this msg is directed to me or I processed it as its dst
    bool pktRecorded = false;      // set to true if this msg is recorded with recordMessageRecord()

    if ((newPathid == 625507881 && srcVid == 32950 && newnode == 46281))       // && (vid == 1092 || vid == 3058)           node6145
        EV_WARN << "ohno" << endl;
    else if ((newPathid == 3453525040 && srcVid == 21097 && newnode == 38175))       // && (vid == 1092 || vid == 3058)     node51026
        EV_WARN << "ohno" << endl;
    else if ((newPathid == 206995616 && srcVid == 13667 && newnode == 21547))       // && (vid == 1092 || vid == 3058)      node376
        EV_WARN << "ohno" << endl;
    else if ((newPathid == 2839240247 && srcVid == 28876 && newnode == 19549))       // && (vid == 1092 || vid == 3058)     node27595
        EV_WARN << "ohno" << endl;
    else if ((newPathid == 4011204785 && srcVid == 15982 && newnode == 18359))       // && (vid == 1092 || vid == 3058)     node30044
        EV_WARN << "ohno" << endl;
    else if ((newPathid == 698354117 && srcVid == 37504 && newnode == 31987))       // && (vid == 1092 || vid == 3058)     node11285
        EV_WARN << "ohno" << endl;

    auto vrouteItr = vlrRoutingTable.vlrRoutesMap.find(newPathid);
    if (vrouteItr != vlrRoutingTable.vlrRoutesMap.end()) {  // newPathid already in vlrRoutingTable
        EV_WARN << "Pathid of setupReply(newnode=" << newnode << ", src=" << srcVid << ", proxy=" << proxy << ") already in my vlrRoutingTable: " << vrouteItr->second << ", me=" << vid << ", vset = " << printVsetToString() << ", tearing down pathid " << newPathid << endl;
        vrrTeardownPath(newPathid, msgPrevHopVid, /*addSrcVset=*/false, /*checkMeEndpoint=*/true, /*infoStr=*/"processSetupReply: newPathid already in vlrRoutingTable");
    } else if (!psetTable.pneiIsLinked(msgPrevHopVid)) {        // if prevHopAddr isn't a LINKED pnei
        EV_WARN << "Previous hop " << msgPrevHopVid << " of setupReply(newnode=" << newnode << ", src=" << srcVid << ", proxy=" << proxy << ") is not a LINKED pnei, tearing down pathid " << newPathid << endl;
        vrrTeardownPath(newPathid, msgPrevHopVid, /*addSrcVset=*/false, /*checkMeEndpoint=*/false, /*infoStr=*/"processSetupReply: previous hop is not a LINKED pnei");   // not removing an existing vroute in vlrRoutingTable
    } else if (newnode == vid) {      // this SetupReply is destined for me
        pktForMe = true;
        bool srcVidInVset = (vset.find(srcVid) != vset.end());    // if true, addedSrc will be true
        bool addedSrc = vrrAdd(replyIncoming, /*srcVid=*/srcVid);
        if (addedSrc) {
            if (recordStatsToFile) {   // record received setupReply for me that indeed adds newPathid to vlrRoutingTable
                recordMessageRecord(/*action=*/1, /*src=*/srcVid, /*dst=*/vid, "SetupReply", /*msgId=*/newPathid, /*hopcount=*/msgHopCount, /*chunkByteLength=*/replyIncoming->getByteLength());    // unimportant params (msgId)
                pktRecorded = true;
            }
            // add srcVid to vset and vlrRoutingTable
            auto itr_bool = vlrRoutingTable.addRoute(newPathid, srcVid, newnode, /*prevhopVid=*/msgPrevHopVid, /*nexthopVid=*/VLRRINGVID_NULL, /*isVsetRoute=*/true);
            itr_bool.first->second.hopcount = msgHopCount;
            std::vector<VlrRingVID>& routePrevhopVids = itr_bool.first->second.prevhopVids;
            setRoutePrevhopVids(routePrevhopVids, replyIncoming->getPrevhopVids(), replyIncoming->getOldestPrevhopIndex());
    
            // vsetInsertRmPending(srcVid) done in vrrAdd()
            vset.at(srcVid).insert(newPathid);

            // set inNetwork = true bc I added srcVid in vset, vset isn't empty
            if (!srcVidInVset)     // if srcVid wasn't in vset
                recordNewVnei(srcVid);

        } else {    // didn't add srcVid to vset, send Teardown for newPathid
            EV_WARN << "SrcVid = " << srcVid << " of setupReply(newnode=" << newnode << ", src=" << srcVid << ", proxy=" << proxy << ") should not be added to my vset, tearing down the new pathid " << newPathid << endl;
            vrrTeardownPath(newPathid, msgPrevHopVid, /*addSrcVset=*/true, /*checkMeEndpoint=*/false, /*infoStr=*/"processSetupReply: srcVid shouldn't be added to vset");   // not removing an existing vroute in vlrRoutingTable
        }
    }
    else {    // this SetupReply isn't destined for me
        if (checkUniPacketHopcountLimit && replyIncoming->getHopcount()+1 >= uniPacketHopcountLimit) {    // packet reached max hopcount
            if (recordStatsToFile && recordDroppedMsg) {   // record dropped message
                recordMessageRecord(/*action=*/4, /*src=*/srcVid, /*dst=*/newnode, "SetupReply", /*msgId=*/replyIncoming->getMessageId(), /*hopcount=*/replyIncoming->getHopcount()+1, /*chunkByteLength=*/replyIncoming->getByteLength(), /*infoStr=*/"hopcount limit reached");   // unimportant params (msgId, hopcount)
                pktRecorded = true;
            }
            vrrTeardownPath(newPathid, msgPrevHopVid, /*addSrcVset=*/false, /*checkMeEndpoint=*/false, /*infoStr=*/"processSetupReply: hopcount limit reached");   // not removing an existing vroute in vlrRoutingTable
            return;
        } else {
            VlrIntOption& vlrOptionIn = replyIncoming->getVlrOptionForUpdate();
            VlrRingVID nextHopVid = findNextHopForSetupReply(vlrOptionIn, /*newnode=*/newnode);
            if (nextHopVid == VLRRINGVID_NULL) {
                EV_WARN << "No next hop found for SetupReply(newnode=" << newnode << ", src=" << srcVid << ", proxy=" << proxy << ") received at me = " << vid << ", tearing down vroute: pathid = " << newPathid << ", newnode = " << newnode << ", src = " << srcVid << ", proxy = " << proxy << endl;
                vrrTeardownPath(newPathid, msgPrevHopVid, /*addSrcVset=*/true, /*checkMeEndpoint=*/false, /*infoStr=*/"processSetupReply: no next hop found");   // not removing an existing vroute in vlrRoutingTable
            } else {    // nexthop found for SetupReply
                // we've checked newPathid not in vlrRoutingTable
                auto itr_bool = vlrRoutingTable.addRoute(newPathid, srcVid, newnode, /*prevhopVid=*/msgPrevHopVid, /*nexthopVid=*/nextHopVid, /*isVsetRoute=*/false);
                std::vector<VlrRingVID>& routePrevhopVids = itr_bool.first->second.prevhopVids;
                unsigned int oldestPrevhopIndex = setRoutePrevhopVidsFromMessage(routePrevhopVids, replyIncoming->getPrevhopVidsForUpdate(), replyIncoming->getOldestPrevhopIndex());
                replyIncoming->setOldestPrevhopIndex(oldestPrevhopIndex);

                // if (msgPrevHopVid == nextHopVid)
                //     EV_ERROR << "ohno src=" << srcVid << " dst=" << newnode << " newPathid=" << newPathid << " msgPrevHopVid=nextHopVid=" << nextHopVid << " at me=" << vid << " hopcount=" << msgHopCount << endl;

                EV_INFO << "Added new vroute: newPathid = " << newPathid << " " << vlrRoutingTable.vlrRoutesMap.at(newPathid) << endl;
                // replyOutgoing->setHopcount(msgHopCount);     // Commented out bc will increment in sendCreatedSetupReply()
                sendCreatedSetupReply(replyIncoming, /*outGateIndex=*/psetTable.getPneiGateIndex(nextHopVid), /*computeChunkLength=*/false);    // no need to recompute chunk length when forwarding
                pktForwarded = true;
            }
        }
    }

    if (recordStatsToFile && recordDroppedMsg && !pktRecorded) {   // record message
        if (pktForMe)
            recordMessageRecord(/*action=*/1, /*src=*/srcVid, /*dst=*/newnode, "SetupReply", /*msgId=*/replyIncoming->getMessageId(), /*hopcount=*/replyIncoming->getHopcount()+1, /*chunkByteLength=*/replyIncoming->getByteLength());
        else if (!pktForwarded)
            recordMessageRecord(/*action=*/4, /*src=*/srcVid, /*dst=*/newnode, "SetupReply", /*msgId=*/replyIncoming->getMessageId(), /*hopcount=*/replyIncoming->getHopcount()+1, /*chunkByteLength=*/replyIncoming->getByteLength());
    }
}

// set routePrevhops: [closest prevhop, .., farthest prevhop]
void Vrr::setRoutePrevhopVids(VlrIntVidVec& routePrevhops, const VlrIntVidVec& msgPrevhops, const unsigned int& oldestPrevhopIndex)
{
    unsigned int i = oldestPrevhopIndex;
    int prevhopsSize = msgPrevhops.size();
    // set routePrevhops from msgPrevhops
    for (int k = 0; k < prevhopsSize; k++) {
        advanceVectorIndexWrapAround(i, -1, /*vecSize=*/prevhopsSize);
        routePrevhops.push_back(msgPrevhops[i]);
    }
}

// routePrevhops: prevhopVids of newly added route in vlrRoutingTable, msgPrevhops: prevhopVids in received SetupReply/AddRoute, oldestPrevhopIndex: index of oldest prevhopVid in msgPrevhops
// set routePrevhops based on msgPrevhops, then add myself to msgPrevhops, return oldestPrevhopIndex in modified msgPrevhops
unsigned int Vrr::setRoutePrevhopVidsFromMessage(VlrIntVidVec& routePrevhops, VlrIntVidVec& msgPrevhops, const unsigned int& oldestPrevhopIndex)
{
    setRoutePrevhopVids(routePrevhops, msgPrevhops, oldestPrevhopIndex);
    unsigned int i = oldestPrevhopIndex;
    int prevhopsSize = msgPrevhops.size();
    // add myself to msgPrevhops
    if (prevhopsSize < routePrevhopVidsSize) {
        msgPrevhops.push_back(vid);
        i = 0;
    } else {    // overwrite oldest prevhopVid in msgPrevhops
        msgPrevhops[i] = vid;
        advanceVectorIndexWrapAround(i, 1, /*vecSize=*/prevhopsSize);
    }
    return i;
}

int Vrr::computeSetupFailByteLength(SetupFailInt* msg) const
{
    // unsigned int src, newnode, proxy;
    int chunkByteLength = VLRRINGVID_BYTELEN * 3;
    // unsigned int srcVset[]
    chunkByteLength += msg->getSrcVsetArraySize() * VLRRINGVID_BYTELEN;
    // // std::vector<unsigned int> trace
    // chunkByteLength += msg->getTrace().size() * VLRRINGVID_BYTELEN;

    // for VlrIntUniPacket
    chunkByteLength += getVlrUniPacketByteLength();

    return chunkByteLength;
}

SetupFailInt* Vrr::createSetupFail(const VlrRingVID& newnode, const VlrRingVID& proxy)
{
    SetupFailInt *msg = new SetupFailInt(/*name=*/"SetupFail");
    msg->setNewnode(newnode);      // vidByteLength
    msg->setProxy(proxy);
    msg->setSrc(vid);

    setupPacketSrcVset(msg);
    msg->setMessageId(++allSendMessageId);
    msg->setHopcount(0);         // number of nodes this message traversed, including the starting and ending nodes

    initializeVlrOption(msg->getVlrOptionForUpdate());

    return msg;
}

// if computeChunkLength = true, compute chunk length because chunk was just created with createSetupFail() or modified after dupShared() from another chunk
// else, no need to compute chunk length because chunk was dupShared() (not modified) from another chunk that has chunkLength set
void Vrr::sendCreatedSetupFail(SetupFailInt *msg, const int& outGateIndex, bool computeChunkLength/*=true*/, double delay/*=0*/)
{
    if (computeChunkLength)
        msg->setByteLength(computeSetupFailByteLength(msg));
    EV_DEBUG << "Sending setupFail: src = " << msg->getSrc() << ", newnode = " << msg->getNewnode() << ", proxy = " << msg->getProxy() << endl;

    msg->getVlrOptionForUpdate().setPrevHopVid(vid);    // set packet prevHopVid to myself
    msg->setHopcount(msg->getHopcount() +1);    // increment packet hopcount
    
    // NOTE addTag should be executed after chunkLength has been set, and chunkLength shouldn't be changed before findTag/getTag

    // all multihop VLR packets (setupReq, setupReply, etc) L3 dst are set to a pnei, greedy routing at L3 in routeDatagram() isn't needed, but we do greedy routing and deal with VlrOption at L4 (in processSetupReq() for example) 
    // udpPacket->addTagIfAbsent<VlrIntOptionReq>()->setVlrOption(vlrOption);      // VlrOption to be set in IP header in datagramLocalOutHook()
    
    sendCreatedPacket(msg, /*unicast=*/true, /*outGateIndex=*/outGateIndex, /*delay=*/delay, /*checkFail=*/true);
}

void Vrr::processSetupFail(SetupFailInt *msgIncoming, bool& pktForwarded)
{
    EV_DEBUG << "Received SetupFail" << endl;

    VlrRingVID msgPrevHopVid = msgIncoming->getVlrOption().getPrevHopVid();
    if (msgPrevHopVid == VLRRINGVID_NULL)
        throw cRuntimeError("Received SetupFail with vlrOption.prevHopVid = null");
    
    VlrRingVID newnode = msgIncoming->getNewnode();
    VlrRingVID srcVid = msgIncoming->getSrc();        
    VlrRingVID proxy = msgIncoming->getProxy();       // can also get proxy using vlrOptionIn->getDstVid()
    EV_INFO << "Processing SetupFail: proxy = " << proxy << ", newnode = " << newnode << ", src = " << srcVid << ", prevhop: " << msgPrevHopVid << endl;

    if (recordStatsToFile && recordReceivedMsg) {   // record received message
        recordMessageRecord(/*action=*/2, /*src=*/srcVid, /*dst=*/newnode, "SetupFail", /*msgId=*/msgIncoming->getMessageId(), /*hopcount=*/msgIncoming->getHopcount()+1, /*chunkByteLength=*/msgIncoming->getByteLength());  // unimportant params (msgId, hopcount)
    }
    bool pktForMe = false;      // set to true if this msg is directed to me or I processed it as its dst
    bool pktRecorded = false;      // set to true if this msg is recorded with recordMessageRecord()

    if (newnode == vid) {
        pktForMe = true;
        EV_INFO << "Handling setupFail to me (processing oVset): src = " << srcVid << ", newnode = " << newnode << ", proxy = " << proxy << ", srcVset = [";
        for (size_t i = 0; i < msgIncoming->getSrcVsetArraySize(); i++)
            EV_INFO << msgIncoming->getSrcVset(i) << " ";
        EV_INFO << "]" << endl;

        // delete WaitSetupReqIntTimer for srcVid in pendingVset (received this setupFail bc I've sent setupReq to srcVid, so srcVid (and its WaitSetupReqIntTimer) should be in my pendingVset, unless it has been added to vset or timed out)
        // pendingVsetErase(srcVid);    // this setupReqTimer will be cancelAndDelete()
        // NOTE not erasing srcVid from pendingVset directly bc it will be added back to pendingVset and setupReq will be sent again right away if vset still not full
        auto itr = pendingVset.find(srcVid);
        if (itr != pendingVset.end() && itr->second) { // if node exists in pendingVset and WaitSetupReqIntTimer != nullptr
            itr->second->setRetryCount(setupReqRetryLimit);     // don't send setupReq any more if vset is full
        }
        // process srcVset, also add srcVid to pendingVset if relevant (though it just refused me as vnei)
        msgIncoming->appendSrcVset(srcVid);
        vrrAdd(msgIncoming, /*srcVid=*/VLRRINGVID_NULL);        // process srcVset
    }
    else {  // this SetupFail isn't destined for me
        VlrIntOption& vlrOptionIn = msgIncoming->getVlrOptionForUpdate();
        VlrRingVID nextHopVid = findNextHopForSetupReply(vlrOptionIn, /*newnode=*/newnode);
        if (nextHopVid != VLRRINGVID_NULL) {
            // forward setupFail to nextHopVid
            if (checkUniPacketHopcountLimit && msgIncoming->getHopcount()+1 >= uniPacketHopcountLimit) {    // packet reached max hopcount
                if (recordStatsToFile && recordDroppedMsg) {   // record dropped message
                    recordMessageRecord(/*action=*/4, /*src=*/srcVid, /*dst=*/newnode, "SetupFail", /*msgId=*/msgIncoming->getMessageId(), /*hopcount=*/msgIncoming->getHopcount()+1, /*chunkByteLength=*/msgIncoming->getByteLength(), /*infoStr=*/"hopcount limit reached");   // unimportant params (msgId, hopcount)
                    pktRecorded = true;
                }
                return;
            } else {
                sendCreatedSetupFail(msgIncoming, /*outGateIndex=*/psetTable.getPneiGateIndex(nextHopVid), /*computeChunkLength=*/false);   // no need to recompute chunk length when forwarding
                pktForwarded = true;
            }
        }
    }

    if (recordStatsToFile && recordDroppedMsg && !pktRecorded) {   // record message
        if (pktForMe)
            recordMessageRecord(/*action=*/1, /*src=*/srcVid, /*dst=*/newnode, "SetupFail", /*msgId=*/msgIncoming->getMessageId(), /*hopcount=*/msgIncoming->getHopcount()+1, /*chunkByteLength=*/msgIncoming->getByteLength());
        else if (!pktForwarded)
            recordMessageRecord(/*action=*/4, /*src=*/srcVid, /*dst=*/newnode, "SetupFail", /*msgId=*/msgIncoming->getMessageId(), /*hopcount=*/msgIncoming->getHopcount()+1, /*chunkByteLength=*/msgIncoming->getByteLength());
    }
}

// create TeardownInt with one pathid and set its chunk length
TeardownInt* Vrr::createTeardown(const VlrPathID& pathid, bool addSrcVset)
{
    TeardownInt *msg = new TeardownInt(/*name=*/"Teardown");
    msg->setSrc(vid);          // for statistics
    // msg->setRebuild(rebuild);
    // msg->setDismantled(dismantled);
    msg->setPathidsArraySize(1);
    unsigned int k = 0;
    // for (const auto& pathid : pathids)
    msg->setPathids(k++, pathid);      // VLRPATHID_BYTELEN
    // VlrPathID pathids[]
    // // unsigned int src
    // // bool rebuild;
    // // bool dismantled;
    int chunkByteLength = k * VLRPATHID_BYTELEN;

    if (addSrcVset) {
        k = setupPacketSrcVset(msg);    // number of nodes added to srcVset
        
        chunkByteLength += k * VLRRINGVID_BYTELEN;
    }
    initializeVlrOption(msg->getVlrOptionForUpdate());

    msg->setMessageId(++allSendMessageId);
    msg->setHopcount(0);

    // for VlrIntUniPacket
    chunkByteLength += getVlrUniPacketByteLength();

    msg->setByteLength(chunkByteLength);
    return msg;
}

void Vrr::sendCreatedTeardown(TeardownInt *msg, VlrRingVID nextHopPnei, double delay/*=0*/)
{
    msg->getVlrOptionForUpdate().setPrevHopVid(vid);    // set packet prevHopVid to myself
    msg->setHopcount(msg->getHopcount() +1);    // increment packet hopcount

    const VlrPathID& oldPathid = msg->getPathids(0);
    // if ((oldPathid == 3882923163) && simTime() > 500)
    //     EV_WARN << "ohno" << endl;

    // Teardown may be sent to msgPrevHopVid which may not be a pnei in my pset
    int nextHopGateIndex = psetTable.findPneiGateIndex(nextHopPnei);
    if (nextHopGateIndex == -1) {
        nextHopGateIndex = psetTable.findRecvNeiGateIndex(nextHopPnei);
        if (nextHopGateIndex == -1)
            throw cRuntimeError("sendCreatedTeardown(nextHopPnei=%d) at me=%d, cannot find gateIndex associated with nextHopPnei in psetTable.vidToStateMap or psetTable.recvVidToGateIndexMap", nextHopPnei, vid);
    }
    sendCreatedPacket(msg, /*unicast=*/true, /*outGateIndex=*/nextHopGateIndex, /*delay=*/delay, /*checkFail=*/true);
}

void Vrr::processTeardown(TeardownInt* msgIncoming, bool& pktForwarded)
{
    EV_DEBUG << "Received Teardown" << endl;

    VlrIntOption& vlrOptionIn = msgIncoming->getVlrOptionForUpdate();
    VlrRingVID msgPrevHopVid = vlrOptionIn.getPrevHopVid();
    if (msgPrevHopVid == VLRRINGVID_NULL)
        throw cRuntimeError("Received Teardown with vlrOption.prevHopVid = null");

    size_t numOfPathids = msgIncoming->getPathidsArraySize();
    EV_INFO << "Processing Teardown: src = " << msgIncoming->getSrc() << ", rebuild = " << msgIncoming->getRebuild() << ", dismantled = " << msgIncoming->getDismantled() << ", pathids = [";
    for (size_t i = 0; i < numOfPathids; i++)
        EV_INFO << msgIncoming->getPathids(i) << " ";
    EV_INFO << "]" << ", prevhop: " << msgPrevHopVid << endl;

    if (recordStatsToFile && recordReceivedMsg) {   // record received message
        recordMessageRecord(/*action=*/2, /*src=*/msgIncoming->getSrc(), /*dst=*/vid, "Teardown", /*msgId=*/msgIncoming->getPathids(0), /*hopcount=*/msgIncoming->getHopcount()+1, /*chunkByteLength=*/msgIncoming->getByteLength());  // unimportant params (msgId, hopcount)
    }
    bool pktForMe = false;      // set to true if this msg is directed to me or I processed it as its dst
    bool pktRecorded = false;      // set to true if this msg is recorded with recordMessageRecord()

    // for (size_t k = 0; k < numOfPathids; ++k) {
    const VlrPathID& oldPathid = msgIncoming->getPathids(0);

    auto vrouteItr = vlrRoutingTable.vlrRoutesMap.find(oldPathid);
    if (vrouteItr == vlrRoutingTable.vlrRoutesMap.end()) {  // oldPathid not found in vlrRoutingTable
        EV_WARN << "The pathid " << oldPathid << " of Teardown is not in my routing table, dropping the Teardown message" << endl;
    }
    else {  // checked oldPathid is in vlrRoutingTable
        // VlrRingRoutingTable::VlrRingRoute vroute = vrouteItr->second;   // copy vroute info before deleting it from routing table
        if (msgPrevHopVid != vrouteItr->second.prevhopVid && msgPrevHopVid != vrouteItr->second.nexthopVid) {
            EV_WARN << "Sender " << msgPrevHopVid << " of Teardown is neither prevhop nor nexthop of pathid " << oldPathid << " to be torn down, dropping the Teardown message w/o deleting vroute from routing table" << endl;
        } else {    // sender of this teardown is either prevhopAddr or nexthopAddr of the vroute
            bool isToNexthop = (msgPrevHopVid == vrouteItr->second.prevhopVid);
            VlrRingVID nextHopVid = (isToNexthop) ? vrouteItr->second.nexthopVid : vrouteItr->second.prevhopVid;
            char nextHopIsUnavailable = vlrRoutingTable.getPrevNextIsUnavailable(vrouteItr->second.isUnavailable, /*getPrev=*/!isToNexthop);     // if isToNexthop, next hop is nexthop, getPrev=false
            // bool isTemporaryRoute = vlrRoutingTable.getIsTemporaryRoute(vrouteItr->second.isUnavailable);
            // bool isDismantledRoute = vlrRoutingTable.getIsDismantledRoute(vrouteItr->second.isUnavailable);
            EV_INFO << "The pathid " << oldPathid << " of Teardown is found in routing table, nextHopAddr: " << nextHopVid << " (specified=" << (nextHopVid!=VLRRINGVID_NULL) << endl;

            if (nextHopVid == VLRRINGVID_NULL) {  // I'm an endpoint of the vroute
                pktForMe = true;
                const VlrRingVID& otherEnd = (isToNexthop) ? vrouteItr->second.fromVid : vrouteItr->second.toVid;
                // bool rebuildTemp = (msgIncoming->getSrc() == otherEnd && msgIncoming->getRebuild() == false) ? false : true;    // don't rebuild route if otherEnd initiated this Teardown with rebuild=false
                
                processTeardownAtEndpoint(oldPathid, otherEnd, msgIncoming);

                vlrRoutingTable.removeRouteByPathID(oldPathid);
            }
            else {  // I'm not an endpoint of this vroute
                EV_DETAIL << "Pathid " << oldPathid << " is removed from routing table, forwarding the Teardown message to nextHopVid = " << nextHopVid << endl;
                if (nextHopIsUnavailable != 1) {
                    sendCreatedTeardown(msgIncoming, /*nextHopPnei=*/nextHopVid);
                    pktForwarded = true;
                } else       // next hop unavailable and regular vroute, remove this vroute from lostPneis.brokenVroutes
                    removeRouteFromLostPneiBrokenVroutes(oldPathid, /*lostPneiVid=*/nextHopVid);

                vlrRoutingTable.removeRouteByPathID(oldPathid);
            }
        }
    }
    if (recordStatsToFile && recordDroppedMsg && !pktRecorded) {   // record message
        if (pktForMe)
            recordMessageRecord(/*action=*/1, /*src=*/msgIncoming->getSrc(), /*dst=*/vid, "Teardown", /*msgId=*/msgIncoming->getPathids(0), /*hopcount=*/msgIncoming->getHopcount()+1, /*chunkByteLength=*/msgIncoming->getByteLength());
        else if (!pktForwarded)
            recordMessageRecord(/*action=*/4, /*src=*/msgIncoming->getSrc(), /*dst=*/vid, "Teardown", /*msgId=*/msgIncoming->getPathids(0), /*hopcount=*/msgIncoming->getHopcount()+1, /*chunkByteLength=*/msgIncoming->getByteLength());
    }
}

// process Teardown for oldPathid where I'm an endpoint     NOTE not removing oldPathid from vlrRoutingTable
// if msgIncoming=nullptr, I didn't receive Teardown but want to remove oldPathid where I'm an endpoint
void Vrr::processTeardownAtEndpoint(const VlrPathID& oldPathid, const VlrRingVID& otherEnd, const TeardownInt* msgIncoming)
{
    auto vrouteItr = vlrRoutingTable.vlrRoutesMap.find(oldPathid);
    ASSERT(vrouteItr != vlrRoutingTable.vlrRoutesMap.end());    // since we should've checked that I'm an endpoint
    // bool isTemporaryRoute = vlrRoutingTable.getIsTemporaryRoute(vrouteItr->second.isUnavailable);

    // for regular vroute remove otherEnd from vset or nonEssRoutes, only add otherEnd to pendingVset if received Teardown doesn't include srcVset
    bool removedFromVset = removeEndpointOnTeardown(oldPathid, /*towardVid=*/otherEnd, /*pathIsVsetRoute=*/vrouteItr->second.isVsetRoute, /*addToPending=*/false);
    // otherEnd should be in pendingVset now if it was once in my vset
    if (msgIncoming != nullptr && msgIncoming->getSrcVsetArraySize() > 0) {     // otherEnd initiated this Teardown and sent its srcVset, probably it no longer needs me as vnei
        vrrAdd(msgIncoming, /*srcVid=*/VLRRINGVID_NULL);        // process srcVset
        
    } else if (removedFromVset) {    // send setupReq to otherEnd
        pendingVsetAdd(otherEnd, /*numTrials=*/setupReqRetryLimit);
    }
}

// check if other end of pathid (towardVid) is in my vset, if so remove; also check if pathid is a non-essential vroute in nonEssRoutes scheduled to be torn down (i.e. other end was in my vset), if so remove
// if addToPending=true and towardVid was in my vset, remove and schedule setupReq to add it back
bool Vrr::removeEndpointOnTeardown(const VlrPathID& pathid, const VlrRingVID& towardVid, bool pathIsVsetRoute, bool addToPending)
{
    bool removedFromVset = false;
    if (pathIsVsetRoute) {
        // remove towardVid from vset
        EV_DETAIL << "Removing endpoint " << towardVid << " of vset route " << pathid << endl;

        auto vneiItr = vset.find(towardVid);
        // ASSERT(vneiItr != vset.end());  // towardVid should be in vset if pathIsVsetRoute=true
        if (vneiItr != vset.end()) {    // towardVid in my vset
            vneiItr->second.erase(pathid);
            if (vneiItr->second.empty()) {   // no more vset-routes to towardVid
                EV_DETAIL << "Removing other endpoint = " << towardVid << " from vset" << endl;

                removedFromVset = vsetEraseAddPending(towardVid, /*addToPending=*/addToPending);  // if true, towardVid was removed from vset and added to pendingVset

                nodesVsetCorrect.erase(vid);

                if (vset.empty()) {     // if vset empty after removing towardVid
                    selfInNetwork = false;
                    cancelEvent(inNetworkWarmupTimer);
                    EV_WARN << "Vset empty after removing endpoint = " << towardVid << ", setting inNetwork = " << selfInNetwork << endl;
                    
                    // Commented out assuming there will be enough vroutes remain intact after failure
                    // if (representative.vid == vid) {    // if I'm the rep, schedule inNetwork to become true if I'm still rep after enough wait time to ensure I can't join an existing overlay (shouldn't happen if there still exists enough vroutes after failure)
                    //     scheduleAt(simTime() + routeSetupReqTraceWaitTime *10, inNetworkWarmupTimer);
                    //     EV_DETAIL << "I'm the rep, inNetwork scheduled to become true at " << inNetworkWarmupTimer->getArrivalTime() << endl;
                    // }
                }
                if (recordStatsToFile) { // write node status update
                    std::ostringstream s;
                    s << "vsetUnfull: inNetwork=" << selfInNetwork << " removedVnei=" << towardVid << " vset=" << printVsetToString() << " vsetSize=" << vset.size();
                    recordNodeStatsRecord(/*infoStr=*/s.str().c_str());   // unused params (stage)
                }
                
            }
            // else, didn't remove towardVid from vset bc there are still vset-routes to towardVid after removing pathid
        }
    }
    else {  // if pathid in nonEssRoutes, its isVsetRoute should have been set to false
        auto nonEssItr = nonEssRoutes.find(pathid);
        if (nonEssItr != nonEssRoutes.end()) {
            EV_DETAIL << "Removing pathid = " << pathid << " from nonEssRoutes before removing it from routing table because of teardown" << endl;
            nonEssRoutes.erase(nonEssItr);
        } 
    }
    return removedFromVset;
}

// remove newPathid from vlrRoutingTable, send Teardown to its prevhopVid, nexthopVid and msgPrevHopVid != VLRRINGVID_NULL
// if checkMeEndpoint=true, check if newPathid is a vset-route at me, if so, remove otherEnd from vset
// if addSrcVset=true, add my vset in Teardown to send
void Vrr::vrrTeardownPath(VlrPathID newPathid, VlrRingVID msgPrevHopVid, bool addSrcVset, bool checkMeEndpoint, const char *infoStr/*=""*/)
{
    std::vector<VlrRingVID> sendTeardownToAddrs;
    // bool addSrcVset = (msgPrevHopVid == VLRRINGVID_NULL);   // include my vset if I initiated this Teardown

    auto vrouteItr = vlrRoutingTable.vlrRoutesMap.find(newPathid);
    if (vrouteItr != vlrRoutingTable.vlrRoutesMap.end()) {  // newPathid already in vlrRoutingTable
        EV_WARN << "The broken pathid " << newPathid << " of setupReply/repairLocalReply is already in my routing table, tearing down both paths with the same pathid" << endl;
        // tear down path recorded in routing table (pathid duplicate, something is wrong, can be a loop)
        sendTeardownToAddrs.push_back(vrouteItr->second.prevhopVid);
        sendTeardownToAddrs.push_back(vrouteItr->second.nexthopVid);
        // get 2-bit status of prevhop and nexthop
        std::vector<char> nextHopStates = {vlrRoutingTable.getPrevNextIsUnavailable(vrouteItr->second.isUnavailable, /*getPrev=*/true), vlrRoutingTable.getPrevNextIsUnavailable(vrouteItr->second.isUnavailable, /*getPrev=*/false)};
        // send Teardown to prevhop and nexthop of newPathid recorded in routing table
        for (size_t i = 0; i < sendTeardownToAddrs.size(); ++i) {
            const VlrRingVID& nextHopVid = sendTeardownToAddrs[i];
            if (nextHopVid != VLRRINGVID_NULL) {  // I'm not an endpoint of the vroute
                const char& nextHopIsUnavailable = nextHopStates[i];
                if (nextHopIsUnavailable != 1) {    // next hop isn't unavailable
                    const VlrRingVID& towardEnd = (nextHopVid == vrouteItr->second.nexthopVid) ? vrouteItr->second.toVid : vrouteItr->second.fromVid;
                    const auto& teardownOut = createTeardown(/*pathid=*/newPathid, /*addSrcVset=*/addSrcVset);
                    sendCreatedTeardown(teardownOut, /*nextHopPnei=*/nextHopVid);

                    if (recordStatsToFile) {   // record sent message
                        char infoCharArray[300] = "vrrTeardownPath: to prevhopVid/nexthopVid | ";
                        // if (infoStr[0] == '\0')    // if infoStr is empty
                        strcat(infoCharArray, infoStr);
                        recordMessageRecord(/*action=*/0, /*src=*/vid, /*dst=*/towardEnd, "Teardown", /*msgId=*/0, /*hopcount=*/0, /*chunkByteLength=*/0, /*infoStr=*/infoCharArray);   // unused params (msgId, hopcount, chunkByteLength) are just assigned to 0
                    }
                } else {    // next hop is unavailable, remove this vroute that have been torn down from lostPneis.brokenVroutes
                    removeRouteFromLostPneiBrokenVroutes(newPathid, /*lostPneiVid=*/nextHopVid);
                }
            }
        }

        if (checkMeEndpoint) {  // check if I'm an endpoint of pathid, if so this may be a vset route, or maybe in nonEssRoutes
            if (vrouteItr->second.fromVid == vid)
                processTeardownAtEndpoint(newPathid, /*otherEnd=*/vrouteItr->second.toVid, /*msgIncoming=*/nullptr);
            else if (vrouteItr->second.toVid == vid)
                processTeardownAtEndpoint(newPathid, /*otherEnd=*/vrouteItr->second.fromVid, /*msgIncoming=*/nullptr);
        }

        vlrRoutingTable.removeRouteByPathID(newPathid);
    }
    // send Teardown to msgPrevHopVid
    if (msgPrevHopVid != VLRRINGVID_NULL) {
        if (std::find(sendTeardownToAddrs.begin(), sendTeardownToAddrs.end(), msgPrevHopVid) == sendTeardownToAddrs.end()) {    // msgPrevHopVid doesn't exist in sendTeardownToAddrs
            const auto& teardownOut = createTeardown(/*pathid=*/newPathid, /*addSrcVset=*/addSrcVset);
            sendCreatedTeardown(teardownOut, /*nextHopPnei=*/msgPrevHopVid);

            if (recordStatsToFile) {   // record sent message
                char infoCharArray[300] = "vrrTeardownPath: to msgPrevHopVid | ";
                strcat(infoCharArray, infoStr);
                recordMessageRecord(/*action=*/0, /*src=*/vid, /*dst=*/VLRRINGVID_NULL, "Teardown", /*msgId=*/0, /*hopcount=*/0, /*chunkByteLength=*/0, /*infoStr=*/infoCharArray);   // unused params (msgId, hopcount, chunkByteLength) are just assigned to 0
            }
        }
    }
}

void Vrr::processTestPacketTimer()
{
    EV_DEBUG << "Processing TestPacket timer at node " << vid << endl;
    if (sendTestPacketStart) {
        // if (representative.heardfromvid != VLRRINGVID_NULL) {   // I have valid rep
        VlrRingVID dstVid;
        // option 3: send TestPacket to last node in testDstList if it's not empty
        if (!testDstList.empty()) {
            dstVid = testDstList.back();
            testDstList.pop_back();
        } else {
            // option 4: send TestPacket to a random node in vidRegistryTable
            dstVid = getRandomVidInRegistry();
        }

        if (dstVid != vid && dstVid != VLRRINGVID_NULL) {    // if selected TestPacket dst isn't myself
            // record statistics as a string
            ++allSendMessageId;     // NOTE  increment allSendMessageId here for the TestPacket to be sent, it's not incremented in createTestPacket()
            recordMessageRecord(/*action=*/0, /*src=*/vid, /*dst=*/dstVid, "TestPacket", /*msgId=*/0, /*hopcount=*/0, /*chunkByteLength=*/0);   // unused params (msgId, hopcount, chunkByteLength) are just assigned to 0

            // try to send TestPacket
            // VlrIntOption *vlrOptionOut = createVlrOption(dstVid);
            VlrIntOption vlrOptionOut;
            initializeVlrOption(vlrOptionOut, /*dstVid=*/dstVid);
            VlrRingVID nextHopVid = findNextHop(vlrOptionOut, /*excludeVid=*/VLRRINGVID_NULL);
            if (nextHopVid == VLRRINGVID_NULL) {
                // delete vlrOptionOut;
                EV_WARN << "No next hop found to send TestPacket at me = " << vid << " to dst = " << dstVid << ", not sending in this round" << endl;
            } else {
                auto msgOutgoing = createTestPacket(dstVid);
                // // NOTE addTag should be executed after chunkLength has been set
                // msgOutgoing->addTag<VlrCreationTimeTag>()->setMessageId(allSendMessageId);
                msgOutgoing->setMessageId(allSendMessageId);
                msgOutgoing->setVlrOption(vlrOptionOut);

                EV_INFO << "Sending TestPacket to dst = " << dstVid << ", nexthop: " << nextHopVid << endl;
                sendCreatedTestPacket(msgOutgoing, /*outGateIndex=*/psetTable.getPneiGateIndex(nextHopVid));
            }
        }
        
    }
    // sendTestPacketStart not yet true
    else if (lastTimeNodeJoinedInNetwork + sendTestPacketOverlayWaitTime <= simTime().dbl()) {
        sendTestPacketStart = true;
    }
    scheduleTestPacketTimer();
}

VlrIntTestPacket* Vrr::createTestPacket(const VlrRingVID& dstVid) const
{
    VlrIntTestPacket *msg = new VlrIntTestPacket(/*name=*/"TestPacket");
    msg->setDst(dstVid);          // vidByteLength
    msg->setSrc(vid);
    msg->setHopcount(0);     // 2 byte

    int chunkByteLength = 2 * VLRRINGVID_BYTELEN + 2;

    initializeVlrOption(msg->getVlrOptionForUpdate());

    // for VlrIntUniPacket
    chunkByteLength += getVlrUniPacketByteLength();

    msg->setByteLength(chunkByteLength);
    return msg;
}

// if computeChunkLength = true, compute chunk length because chunk was just created with createSetupReq() or modified after dupShared() from another chunk
// else, no need to compute chunk length because chunk was dupShared() (not modified) from another chunk that has chunkLength set
void Vrr::sendCreatedTestPacket(VlrIntTestPacket *msg, const int& outGateIndex, double delay/*=0*/)
{
    EV_DEBUG << "Sending TestPacket: dst = " << msg->getDst() << ", src = " << msg->getSrc() << endl;

    msg->getVlrOptionForUpdate().setPrevHopVid(vid);    // set packet prevHopVid to myself
    msg->setHopcount(msg->getHopcount() +1);    // increment packet hopcount
    
    // NOTE addTag should be executed after chunkLength has been set, and chunkLength shouldn't be changed before findTag/getTag

    // all multihop VLR packets (setupReq, setupReply, etc) L3 dst are set to a pnei, greedy routing at L3 in routeDatagram() isn't needed, but we do greedy routing and deal with VlrOption at L4 (in processSetupReq() for example) 
    // udpPacket->addTagIfAbsent<VlrIntOptionReq>()->setVlrOption(vlrOption);      // VlrOption to be set in IP header in datagramLocalOutHook()
    
    sendCreatedPacket(msg, /*unicast=*/true, /*outGateIndex=*/outGateIndex, /*delay=*/delay, /*checkFail=*/true);
}

void Vrr::processTestPacket(VlrIntTestPacket *msgIncoming, bool& pktForwarded)
{
    EV_DEBUG << "Received TestPacket" << endl;

    VlrIntOption& vlrOptionIn = msgIncoming->getVlrOptionForUpdate();
    VlrRingVID msgPrevHopVid = vlrOptionIn.getPrevHopVid();
    if (msgPrevHopVid == VLRRINGVID_NULL)
        throw cRuntimeError("Received TestPacket with vlrOption.prevHopVid = null");

    VlrRingVID dstVid = msgIncoming->getDst();
    VlrRingVID srcVid = msgIncoming->getSrc();
    unsigned int msgHopCount = msgIncoming->getHopcount() +1;

    EV_INFO << "Processing TestPacket: src = " << srcVid << ", dst = " << dstVid << ", hopCount = " << msgHopCount << ", prevhop: " << msgPrevHopVid << endl;

    // // for statistics measurement
    // numTestPacketReceived++;

    // checked I have a valid rep
    if (dstVid == vid) {
        EV_INFO << "Received TestPacket to me: src = " << srcVid << ", hopCount = " << msgHopCount << endl;

        // record received TestPacket for me
        // auto vlrCreationTag = msgIncoming->findTag<VlrCreationTimeTag>();
        // if (vlrCreationTag) {   // if <VlrCreationTimeTag> tag present
        recordMessageRecord(/*action=*/1, /*src=*/srcVid, /*dst=*/vid, "TestPacket", /*msgId=*/msgIncoming->getMessageId(), /*hopcount=*/msgHopCount, /*chunkByteLength=*/msgIncoming->getByteLength());
        // }

        // emit(testpacketReceivedSignal, srcVid);     // emit src of this TestPacket
    }
    else {  // this TestPacket isn't destined for me
        // if (recordStatsToFile && recordReceivedMsg)   // Commented out bc we're processing TestPacket, sendTestPacket must be true
        // Commented out and only record "arrived"/"dropped" TestPacket     NOTE record "dropped" TestPacket even if recordDroppedMsg=false to check see how its hopcount
        // recordMessageRecord(/*action=*/2, /*src=*/srcVid, /*dst=*/dstVid, "TestPacket", /*msgId=*/msgIncoming->getMessageId(), /*hopcount=*/msgHopCount, /*chunkByteLength=*/msgIncoming->getByteLength());  // unimportant params (msgId, hopcount)

        if (checkUniPacketHopcountLimit && msgIncoming->getHopcount()+1 >= uniPacketHopcountLimit) {    // packet reached max hopcount
            if (recordStatsToFile /*&& recordDroppedMsg*/) {   // record dropped message
                recordMessageRecord(/*action=*/4, /*src=*/srcVid, /*dst=*/dstVid, "TestPacket", /*msgId=*/msgIncoming->getMessageId(), /*hopcount=*/msgIncoming->getHopcount()+1, /*chunkByteLength=*/msgIncoming->getByteLength(), /*infoStr=*/"hopcount limit reached");   // unimportant params (msgId, hopcount)
            }
            return;
        } else {
            // forward this TestPacket with findNextHop()
            // VlrIntOption* vlrOptionOut = vlrOptionIn->dup();
            VlrRingVID nextHopVid = findNextHop(vlrOptionIn, /*excludeVid=*/VLRRINGVID_NULL);
            if (nextHopVid == VLRRINGVID_NULL) {
                // delete vlrOptionOut;
                EV_WARN << "No next hop found for TestPacket received at me = " << vid << ", dropping packet: src = " << srcVid << ", dst = " << dstVid << ", hopCount = " << msgHopCount << endl;
                if (recordStatsToFile /*&& recordDroppedMsg*/) {   // record dropped message
                    recordMessageRecord(/*action=*/4, /*src=*/srcVid, /*dst=*/dstVid, "TestPacket", /*msgId=*/msgIncoming->getMessageId(), /*hopcount=*/msgIncoming->getHopcount()+1, /*chunkByteLength=*/msgIncoming->getByteLength(), /*infoStr=*/"no next hop");   // unimportant params (msgId, hopcount)
                }
            } else {    // nexthop found to forward TestPacket
                // auto msgOutgoing = staticPtrCast<VlrIntTestPacket>(msgIncoming->dupShared());
                // msgIncoming->setHopcount(msgHopCount);
                sendCreatedTestPacket(msgIncoming, /*outGateIndex=*/psetTable.getPneiGateIndex(nextHopVid));    // computeChunkLength=false
                pktForwarded = true;
            }
        }
    }
}

void Vrr::processWaitRepairLinkTimer(WaitRepairLinkIntTimer *repairLinkTimer)
{
    // WaitRepairLinkIntTimer *repairLinkTimer = check_and_cast<WaitRepairLinkIntTimer *>(message);
    VlrRingVID targetVid = repairLinkTimer->getDst();
    int retryCount = repairLinkTimer->getRetryCount();
    EV_WARN << "Processing wait repairLinkReq timer timeout (useRepairLocal=" << sendRepairLocalNoTemp << ") of lost pnei " << targetVid << ", retryCount: " << retryCount << endl;

    if (retryCount < repairLinkReqRetryLimit) {
        
        auto lostPneiItr = lostPneis.find(targetVid);
        ASSERT(lostPneiItr != lostPneis.end());    //  targetVid must be in lostPneis

        std::vector<VlrPathID> repairedBrokenPathids;  // brokenPathids that are repaired by RepairLocalReply, will be removed from lostPneiItr->second.brokenVroutes after traversing it (don't modify while traversing it)
        
        // send RepairLinkReq for brokenVroutes whose prevhop is lost
        for (const auto& brokenPathid : lostPneiItr->second.brokenVroutes) {
            auto vrouteItr = vlrRoutingTable.vlrRoutesMap.find(brokenPathid);
            if (vrouteItr != vlrRoutingTable.vlrRoutesMap.end()) {  // pathid should be in vlrRoutingTable
                // NOTE temporary route or dismantled vroute shouldn't be in brokenVroutes
                // one of both of prevhop and nexthop is the lost pnei
                if (lostPneiItr->first == vrouteItr->second.prevhopVid) { // route prevhop == lost pnei
                    std::vector<VlrRingVID> linkTrace;  // [dst of RepairLocalReply (, second last hop)]
                    
                    VlrRingVID prevprevhop = VLRRINGVID_NULL;
                    if (vrouteItr->second.prevhopVids.size() >= 2)
                        prevprevhop = vrouteItr->second.prevhopVids[1];
                    // check if fromVid is a LINKED pnei
                    if (psetTable.pneiIsLinked(vrouteItr->second.fromVid))
                        linkTrace.push_back(vrouteItr->second.fromVid);
                    else if (prevprevhop != VLRRINGVID_NULL) {
                        if (psetTable.pneiIsLinked(prevprevhop))    // check if prevprevhop is a LINKED pnei
                            linkTrace.push_back(prevprevhop);
                        else {      // check if prevprevhop is a 2-hop pnei
                            auto mpneiItr = psetTable.mpneiToPneiMap.find(prevprevhop);
                            if (mpneiItr != psetTable.mpneiToPneiMap.end()) {
                                ASSERT(!mpneiItr->second.empty());      // assert there's pnei associated with this mpnei
                                // const VlrRingVID& nextHopVid = *(mpneiItr->second.begin());
                                for (const auto& nextHopVid : mpneiItr->second) {    // find a pnei != nexthop that's connected to prevprevhop, bc we assume prevhop != nexthop
                                    if (nextHopVid != vrouteItr->second.nexthopVid) {
                                        ASSERT(psetTable.pneiIsLinked(nextHopVid));     // should be true bc when a pnei is no longer LINKED, we remove mpneis associated with it
                                        // linkTrace: [prevprevhop, pnei that has prevprevhop as mpnei]
                                        linkTrace.push_back(prevprevhop);
                                        linkTrace.push_back(nextHopVid);
                                        break;
                                    }
                                }
                            }
                        }
                    }
                    if (!linkTrace.empty()) {   // found a way to repair brokenPathid
                        EV_WARN << "Sending RepairLocalReply for brokenPathid = " << brokenPathid << ": " << vrouteItr->second << ", linkTrace=" << linkTrace << endl;
                        // if (vid == 3058 || vid == 1092 || vrouteItr->second.nexthopVid == vrouteItr->second.prevhopVid)
                        //     EV_INFO << "ohno" << endl;
                        // change route prevhop (unavailable) to next hop in linkTrace, which I've checked is LINKED pnei
                        const VlrRingVID& nextHopVid = linkTrace.back();
                        vrouteItr->second.prevhopVid = nextHopVid;
                        vlrRoutingTable.setRouteItrPrevNextIsUnavailable(vrouteItr, /*setPrev=*/true, /*value=*/0);     // set route prevhop state to available 
                        vlrRoutingTable.addRouteEndInEndpointMap(brokenPathid, vrouteItr->second.fromVid);     // add back route fromVid in endpointToRoutesMap
                        // change route prevhopVids (closest node at front) based on reversed linkTrace (closest node at back)
                        std::vector<VlrRingVID>& routePrevhopVids = vrouteItr->second.prevhopVids;
                        routePrevhopVids.clear();
                        for (int i = linkTrace.size()-1; i >= 0; i--)
                            routePrevhopVids.push_back(linkTrace[i]);
                        
                        // will remove brokenPathid from lostPneis[prevhopVid].brokenVroutes
                        if (lostPneiItr->first != vrouteItr->second.nexthopVid)  // route prevhop == lost pnei, ensure route nexthop != lost pnei
                            repairedBrokenPathids.push_back(brokenPathid);

                        // send RepairLocalReply using linkTrace
                        auto replyOutgoing = createRepairLocalReply();
                        replyOutgoing->getPathidToPrevhopMapForUpdate().insert({brokenPathid, {vrouteItr->second.fromVid, vrouteItr->second.toVid}});
                        replyOutgoing->setLinkTrace(linkTrace);
                        sendCreatedRepairLocalReply(replyOutgoing, /*outGateIndex=*/psetTable.getPneiGateIndex(nextHopVid), /*computeChunkLength=*/true);

                        if (recordStatsToFile) {   // record sent message
                            recordMessageRecord(/*action=*/0, /*src=*/vid, /*dst=*/linkTrace.at(0), "RepairLocalReply", /*msgId=*/0, /*hopcount=*/0, /*chunkByteLength=*/0);   // unused params (msgId, hopcount, chunkByteLength) are just assigned to 0
                        }
                    }
                }   // else, route nexthop == lost pnei
            }
        }
        // remove repaired vroutes that are no longer using lost pnei as prevhop/nexthop
        for (const auto& brokenPathid : repairedBrokenPathids)
            lostPneiItr->second.brokenVroutes.erase(brokenPathid);
        
        // reschedule the wait repairLink timer
        int retryCount = repairLinkTimer->getRetryCount();
        repairLinkTimer->setRetryCount(++retryCount);
        double reqWaitTime = repairLinkReqWaitTime;
        if (lostPneiItr->second.brokenVroutes.empty())  // no more brokenVroutes associated lostPneis[targetVid], can remove right now
            reqWaitTime = 0;
        scheduleAt(simTime() + reqWaitTime, repairLinkTimer);
        EV_DETAIL << "Rescheduling repairLinkReq timer: dst = " << targetVid << ", retryCount = " << retryCount << endl;
    }
    else {  // tear down all vroutes broken by lost pnei targetVid that haven't been repaired, delete targetVid from lostPneis
        auto lostPneiItr = lostPneis.find(targetVid);
        ASSERT(lostPneiItr != lostPneis.end());
        // const L3Address& pneiAddr = lostPneiItr->second.address;
        EV_WARN << "Delete node " << targetVid << " from lostPneis, broken vroutes = [";
        for (const auto& pathid : lostPneiItr->second.brokenVroutes) 
            EV_WARN << pathid << " ";
        EV_WARN << "]" << endl;

        if (!psetTable.pneiIsLinked(targetVid)) {   // confirm targetVid is no longer LINKED
            
            std::map<VlrRingVID, std::pair<std::vector<VlrPathID>, char>> nextHopToPathidsMap;    // for Teardown to send, map next hop address to (pathids, next hop 2-bit isUnavailable) pair
            for (const auto& oldPathid : lostPneiItr->second.brokenVroutes) {
                auto vrouteItr = vlrRoutingTable.vlrRoutesMap.find(oldPathid);
                if (vrouteItr != vlrRoutingTable.vlrRoutesMap.end()) {  // pathid still in vlrRoutingTable
                    if (vrouteItr->second.isUnavailable != 0 /*&& !vlrRoutingTable.getIsDismantledRoute(vrouteItr->second.isUnavailable)*/) {   // this is a regular vroute, unavailable or patched (using temporary route)
                        // send Teardown to prevhop/nexthop that's still LINKED, bc pnei can't be reached (sending a futile Teardown to it can cause collision, other nodes along oldPathid won't process the unicast Teardown not directed to themselves)
                        bool isToNexthop = (vrouteItr->second.prevhopVid == targetVid);
                        const VlrRingVID& nextHopVid = (isToNexthop) ? vrouteItr->second.nexthopVid : vrouteItr->second.prevhopVid;
                        
                        char nextHopIsUnavailable = vlrRoutingTable.getPrevNextIsUnavailable(vrouteItr->second.isUnavailable, /*getPrev=*/!isToNexthop);

                        if (nextHopVid != VLRRINGVID_NULL) {     // I'm not an endpoint of the vroute
                            // remove regular route from vlrRoutingTable if remaining next hop is blocked/patched/unavailable, as dismantled vroute shouldn't involve temporary route
                            // bool removeFromRoutingTable = (!keepDismantledRoute || nextHopIsUnavailable != 0);
                            if (nextHopIsUnavailable != 1) {
                                EV_DETAIL << "Teardown (pathid = " << oldPathid << ") will be sent to nexthop " << nextHopVid << endl;
                                const VlrRingVID& towardEnd = (isToNexthop) ? vrouteItr->second.toVid : vrouteItr->second.fromVid;
                                const auto& teardownOut = createTeardown(/*pathid=*/oldPathid, /*addSrcVset=*/false);
                                sendCreatedTeardown(teardownOut, /*nextHopPnei=*/nextHopVid);

                                if (recordStatsToFile) {   // record sent message
                                    recordMessageRecord(/*action=*/0, /*src=*/vid, /*dst=*/towardEnd, "Teardown", /*msgId=*/0, /*hopcount=*/0, /*chunkByteLength=*/0, /*infoStr=*/"repairLinkReq retry limit");   // unused params (msgId, hopcount, chunkByteLength) are just assigned to 0
                                }

                            } else { // next hop unavailable and regular vroute, remove this vroute from lostPneis.brokenVroutes
                                if (nextHopVid != targetVid)    // if vrouteItr->second.nexthopVid == vrouteItr->second.prevhopVid == targetVid, no need to remove pathid from lostPneis[targetVid].brokenVroutes bc lostPneis[targetVid] will be erased
                                    removeRouteFromLostPneiBrokenVroutes(oldPathid, /*lostPneiVid=*/nextHopVid);
                            }

                            // if (removeFromRoutingTable) {
                            EV_DETAIL << "Pathid " << oldPathid << " is removed from vlrRoutingTable, nextHopIsUnavailable = " << (int)nextHopIsUnavailable << endl;
                            // delete route in endpointToRoutesMap
                            vlrRoutingTable.removeRouteEndsFromEndpointMap(oldPathid, vrouteItr->second);
                            // delete route in vlrRoutesMap
                            vlrRoutingTable.vlrRoutesMap.erase(vrouteItr);
                            // }
                            // else {    // endpoint connected by the remaining next hop is still available
                            //     // set lost pnei in route unavailable
                            //     vlrRoutingTable.setRouteItrPrevNextIsUnavailable(vrouteItr, /*setPrev=*/isToNexthop, /*value=*/1);
                            //     // set vroute as dismantled
                            //     vlrRoutingTable.setRouteItrIsDismantled(vrouteItr, /*setDismantled=*/true, /*setPrev=*/isToNexthop);
                            //     // remove endpoint connected by lost pnei in endpointToRoutesMap
                            //     VlrRingVID lostEnd = (isToNexthop) ? vrouteItr->second.fromVid : vrouteItr->second.toVid;
                            //     EV_DETAIL << "Pathid " << oldPathid << " is torn down but kept in vlrRoutingTable and added to dismantledRoutes, endpoint " << lostEnd << " becomes unavailable" << endl;
                            //     vlrRoutingTable.removeRouteEndFromEndpointMap(oldPathid, lostEnd);

                            //     dismantledRoutes.insert({oldPathid, simTime() + dismantledRouteExpiration});
                            // }

                        } else {
                            // check if I'm an endpoint of pathid, if so this may be a vset route, or maybe in nonEssRoutes (brokenVroutes shouldn't be temporary or dismantled)
                            if (vrouteItr->second.fromVid == vid)
                                processTeardownAtEndpoint(oldPathid, /*otherEnd=*/vrouteItr->second.toVid, /*msgIncoming=*/nullptr);
                            else if (vrouteItr->second.toVid == vid)
                                processTeardownAtEndpoint(oldPathid, /*otherEnd=*/vrouteItr->second.fromVid, /*msgIncoming=*/nullptr);
                        
                            // delete route in endpointToRoutesMap
                            vlrRoutingTable.removeRouteEndsFromEndpointMap(oldPathid, vrouteItr->second);
                            // delete route in vlrRoutesMap
                            vlrRoutingTable.vlrRoutesMap.erase(vrouteItr);
                        }
                    }
                }
            }
        }
        // NOTE when there is a valid temporary route to lost pnei, this WaitRepairLinkTimer shouldn't timeout, so tempVlinks is likely empty
        // put temporary vlinks to lost pnei in nonEssRoutes as they are no longer necessary for broken vroutes
        // tear down the temporary route to notify the lost pnei of the removal of these broken vroutes
        // delayLostPneiTempVlinksTeardown(lostPneiItr->second.tempVlinks, /*delay=*/0);
        
        cancelAndDelete(lostPneiItr->second.timer);
        lostPneis.erase(lostPneiItr);
    }
}

// remove pathid from lostPneis[lostPnei].brokenVroutes
void Vrr::removeRouteFromLostPneiBrokenVroutes(const VlrPathID& pathid, const VlrRingVID& lostPneiVid)
{
    auto lostPneiItr = lostPneis.find(lostPneiVid);
    if (lostPneiItr != lostPneis.end()) {
        lostPneiItr->second.brokenVroutes.erase(pathid);
        
        if (lostPneiItr->second.brokenVroutes.empty()) {    // if lostPneis[lostPnei].brokenVroutes empty after removing pathid
            // delayLostPneiTempVlinksTeardown(lostPneiItr->second.tempVlinks, /*delay=*/0);   // notify lostPnei asap that my brokenVroutes now empty
            cancelAndDelete(lostPneiItr->second.timer);
            lostPneis.erase(lostPneiItr);
        }
    }
}

int Vrr::computeRepairLocalReplyByteLength(RepairLocalReplyInt* msg) const
{
    int chunkByteLength = 0;
    // // std::set<VlrPathID> brokenPathids;
    // chunkByteLength += msg->getBrokenPathids().size() * VLRPATHID_BYTELEN;
    // std::map<VlrPathID, std::vector<unsigned int>> pathidToPrevhopMap;
    const auto& pathidToPrevhopMap = msg->getPathidToPrevhopMap();
    for (auto it = pathidToPrevhopMap.begin(); it != pathidToPrevhopMap.end(); ++it)
        chunkByteLength += VLRPATHID_BYTELEN + it->second.size() * VLRRINGVID_BYTELEN;
    // std::vector<unsigned int> linkTrace
    chunkByteLength += msg->getLinkTrace().size() * VLRRINGVID_BYTELEN;
    // // std::vector<unsigned int> prevhopVids
    // chunkByteLength += routePrevhopVidsSize * VLRRINGVID_BYTELEN;
    // // // L3Address srcAddress;
    // // chunkByteLength += addressByteLength;

    return chunkByteLength;
}

// still need to set linkTrace and pathidToPrevhopMap
RepairLocalReplyInt* Vrr::createRepairLocalReply()
{
    RepairLocalReplyInt *msg = new RepairLocalReplyInt(/*name=*/"RepairLocalReply");
    msg->setSrc(vid);
    // msg->getPrevhopVidsForUpdate().push_back(vid);
    // msg->setOldestPrevhopIndex(0);

    initializeVlrOption(msg->getVlrOptionForUpdate());

    msg->setMessageId(++allSendMessageId);
    msg->setHopcount(0);

    return msg;
}

// if computeChunkLength = true, compute chunk length because chunk was just created with createSetupReq() or modified after dupShared() from another chunk
// else, no need to compute chunk length because chunk was dupShared() (not modified) from another chunk that has chunkLength set
void Vrr::sendCreatedRepairLocalReply(RepairLocalReplyInt *msg, const int& outGateIndex, bool computeChunkLength/*=true*/, double delay/*=0*/)
{
    if (computeChunkLength)
        msg->setByteLength(computeRepairLocalReplyByteLength(msg));
    EV_DEBUG << "Sending repairLocalReply: linkTrace = " << msg->getLinkTrace() << ", brokenPathidToPrevhopMap size: " << msg->getPathidToPrevhopMap().size() << endl;

    msg->getVlrOptionForUpdate().setPrevHopVid(vid);    // set packet prevHopVid to myself
    msg->setHopcount(msg->getHopcount() +1);    // increment packet hopcount
    
    // NOTE addTag should be executed after chunkLength has been set, and chunkLength shouldn't be changed before findTag/getTag

    // all multihop VLR packets (setupReq, setupReply, etc) L3 dst are set to a pnei, greedy routing at L3 in routeDatagram() isn't needed, but we do greedy routing and deal with VlrOption at L4 (in processSetupReq() for example) 
    // udpPacket->addTagIfAbsent<VlrIntOptionReq>()->setVlrOption(vlrOption);      // VlrOption to be set in IP header in datagramLocalOutHook()
    
    sendCreatedPacket(msg, /*unicast=*/true, /*outGateIndex=*/outGateIndex, /*delay=*/delay, /*checkFail=*/true);
}

void Vrr::processRepairLocalReply(RepairLocalReplyInt *replyIncoming, bool& pktForwarded)
{
    EV_DEBUG << "Received RepairLocalReply" << endl;

    VlrRingVID msgPrevHopVid = replyIncoming->getVlrOption().getPrevHopVid();
    if (msgPrevHopVid == VLRRINGVID_NULL)
        throw cRuntimeError("Received RepairLocalReply with vlrOption.prevHopVid = null");
        
    VlrRingVID srcVid = replyIncoming->getSrc();
    VlrRingVID dstVid = replyIncoming->getLinkTrace().at(0);
    ASSERT(!replyIncoming->getPathidToPrevhopMap().empty());
    auto pathidMapItr = replyIncoming->getPathidToPrevhopMap().begin();
    const VlrPathID& newPathid = pathidMapItr->first;

    EV_INFO << "Processing RepairLocalReply: src = " << srcVid << ", dstVid = " << dstVid << ", pathid = " << newPathid << ", prevhop: " << msgPrevHopVid << endl;

    if (recordStatsToFile && recordReceivedMsg) {   // record received message
        recordMessageRecord(/*action=*/2, /*src=*/srcVid, /*dst=*/dstVid, "RepairLocalReply", /*msgId=*/replyIncoming->getMessageId(), /*hopcount=*/replyIncoming->getHopcount()+1, /*chunkByteLength=*/replyIncoming->getByteLength());    // unimportant params (msgId)
    }
    bool pktForMe = false;      // set to true if this msg is directed to me or I processed it as its dst
    bool pktRecorded = false;      // set to true if this msg is recorded with recordMessageRecord()

    auto vrouteItr = vlrRoutingTable.vlrRoutesMap.find(newPathid);
    bool teardownPathid = false;
    if (vrouteItr != vlrRoutingTable.vlrRoutesMap.end()) {  // newPathid already in vlrRoutingTable
        if (dstVid != vid) { // newPathid shouldn't be in vlrRoutingTable, tear down
            teardownPathid = true;
            // tear down path recorded in routing table (pathid duplicate, something is wrong, can be a loop)
            vrrTeardownPath(newPathid, msgPrevHopVid, /*addSrcVset=*/false, /*checkMeEndpoint=*/true);
        }
    } else {    // newPathid not found in vlrRoutingTable
        if (dstVid == vid) {    // newPathid should be in vlrRoutingTable
            EV_WARN << "Handling RepairLocalReply to me=" << vid << ", but broken pathid " << newPathid << " is not found in my routing table, tearing down pathid" << endl;
            teardownPathid = true;
            vrrTeardownPath(newPathid, msgPrevHopVid, /*addSrcVset=*/false, /*checkMeEndpoint=*/false);   // not removing an existing vroute in vlrRoutingTable
        }
    }
    if (!teardownPathid) {
        // checked newPathid not in vlrRoutingTable
        if (!psetTable.pneiIsLinked(msgPrevHopVid)) {        // if prevHopAddr isn't a LINKED pnei 
            EV_WARN << "Previous hop " << msgPrevHopVid << " of repairLocalReply is not a LINKED pnei, tearing down pathid " << newPathid << endl;
            teardownPathid = true;
            // tear down newPathid, i.e. send Teardown to msgPrevHopVid
            vrrTeardownPath(newPathid, msgPrevHopVid, /*addSrcVset=*/false, /*checkMeEndpoint=*/false);   // not removing an existing vroute in vlrRoutingTable
        } else {
            if (dstVid == vid) {
                pktForMe = true;
                EV_WARN << "Handling RepairLocalReply to me=" << vid << ": src = " << srcVid << ", pathid = " << newPathid << ": [";
                for (const auto& elem : pathidMapItr->second)
                    EV_WARN  << elem << " ";
                EV_WARN << "]" << endl;

                auto vrouteItr = vlrRoutingTable.vlrRoutesMap.find(newPathid);
                // checked newPathid still in vlrRoutingTable
                ASSERT(vrouteItr != vlrRoutingTable.vlrRoutesMap.end());  // pathid still in vlrRoutingTable
                // check if route nexthop is unavailable, if so, make it available
                if (vlrRoutingTable.getPrevNextIsUnavailable(vrouteItr->second.isUnavailable, /*getPrev=*/false) == 1) { // route nexthop is unavailable
                    // remove route from lostPneis[nexthopVid].brokenVroutes
                    VlrRingVID nexthopVid = vrouteItr->second.nexthopVid;    // map nexthopAddr L3Address (lost pnei) to VlrRingVID
                    auto lostPneiItr = lostPneis.find(nexthopVid);
                    if (lostPneiItr != lostPneis.end()) {
                        lostPneiItr->second.brokenVroutes.erase(newPathid);
                    }
                    vlrRoutingTable.setRouteItrPrevNextIsUnavailable(vrouteItr, /*setPrev=*/false, /*value=*/0);     // set route nexthop state to available
                    vlrRoutingTable.addRouteEndInEndpointMap(newPathid, vrouteItr->second.toVid);     // add back route toVid in endpointToRoutesMap
                }
                EV_WARN << "Changing vroute nexthop from " << vrouteItr->second.nexthopVid << " to " << msgPrevHopVid << " for pathid = " << newPathid << endl;
                // change route nexthop (unavailable) to prev hop in linkTrace, which I've checked is LINKED pnei
                vrouteItr->second.nexthopVid = msgPrevHopVid;
                // since we updated route nexthop, no need to change route prevhopVids

            } else {  // this repairLocalReply isn't destined for me
                unsigned int nextHopIndex = getNextHopIndexInTrace(replyIncoming->getLinkTraceForUpdate(), /*preferShort=*/false);
                // replyIncoming->getTrace(): [start node, .., parent node]
                if (nextHopIndex >= replyIncoming->getLinkTrace().size()) {
                    EV_WARN << "No next hop found for RepairLocalReply with trace received at me = " << vid << " because no node in trace is a LINKED pnei, tearing down vroute " << newPathid << ", src = " << srcVid << ", linkTrace = " << replyIncoming->getLinkTrace() << endl;
                    // send Teardown to msgPrevHopVid
                    vrrTeardownPath(newPathid, msgPrevHopVid, /*addSrcVset=*/false, /*checkMeEndpoint=*/false);   // not removing an existing vroute in vlrRoutingTable
                }
                else {  // found LINKED next hop in linkTrace
                    const std::vector<VlrRingVID>& linkTrace = replyIncoming->getLinkTrace();
                    VlrRingVID nextHopVid = linkTrace.at(nextHopIndex);
                    // add newPathid to vlrRoutingTable
                    const std::vector<VlrRingVID>& routeEndAndPrev = pathidMapItr->second;
                    auto itr_bool = vlrRoutingTable.addRoute(newPathid, /*fromVid=*/routeEndAndPrev[0], /*toVid=*/routeEndAndPrev[1], /*prevhopVid=*/nextHopVid, /*nexthopVid=*/msgPrevHopVid, /*isVsetRoute=*/false);

                    // change route prevhopVids (closest node at front) based on reversed linkTrace (closest node at back)
                    ASSERT(linkTrace.size() == 1 && nextHopVid == dstVid);    // bc linkTrace can contain at most two nodes in VRR local repair, if this isn't linkTrace[0], next hop has to be it
                    std::vector<VlrRingVID>& routePrevhopVids = itr_bool.first->second.prevhopVids;     // should be empty bc just added this vroute
                    routePrevhopVids.push_back(dstVid);

                    // if (nextHopVid == msgPrevHopVid)
                    //     EV_INFO << "ohno" << endl;

                    EV_INFO << "Added new vroute: newPathid = " << newPathid << ", sending RepairLocalReply to dst = " << dstVid << ", src = " << srcVid << ", trace = " << replyIncoming->getLinkTrace() << endl;
                    sendCreatedRepairLocalReply(replyIncoming, /*outGateIndex=*/psetTable.getPneiGateIndex(nextHopVid), /*computeChunkLength=*/true);
                    pktForwarded = true;
                }
            }
        }
    }

    if (recordStatsToFile && recordDroppedMsg && !pktRecorded) {   // record message
        if (pktForMe)
            recordMessageRecord(/*action=*/1, /*src=*/srcVid, /*dst=*/dstVid, "RepairLocalReply", /*msgId=*/replyIncoming->getMessageId(), /*hopcount=*/replyIncoming->getHopcount()+1, /*chunkByteLength=*/replyIncoming->getByteLength());
        else if (!pktForwarded)
            recordMessageRecord(/*action=*/4, /*src=*/srcVid, /*dst=*/dstVid, "RepairLocalReply", /*msgId=*/replyIncoming->getMessageId(), /*hopcount=*/replyIncoming->getHopcount()+1, /*chunkByteLength=*/replyIncoming->getByteLength());
    }
}

void Vrr::processFillVsetTimer()
{
    EV_DEBUG << "Processing fillVsetTimer at node " << vid << endl;
    //
    // send setupReq to potential vnei to join vnetwork and fill vset
    //
    // if I'm not inNetwork (and not in inNetwork warmup (unless I'm rep and just lost all vneis)), pick a pnei that's inNetwork and send setupReq to my own vid (potential vneis in pendingVset may be outdated)
    // if I have a setupReq timer scheduled in pendingVset, probably I've sent (or planning to send) a setupReq/setupReqTrace to a potential vnei, wait for timeout b4 sending a setupReq as a new join node
    // if repSeqObserveTimer is scheduled, i.e. I'm NOT rep and !inNetwork and just cleared vset and pendingVset bc rep-timeout, I don't send setupReq to pnei inNetwork bc it may rep-timeout soon
    if (!selfInNetwork && vset.empty() && pendingVset.empty()) {
        VlrRingVID proxy = getProxyForSetupReq();
        if (proxy != VLRRINGVID_NULL) {
            // send setupReq to my own vid
            sendSetupReq(vid, proxy, /*setupReqTimer=*/nullptr);
        } else if (!inNetworkWarmupTimer->isScheduled()) {
            scheduleAt(simTime() + inNetworkEmptyVsetWarmupTime, inNetworkWarmupTimer);
            EV_DETAIL << "I'm not inNetwork, vset and pendingVset empty, inNetwork scheduled to become true at " << inNetworkWarmupTimer->getArrivalTime() << endl;
        }
    }

    //
    // purge representativeMap of expired records
    //
    simtime_t currTime = simTime();
    for (auto it = representativeMap.begin(); it != representativeMap.end(); )
        if (it->second.lastHeard <= currTime - repSeqPersistenceInterval) {
            expiredErasedReps[it->first] = it->second.sequencenumber;
            representativeMap.erase(it++);     // step1: it_tobe = it+1; step2: recentSetupReqFrom.erase(it); step3: it = it_tobe;
        } else
            it++;
    
    //
    // write message records to file if allSendRecords has become large
    // write node records to file
    //
    if (recordStatsToFile) {
        if (allSendRecords.size() > 10000)
            writeToResultMessageFile();
        if (allNodeRecords.size() > 10000)     // whichever node first sees size exceeding limit should write to node file
            writeToResultNodeFile();
    }
    
    scheduleFillVsetTimer();    // schedule the next self-message
}

VlrRingVID Vrr::findNextHop(VlrIntOption& vlrOption, VlrRingVID excludeVid/*=VLRRINGVID_NULL*/)
{
    VlrRingVID dstVid = vlrOption.getDstVid();
    // ASSERT(dstVid != vid);  // not supposed to find nexthop to myself, bc no other node is closer to me than myself
    if (dstVid == vid)
        EV_WARN << "findNextHop(dst= " << dstVid << ", excludeVid = " << excludeVid << ") at me=" << vid << ", dst==me" << endl;
    EV_INFO << "Finding nexthop for destination vid = " << dstVid << ", excludeVid = " << excludeVid << endl;

    // first check if dstVid in pset
    // NOTE we only route to pnei that had joined overlay since last time it became my LINKED pnei (hadBeenInNetwork=true), bc if a node hasn't joined overlay it can't do greedy routing; for vroute endpoint, even though we aren't sure if it's inNetwork, at least it has joined overlay
    auto nextVidResult = getClosestPneiTo(dstVid, excludeVid, /*checkInNetwork=*/false);
    VlrRingVID& nextVid = nextVidResult.first;
    unsigned int& minDistance = nextVidResult.second;
    // if no LINKED && inNetwork pnei in pset, we may still check vlrRoutingTable since pnei may be in inNetwork warmup -- vroute should be valid as long as it's not removed from vlrRoutingTable (due to teardown)
    if (nextVid == VLRRINGVID_NULL && minDistance != VLRRINGVID_MAX) {
        EV_WARN << "No pnei in PsetTable, hence no nexthop available" << endl;
        return VLRRINGVID_NULL;
    } else if (nextVid == dstVid) { // if excludeVid == dstVid, this can't happen
        EV_INFO << "Destination vid = " << dstVid << " found in PsetTable as nexthop" << endl;
        vlrOption.setCurrentPathid(VLRPATHID_INVALID);
        vlrOption.setTempPathid(VLRPATHID_INVALID);
        return dstVid;
    }
    VlrRingVID nexthopVid = nextVid;    // vid of a pnei to get to dstVid

    // // check if message was using a temporary route
    // VlrPathID tempPathid = vlrOption.getTempPathid();
    // if (tempPathid != VLRPATHID_INVALID) {
    //     VlrRingVID tempTowardVid = vlrOption.getTempTowardVid();
    //     auto tempPathItr = vlrRoutingTable.vlrRoutesMap.find(tempPathid);
    //     if (tempPathItr != vlrRoutingTable.vlrRoutesMap.end()) {   // tempPathid is in vlrRoutingTable
    //         if (tempTowardVid == vid) {     // reached the end of temporary route portion
    //             EV_INFO << "Reached tempTowardVid=" << tempTowardVid << " in temporary pathid=" << tempPathid << ", current pathid=" << vlrOption.getCurrentPathid() << endl;
    //         } else {    // send to next hop in temporary route
    //             VlrRingVID nextHopVid = vlrRoutingTable.getRouteItrNextHop(tempPathItr, tempTowardVid);
    //             EV_INFO << "Found nexthop in temporary pathid=" << tempPathid << ", tempTowardVid=" << tempTowardVid << ", current vroute towardVid=" << vlrOption.getTowardVid() << ", nextHopVid=" << nextHopVid << endl;
    //             return nextHopVid;
    //         }
    //     } else {    // tempPathid not found in vlrRoutingTable
    //         EV_WARN << "Current temporary pathid=" << tempPathid << " (tempTowardVid=" << tempTowardVid << ") selected at previous hop but not in VlrRingRoutingTable at current node = " << vid << ", selecting new vroute" << endl;
    //         if (prevHopVid != VLRRINGVID_NULL) {   // prevHopAddrPtr != nullptr
    //             // const L3Address& prevHopAddr = *prevHopAddrPtr;
    //             EV_INFO << "Sending Teardown to previous hop " << prevHopVid << " for pathid = " << tempPathid << endl;
    //             // tear down tempPathid, i.e. send Teardown to prevHopAddr
    //             auto teardownOut = createTeardownOnePathid(tempPathid, /*addSrcVset=*/false, /*rebuild=*/true);
    //             sendCreatedTeardown(teardownOut, /*nextHopVid=*/prevHopVid);

    //             if (recordStatsToFile) {   // record sent message
    //                 recordMessageRecord(/*action=*/0, /*src=*/vid, /*dst=*/VLRRINGVID_NULL, "Teardown", /*msgId=*/0, /*hopcount=*/0, /*chunkByteLength=*/0, /*infoStr=*/"findNextHop: tempPathid not found in vlrRoutingTable");   // unused params (msgId, hopcount, chunkByteLength) are just assigned to 0
    //             }
    //         }
    //         vlrOption.setCurrentPathid(VLRPATHID_INVALID);     // currPathid can no longer be followed bc temporary route is broken, select new vroute
    //         vlrOption.setTempPathid(VLRPATHID_INVALID);
    //     }
    // }

    // check if I can use rep path to dstVid, assume no loop in rep path    NOTE representativeMap shouldn't contain my own vid
    bool usingRepPath = false;
    if (!representativeMap.empty()) {
        auto repResult = getClosestRepresentativeTo(dstVid, excludeVid);
        // if (repResult.first == VLRRINGVID_NULL), then repResult.second = VLRRINGVID_MAX, if this node has any available pnei, minDistance < VLRRINGVID_MAX, won't be using rep-path
        if (std::get<0>(repResult) == dstVid) { // if excludeVid == dstVid, this can't happen
            nexthopVid = std::get<2>(repResult);
            EV_INFO << "Destination vid = " << dstVid << " found in representativeMap, setting nexthop to rep parent = " << nexthopVid << endl;
            vlrOption.setCurrentPathid(VLRPATHID_REPPATH);
            vlrOption.setTowardVid(dstVid);
            vlrOption.setTempPathid(VLRPATHID_INVALID);
            return nexthopVid;
        } else if (std::get<1>(repResult) < minDistance) {     // if no qualified rep was found as nextVid, minDistance = VLRRINGVID_MAX 
            minDistance = std::get<1>(repResult);
            nextVid = std::get<0>(repResult);
            nexthopVid = std::get<2>(repResult);
            usingRepPath = true;
        }
    }
    
    // checked message isn't using a temporary route
    // check if dstVid in vlrRoutingTable
    bool usingVroute = false;    // if we're routing with a vroute, need to ensure we use the same vroute if nextVid == towardVid
    auto rtResult = getClosestVendTo(dstVid, excludeVid);
    // if (rtResult.first == VLRRINGVID_NULL), then rtResult.second = VLRRINGVID_MAX, if this node has any available pnei, minDistance < VLRRINGVID_MAX, won't be using vroute; if this node has no LINKED && inNetwork pnei, minDistance = VLRRINGVID_MAX
    if (std::get<0>(rtResult) == dstVid) { // if excludeVid == dstVid, this can't happen
        EV_DEBUG << "Destination vid = " << dstVid << " found in VlrRingRoutingTable as towardVid" << endl;
        nextVid = dstVid;
        minDistance = 0;
        usingVroute = true;
    } else if (std::get<1>(rtResult) < minDistance) {     // if no qualified pnei was found as nextVid, minDistance = VLRRINGVID_MAX 
        minDistance = std::get<1>(rtResult);
        nextVid = std::get<0>(rtResult);
        usingVroute = true;
    }
    
    if (nextVid == VLRRINGVID_NULL) {
        EV_WARN << "No LINKED && inNetwork pnei or vroute endpoint exists for dstVid=" << dstVid << " (excludeVid = " << excludeVid << ")" << endl;
        return VLRRINGVID_NULL;
    }
    ASSERT(nextVid != excludeVid);
    // finally compare minDistance with my own distance to dst
    unsigned int meDstDistance = computeVidDistance(vid, dstVid);
    bool useNextVid = (minDistance < meDstDistance);
    // if (!useNextVid && minDistance == meDstDistance && nextVid != vid && getVid_CCW_Distance(dstVid, nextVid) < getVid_CCW_Distance(dstVid, vid))   // nextVid in ccw direction of dst, I'm in cw direction of dst
    //     useNextVid = true;
    if (useNextVid) {
        // if message was using a vroute (currPathid), ensure nextVid isn't farther from dst than towardVid -- only possible if currPathid isn't in my vlrRoutingTable, or next hop of currPathid is unavailable (i.e. a lost pnei)
        // VlrPathID currPathid = vlrOption.getCurrentPathid();
        // auto currPathItr = vlrRoutingTable.vlrRoutesMap.end();
        // unsigned int oldDistance = VLRRINGVID_MAX;       // distance btw towardVid and dst, only valid when currPathid != VLRPATHID_INVALID
        // if (currPathid != VLRPATHID_INVALID) {
        //     if (currPathid != VLRPATHID_REPPATH) {
        //         currPathItr = vlrRoutingTable.vlrRoutesMap.find(currPathid);
        //         // vroute selected at previous hop but not found at this hop, broken vroute, send teardown message to prevHopAddr about currPathid
        //         if (currPathItr == vlrRoutingTable.vlrRoutesMap.end()) {
        //             EV_WARN << "Current pathid=" << currPathid << " (towardVid=" << vlrOption.getTowardVid() << ") selected at previous hop but not in VlrRingRoutingTable at current node = " << vid << ", selecting new vroute" << endl;
        //             if (prevHopVid != VLRRINGVID_NULL) {   // prevHopAddrPtr != nullptr
        //                 if (tempPathid != VLRPATHID_INVALID) {  // this message arrived through temporary route, previous hop doesn't have currPathid, need to notify other end of tempPathid
        //                     auto tempPathItr = vlrRoutingTable.vlrRoutesMap.find(tempPathid);
        //                     if (tempPathItr != vlrRoutingTable.vlrRoutesMap.end()) {
        //                         VlrRingVID otherEnd = (vid == tempPathItr->second.fromVid) ? tempPathItr->second.toVid : tempPathItr->second.fromVid;
        //                         auto lostPneiItr = lostPneis.find(otherEnd);
        //                         if (lostPneiItr != lostPneis.end()) {
        //                             // const L3Address& prevHopAddr = lostPneiItr->second.address;  // send Teardown of currPathid to other end of tempPathid
        //                             EV_WARN << "Sending Teardown for unfound currPathid = " << currPathid << " through tempPathid=" << tempPathid << " to previous hop vid=" << otherEnd << endl;
        //                             auto teardownOut = createTeardownOnePathid(currPathid, /*addSrcVset=*/false, /*rebuild=*/true);
        //                             sendCreatedTeardownToNextHop(teardownOut, /*nextHopVid=*/otherEnd, /*nextHopIsUnavailable=*/2);

        //                             if (recordStatsToFile) {   // record sent message
        //                                 recordMessageRecord(/*action=*/0, /*src=*/vid, /*dst=*/VLRRINGVID_NULL, "Teardown", /*msgId=*/0, /*hopcount=*/0, /*chunkByteLength=*/0, /*infoStr=*/"findNextHop: currPathid not found in vlrRoutingTable");   // unused params (msgId, hopcount, chunkByteLength) are just assigned to 0
        //                             }
        //                         } else
        //                             EV_WARN << "Received Teardown for unfound currPathid = " << currPathid << " through tempPathid=" << tempPathid << " but otherEnd=" << otherEnd << " of temporary path not found in lostPneis" << endl;
        //                     } 
        //                 } else if (vlrOption.getTowardVid() != vid) {    // previous hop is directly connected, if towardVid is me, this may be a dismantled route not an inconsistency, also it's probably not a big deal when I don't have a route for which I'm the endpoint bc I'm the dst anyway
        //                     // const L3Address& prevHopAddr = *prevHopAddrPtr;
        //                     EV_WARN << "Sending Teardown for unfound currPathid = " << currPathid << " to previous hop " << prevHopVid << endl;
        //                     // tear down currPathid, i.e. send Teardown to prevHopAddr
        //                     auto teardownOut = createTeardownOnePathid(currPathid, /*addSrcVset=*/false, /*rebuild=*/true);
        //                     sendCreatedTeardown(teardownOut, /*nextHopVid=*/prevHopVid);

        //                     if (recordStatsToFile) {   // record sent message
        //                         recordMessageRecord(/*action=*/0, /*src=*/vid, /*dst=*/VLRRINGVID_NULL, "Teardown", /*msgId=*/0, /*hopcount=*/0, /*chunkByteLength=*/0, /*infoStr=*/"findNextHop: currPathid not found in vlrRoutingTable");   // unused params (msgId, hopcount, chunkByteLength) are just assigned to 0
        //                     }
        //                 }
        //             }
        //         }
        //     }
        //     // ensure nextVid isn't farther from dst than towardVid selected at previous hop, otherwise forwarding isn't greedy, will likely result in loop
        //         // if currPathid is found in my vlrRoutingTable, but next hop to towardVid is unavailable, towardVid may not be in endpointToRoutesMap, so nextVid returned by getClosestVendTo() may be farther from dst than towardVid
        //     if (vlrOption.getTowardVid() == nextVid)
        //         oldDistance = minDistance;
        //     else {
        //         oldDistance = computeVidDistance(vlrOption.getTowardVid(), dstVid);
        //         // NOTE if both towardVid and another node A equally close to towardVid are my pneis, getClosestPneiTo() may not prefer towardVid, but sending to a pnei equally close to dst as towardVid won't result in loop
        //         if (minDistance > oldDistance /*|| (minDistance == oldDistance && usingVroute)*/) {       // only allow nextVid to be towardVid selected at previous hop or closer to dst than towardVid
        //             EV_WARN << "No nexthop is closer to dst=" << dstVid << " than towardVid=" << vlrOption.getTowardVid() << " selected at previous hop ";
        //             if (prevHopVid != VLRRINGVID_NULL)  // prevHopAddrPtr != nullptr
        //                 EV_WARN << prevHopVid;
        //             EV_WARN << " (current pathid=" << currPathid << "), closest one found is nextVid=" << nextVid << " (usingVroute=" << usingVroute << ")" << endl;
        //             return VLRRINGVID_NULL;
        //         }
        //     }
        // }        
        if (!usingVroute) {     // using physical link
            ASSERT(nextVid != vid && nexthopVid != vid);    // psetTable and representativeMap shouldn't contain my own vid
            // if (usingMPnei) {   // nextVid in overheardMPneis
            //     nexthopVid = overheardMPneis[nextVid].first;
            //     EV_INFO << "Found nexthop to dstVid=" << dstVid << " in overheardMPneis, nextVid=" << nextVid << ", nexthopVid=" << nexthopVid << endl;
            //     vlrOption.setCurrentPathid(VLRPATHID_INVALID);
            // }
            if (usingRepPath) {  // nextVid is representative.vid
                EV_INFO << "Using representative as nexthop to dstVid=" << dstVid << ", nextVid=" << nextVid << ", nexthopVid=" << nexthopVid << endl;
                vlrOption.setCurrentPathid(VLRPATHID_REPPATH);
                vlrOption.setTowardVid(nextVid);
            } else {    // nextVid in PsetTable
                EV_INFO << "Found nexthop to dstVid=" << dstVid << " on a physical link, nextVid=" << nextVid << ", nexthopVid=" << nexthopVid << endl;
                vlrOption.setCurrentPathid(VLRPATHID_INVALID);
            }
            vlrOption.setTempPathid(VLRPATHID_INVALID);

            // if ((dstVid == 3026 || dstVid == 12954) && (vid == 1092 || vid == 3058) && simTime() > 550)
            //     EV_WARN << "ohno dstVid == "<< dstVid << ", usingVroute=" << usingVroute << ", usingRepPath=" << usingRepPath << ", nextVid=" << nextVid << ", nexthopVid=" << nexthopVid << ", psetTable=" << psetTable << ", repMap=" << printRepresentativeMapToString() << endl;
            return nexthopVid;
        } else {        // using vroute in VlrRingRoutingTable
            ASSERT(nextVid != vid);     // checked if minDistance == meDstDistance, vid != nextVid
            // if (nextVid == vid) {
            //     EV_WARN << "Closest nextVid found for dstVid=" << dstVid << " (excludeVid = " << excludeVid << ") is me=" << vid << endl;
            //     return VLRRINGVID_NULL;
            // }
            VlrPathID& nextPathid = std::get<2>(rtResult);
            // L3Address nextHopAddr;
            if (nextPathid == VLRPATHID_INVALID) {     // excludeVid can't be next hop AND allowTempRoute=true, vroute hasn't been selected yet
                bool usingCurrPathid = false;
                // if (currPathid != VLRPATHID_INVALID && currPathid != VLRPATHID_REPPATH) {    // message was using a vroute (currPathid)
                //     // if nextVid == towardVid (towardVid is still one of closest endpoints to dst), use currPathid if it is available
                //     if (vlrOption.getTowardVid() == nextVid) {
                //         if (vlrRoutingTable.findRouteEndInEndpointMap(currPathid, nextVid)) {    // next hop in currPathid to towardVid is available
                //             nextPathid = currPathid;
                //             nexthopVid = vlrRoutingTable.getRouteItrNextHop(currPathItr, nextVid);
                //             EV_INFO << "Found nexthop to dstVid=" << dstVid << " using current pathid=" << currPathid << ", towardVid=" << nextVid << ", nextHopVid=" << nexthopVid << endl;
                //             usingCurrPathid = true;
                //         }
                //     }
                // }
                if (!usingCurrPathid) {    // use a vroute that goes toward endpoint nextVid
                    int endpointRouteIndex = 0;     // always select the smallest pathid to reach nextVid
                    // use a random vrouteID in endpointToRoutesMap[nextVid] set
                    // int endpointRouteIndex = intuniform(0, vlrRoutingTable.getNumRoutesToEndInEndpointMap(nextVid) - 1);   // must be in range [0, set_size)
                    std::pair<VlrPathID, VlrRingVID> vrouteNhResult = vlrRoutingTable.getRouteToEndpoint(nextVid, /*endpointRouteIndex=*/endpointRouteIndex);
                    EV_INFO << "Found nexthop to dstVid=" << dstVid << " using new vroute pathid=" << vrouteNhResult.first << ", towardVid=" << nextVid << ", nextHopVid=" << vrouteNhResult.second << endl;
                    nextPathid = vrouteNhResult.first;
                    nexthopVid = vrouteNhResult.second;
                }
            } else {    // excludeVid may be next hop, so vroute has been selected to avoid using excludeVid as next hop
                nexthopVid = vlrRoutingTable.getRouteNextHop(nextPathid, nextVid);
                EV_INFO << "Found nexthop to dstVid=" << dstVid << " using selected pathid=" << nextPathid << ", towardVid=" << nextVid << ", nextHopVid=" << nexthopVid << endl;
            }
            // // Commented out to use another vroute to towardVid and hope this doesn't result in a loop :(
            //     // if nextVid isn't closer to dst than towardVid and we're still using vroute but we aren't using currPathid, this means currPathid isn't in my vlrRoutingTable, or next hop of currPathid is unavailable (i.e. a lost pnei)
            // if (currPathid != VLRPATHID_INVALID && minDistance == oldDistance && nextPathid != currPathid) {  // message was using a vroute (currPathid) to towardVid, now I'm still routing to towardVid but using a different vroute (nextPathid), this can result in a loop
            //     // if (currPathItr != vlrRoutingTable.vlrRoutesMap.end() && !vlrRoutingTable.getIsDismantledRoute(currPathItr->second.isUnavailable))     // only disallow changing currPathid if it is still in vlrRoutingTable but next hop is unavailable, bc it's likely to result in loop since I should have currPathid available
            //     EV_WARN << "No nexthop is closer to dst=" << dstVid << " than towardVid=" << vlrOption.getTowardVid() << " selected at previous hop ";
            //     if (prevHopVid != VLRRINGVID_NULL)  // prevHopAddrPtr != nullptr
            //         EV_WARN << prevHopVid;
            //     EV_WARN << " (currentPathid=" << currPathid << ") but currentPathid isn't found or next hop in currentPathid is unavailable, best pathid found is nextPathid=" << nextPathid << "(nextVid=" << nextVid << ")" << endl;
            //     return VLRRINGVID_NULL;
            // }
            
            // can double check nextHopAddr is not excludeVid here
            // check if nextHopAddr is a LINKED pnei or connected via temporary route
            vlrOption.setCurrentPathid(nextPathid);
            vlrOption.setTowardVid(nextVid);
            if (vlrRoutingTable.isRouteItrNextHopAvailable(vlrRoutingTable.vlrRoutesMap.find(nextPathid), nextVid) == 0) {    // if next hop in nextPathid is unavailable
                EV_WARN << "Selected nexthop to dstVid=" << dstVid << " at me=" << vid << " using pathid=" << nextPathid << ", towardVid=" << nextVid << ", nextHopVid=" << nexthopVid << " but next hop is unavailable" << endl;
                return VLRRINGVID_NULL;
            }
            // if (allowTempRoute && vlrRoutingTable.isRouteItrNextHopAvailable(vlrRoutingTable.vlrRoutesMap.find(nextPathid), nextVid) == 2) {    // vroute uses temporary route; only if allowTempRoute=true can the selected vroute be using a temporary route
            //     // VlrRingVID lostPneiVid = getVidFromAddressInRegistry(nextHopAddr);    // map nextHopAddr L3Address (lost pnei) to VlrRingVID
            //     VlrRingVID lostPneiVid = nexthopVid;
            //     auto lostPneiItr = lostPneis.find(lostPneiVid);
            //     if (lostPneiItr != lostPneis.end()) {
            //         tempPathid = *(lostPneiItr->second.tempVlinks.begin());
            //         nexthopVid = vlrRoutingTable.getRouteNextHop(tempPathid, lostPneiVid);
            //         vlrOption.setTempPathid(tempPathid);
            //         vlrOption.setTempTowardVid(lostPneiVid);
            //     } else {
            //         auto nextPathItr = vlrRoutingTable.vlrRoutesMap.find(nextPathid);
            //         if (nextPathItr == vlrRoutingTable.vlrRoutesMap.end())
            //             EV_WARN << "Selected nextPathid doesn't exist in vlrRoutingTable!" << endl;
            //         else
            //             EV_WARN << "Selected nextPathid=" << nextPathid << ": isVsetRoute=" << nextPathItr->second.isVsetRoute << ", isUnavailable=" << (int)nextPathItr->second.isUnavailable << ", prevhopRepairRouteSent=" << nextPathItr->second.prevhopRepairRouteSent << ", prevhopVids=" << nextPathItr->second.prevhopVids << endl;
            //         EV_WARN << "Selected nexthop to dstVid=" << dstVid << " using pathid=" << nextPathid << ", towardVid=" << nextVid << ", nextHopVid=" << lostPneiVid << " connected via temporary route, but not found in lostPneis" << endl;
            //         return VLRRINGVID_NULL;
            //     }
            // } else  // nextHopAddr is a LINKED pnei
            vlrOption.setTempPathid(VLRPATHID_INVALID);
            // if ((dstVid == 3026 || dstVid == 12954) && (vid == 1092 || vid == 3058) && simTime() > 550)
            //     EV_WARN << "ohno dstVid == " << dstVid << ", usingVroute=" << usingVroute << ", using pathid=" << nextPathid << ", towardVid=" << nextVid << ", nexthopVid=" << nexthopVid << ", vlrRoutingTable=" << vlrRoutingTable << endl;
            return nexthopVid;
        }
    } else {     // minDistance > my own distance to dst, or minDistance == my own distance and nextVid != my vid and nextVid in ccw direction of dst
        EV_WARN << "No nexthop is closer or equally close as me = " << vid << " to dstVid=" << dstVid << ", closest one found is nextVid=" << nextVid << ", usingVroute=" << usingVroute << endl;
        return VLRRINGVID_NULL;
    }
}

// for routing SetupReply/SetupFail, first check if newnode is a LINKED pnei, if so send SetupReply to it directly
VlrRingVID Vrr::findNextHopForSetupReply(VlrIntOption& vlrOption, VlrRingVID newnode)
{
    const std::map<VlrRingVID, PsetTable<VlrRingVID>::PsetTableValue>& vidMap = psetTable.vidToStateMap;
    EV_DEBUG << "Finding nexthop for SetupReply/SetupFail: newnode = " << newnode << endl;
    auto itr = vidMap.find(newnode);        // itr type: std::map<...>::const_iterator bc this const function can't change vidMap
    if (itr != vidMap.cend() && itr->second.state == PNEI_LINKED) { // newnode is a LINKED pnei
        EV_INFO << "Found nexthop for SetupReply/SetupFail: newnode = " << newnode << " is a LINKED pnei" << endl;
        vlrOption.setCurrentPathid(VLRPATHID_INVALID);
        vlrOption.setTempPathid(VLRPATHID_INVALID);
        return newnode;
    }
    // newnode isn't LINKED in my pset
    if (vlrOption.getDstVid() == vid) {    // I'm the proxy, but newnode isn't a LINKED pnei
        EV_WARN << "No next hop because me = " << vid << " is the proxy but newnode = " << newnode << " is not a LINKED pnei" << endl;
        return VLRRINGVID_NULL;
    }

    // continue to find nexthop to proxy
    return findNextHop(vlrOption, /*excludeVid=*/VLRRINGVID_NULL);
}

// for routing SetupReply/SetupFail, check if any node along the trace before myself is a LINKED pnei, if so return its index in trace
// if preferShort=false, use the last node who's a LINKED pnei before myself in trace
// nodes after and including myself will be removed from trace; if no LINKED pnei found before myself, return originalTrace.size()
// trace contains [dst, .., nextHop, (me, skipped nodes..)]
unsigned int Vrr::getNextHopIndexInTrace(VlrIntVidVec& trace, bool preferShort/*=true*/) const
{
    EV_DEBUG << "Finding nexthop for SetupReply/SetupFail with trace: " << trace << endl;
    // std::set<VlrRingVID> linkedPneis = psetTable.getPneisLinkedSet();
    unsigned int nextHopIndex = trace.size();
    bool nextHopFound = false;
    unsigned int i;
    for (i = 0; i < trace.size(); i++) {   // check every node in trace except the last one
        if (trace[i] == vid)        // trace[i] is myself
            break;
        else if ((!nextHopFound || !preferShort) && psetTable.pneiIsLinked(trace[i])) {    // trace[i] is a LINKED pnei
            nextHopIndex = i;
            nextHopFound = true;
        }
    }
    trace.erase(trace.begin()+i, trace.end());      // erase nodes after and including myself from trace
    return nextHopIndex;
}

//
// get a LINKED (&& inNetwork if checkInNetwork=true) pnei closest to targetVid (or targetVid itself if it's a LINKED pnei) with wrap-around searching in map, also return its distance to targetVid from computeVidDistance()
// if checkInNetwork=false, get a LINKED && hadBeenInNetwork pnei
// excludeVid won't be considered as the closest pnei, if no node excluded, excludeVid = VLRRINGVID_NULL
// return (VLRRINGVID_NULL, 0) if no pnei in pset -- no need to check vroutes
// return (VLRRINGVID_NULL, VLRRINGVID_MAX) if no LINKED && inNetwork pnei in pset -- can still check vroutes, pnei may be in inNetwork warmup, if pnei is no longer available I should've received Teardown for its vroutes
//
std::pair<VlrRingVID, unsigned int> Vrr::getClosestPneiTo(VlrRingVID targetVid, VlrRingVID excludeVid, bool checkInNetwork/*=false*/) const
{
    // result.first: closestVid, result.second: distance btw closestVid and targetVid
    // largest distance btw two nodes in a ring with largest allowable vid = VLRRINGVID_MAX is VLRRINGVID_MAX/2, considering distance = min(cwDist, ccwDist)
    std::pair<VlrRingVID, unsigned int> result = std::make_pair(VLRRINGVID_NULL, VLRRINGVID_MAX);
    const std::map<VlrRingVID, PsetTable<VlrRingVID>::PsetTableValue>& vidMap = psetTable.vidToStateMap;
    EV_DEBUG << "Finding LINKED pnei closest to target vid = " << targetVid << ", excludeVid = " << excludeVid << ", checkInNetwork = " << checkInNetwork << endl;
    if (vidMap.empty()) {
        EV_DEBUG << "No physical neighbour in PsetTable" << endl;
        result.second = 0;  // signify "no pnei in pset" or "no need to check vlrRoutingTable"
        return result;
    }
    auto itr = vidMap.lower_bound(targetVid);  // itr type: std::map<...>::const_iterator bc this const function can't change vidMap
    if (itr != vidMap.cend()) {
        if (itr->first == targetVid && itr->second.state == PNEI_LINKED && itr->second.hadBeenInNetwork && itr->first != excludeVid) {     // targetVid is a LINKED pnei
            EV_DEBUG << "Found LINKED pnei " << targetVid << " in PsetTable" << endl;
            result.first = targetVid;
            return result;
        }
    }
    else        // if (itr == vidMap.cend())
        itr = vidMap.cbegin();  // ensure itr->first is valid, since we've checked vidToStateMap isn't empty
    
    // define a lambda function to determine if a pnei is LINKED (&& inNetwork if checkInNetwork=true) && != excludeVid
    auto pneiItrQualified = [excludeVid, checkInNetwork](std::map<VlrRingVID, PsetTable<VlrRingVID>::PsetTableValue>::const_iterator pneiItr) 
    { 
        return (pneiItr->second.state == PNEI_LINKED && pneiItr->second.hadBeenInNetwork && (!checkInNetwork || pneiItr->second.inNetwork) && pneiItr->first != excludeVid);
    };

    VlrRingVID& closestPnei = result.first;
    unsigned int& minDistance = result.second;
    // minDistance = VLRRINGVID_MAX;
    auto itrlow = itr;
    advanceIteratorWrapAround(itrlow, -1, vidMap.cbegin(), vidMap.cend());  // NOTE itrlow is const_iterator, so begin and end also need to be const_iterator --> cbegin() and cend()
    if (itr->first == targetVid)    // targetVid is an unqualified pnei
        advanceIteratorWrapAround(itr, 1, vidMap.cbegin(), vidMap.cend());
    
    // itr now points to a pnei in cw direction of targetVid, itrlow points to a pnei in ccw direction, both not equal targetVid

    // use itrlow to get a qualified pnei in ccw direction of targetVid
    bool isPneiItrQualified = false;
    while (itrlow != itr && !isPneiItrQualified) {
        isPneiItrQualified = pneiItrQualified(itrlow);
        if (!isPneiItrQualified)    // if itrlow->first is a qualified vid, itrlow still will point to the qualified vid after while loop
            advanceIteratorWrapAround(itrlow, -1, vidMap.cbegin(), vidMap.cend());
        else {
            closestPnei = itrlow->first;
            minDistance = computeVidDistance(targetVid, closestPnei);
            EV_DEBUG << "Found LINKED pnei " << closestPnei << " in ccw direction of target vid=" << targetVid << " in PsetTable" << endl;
        }
    }
    // check itr, since while loop condition for checking itrlow is itrlow != itr
    isPneiItrQualified = pneiItrQualified(itr);
    if (isPneiItrQualified) {
        unsigned int distance = computeVidDistance(targetVid, itr->first);
        if (distance < minDistance) {
            closestPnei = itr->first;
            minDistance = distance;
            EV_DEBUG << "Found LINKED pnei " << closestPnei << " in cw direction of target vid=" << targetVid << " in PsetTable" << endl;
        }
    } else if (itr != itrlow)   // if itr == itrlow, then every vid has been checked except the one pointed by itr, now every vid has been checked
        advanceIteratorWrapAround(itr, 1, vidMap.cbegin(), vidMap.cend());
    // use itr to get a qualified pnei in cw direction of targetVid
    while (itr != itrlow && !isPneiItrQualified) {
        isPneiItrQualified = pneiItrQualified(itr);
        if (!isPneiItrQualified)
            advanceIteratorWrapAround(itr, 1, vidMap.cbegin(), vidMap.cend());
        else {
            unsigned int distance = computeVidDistance(targetVid, itr->first);
            if (distance < minDistance) {
                closestPnei = itr->first;
                minDistance = distance;
                EV_DEBUG << "Found LINKED pnei " << closestPnei << " in cw direction of target vid=" << targetVid << " in PsetTable" << endl;
            }
        }
    }
   
    if (closestPnei != VLRRINGVID_NULL) {
        EV_DEBUG << "Found LINKED pnei " << closestPnei << " in PsetTable that is closest to vid = " << targetVid << ", distance = " << minDistance << endl;
    }  
    else
        EV_DEBUG << "No LINKED pnei in PsetTable" << endl;
    
    return result;
}

//
// get a rep closest to targetVid with wrap-around searching in map, also return its distance to targetVid from computeVidDistance(), and next hop in rep-path
// excludeVid won't be considered, if no node excluded, excludeVid = VLRRINGVID_NULL
//
std::tuple<VlrRingVID, unsigned int, VlrRingVID> Vrr::getClosestRepresentativeTo(VlrRingVID targetVid, VlrRingVID excludeVid) const
{
    // result.first: closestVid, result.second: distance btw closestVid and targetVid
    // largest distance btw two nodes in a ring with largest allowable vid = VLRRINGVID_MAX is VLRRINGVID_MAX/2, considering distance = min(cwDist, ccwDist)
    std::tuple<VlrRingVID, unsigned int, VlrRingVID> result = std::make_tuple(VLRRINGVID_NULL, VLRRINGVID_MAX, VLRRINGVID_NULL);
    const std::map<VlrRingVID, Representative>& vidMap = representativeMap;
    EV_DEBUG << "Finding rep closest to target vid = " << targetVid << ", excludeVid = " << excludeVid << endl;
    if (vidMap.empty()) {
        EV_DEBUG << "No rep in representativeMap" << endl;
        return result;
    }

    VlrRingVID& closestPnei = std::get<0>(result);
    unsigned int& minDistance = std::get<1>(result);
    VlrRingVID& closestNextHop = std::get<2>(result);

    // define a lambda function to determine if a rep != excludeVid and hasn't expired and next hop in rep-path is LINKED
    simtime_t expiredLastheard = simTime() - repSeqValidityInterval;
    auto repMapItrQualified = [excludeVid, expiredLastheard](std::map<VlrRingVID, Representative>::const_iterator repMapItr) 
    { 
        return (repMapItr->first != excludeVid && repMapItr->second.heardfromvid != VLRRINGVID_NULL /*&& repMapItr->second.heardfromvid != excludeVid*/ && repMapItr->second.lastHeard > expiredLastheard);
    };

    auto itr = vidMap.lower_bound(targetVid);  // itr type: std::map<...>::const_iterator bc this const function can't change vidMap
    if (itr != vidMap.cend()) {
        if (itr->first == targetVid && repMapItrQualified(itr)) {     // targetVid is a valid rep
            EV_DEBUG << "Found rep " << targetVid << " in representativeMap" << endl;
            closestPnei = targetVid;
            minDistance = 0;
            closestNextHop = itr->second.heardfromvid;
            return result;
        }
    }
    else        // if (itr == vidMap.cend())
        itr = vidMap.cbegin();  // ensure itr->first is valid, since we've checked vidToStateMap isn't empty


    auto itrlow = itr;
    advanceIteratorWrapAround(itrlow, -1, vidMap.cbegin(), vidMap.cend());  // NOTE itrlow is const_iterator, so begin and end also need to be const_iterator --> cbegin() and cend()
    if (itr->first == targetVid)    // targetVid is an unqualified pnei
        advanceIteratorWrapAround(itr, 1, vidMap.cbegin(), vidMap.cend());
    
    // itr now points to a pnei in cw direction of targetVid, itrlow points to a pnei in ccw direction, both not equal targetVid

    // use itrlow to get a qualified pnei in ccw direction of targetVid
    bool isPneiItrQualified = false;
    while (itrlow != itr && !isPneiItrQualified) {
        isPneiItrQualified = repMapItrQualified(itrlow);
        if (!isPneiItrQualified)    // if itrlow->first is a qualified vid, itrlow still will point to the qualified vid after while loop
            advanceIteratorWrapAround(itrlow, -1, vidMap.cbegin(), vidMap.cend());
        else {
            closestPnei = itrlow->first;
            minDistance = computeVidDistance(targetVid, closestPnei);
            closestNextHop = itrlow->second.heardfromvid;
            EV_DEBUG << "Found qualified rep " << closestPnei << " in ccw direction of target vid=" << targetVid << " in representativeMap" << endl;
        }
    }
    // check itr, since while loop condition for checking itrlow is itrlow != itr
    isPneiItrQualified = repMapItrQualified(itr);
    if (isPneiItrQualified) {
        unsigned int distance = computeVidDistance(targetVid, itr->first);
        if (distance < minDistance) {
            closestPnei = itr->first;
            minDistance = distance;
            closestNextHop = itr->second.heardfromvid;
            EV_DEBUG << "Found qualified rep " << closestPnei << " in cw direction of target vid=" << targetVid << " in representativeMap" << endl;
        }
    } else if (itr != itrlow)   // if itr == itrlow, then every vid has been checked except the one pointed by itr, now every vid has been checked
        advanceIteratorWrapAround(itr, 1, vidMap.cbegin(), vidMap.cend());
    // use itr to get a qualified pnei in cw direction of targetVid
    while (itr != itrlow && !isPneiItrQualified) {
        isPneiItrQualified = repMapItrQualified(itr);
        if (!isPneiItrQualified)
            advanceIteratorWrapAround(itr, 1, vidMap.cbegin(), vidMap.cend());
        else {
            unsigned int distance = computeVidDistance(targetVid, itr->first);
            if (distance < minDistance) {
                closestPnei = itr->first;
                minDistance = distance;
                closestNextHop = itr->second.heardfromvid;
                EV_DEBUG << "Found qualified rep " << closestPnei << " in cw direction of target vid=" << targetVid << " in representativeMap" << endl;
            }
        }
    }
   
    if (closestPnei != VLRRINGVID_NULL) {
        EV_DEBUG << "Found qualified rep " << closestPnei << " in representativeMap that is closest to vid = " << targetVid << ", distance = " << minDistance << endl;
    }  
    else
        EV_DEBUG << "No qualified rep in representativeMap" << endl;
    
    return result;
}

//
// get a vroute endpoint closest to targetVid (or targetVid itself if it's an endpoint, may return my own vid) with wrap-around searching in map, return VLRRINGVID_NULL if no qualified endpoints in vlrRoutingTable
// excludeVid won't be considered as next endpoint, but can be returned as the next physical hop, if no node excluded, excludeVid = VLRRINGVID_NULL
// result<0>: closest endpoint, result<1>: distance btw result<0> and targetVid, result<2>: vroute pathid to result<0>, VLRPATHID_INVALID if any vroute to result<0> is ok
// NOTE may not return my own vid if I'm the closest to targetVid (my own vid exists as endpoint in routing table)
//
std::tuple<VlrRingVID, unsigned int, VlrPathID> Vrr::getClosestVendTo(VlrRingVID targetVid, VlrRingVID excludeVid) const
{
    std::tuple<VlrRingVID, unsigned int, VlrPathID> result = std::make_tuple(VLRRINGVID_NULL, VLRRINGVID_MAX, VLRPATHID_INVALID);
    const std::map<VlrRingVID, std::set<VlrPathID>>& vidMap = vlrRoutingTable.endpointToRoutesMap;
    EV_DEBUG << "Finding vroute endpoint closest to target vid = " << targetVid << ", excludeVid = " << excludeVid << endl;
    if (vidMap.empty()) {
        EV_DEBUG << "No vroute endpoint in VlrRingRoutingTable" << endl;
        return result;
    }
    VlrRingVID& closestVend = std::get<0>(result);
    unsigned int& minDistance = std::get<1>(result);
    VlrPathID& closestPathid = std::get<2>(result);
    VlrPathID nextPathid = VLRPATHID_INVALID;       // selected pathid to reach selected endpoint
    
    // bool isExcludePnei = (excludeVid != VLRRINGVID_NULL);   // true if excludeVid could be a prevhop/nexthop in a vroute at this node
    // bool isExcludeLinked = psetTable.pneiIsLinked(excludeVid);  // true if excludeVid is currently a LINKED pnei
    // // prevhop/nexthop in a vroute may be connected by a temporary route (in which case it's a lost pnei) and not my LINKED pnei
    // isExcludePnei = isExcludePnei && (isExcludeLinked || lostPneis.find(excludeVid) != lostPneis.end());
    // // L3Address excludeAddr;      // L3Address of excludeVid
    // // if (isExcludePnei) {
    // //     if (isExcludeLinked)
    // //         excludeAddr = psetTable.getPneiL3Address(excludeVid);
    // //     else    // excludeVid is in lostPneis
    // //         excludeAddr = lostPneis.at(excludeVid).address;
    // }
        
    // const VlrPathID& currPathid = vlrOption.getCurrentPathid();
    // VlrRingVID towardVid = vlrOption.getTowardVid();
    // bool usingVroute = (currPathid != VLRPATHID_INVALID);   // if previous hop routed the message on a vroute
    

    // define a lambda function to determine if an endpoint != excludeVid, but next hop can be excludeVid
    auto vendItrQualified = [this, excludeVid, /*isExcludePnei,*/ &nextPathid](std::map<VlrRingVID, std::set<VlrPathID>>::const_iterator vendItr) -> bool
    {
        if (vendItr->first == excludeVid)
            return false;
        if (vendItr->first == vid)  // next hop to myself isUnspecified()
            return true;    // allow function to return my own vid
        // if (!isExcludePnei) // if return true here, nextPathid should be VLRPATHID_INVALID, and every vend at this node should return true here, so no need to worry one vend set nextPathid, then later vend return here but nextPathid isn't set back to VLRPATHID_INVALID
        return true;
        // // // check if excludeVid is the next hop toward vend
        // // if (usingVroute && vendItr->first == towardVid) {
        // //     // NOTE if next hop of currPathid is unavailable, currPathid won't be in endpointToRoutesMap[towardVid]
        // //     if ((vendItr->second).find(currPathid) != (vendItr->second).end()) {    // next hop in currPathid isn't unavailable
        // //         const auto vrouteItr = vlrRoutingTable.vlrRoutesMap.find(currPathid);
        // //         // if currPathid in endpointToRoutesMap and its next hop toward vend isn't excludeVid AND
        // //         // if allowTempRoute=false, next hop in currPathid doesn't use temporary route, if allowTempRoute=true AND next hop in currPathid uses temporary route, currPathid can't be dismantled (dismantled route shouldn't use temporary route but currPathid might have expired but not removed yet)
        // //         bool nextHopLinked = (vlrRoutingTable.isRouteItrNextHopAvailable(vrouteItr, vendItr->first) == 1);
        // //         if ((!isExcludePnei || vlrRoutingTable.getRouteItrNextHop(vrouteItr, vendItr->first) != excludeVid) && ((allowTempRoute && (nextHopLinked || !vlrRoutingTable.getIsDismantledRoute(vrouteItr->second.isUnavailable))) || nextHopLinked)) {
        // //             nextPathid = currPathid;
        // //             return true;
        // //         }
        // //     }
        // //     // if don't allow changing currPathid when selected endpoint == towardVid (to eliminate possibility of loop), return false here
        // //     // return false;
        // // }
        // int randomRouteIndex = 0;   // select first qualified vroute in vendItr->second
        // // int randomRouteIndex = intuniform(0, vendItr->second.size()-1);   // to avoid selecting the same vroute to vendItr->first each time, select last qualified vroute before (and including) vendItr->second[randomRouteIndex]
        // int routeIndex = 0;   // to avoid selecting the same vroute to vendItr->first each time, select last qualified vroute before vendItr->second[randomRouteIndex]
        // bool foundQualifiedVend = false;
        // for (const auto& pathid : vendItr->second) {
        //     // as long as there's one vroute to vend whose next hop isn't excludeVid and satisfies allowTempRoute requirement, vend is qualified
        //     const auto vrouteItr = vlrRoutingTable.vlrRoutesMap.find(pathid);
        //     // bool nextHopLinked = (vlrRoutingTable.isRouteItrNextHopAvailable(vrouteItr, vendItr->first) == 1);
        //     if (!isExcludePnei || vlrRoutingTable.getRouteItrNextHop(vrouteItr, vendItr->first) != excludeVid) {   // && ((allowTempRoute && (nextHopLinked || !vlrRoutingTable.getIsDismantledRoute(vrouteItr->second.isUnavailable))) || nextHopLinked)
        //         nextPathid = pathid;
        //         foundQualifiedVend = true;
        //         // prioritize vroute that doesn't use temporary route toward vend
        //         // if (allowTempRoute && vlrRoutingTable.isRouteItrNextHopAvailable(vrouteItr, vendItr->first)==2) // next hop uses temporary route, keep checking if other vroutes doesn't use temporary route toward vend
        //         //     ;
        //         // else    // next hop doesn't use temporary route, this vroute is no need to check other vroutes toward vend
        //         //     return true;

        //         // break loop as long as a qualified is found
        //         // return true;
        //     }
        //     // select last qualified vroute before (and including) vendItr->second[randomRouteIndex]
        //     if (foundQualifiedVend && routeIndex >= randomRouteIndex)   // routeIndex reached randomRouteIndex and I've found a qualified vroute
        //         return true;
            
        //     routeIndex++;
        // }
        // if (foundQualifiedVend)
        //     return true;

        // return false;
    };

    auto itr = vidMap.lower_bound(targetVid);  // itr type: std::map<...>::const_iterator bc this const function can't change vidMap
    if (itr != vidMap.cend()) {
        if (itr->first == targetVid && vendItrQualified(itr)) {     // targetVid is in vidMap, return targetVid only if targetVid != excludeVid
            EV_DEBUG << "Found target vid=" << targetVid << " in VlrRingRoutingTable" << endl;
            closestVend = targetVid;
            closestPathid = nextPathid;
            return result;
        }
    }
    else  // if (itr == vidMap.cend())
        itr = vidMap.cbegin();      // ensure itr->first is valid, since we've checked vidMap isn't empty
    
    auto itrlow = itr;
    advanceIteratorWrapAround(itrlow, -1, vidMap.cbegin(), vidMap.cend());  // NOTE itrlow is const_iterator, so begin and end also need to be const_iterator --> cbegin() and cend()
    if (itr->first == targetVid)
        advanceIteratorWrapAround(itr, 1, vidMap.cbegin(), vidMap.cend());
    
    // itr now points to a vend in cw direction of targetVid, itrlow points to a vend in ccw direction, both not equal targetVid

    // use itrlow to get a qualified vend in ccw direction of targetVid
    bool isVendItrQualified = false;
    while (itrlow != itr && !isVendItrQualified) {
        isVendItrQualified = vendItrQualified(itrlow);
        if (!isVendItrQualified)    // if itrlow->first is a qualified vend, itrlow still will point to the qualified vend after while loop
            advanceIteratorWrapAround(itrlow, -1, vidMap.cbegin(), vidMap.cend());
        else {
            closestVend = itrlow->first;
            minDistance = computeVidDistance(targetVid, closestVend);
            closestPathid = nextPathid;
            EV_DEBUG << "Found qualified vroute endpoint " << closestVend << " in ccw direction of target vid=" << targetVid << " in VlrRingRoutingTable" << endl;
        }
    }
    // check itr, since while loop condition for checking itrlow is itrlow != itr
    isVendItrQualified = vendItrQualified(itr);
    if (isVendItrQualified) {
        unsigned int distance = computeVidDistance(targetVid, itr->first);
        if (distance < minDistance) {
            closestVend = itr->first;
            minDistance = distance;
            closestPathid = nextPathid;
            EV_DEBUG << "Found qualified vroute endpoint " << closestVend << " in cw direction of target vid=" << targetVid << " in VlrRingRoutingTable" << endl;
        }
        // else if (distance == minDistance && usingVroute && itr->first == towardVid) {
        //     // if towardVid has the same distance to targetVid as closestVend found so far, use towardVid
        //     closestVend = itr->first;
        //     closestPathid = nextPathid;
        //     EV_DEBUG << "Found current vroute endpoint " << closestVend << " (vlrOption->towardVid) in cw direction of target vid=" << targetVid << " in VlrRingRoutingTable" << endl;
        // }
    } else if (itr != itrlow)   // if itr == itrlow, then every vend has been checked except the one pointed by itr, now every vend has been checked
        advanceIteratorWrapAround(itr, 1, vidMap.cbegin(), vidMap.cend());
    // use itr to get a qualified vend in cw direction of targetVid
    while (itr != itrlow && !isVendItrQualified) {
        isVendItrQualified = vendItrQualified(itr);
        if (!isVendItrQualified)
            advanceIteratorWrapAround(itr, 1, vidMap.cbegin(), vidMap.cend());
        else {
            unsigned int distance = computeVidDistance(targetVid, itr->first);
            if (distance < minDistance) {
                closestVend = itr->first;
                minDistance = distance;
                closestPathid = nextPathid;
                EV_DEBUG << "Found qualified vroute endpoint " << closestVend << " in cw direction of target vid=" << targetVid << " in VlrRingRoutingTable" << endl;
            }
            // else if (distance == minDistance && usingVroute && itr->first == towardVid) {
            //     // if towardVid has the same distance to targetVid as closestVend found so far, use towardVid
            //     closestVend = itr->first;
            //     closestPathid = nextPathid;
            //     EV_DEBUG << "Found current vroute endpoint " << closestVend << " (vlrOption->towardVid) in cw direction of target vid=" << targetVid << " in VlrRingRoutingTable" << endl;
            // }
        }
    }
   
    if (closestVend != VLRRINGVID_NULL) {
        EV_DEBUG << "Found vroute endpoint " << closestVend << " in VlrRingRoutingTable that is closest to vid = " << targetVid << ", distance = " << minDistance << ", pathid = " << closestPathid << endl;
    }  
    else
        EV_DEBUG << "No qualified vroute endpoint in VlrRingRoutingTable that is closest to vid = " << targetVid << endl;
    
    return result;
}

std::set<VlrRingVID> Vrr::convertVsetToSet() const
{
    std::set<VlrRingVID> vsetSet;
    for (auto& vnei : vset) 
        vsetSet.insert(vnei.first);
    return vsetSet;
}

std::string Vrr::printVsetToString() const
{
    std::string str ("{");      // must use double quotes as string constructed from c-string (null-terminated character sequence): string (const char* s)
    for (auto& vnei : vset) {
        // str += std::to_string(vnei) + ' ';
        str += std::to_string(vnei.first) + ' ';
    }
    str += '}';
    return str;
}

std::string Vrr::printPendingVsetToString() const
{
    std::ostringstream s;
    s << '{';
    for (auto& pair : pendingVset) {
        s << pair.first;
        if (pair.second != nullptr && pair.second->isScheduled())
            s << ':' << pair.second->getRetryCount();
        s << ' ';
    }
    s << '}';
    return s.str();
}

std::string Vrr::printRepresentativeMapToString() const
{
    std::ostringstream s;
    s << '{';
    for (auto& rep : representativeMap) {
        s << rep.first << ":s" << rep.second.sequencenumber << ":h" << rep.second.lastHeard << ' ';
    }
    s << '}';
    return s.str();
}

std::string Vrr::printVlrOptionToString(const VlrIntOption& vlrOption) const
{
    std::ostringstream s;
    s << '{';
    s << "dst=" << vlrOption.getDstVid() << " toward=" << vlrOption.getTowardVid() << " currpath=" << vlrOption.getCurrentPathid();
    s << '}';
    return s.str();
}

// print pathids in vlrRoutingTable whose toVid is me
std::string Vrr::printRoutesToMeToString() const
{
    std::string str ("{");      // must use double quotes as string constructed from c-string (null-terminated character sequence): string (const char* s)
    const auto epMapItr = vlrRoutingTable.endpointToRoutesMap.find(vid);
    if (epMapItr != vlrRoutingTable.endpointToRoutesMap.end()) {
        for (const auto& pathid : epMapItr->second) {
            auto vrouteItr = vlrRoutingTable.vlrRoutesMap.find(pathid);
            ASSERT(vrouteItr != vlrRoutingTable.vlrRoutesMap.end());    // pathid in endpointToRoutesMap should exist in vlrRoutingTable
            // bool isTemporaryRoute = vlrRoutingTable.getIsTemporaryRoute(vrouteItr->second.isUnavailable);
            // bool isDismantledRoute = vlrRoutingTable.getIsDismantledRoute(vrouteItr->second.isUnavailable);
            if (vrouteItr->second.toVid == vid) {       // vroute.toVid == me, wanted (i.e. not patched)
                // str += std::to_string(pathid) + ' ';
                // record for each vroute whose toVid == me "fromVid:toVid:hopcount "
                std::ostringstream s;
                s << vrouteItr->second.fromVid << ':' << vrouteItr->second.toVid << ':' << vrouteItr->second.hopcount << ' ';
                str += s.str();
            }
        }
    }
    str += '}';
    return str;
}

void Vrr::writeRoutingTableToFile()
{
    EV_DEBUG << "Processing writeRoutingTableTimer at node " << vid << endl;
    const char *fname = par("routingTableVidCSVFile");
    std::ofstream resultFile;
    if (!routingTableVidCSVFileCreated)     // I'm the first node to write to this file
        resultFile.open(fname, std::ofstream::out);   // open in write mode
    else 
        resultFile.open(fname, std::ofstream::app);   // open in append mode
        
    if (!resultFile.is_open()) {
        std::string filename(fname);
        // char errmsg[80] = "Unable to open routingTable csv file ";
        // throw cRuntimeError(strcat(errmsg, fname));
        EV_WARN << "Unable to open routingTable csv file " << fname << endl;
    } else {    // write to file
        if (!routingTableVidCSVFileCreated) {    // initialize file with header line
            // resultFile << "vid,time" << endl;
            routingTableVidCSVFileCreated = true;
        }
        resultFile << "Node " << vid << ',' << simTime() << endl;

        for (auto vrouteItr = vlrRoutingTable.vlrRoutesMap.begin(); vrouteItr != vlrRoutingTable.vlrRoutesMap.end(); ++vrouteItr) {
            // bool isTemporaryRoute = vlrRoutingTable.getIsTemporaryRoute(vrouteItr->second.isUnavailable);
            // bool isDismantledRoute = vlrRoutingTable.getIsDismantledRoute(vrouteItr->second.isUnavailable);
            // if (!isTemporaryRoute && !isDismantledRoute) {
                // record for each vroute whose toVid == me "[pathid],fromVid,toVid,prevhopVid,nexthopVid\n"
            // resultFile << '[' << vrouteItr->first << "],";
            resultFile << vrouteItr->second.fromVid << ',' << vrouteItr->second.toVid << ',';
            resultFile << vrouteItr->second.prevhopVid << ',' << vrouteItr->second.nexthopVid << endl;
        }
        resultFile.close();
    }
}


}; // namespace omnetvlr

