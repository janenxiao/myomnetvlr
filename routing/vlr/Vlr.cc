//
// This file is part of an OMNeT++/OMNEST simulation example.
//
// Copyright (C) 1992-2015 Andras Varga
//
// This file is distributed WITHOUT ANY WARRANTY. See the file
// `license' for details on this and other legal matters.
//

#include "Vlr.h"

#include <fstream>      // for reading/writing to file
#include <sstream>      // for std::stringstream, std::ostringstream, std::istringstream
#include <algorithm>      // for std::copy(..), std::find(..), std::set_intersection(..)

namespace omnetvlr {

Define_Module(Vlr);

Vlr::~Vlr()
{
    cancelAndDelete(beaconTimer);
    cancelAndDelete(purgeNeighborsTimer);
    cancelAndDelete(fillVsetTimer);
    cancelAndDelete(inNetworkWarmupTimer);
    cancelAndDelete(repSeqExpirationTimer);
    cancelAndDelete(repSeqObserveTimer);
    cancelAndDelete(delayedRepairReqTimer);
    cancelAndDelete(recentReplacedVneiTimer);
    cancelAndDelete(testPacketTimer);
    
    cancelPendingVsetTimers();
    cancelRepairLinkTimers();
}

// VlrBase
//
// static VLR parameters -- unit: seconds
const double Vlr::routeSetupReqWaitTime = 6;       // should be same or longer than recentSetupReqExpiration to ensure my setupReq isn't blocked because a setupReply was sent by dst but never arrived
const double Vlr::routeSetupReqTraceWaitTime = 100;       // should be enough time for setupReqTrace to traverse the whole network
// const double Vlr::repairLinkReqWaitTime = 35;       // should be enough time for repairLinkReq to propagate to max flooding range and for other end to detect link failure (max time difference for a link failure to be detected at both link ends) and send repairLinkReply back, should be larger than neighborValidityInterval + propagation time of repairLinkReq and repairLinkReply
double Vlr::repairLinkReqSmallEndSendDelay = 0;       // wait time for repairLinkReq timer at the larger link end should be repairLinkReqWaitTime + repairLinkReqSmallEndSendDelay
// const double Vlr::repairLinkReqfailureSimulateLinkMaxDelay = 5;      // should be smaller than neighborValidityInterval, since failureSimulateLink=true omits time difference for a link failure to be detected at both link ends
const double Vlr::nonEssRouteExpiration = 50;       // should be enough time for setupReply of all other vneis of newnode to reach newnode, i.e. joining process is finished
const double Vlr::patchedRouteExpiration = 100;       // should be enough time for setupReq to follow the patched vroute to reach dst, and some buffer in case another link failure happens in vroute, larger than patchedRouteRepairSetupReqMaxDelay + routeSetupReqWaitTime + buffer for link failure (repairLinkReqWaitTime) and for pendingVneis crowded out by vneis with patched vset-route to be added back and patched nonEss vroutes to be repaired
const double Vlr::patchedRouteRepairSetupReqMaxDelay = 10;       // should be smaller than patchedRouteExpiration - routeSetupReqWaitTime so setupReq can utilize the patched route
const double Vlr::dismantledRouteExpiration = 100;       // should be enough time for setupReqTrace to utilize the dismantled vroute to reach dst
const double Vlr::overHeardTraceExpiration = 100;    // should be larger than setupNonEssRoutesInterval to ensure recorded overheard trace doesn't expire till next time we try to build nonEss vroute to pendingVnei
const double Vlr::recentSetupReqExpiration = 5;    // should be enough time for my setupReply to reach newnode before I process a setupReq from it again
const double Vlr::recentReqFloodExpiration = 40;    // should be enough time for flood request to propagate beyond my neighbourhood so that I don't rebroadcast the same flood request
const double Vlr::delayedRepairLinkReqExpireTimeCoefficient = 7;    // coefficient * beaconInterval -> should be max time difference for a link failure to be detected at both link ends NOTE one end may detect link failure because of a failed packet b4 the other end expires in pset, hence should be larger than neighborValidityInterval
const double Vlr::inNetworkWarmupTime = 5;         // should be larger than fillVsetInterval bc if a newnode (not in overlay) receives a setupReq for it, it should wait for at least one processFillVsetTimer() to schedule setupReq to get more potential vneis in pendingVset b4 setting selfInNetwork=true
const double Vlr::fillVsetInterval = 3;
const double Vlr::notifyVsetSendInterval = 50;      // should be larger than fillVsetInterval bc NotifyVset is sent in processFillVsetTimer()
const double Vlr::setupSecondVsetRoutesInterval = 100;      // should be larger than fillVsetInterval bc vset is checked in processFillVsetTimer()
const double Vlr::setupNonEssRoutesInterval = 50;      // should be larger than fillVsetInterval bc pendingVset is checked in processFillVsetTimer()
double Vlr::recentUnavailableRouteEndExpiration;      // should be larger than repairLinkReqWaitTime to give enough time for broken vroute to be repaired

const double Vlr::maxFloodJitter = 1;      // in seconds

const double Vlr::testSendInterval = 5;
// time to start sending TestPacket
const double Vlr::sendTestPacketOverlayWaitTime = 100;
double Vlr::lastTimeNodeJoinedInNetwork;
bool Vlr::sendTestPacketStart;

// unsigned int Vlr::totalNumBeaconSent = 0; 
bool Vlr::firstRepSeqTimeoutCSVFileWritten = false;
// bool Vlr::totalNumBeacomSentCSVFileCreated = false;
unsigned int Vlr::totalNumRepSeqTimeout = 0;
double Vlr::firstRepSeqTimeoutTime = 0;
double Vlr::finalRepSeqTimeoutTime = 0;
VlrRingVID Vlr::finalRepSeqTimeoutNode;
std::set<unsigned int> Vlr::nodesRepSeqValid;

// VlrRing
//
double Vlr::pendingVneiValidityInterval;


void Vlr::initialize()
{
    RoutingBase::initialize();

    // VlrBase
    beaconInterval = par("beaconInterval");
    maxJitter = par("maxJitter");
    neighborValidityInterval = par("neighborValidityInterval");
    // pneiDiscoveryTime = par("pneiDiscoveryTime");
    repSeqValidityInterval = par("repSeqValidityInterval");
    repSeqPersistenceInterval = par("repSeqPersistenceInterval");
    inNetworkEmptyVsetWarmupTime = par("inNetworkEmptyVsetWarmupTime");
    
    setupReqRetryLimit = par("setupReqRetryLimit");

    repairLinkReqFloodTTL = par("repairLinkReqFloodTTL");
    routePrevhopVidsSize = par("routePrevhopVidsSize");
    repairLinkReqWaitTime = par("repairLinkReqWaitTime");

    // context
    host = getParentModule();

    // internal
    beaconTimer = new cMessage("BeaconTimer");
    purgeNeighborsTimer = new cMessage("PurgeNeighborsTimer");
    fillVsetTimer = new cMessage("fillVsetTimer");
    inNetworkWarmupTimer = new cMessage("inNetworkWarmupTimer");
    repSeqExpirationTimer = new cMessage("repSeqExpirationTimer");
    repSeqObserveTimer = new cMessage("repSeqObserveTimer");
    delayedRepairReqTimer = new cMessage("delayedRepairReqTimer");
    recentReplacedVneiTimer = new cMessage("recentReplacedVneiTimer");

    setupTempRoute = par("setupTempRoute");
    keepDismantledRoute = par("keepDismantledRoute");
    checkOverHeardTraces = par("checkOverHeardTraces");
    sendPeriodicNotifyVset = par("sendPeriodicNotifyVset");
    sendNotifyVsetToReplacedVnei = par("sendNotifyVsetToReplacedVnei");
    sendRepairLocalNoTemp = par("sendRepairLocalNoTemp");
    if (sendRepairLocalNoTemp) {
        setupTempRoute = true;
        repairLinkReqSmallEndSendDelay = 0;     // no need small end delay bc repairLocalReq must be initiated by the end closer to toVid for each route, for failed link (a,b) if both contain prevhopVids for some routes, both need to send repairLocalReq
    }
    recentUnavailableRouteEndExpiration = repairLinkReqWaitTime + repairLinkReqSmallEndSendDelay;

    // statistics measurement
    sendTestPacket = par("sendTestPacket");
    testPacketTimer = new cMessage("testPacketTimer");
    recordReceivedMsg = par("recordReceivedMsg");
    recordDroppedMsg = par("recordDroppedMsg");

    if (sendTestPacket) {
        initializeSelfTestDstList();

        allSendRecords.reserve(allSendRecordsCapacity);     // request the minimum vector capacity at the start to avoid reallocation
    }

    // VlrRing
    vsetHalfCardinality = par("vsetHalfCardinality");
    int backupVsetHalfCardinality = par("backupVsetHalfCardinality");
    vsetAndBackupHalfCardinality = vsetHalfCardinality + backupVsetHalfCardinality;
    pendingVsetHalfCardinality = std::max(vsetAndBackupHalfCardinality, 2 * vsetHalfCardinality); // we try to maintain vroutes to vsetAndBackupHalfCardinality nodes close to me in ccw/cw direction (vsetHalfCardinality in vset, backupVsetHalfCardinality in pendingVset), when failures happen and all 2*vsetHalfCardinality vneis added to pendingVset, don't want them to crowd out 2*backupVsetHalfCardinality pendingVneis to which I've built vroutes
    int pendingVsetHalfCardPar = par("pendingVsetHalfCardinality");
    if (pendingVsetHalfCardPar > pendingVsetHalfCardinality)
        pendingVsetHalfCardinality = pendingVsetHalfCardPar;
    vsetAndPendingHalfCardinality = vsetHalfCardinality + pendingVsetHalfCardinality;

    representativeFixed = par("representativeFixed");
    startingRootFixed = par("startingRootFixed");
    if (representativeFixed || startingRootFixed) {
        VlrRingVID repVid;
        if ((repVid = readRepresentativeVidFromFile()) != VLRRINGVID_NULL)
            representative.vid = repVid;   // assign representative if repPosNodeIdCSVFile provided
        else    // repVid == VLRRINGVID_NULL
            representative.vid = par("representativeVid");
    } else {
        representative.vid = VLRRINGVID_NULL;
        representative.heardfromvid = VLRRINGVID_NULL;
    }

    pendingVneiValidityInterval = patchedRouteExpiration + setupNonEssRoutesInterval; // 2 * pendingVsetHalfCardinality * notifyVsetSendInterval;  // max validity time of a vroute (from heardFrom to pendingVnei), should be larger than setupNonEssRoutesInterval

    totalNumBeacomSentWriteTime = par("totalNumBeacomSentWriteTime");
    repFixedEarlyBeaconOnNewRepSeqNum = par("repFixedEarlyBeaconOnNewRepSeqNum");
    repBeaconExcludeOldRepSeqNum = false;     // if !representativeFixed (using representativeMap), exclude rep seqNo that wasn't updated in previous beaconInterval
    const char *repBeaconExcludeOldRepSeqNumPar = par("repBeaconExcludeOldRepSeqNum");
    if (strlen(repBeaconExcludeOldRepSeqNumPar) > 0) {
        if (strcmp(repBeaconExcludeOldRepSeqNumPar, "true") == 0)   // cstrings are equal
            repBeaconExcludeOldRepSeqNum = true;
        else if (strcmp(repBeaconExcludeOldRepSeqNumPar, "false") == 0)   // cstrings are equal
            repBeaconExcludeOldRepSeqNum = false;
    }
    
    // initialize static non-const variables for multiple ${repetition} or ${runnumber} bc cmdenv runs simulations in the same process, this means that e.g. if one simulation run writes a global variable, subsequent runs will also see the change
    lastTimeNodeJoinedInNetwork = sendTestPacketOverlayWaitTime;
    sendTestPacketStart = false;

    firstRepSeqTimeoutCSVFileWritten = false;
    totalNumRepSeqTimeout = 0;
    firstRepSeqTimeoutTime = 0;
    finalRepSeqTimeoutTime = 0;
    finalRepSeqTimeoutNode = VLRRINGVID_NULL;
    nodesRepSeqValid.clear();

    ASSERT(psetTable.vidToStateMap.empty());
    ASSERT(representativeMap.empty());
    ASSERT(vlrRoutingTable.vlrRoutesMap.empty());
    ASSERT(pendingVset.empty());

}

void Vlr::handleMessage(cMessage *message)
{
    if (message->isSelfMessage())
        processSelfMessage(message);
    else
        processMessage(message);
}

void Vlr::processSelfMessage(cMessage *message)
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
    // self-message defined in Vlr
    else if (message == beaconTimer)
        processBeaconTimer();
    else if (message == purgeNeighborsTimer)
        processPurgeNeighborsTimer();
    else if (message == fillVsetTimer)
        processFillVsetTimer();
    else if (message == inNetworkWarmupTimer)
        processInNetworkWarmupTimer();
    else if (message == repSeqExpirationTimer)
        processRepSeqExpirationTimer();
    else if (message == repSeqObserveTimer)
        processRepSeqObserveTimer();
    else if (message == delayedRepairReqTimer)
        processDelayedRepairReqTimer();
    else if (message == recentReplacedVneiTimer)
        processRecentReplacedVneiTimer();
    else if (message == testPacketTimer)
        processTestPacketTimer();
    else if (auto waitsetupReqTimer = dynamic_cast<WaitSetupReqIntTimer *>(message))
        processWaitSetupReqTimer(waitsetupReqTimer);
    else if (auto waitrepairLinkTimer = dynamic_cast<WaitRepairLinkIntTimer *>(message))
        processWaitRepairLinkTimer(waitrepairLinkTimer);
}

void Vlr::processMessage(cMessage *message)
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
            else if (auto addRoute = dynamic_cast<AddRouteInt *>(message))
                processAddRoute(addRoute, pktForwarded);
            else if (auto teardown = dynamic_cast<TeardownInt *>(message))
                processTeardown(teardown, pktForwarded);
            else if (auto testpacket = dynamic_cast<VlrIntTestPacket *>(message))
                processTestPacket(testpacket, pktForwarded);
            else if (auto repairLinkReq = dynamic_cast<RepairLinkReqFloodInt *>(message))
                processRepairLinkReqFlood(repairLinkReq, pktForwarded);
            else if (auto repairLinkReply = dynamic_cast<RepairLinkReplyInt *>(message))
                processRepairLinkReply(repairLinkReply, pktForwarded);
            else if (auto repairLinkFail = dynamic_cast<RepairLinkFailInt *>(message))
                processRepairLinkFail(repairLinkFail, pktForwarded);
            else if (auto repairRoute = dynamic_cast<RepairRouteInt *>(message))
                processRepairRoute(repairRoute, pktForwarded);
            else if (auto notifyVset = dynamic_cast<NotifyVsetInt *>(message))
                processNotifyVset(notifyVset, pktForwarded);
            else if (auto repairLocalReq = dynamic_cast<RepairLocalReqFloodInt *>(message))
                processRepairLocalReqFlood(repairLocalReq, pktForwarded);
            else if (auto repairLocalReply = dynamic_cast<RepairLocalReplyInt *>(message))
                processRepairLocalReply(repairLocalReply, pktForwarded);
            else if (auto repairLocalPrev = dynamic_cast<RepairLocalPrevInt *>(message))
                processRepairLocalPrev(repairLocalPrev, pktForwarded);
        }
    }

    if (!pktForwarded)
        delete message;
}

VlrRingVID Vlr::readRepresentativeVidFromFile()
{
    const char *fname = par("repPosNodeIdCSVFile");
    if (strlen(fname) > 0) {    // repPosNodeId assignment file provided
        std::ifstream vidFile(fname);
        if (!vidFile.good())
            throw cRuntimeError("Unable to load repPosNodeId assignment file");

        const char *repPosChoicePar = par("repPosNodeIdChoice");
        if (strlen(repPosChoicePar) == 0)     // repPos vid choice not provided
            throw cRuntimeError("RepPosNodeId assignment file provided but repPosChoice not specified");
        
        cStringTokenizer tokenizer(repPosChoicePar, /*delimiters=*/", ");
        const char *repPosChoice = tokenizer.nextToken();
        ASSERT(repPosChoice != nullptr);    // e.g. "nodeAtCentre"
        const char *token = tokenizer.nextToken();
        ASSERT(token != nullptr);
        int repPosIndex = atoi(token);
        
        const char *startstr = repPosChoice;
        std::vector<VlrRingVID> repPosNodeIdList;
        std::string line;
        while (std::getline(vidFile, line)) {   // for each line in file
            if (line.rfind(startstr, 0) == 0) { // line begins with startstr
                cStringTokenizer tokenizer(line.c_str() + strlen(startstr), /*delimiters=*/", ");    // start parsing after startstr
                while ((token = tokenizer.nextToken()) != nullptr)
                    repPosNodeIdList.push_back( (VlrRingVID)std::stoul(token, nullptr, 0) ); // base=0: base used is determined by format in vidstr
                break;  // skip more lines in repPosNodeIdCSVFile bc we've found the row starting with repPosChoice
            }
        }
        if (repPosIndex >= repPosNodeIdList.size())
            throw cRuntimeError("repPosNodeIdCSVFile error: repPosIndex is larger than repPosNodeId row starting with repPosChoice");
        
        VlrRingVID repId = repPosNodeIdList[repPosIndex];
        EV_INFO << "Initialized representative vid  = " << repId << " at me=" << vid << " with repPosNodeId assignment file" << endl;

        return repId;
    }
    return VLRRINGVID_NULL;
}

//
// beacon timers
//

void Vlr::scheduleBeaconTimer(bool firstTimer/*=false*/, double maxDelay/*=0*/)
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
    } else if (maxDelay > 0) {
        EV_DEBUG << "Scheduling beacon timer within maxDelay = " << maxDelay << endl;
        double random = uniform(0, 1);
        scheduleAt(simTime() + random * maxDelay, beaconTimer);
        // calibratedBeaconTime = beaconTimer->getArrivalTime();
    } else {
        EV_DEBUG << "Scheduling beacon timer" << endl;
        // calibratedBeaconTime = calibratedBeaconTime + beaconInterval;
        double random = uniform(-1, 1);
        // scheduleAt(calibratedBeaconTime + random * maxJitter, beaconTimer);
        scheduleAt(simTime() + beaconInterval + random * maxJitter, beaconTimer);
    }
}

void Vlr::processBeaconTimer()
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

void Vlr::schedulePurgeNeighborsTimer()
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

simtime_t Vlr::getNextNeighborExpiration() const
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

void Vlr::purgeNeighbors()
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

void Vlr::processPurgeNeighborsTimer()
{
    EV_DEBUG << "Processing purge neighbors timer" << endl;
    purgeNeighbors();
    schedulePurgeNeighborsTimer();
}

void Vlr::scheduleFillVsetTimer(bool firstTimer/*=false*/, double maxDelay/*=0*/)  // default parameter value should only be defined in function declaration
{
    if (firstTimer) {
        EV_DEBUG << "Scheduling 1st fillVsetTimer after at least pneiDiscoveryTime" << endl;
        scheduleAt(simTime() + uniform(1, 6) * beaconInterval, fillVsetTimer);    // uniform(1, 2) * pneiDiscoveryTime
    } else if (maxDelay > 0) {
        EV_DEBUG << "Scheduling fillVsetTimer within maxDelay = " << maxDelay << endl;
        scheduleAt(simTime() + uniform(0, 1) * maxDelay, fillVsetTimer);
    } else {
        EV_DEBUG << "Scheduling fillVsetTimer" << endl;
        scheduleAt(simTime() + uniform(0.8, 1.2) * fillVsetInterval, fillVsetTimer);
    }
}

void Vlr::scheduleTestPacketTimer(bool firstTimer/*=false*/)
{
    if (firstTimer) {
        EV_DEBUG << "Scheduling 1st TestPacket timer after at least sendTestPacketOverlayWaitTime = " << sendTestPacketOverlayWaitTime << endl;
        scheduleAt(simTime() + sendTestPacketOverlayWaitTime + uniform(0.5, 1.5) * testSendInterval, testPacketTimer);
    } else {
        EV_DEBUG << "Scheduling TestPacket timer" << endl;
        scheduleAt(simTime() + uniform(0.5, 1.5) * testSendInterval, testPacketTimer);
    }
}

void Vlr::sendBeacon()
{
    // else if (representative.heardfromvid != VLRRINGVID_NULL && representative.sequencenumber == lastBeaconRepSeqnum && representative.vid == lastBeaconRepVid)
    //     lastBeaconRepSeqnumUnchanged = true;
    // else
    //     lastBeaconRepSeqnumUnchanged = false;
    
    // lastBeaconRepSeqnum = representative.sequencenumber;
    // lastBeaconRepVid = (representative.heardfromvid != VLRRINGVID_NULL) ? representative.vid : VLRRINGVID_NULL;     // representative.vid isn't connected if representative.heardfromvid == VLRRINGVID_NULL
    if (totalNumBeacomSentWriteTime == 0 || simTime() < totalNumBeacomSentWriteTime)
        totalNumBeaconSent++;

    auto beacon = createBeacon();
    EV_INFO << "Sending beacon: vid = " << beacon->getVid() << ", inNetwork = " << beacon->getInNetwork() << ", number of pset neighbours = " << beacon->getPsetNeighbourArraySize() << ", rep = " << beacon->getRepstate().vid << endl;
    sendCreatedPacket(beacon, /*unicast=*/false, /*outGateIndex=*/-1, /*delay=*/0);
}

VlrIntBeacon* Vlr::createBeacon()
{
    VlrIntBeacon *beacon = new VlrIntBeacon(/*name=*/"Beacon");
    // unsigned int vid
    // bool inNetwork
    beacon->setVid(vid);                    // vidByteLength
    beacon->setInNetwork(selfInNetwork);    // 1 bit, neglected
    int chunkByteLength = VLRRINGVID_BYTELEN;
    
    if (representativeFixed) {  // only predefined rep will be broadcasted
        // VlrIntRepState repstate
        VlrIntRepState& repstate = beacon->getRepstateForUpdate();
        repstate.vid = VLRRINGVID_NULL;
        if (representative.heardfromvid != VLRRINGVID_NULL) {       // I have rep-path to predefined rep
            bool sendRepInfo = true;
            if (representative.vid == vid)
                representative.sequencenumber++;
            else if (repBeaconExcludeOldRepSeqNum && repSeqExpirationTimer->isScheduled() && simTime() - (repSeqExpirationTimer->getArrivalTime() - repSeqValidityInterval) > beaconInterval + maxJitter) {  // lastHeard of rep > max interval btw two beacons
                if (representative.sequencenumber > representative.lastBeaconSeqnum)    // bc I must have sent a beacon after lastHeard of rep with new rep seqNo, so lastBeaconSeqnum should have been updated to sequencenumber
                    throw cRuntimeError("Skipping repstate for representativeFixed with new seqNo = %d than last beaconed seqNo = %d at me=%d", representative.sequencenumber, representative.lastBeaconSeqnum, vid);
                sendRepInfo = false;
            }
            if (sendRepInfo) {
                repstate.vid = representative.vid;                          // vidByteLength
                repstate.sequencenumber = representative.sequencenumber;    // 4 byte
                // repstate.hopcount = representative.hopcount;             // 2 byte
                repstate.inNetwork = representative.inNetwork;              // 1 bit, ignored
            }
            representative.lastBeaconSeqnumUnchanged = (representative.sequencenumber == representative.lastBeaconSeqnum);
            representative.lastBeaconSeqnum = representative.sequencenumber;
        } else
            representative.lastBeaconSeqnumUnchanged = false;

        chunkByteLength += VLRRINGVID_BYTELEN + 4;
    } else {
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
        } else  // vset empty
            sendSelfRep = true;
        if (sendSelfRep) {
            repstate.vid = vid;                             // vidByteLength
            repstate.sequencenumber = ++selfRepSeqnum;      // 4 byte
            // repstate.hopcount = representative.hopcount; // 2 byte
            repstate.inNetwork = selfInNetwork;             // 1 bit, ignored
            // EV_WARN << "Sending beacon with me as rep: me = " << vid << ", selfRepSeqnum = " << selfRepSeqnum << ", inNetwork = " << selfInNetwork << ", vset = " << printVsetToString() << endl;
        }
        std::set<VlrRingVID> includedReps;      // reps included in this beacon or could have been included
        simtime_t expiredLastheard = simTime() - repSeqValidityInterval;
        for (auto repMapItr = representativeMap.begin(); repMapItr != representativeMap.end(); ++repMapItr) {
            if (repMapItr->second.heardfromvid != VLRRINGVID_NULL && repMapItr->second.lastHeard > expiredLastheard) {  // rep hasn't expired
                if (repBeaconExcludeOldRepSeqNum && simTime() - repMapItr->second.lastHeard > beaconInterval + maxJitter) { // rep excluded bc lastHeard isn't within max interval btw beacons
                    if (repstate2.vid == VLRRINGVID_NULL || repstate.vid == VLRRINGVID_NULL) {   // this rep could have been included
                        repMapItr->second.lastBeaconSeqnumUnchanged = (repMapItr->second.sequencenumber == repMapItr->second.lastBeaconSeqnum);
                        includedReps.insert(repMapItr->first);
                    } else      // both repstate and repstate2 have been added
                         break;
                }
                else {      // rep can be added in beacon
                    if (repstate2.vid == VLRRINGVID_NULL) {     // repstate2 hasn't been added
                        repstate2.vid = repMapItr->first;
                        repstate2.sequencenumber = repMapItr->second.sequencenumber;
                        repstate2.inNetwork = repMapItr->second.inNetwork;
                        // repMapItr->second.sequencenumber won't be 0, if I didn't send rep in my last beacon, repMapItr->second.lastBeaconSeqnum would be 0
                        repMapItr->second.lastBeaconSeqnumUnchanged = (repMapItr->second.sequencenumber == repMapItr->second.lastBeaconSeqnum);
                        repMapItr->second.lastBeaconSeqnum = repMapItr->second.sequencenumber;
                        includedReps.insert(repMapItr->first);
                    } else if (repstate.vid == VLRRINGVID_NULL) {     // repstate hasn't been added
                        repstate.vid = repMapItr->first;
                        repstate.sequencenumber = repMapItr->second.sequencenumber;
                        repstate.inNetwork = repMapItr->second.inNetwork;
                        repMapItr->second.lastBeaconSeqnumUnchanged = (repMapItr->second.sequencenumber == repMapItr->second.lastBeaconSeqnum);
                        repMapItr->second.lastBeaconSeqnum = repMapItr->second.sequencenumber;
                        includedReps.insert(repMapItr->first);
                    } else      // both repstate and repstate2 have been added
                        break;
                }
            }
        }
        // only keep lastBeaconSeqnum for rep included in this beacon or excluded (bc didn't receive new rep seqNo in last beaconInterval) but hasn't expired
        for (auto repMapItr = representativeMap.begin(); repMapItr != representativeMap.end(); ++repMapItr)
            // if (repMapItr->first != repstate2.vid && repMapItr->first != repstate.vid)
            if (includedReps.find(repMapItr->first) == includedReps.end())      // rep not in includedReps
                repMapItr->second.lastBeaconSeqnum = 0;

        chunkByteLength += (VLRRINGVID_BYTELEN + 4) * 2;
    }
    
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

void Vlr::processBeacon(VlrIntBeacon *beacon, bool& pktForwarded)
{
    int pneiGateIndex = beacon->getArrivalGate()->getIndex();
    VlrRingVID pneiVid = beacon->getVid();
    bool pneiInNet = beacon->getInNetwork();
    unsigned int pneiPsetCount = beacon->getPsetNeighbourArraySize();
    const VlrIntRepState& pneiRepState = beacon->getRepstate();
    EV_INFO << "Processing beacon (me=" << vid << "): vid = " << pneiVid << ", gateIndex = " << pneiGateIndex << ", inNetwork = " << pneiInNet << ", number of pset neighbours = " << pneiPsetCount << ", rep = " << pneiRepState.vid << endl;

    // std::vector<VlrRingVID> linked2hopPneis;      // linked pneis of sender
    bool pneiCanHearMe = false;
    for (unsigned int i = 0; i < pneiPsetCount; i++) {
        if (beacon->getPsetNeighbour(i) == vid) {   // sender can hear me
            pneiCanHearMe = true;
            break;
        }
        // else if (beacon->getPsetNeighbourIsInNetwork(i))    // record linked && inNetwork pnei of sender
        //     linked2hopPneis.push_back(beacon->getPsetNeighbour(i));
    }
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

    std::set<VlrRingVID> dummySet {};   // not recording 2-hop pneis for Vlr
    psetTable.setNeighbour(pneiVid, pneiGateIndex, pneiState, pneiInNet, /*mpneis=*/dummySet);
    
    updateRepState(pneiVid, pneiRepState, pneiCanHearMe);   // update representative
    if (!representativeFixed) {
        const VlrIntRepState& pneiRepState2 = beacon->getRepstate2();
        updateRepState(pneiVid, pneiRepState2, pneiCanHearMe);   // update representative
    }

    // if this is a linked pnei, add 2-hop neighbours heard in beacon to overheardMPneis
    // if (pneiCanHearMe && checkOverHeardMPneis) {
    //     for (const auto& mpnei : linked2hopPneis)
    //         overheardMPneis[mpnei] = std::make_pair(pneiVid, simTime() + neighborValidityInterval);   // initialize expireTime of overheard trace to 2-hop pnei
    // }

     // schedulePurgeNeighborsTimer();
}

void Vlr::updateRepState(VlrRingVID pnei, const VlrIntRepState& pneiRepState, bool pneiIsLinked) {
    const VlrRingVID& pneiRepVid = pneiRepState.vid;
    EV_DETAIL << "Processing RepState of pnei " << pnei << ": rep = " << pneiRepVid << ", seqNo = " << pneiRepState.sequencenumber << ", isLinked = " << pneiIsLinked << endl;

    if (pneiIsLinked && pneiRepVid != VLRRINGVID_NULL && pneiRepVid != vid) {
        if (representativeFixed) {      // only accept predefined rep
            if (pneiRepVid == representative.vid) {   // pnei has the same rep as me
                if (pneiRepState.sequencenumber > representative.sequencenumber) {  // shouldn't happen if I'm the predefined rep
                    representative.sequencenumber = pneiRepState.sequencenumber;
                    // if (representative.vid != vid) {   // myself isn't the rep
                    // if (representative.heardfromvid == VLRRINGVID_NULL) {    // I didn't have valid rep but now has valid rep, update my status in vidRegistryTable
                    //     setSelfConnInRegistry(/*conn=*/true);
                    // }
                    nodesRepSeqValid.insert(vid);

                    representative.heardfromvid = pnei;
                    representative.inNetwork = pneiRepState.inNetwork;
                    // schedule a new rep seqNo expiration timer
                    if (repSeqExpirationTimer->isScheduled())
                        cancelEvent(repSeqExpirationTimer);
                    scheduleAt(simTime() + repSeqValidityInterval, repSeqExpirationTimer);
                    // reschedule beacon timer to notify pneis of the new rep seqNo soon
                    if (repFixedEarlyBeaconOnNewRepSeqNum && representative.lastBeaconSeqnumUnchanged) {
                        // NOTE beaconTimer should always be scheduled unless in sendBeacon() or scheduleBeaconTimer()
                        double maxDelayToNextTimer = 0.1;
                        if (!beaconTimer->isScheduled() || beaconTimer->getArrivalTime() - simTime() >= maxDelayToNextTimer) {    // only reschedule beaconTimer if it won't come within maxDelayToNextTimer
                            cancelEvent(beaconTimer);
                            scheduleBeaconTimer(/*firstTimer=*/false, /*maxDelay=*/maxDelayToNextTimer);
                        }
                    }
                    // if (checkOverHeardTraces)   // checking whether representative belongs to vset isn't necessary bc Vlr doesn't merge rings
                    // add representative to pendingVset, always needed to repair partition     NOTE when representativeFixed, representative.vid should always be inNetwork
                    pendingVsetAdd(representative.vid, /*heardFrom=*/VLRRINGVID_NULL, /*addToEmpty=*/false, /*nodeAlive=*/false);
                }
                EV_INFO << "Representative heard from pnei " << pnei << ": " << representative << endl;
            }
        } else {    // no predefined rep
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
                        // search "nodeStats" in all constructlog files: grep --include=*constructlog*.txt -irnwl "." -e "nodeStats"
                        // search "numNodesVsetCorrect=" in given file, only return last 2 lines: grep nodeStats_squareGrid_49.csv -e "numNodesVsetCorrect=" | tail -2
                        // search first occurence of "nodeStats" in each nodeStats file and print 1 lines before&after the match: grep --include=*nodeStats*.csv -irn -m1 -C1 "." -e "nodeStats"

                        //# find . -path "*/rootNoFixed-r1_notestpkt/nodeStats_*.csv" -type f             # find all files under current directories (including subdirectories) that matches the pattern
                        //# find . -path "*/rootNoFixed-r1_notestpkt/nodeStats_*.csv" -type f -delete     # delete those files
                    }
                    // else {
                    //     std::ostringstream s;
                    //     s << "Added new rep=" << pneiRepVid << ", new seqNo=" << pneiRepState.sequencenumber << ", heard from pnei=" << pnei;
                    //     recordNodeStatsRecord(/*infoStr=*/s.str().c_str());
                    // }
                    if (representativeMap.size() > representativeMapActualMaxSize)
                        representativeMapActualMaxSize = representativeMap.size();
                }
                EV_WARN << "New representative heard from pnei " << pnei << ": " << pneiRepVid << endl;
            } else {    // pneiRepVid already in representativeMap
                if (pneiRepState.sequencenumber > repMapItr->second.sequencenumber) {   // received a larger seqNo for pneiRepVid
                    repMapUpdated = true;
                } else {    // received seqNo <= my current seqNo for pneiRepVid
                    repMapItr->second.lastHeardOldseqnum = simTime().dbl();
                }
            }

            if (repMapUpdated) {
                // EV_WARN << "Heard new representative seqNum in beacon, me=" << vid << ", rep = " << repMapItr->first << ", seqNum = " << pneiRepState.sequencenumber << endl;
                repMapItr->second.sequencenumber = pneiRepState.sequencenumber;
                repMapItr->second.heardfromvid = pnei;
                repMapItr->second.lastHeard = simTime();
                repMapItr->second.lastHeardOldseqnum = simTime().dbl();
                repMapItr->second.inNetwork = pneiRepState.inNetwork;

                // add representative to pendingVset, always needed to repair partition
                if (repMapItr->second.inNetwork)    // bc any node can be added as representative, I only add node when it's inNetwork
                    pendingVsetAdd(repMapItr->first, /*heardFrom=*/VLRRINGVID_NULL, /*addToEmpty=*/false, /*nodeAlive=*/false);

                EV_INFO << "Representative heard from pnei " << pnei << ": " << pneiRepVid << endl;
            }   
        }
    }
    
}

void Vlr::handleLinkedPnei(const VlrRingVID& pneiVid)
{
    EV_DEBUG << "Received beacon from a LINKED pnei " << pneiVid << " in PsetTable at me=" << vid << endl;
    // if pnei in lostPneis (maybe was a lost pnei) but is now LINKED, set vroutes broken by pnei available
    auto lostPneiItr = lostPneis.find(pneiVid);
    if (lostPneiItr != lostPneis.end() && checkLostPneiTempVlinksNotRecent(lostPneiItr->second.tempVlinks)) {    //  pneiVid in lostPneis and none of tempVlinks was just built by me 
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
        // put temporary vlinks to lost pnei in nonEssRoutes as they are no longer necessary for broken vroutes
        delayLostPneiTempVlinksTeardown(lostPneiItr->second.tempVlinks, /*delay=*/nonEssRouteExpiration);   // wait some time before tearing down tempVlinks so otherEnd also detects me as pnei  
        
        cancelAndDelete(lostPneiItr->second.timer);
        lostPneis.erase(lostPneiItr);
    }
}

void Vlr::handleLostPneis(const std::vector<VlrRingVID>& pneiVids, const std::vector<VlrPathID> *rmVroutesPtr/*=nullptr*/)
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

    if (representativeFixed && representative.heardfromvid == VLRRINGVID_NULL) {        // if I don't have a valid rep, I shouldn't have any vroutes, hence don't care about lost of pnei
        EV_WARN << "No valid rep heard: " << representative << ", cannot repair routes, skipping handleLostPneis to tear down all vroutes" << endl;
        return;
    }

    std::map<VlrRingVID, std::vector<VlrPathID>> nextHopToPathidsMap;    // for Teardown to send, map next hop address to pathids (since these are temporary routes, next hop should always be available)
    for (size_t i = 0; i < pneiVids.size(); ++i) {
        const VlrRingVID& pneiVid = pneiVids[i];
        // const L3Address& pneiAddr = pneiAddrs[i];

        // // remove records in overheardMPneis with nexthop == pnei
        // for (auto mpneiItr = overheardMPneis.begin(); mpneiItr != overheardMPneis.end(); )
        //     if (mpneiItr->second.first == pneiVid) 
        //         overheardMPneis.erase(mpneiItr++);
        //     else
        //         mpneiItr++;

        // find vroutes in vlrRoutingTable with prevhop/nexthop == pnei, set them unavailable
        ASSERT(pneiVid != VLRRINGVID_NULL);
        for (auto vrouteItr = vlrRoutingTable.vlrRoutesMap.begin(); vrouteItr != vlrRoutingTable.vlrRoutesMap.end(); ) {
            if (vrouteItr->second.prevhopVid == pneiVid || vrouteItr->second.nexthopVid == pneiVid) {   // route prevhop/nexthop == pnei
                const VlrPathID& oldPathid = vrouteItr->first;     // pathid broken by lost pnei

                bool removeVroute = false;
                if (rmVroutesPtr && std::find(rmVroutesPtr->begin(), rmVroutesPtr->end(), oldPathid) != rmVroutesPtr->end())  // pathid found in vroutes to remove
                    removeVroute = true;

                bool isTemporaryRoute = vlrRoutingTable.getIsTemporaryRoute(vrouteItr->second.isUnavailable);
                bool isDismantledRoute = vlrRoutingTable.getIsDismantledRoute(vrouteItr->second.isUnavailable);
                
                // for temporary route, or if setupTempRoute=false, send Teardown without link repairing
                if (isTemporaryRoute || isDismantledRoute || !setupTempRoute || removeVroute) {
                    // send Teardown to prevhop/nexthop that's still LINKED, bc pnei can't be reached (sending a futile Teardown to it can cause collision, other nodes along oldPathid won't process the unicast Teardown not directed to themselves)
                    bool isPrevhopLost = (vrouteItr->second.prevhopVid == pneiVid);
                    const VlrRingVID& nextHopVid = (isPrevhopLost) ? vrouteItr->second.nexthopVid : vrouteItr->second.prevhopVid;
                    if (nextHopVid != VLRRINGVID_NULL) {  // I'm not an endpoint of the vroute
                        char nextHopIsUnavailable = vlrRoutingTable.getPrevNextIsUnavailable(vrouteItr->second.isUnavailable, /*getPrev=*/!isPrevhopLost);
                        // if route isn't dismantled, remove route from vlrRoutingTable if remaining next hop is unavailable/patched
                        // if route is dismantled, remove route from vlrRoutingTable if remaining next hop is blocked/unavailable
                        bool removeFromRoutingTable = (isTemporaryRoute || !keepDismantledRoute || nextHopIsUnavailable != 0);
                        // if route isn't already torn down, always send Teardown to remaining next hop
                        // if route is already torn down (dismantled), send Teardown only if remaining next hop is blocked, i.e. next hop 2-bit status == 3     NOTE next hop status can't be 2 (patched)
                        if ((!isDismantledRoute && nextHopIsUnavailable != 1) || nextHopIsUnavailable == 3) {
                            EV_DETAIL << "Teardown (pathid = " << oldPathid << ") will be sent to " << nextHopVid << endl;
                            auto nextHopItr = nextHopToPathidsMap.find(nextHopVid);
                            if (nextHopItr == nextHopToPathidsMap.end())
                                nextHopToPathidsMap[nextHopVid] = {oldPathid};
                            else {
                                nextHopItr->second.push_back(oldPathid);
                                //std::get<1>(nextHopItr->second) = std::get<1>(nextHopItr->second) | !removeFromRoutingTable;    // set dismantled to true in Teardown when there's at least 1 vroute in pathids that can be kept as dismantled route
                            }
                        }

                        if (removeFromRoutingTable) {
                            EV_DETAIL << "Pathid " << oldPathid << " is removed from routing table, isTemporaryRoute=" << isTemporaryRoute << ", keepDismantledRoute=" << keepDismantledRoute << ", nextHopIsUnavailable=" << (int)nextHopIsUnavailable << endl;
                            // delete route in endpointToRoutesMap
                            vlrRoutingTable.removeRouteEndsFromEndpointMap(oldPathid, vrouteItr->second);
                            // delete route in vlrRoutesMap
                            vlrRoutingTable.vlrRoutesMap.erase(vrouteItr++);
                        } else {    // endpoint connected by the remaining next hop is still available
                            // set lost pnei in route unavailable
                            vlrRoutingTable.setRouteItrPrevNextIsUnavailable(vrouteItr, /*setPrev=*/isPrevhopLost, /*value=*/1);
                            // set vroute as dismantled
                            vlrRoutingTable.setRouteItrIsDismantled(vrouteItr, /*setDismantled=*/true, /*setPrev=*/isPrevhopLost);
                            // remove endpoint connected by lost pnei in endpointToRoutesMap
                            VlrRingVID lostEnd = (isPrevhopLost) ? vrouteItr->second.fromVid : vrouteItr->second.toVid;
                            EV_DETAIL << "Pathid " << oldPathid << " is kept in routing table and added to dismantledRoutes, endpoint " << lostEnd << " becomes unavailable" << endl;
                            vlrRoutingTable.removeRouteEndFromEndpointMap(oldPathid, lostEnd);

                            dismantledRoutes.insert({oldPathid, simTime() + dismantledRouteExpiration});
                            vrouteItr++;
                        }
                    } else {    // I'm an endpoint of pathid
                        // either the other endpoint is in lostPneis, or this pathid is in nonEssRoutes
                        if (vrouteItr->second.fromVid == vid)
                            removeEndpointOnTeardown(oldPathid, /*towardVid=*/vrouteItr->second.toVid, /*pathIsVsetRoute=*/vrouteItr->second.isVsetRoute, /*pathIsTemporary=*/vrouteItr->second.isUnavailable);
                        else if (vrouteItr->second.toVid == vid)
                            removeEndpointOnTeardown(oldPathid, /*towardVid=*/vrouteItr->second.fromVid, /*pathIsVsetRoute=*/vrouteItr->second.isVsetRoute, /*pathIsTemporary=*/vrouteItr->second.isUnavailable);
                    
                        // delete route in endpointToRoutesMap
                        vlrRoutingTable.removeRouteEndsFromEndpointMap(oldPathid, vrouteItr->second);
                        // delete route in vlrRoutesMap
                        vlrRoutingTable.vlrRoutesMap.erase(vrouteItr++);
                    }
                }
                // NOTE for vroutes in nonEssRoutes, I can't send Teardown bc next hop toward the other end is the lost pnei, but the lost pnei doesn't know this vroute is non-essential and will add it to brokenVroutes and schedule repairLinkReq for it
                // vroutes in nonEssRoutes will expire and be removed from vlrRoutingTable anyway
                // else if (nonEssRoutes.find(oldPathid) != nonEssRoutes.end()) {
                // }
                else {  // for vroute, set it unavailable and add to lostPneis
                    if (vrouteItr->second.prevhopVid == pneiVid) {   // route prevhop == lost pnei
                        // set route prevhop unavailable
                        vlrRoutingTable.setRouteItrPrevNextIsUnavailable(vrouteItr, /*setPrev=*/true, /*value=*/1);
                        // remove route fromVid in endpointToRoutesMap
                        vlrRoutingTable.removeRouteEndFromEndpointMap(oldPathid, vrouteItr->second.fromVid);
                        
                        // if route fromVid is no longer an available endpoint, add it to recentUnavailableRouteEnd to leave time for repair
                        if (!vlrRoutingTable.findAvailableRouteEndInEndpointMap(vrouteItr->second.fromVid))
                            recentUnavailableRouteEnd.insert({vrouteItr->second.fromVid, {oldPathid, simTime() + recentUnavailableRouteEndExpiration}});
                    } else {    // route nexthop == lost pnei
                        // set route nexthop unavailable
                        vlrRoutingTable.setRouteItrPrevNextIsUnavailable(vrouteItr, /*setPrev=*/false, /*value=*/1);
                        // remove route toVid in endpointToRoutesMap
                        vlrRoutingTable.removeRouteEndFromEndpointMap(oldPathid, vrouteItr->second.toVid);

                        // if route toVid is no longer an available endpoint, add it to recentUnavailableRouteEnd to leave time for repair
                        if (!vlrRoutingTable.findAvailableRouteEndInEndpointMap(vrouteItr->second.toVid))
                            recentUnavailableRouteEnd.insert({vrouteItr->second.toVid, {oldPathid, simTime() + recentUnavailableRouteEndExpiration}}); 
                    }

                    auto lostPneiItr = lostPneis.find(pneiVid);
                    if (lostPneiItr == lostPneis.end())
                        lostPneis.insert({pneiVid, {{oldPathid}, nullptr, {}}});
                    else
                        lostPneiItr->second.brokenVroutes.insert(oldPathid);
                    
                    vrouteItr++;
                }
                
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
            lostPneiItr->second.timer->setRetryCount(0);
            // double delay = (failureSimulateLink && !failureSimulationMap.empty()) ? uniform(0, repairLinkReqfailureSimulateLinkMaxDelay) : 0.01;  // if failureSimulateLink, all link failures will be detected at the same time at both ends, thus add random delay to avoid many nodes flooding at the same time
            double delay = 0.01;    // add minimal delay to ensure I check delayedRepairLinkReq before sending repairLinkReq, bc even if I'm the larger link end, I may have detected link failure too late that the smaller end has already sent repairLinkReq to me
            // NOTE each end of the failed link may need to send RepairLinkReq if it has lost prevhop of some brokenVroute, but if both ends still alive and close, only one temporary route needs to be setup btw them, hence the smaller link end will send RepairLinkReq later
            if (vid > pneiVid) {
                // schedule a repairLinkReq to send now
                scheduleAt(simTime() +delay, lostPneiItr->second.timer);
                EV_WARN << "Scheduling a RepairLinkReq to lost pnei " << pneiVid << " right away at " << lostPneiItr->second.timer->getArrivalTime() << endl;
            }
            else {    
                // schedule a repairLinkReq to send later
                // NOTE the larger link end will wait for repairLinkReqWaitTime + repairLinkReqSmallEndSendDelay before tearing down the broken vroutes, in case its nexthop has failed and its nextnexthop is the smaller link end of the failed link (nexthop, nextnexthop)
                scheduleAt(simTime() +delay + repairLinkReqSmallEndSendDelay, lostPneiItr->second.timer);
                EV_WARN << "Scheduling a RepairLinkReq to lost pnei " << pneiVid << " at " << lostPneiItr->second.timer->getArrivalTime() << endl;
                
                // schedule a timer to tear down broken vroutes if they aren't repaired in time
                // lostPneiItr->second.timer->setRetryCount(repairLinkReqRetryLimit);
                // scheduleAt(simTime() + 1.2 * repairLinkReqWaitTime, lostPneiItr->second.timer);
                // EV_DETAIL << "Scheduling a WaitRepairLinkIntTimer to tear down broken vroutes because of lost pnei " << pneiVid << endl;
            }
        }
    }
    // send Teardown for routes broken by lost pneis
    // NOTE next hop can't be using temporary route because if temporary routes are allowed, this regular vroute should be added to brokenVroutes rather than being torn down; only temporary and dismantled routes need to send Teardown
    for (const auto& mappair : nextHopToPathidsMap) {
        if (std::find(pneiVids.begin(), pneiVids.end(), mappair.first) == pneiVids.end()) {  // if nextHopAddr of Teardown isn't one of removed pneis
            const auto& teardownOut = createTeardown(/*pathids=*/mappair.second, /*addSrcVset=*/false, /*rebuild=*/true, /*dismantled=*/true);
            sendCreatedTeardown(teardownOut, /*nextHopPnei=*/mappair.first);

            if (recordStatsToFile) {   // record sent message
                std::ostringstream s;
                s << "handleLostPneis " << mappair.second;
                recordMessageRecord(/*action=*/0, /*src=*/vid, /*dst=*/VLRRINGVID_NULL, "Teardown", /*msgId=*/0, /*hopcount=*/0, /*chunkByteLength=*/0, /*infoStr=*/"handleLostPneis");   // unused params (msgId, hopcount, chunkByteLength) are just assigned to 0
            }
        }
    }
    // check delayedRepairLinkReq to see if I can reply to brokenVroutes in received repairLinkReq after new link breakage detected
    if (!delayedRepairReqTimer->isScheduled())
        scheduleAt(simTime(), delayedRepairReqTimer);
}

void Vlr::processDelayedRepairReqTimer()
{
    if (setupTempRoute) {
        if (sendRepairLocalNoTemp) {    // use repairLocalReq
            processDelayedRepairLocalReq();
        } else {    // use repairLinkReq
            processDelayedRepairLinkReq();
        }
    }
}

NotifyLinkFailureInt* Vlr::createNotifyLinkFailure(bool simLinkUp)
{
    EV_DEBUG << "Creating NotifyLinkFailure" << endl;
    NotifyLinkFailureInt *msg = new NotifyLinkFailureInt(/*name=*/"NotifyLinkFailure");
    msg->setSrc(vid);                       // vidByteLength
    msg->setSimLinkUp(simLinkUp);             // 1 bit, neglected
    int chunkByteLength = VLRRINGVID_BYTELEN;

    msg->setByteLength(chunkByteLength);
    return msg;
}

void Vlr::sendNotifyLinkFailure(const int& outGateIndex, bool simLinkUp)
{
    NotifyLinkFailureInt *msg = createNotifyLinkFailure(simLinkUp);

    EV_INFO << "Sending NotifyLinkFailure" << endl;
    sendCreatedPacket(msg, /*unicast=*/true, /*outGateIndex=*/outGateIndex, /*delay=*/0, /*checkFail=*/false);
}

void Vlr::processNotifyLinkFailure(NotifyLinkFailureInt *msgIncoming, bool& pktForwarded)
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

void Vlr::processFailedPacket(cPacket *packet, unsigned int pneiVid)
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

void Vlr::processRepSeqExpirationTimer()
{
    // a valid rep seqNo timed out
    EV_DEBUG << "Processing repExpirationTimer at node " << vid << ", representative " << representative << " expired, leaving network" << endl;
    ASSERT(representativeFixed);    // repSeqExpirationTimer only scheduled when rep is fixed
    
    writeFirstRepSeqTimeoutToFile(/*writeAtFinish=*/false);     // record firstRepSeqTimeoutTime and finalRepSeqTimeoutTime
    if (totalNumBeacomSentWriteTime == 0 || simTime() < totalNumBeacomSentWriteTime)
        totalNumRepSeqTimeout++;
    nodesRepSeqValid.erase(vid);

    if (recordStatsToFile) { // write node status update
        recordNodeStatsRecord(/*infoStr=*/"repLost");   // unused params (stage)
    }

    // tear down all vroutes in routing table, inNetwork = false
    leaveOverlay();
    representative.heardfromvid = VLRRINGVID_NULL;      // this means my rep seqNo is invalid
    // schedule observe period (don't send setupReq) after rep-timeout
    scheduleAt(simTime() + repSeqValidityInterval, repSeqObserveTimer);
    
}

void Vlr::leaveOverlay()
{
    // tear down all vroutes in vlrRoutingTable
    // NOTE not sending Teardown for broken vroutes over temporary routes, since tearing down temporary routes should notify the lost pnei to tear down broken vroutes
    std::map<VlrRingVID, std::vector<VlrPathID>> nextHopToPathidsMap;    // for Teardown to send, map next hop address to pathids
    for (auto vrouteItr = vlrRoutingTable.vlrRoutesMap.begin(); vrouteItr != vlrRoutingTable.vlrRoutesMap.end(); ) {
        const VlrPathID& oldPathid = vrouteItr->first;     // pathid to tear down
        // send Teardown to route prevhop and nexthop
        std::vector<VlrRingVID> sendTeardownToAddrs = {vrouteItr->second.prevhopVid, vrouteItr->second.nexthopVid};
        // get 2-bit isUnavailable of prevhop and nexthop
        std::vector<char> nextHopStates = {vlrRoutingTable.getPrevNextIsUnavailable(vrouteItr->second.isUnavailable, /*getPrev=*/true), vlrRoutingTable.getPrevNextIsUnavailable(vrouteItr->second.isUnavailable, /*getPrev=*/false)};
        EV_DETAIL << "Teardown (pathid = " << oldPathid << ") will be sent to [";
        for (const auto& nextHopVid : sendTeardownToAddrs)
            if (nextHopVid != VLRRINGVID_NULL)
                EV_DETAIL << nextHopVid << ' ';
        EV_DETAIL << ']' << endl;

        for (size_t i = 0; i < sendTeardownToAddrs.size(); ++i) {
            const VlrRingVID& nextHopVid = sendTeardownToAddrs[i];
            const char& nextHopIsUnavailable = nextHopStates[i];
            if (nextHopVid != VLRRINGVID_NULL && (nextHopIsUnavailable == 0 || nextHopIsUnavailable == 3)) {  // I'm not an endpoint of the vroute, and next hop isn't a lost pnei
                auto nextHopItr = nextHopToPathidsMap.find(nextHopVid);
                if (nextHopItr == nextHopToPathidsMap.end()) 
                    nextHopToPathidsMap[nextHopVid] = {oldPathid};
                else
                    nextHopItr->second.push_back(oldPathid);
            }
        }
        // will delete all routes in endpointToRoutesMap and vlrRoutesMap later
        vrouteItr++;
    }
    // send Teardown with multiple pathids if possible
    for (const auto& mappair : nextHopToPathidsMap) {
        const auto& teardownOut = createTeardown(/*pathids=*/mappair.second, /*addSrcVset=*/false, /*rebuild=*/false, /*dismantled=*/false);
        sendCreatedTeardown(teardownOut, /*nextHopPnei=*/mappair.first);

        if (recordStatsToFile) {   // record sent message
            recordMessageRecord(/*action=*/0, /*src=*/vid, /*dst=*/VLRRINGVID_NULL, "Teardown", /*msgId=*/0, /*hopcount=*/0, /*chunkByteLength=*/0, /*infoStr=*/"leaveOverlay");   // unused params (msgId, hopcount, chunkByteLength) are just assigned to 0
        }
    }
    // we've sent Teardown for all vroutes, reset overlay state
    clearState(/*clearPsetAndRep=*/false);
}

void Vlr::processRepSeqObserveTimer()
{
    // wait time over after rep-timeout, can join overlay again if pnei inNetwork
    EV_DEBUG << "Processing repSeqObserveTimer at node " << vid << ", observation period after rep-timeout is over, ready to join vnetwork" << endl;
}

void Vlr::processInNetworkWarmupTimer()
{
    EV_DEBUG << "Processing inNetwork warmup timer at node " << vid << endl;
    if (!selfInNetwork) {
        bool allowInNetworkWithEmptyVset = (!representativeFixed && !startingRootFixed);    // if startingRoot is fixed, can only become inNetwork if vset isn't empty or I'm startingRoot
        
        if (!vset.empty() || representative.vid == vid || allowInNetworkWithEmptyVset) {
            ASSERT(!representativeFixed || representative.heardfromvid != VLRRINGVID_NULL);     // since vset not empty or I'm rep myself
            bool allowInNetwork = true;
            if (vset.size() < 2 * vsetHalfCardinality) {    // vset not full 
                if (pendingVsetChangedSinceLastCheckAndSchedule && !pendingVset.empty()) {     // only use hasSetupReqPending() when pendingVsetChangedSinceLastCheckAndSchedule=false or pendingVset empty
                    allowInNetwork = false;
                    ASSERT(fillVsetTimer->isScheduled());   // fillVsetTimer should always be scheduled, especially now that vset isn't empty

                    double maxDelayToNextTimer = beaconInterval;
                    if (fillVsetTimer->getArrivalTime() - simTime() >= maxDelayToNextTimer) {    // only reschedule fillVsetTimer if it won't come within maxDelayToNextTimer
                        cancelEvent(fillVsetTimer);
                        scheduleFillVsetTimer(/*firstTimer=*/false, /*maxDelay=*/maxDelayToNextTimer);
                    }
                    // reschedule to after pendingVsetChangedSinceLastCheckAndSchedule becomes false in processFillVsetTimer()
                    scheduleAt(fillVsetTimer->getArrivalTime() + 1, inNetworkWarmupTimer);
                    EV_INFO << "selfInNetwork warm-up time is over, but vset isn't full and pendingVsetChangedSinceLastCheckAndSchedule=true, extending inNetwork warm-up" << endl;
                    return;
                }
                // checked pendingVsetChangedSinceLastCheckAndSchedule = false or pendingVset is currently empty
                simtime_t_cref someArrivalTime = getASetupReqPendingArrivalTime();
                if (!someArrivalTime.isZero()) {    // hasSetupReqPending() = true, I might be repairing vroute to a lost vnei, wait till setupReq timer timeout
                    allowInNetwork = false;
                    scheduleAt(someArrivalTime + 1, inNetworkWarmupTimer);
                    EV_INFO << "selfInNetwork warm-up time is over, but vset isn't full and I have scheduled setupReq timer, extending inNetwork warm-up" << endl;
                } // else, hasSetupReqPending() = false, keep allowInNetwork = true
            }
            if (allowInNetwork && allowInNetworkWithEmptyVset && vset.empty()) {     // startingRoot isn't fixed and I'm allowed inNetwork with empty vset
                // assert I don't have LINKED && inNetwork pnei
                VlrRingVID proxy = getProxyForSetupReq();
                if (proxy != VLRRINGVID_NULL && fillVsetTimer->isScheduled()) {     // fillVsetTimer should always be scheduled after node start
                    allowInNetwork = false;
                    scheduleAt(fillVsetTimer->getArrivalTime() + 1, inNetworkWarmupTimer);
                    EV_INFO << "selfInNetwork warm-up time is over, startingRoot isn't fixed and vset is empty, but I have LINKED && inNetwork pnei, extending inNetwork warm-up" << endl;
                } else {
                    // if I have a valid rep smaller than me, wait for it to start the overlay
                    simtime_t expiredLastheard = simTime() - repSeqValidityInterval;
                    for (auto repMapItr = representativeMap.begin(); repMapItr != representativeMap.end(); ++repMapItr) {
                        if (repMapItr->first < vid && repMapItr->second.heardfromvid != VLRRINGVID_NULL && repMapItr->second.lastHeard > expiredLastheard) {  // rep hasn't expired
                            allowInNetwork = false;
                            scheduleAt(repMapItr->second.lastHeard + repSeqValidityInterval + 1, inNetworkWarmupTimer);
                            EV_INFO << "selfInNetwork warm-up time is over, startingRoot isn't fixed and vset is empty, but I have heard a representative smaller than me, extending inNetwork warm-up" << endl;
                            break;
                        } else if (repMapItr->first > vid)
                            break;
                    }
                }
            }
            if (allowInNetwork) {    // either vset is full or I have no scheduled setupReq to send
                selfInNetwork = true;
                if (vset.size() >= 2 * vsetHalfCardinality) {   // not likely bc inNetwork warmup timer would've been cancelled when vset become full
                    EV_INFO << "selfInNetwork warm-up time is over, setting inNetwork = " << selfInNetwork << endl;
                    if (recordStatsToFile) { // write node status update
                        std::ostringstream s;
                        s << "vsetFull: inNetwork=" << selfInNetwork << " vset=" << printVsetToString() << " vsetSize=" << vset.size();
                        recordNodeStatsRecord(/*infoStr=*/s.str().c_str());   // unused params (stage)
                    }
                } else {    // vset unfull yet
                    EV_WARN << "selfInNetwork warm-up time is over, setting inNetwork = " << selfInNetwork << " with unfull vset = " << printVsetToString() << endl;

                    if (recordStatsToFile) { // write node status update
                        std::ostringstream s;
                        s << "inNetwork=" << selfInNetwork << " vset=" << printVsetToString() << " vsetSize=" << vset.size();
                        recordNodeStatsRecord(/*infoStr=*/s.str().c_str());   // unused params (stage)
                    }
                }
            }
        } else {
            EV_INFO << "selfInNetwork warm-up time is over, allowInNetworkWithEmptyVset=" << allowInNetworkWithEmptyVset << ", vset size = " << vset.size() << ", inNetwork still false" << endl;
        }
    }
}

void Vlr::processWaitSetupReqTimer(WaitSetupReqIntTimer *setupReqTimer)
{
    // WaitSetupReqIntTimer *setupReqTimer = check_and_cast<WaitSetupReqIntTimer *>(message);
    VlrRingVID targetVid = setupReqTimer->getDst();
    int retryCount = setupReqTimer->getRetryCount();
    bool repairRoute = setupReqTimer->getRepairRoute();     // setupReq should be sent with repairRoute=true and traceVec to record trace toward dst
    // bool allowSetupReqTrace = setupReqTimer->getAllowSetupReqTrace();     // setupReqTrace should be sent if setupReqRetryLimit has been reached
    bool reqVnei = setupReqTimer->getReqVnei();     // setupReq should be sent with reqVnei=true and traceVec to record trace toward dst
    char timerType = setupReqTimer->getTimerType();     // 0: pendingVset[dst], 1: vset[dst]

    EV_DETAIL << "(me=" << vid << ") Processing wait setupReq timer to node " << targetVid << ", WaitSetupReqIntTimer retryCount: " << retryCount << ", repairRoute: " << repairRoute
            << ", reqVnei: " << reqVnei << ", timerType: " << (int)timerType << endl;

    if (representativeFixed && representative.heardfromvid == VLRRINGVID_NULL) {        // if I don't have a valid rep, I shouldn't try to build vroute with setupReq
        EV_WARN << "No valid rep heard: " << representative << ", cannot send setupReq message, scheduling repSeqExpirationTimer to tear down all vroutes" << endl;
        scheduleAt(simTime(), repSeqExpirationTimer);
        return;
    }

    bool resendSetupReq = false;
    
    if (timerType == 0) {    // targetVid is in pendingVset

        // NOTE targetVid is in pendingVset and cannot be a current vnei, hence if reqVnei=false, this setupReq will build a nonEss vroute to targetVid
        if (!reqVnei && retryCount > 0 && IsLinkedPneiOrAvailableWantedRouteEnd(targetVid)) {   // targetVid is linked pnei or available and wanted vroute endpoint in vlrRoutingTable, no need to build nonEss vroute to it any more
            EV_INFO << "PendingVset node " << targetVid << " is a linked pnei or existing vroute endpoint (not in nonEssUnwantedRoutes), no need to setup nonEss vroute, deleting setupReq timer" << endl;
            auto itr = pendingVset.find(targetVid);
            if (itr != pendingVset.end() && itr->second.setupReqTimer) {    // should be true since setupReqTimer was allocated and scheduled
                cancelAndDelete(itr->second.setupReqTimer);
                itr->second.setupReqTimer = nullptr;
            }
            return;
        }
        
        // Commented out when setupReqRetryLimit = VLRSETUPREQ_THRESHOLD -- NOTE if retryCount > setupReqRetryLimit, it's not considered a proper retry, retryCount is probably set manually, manual retryCount range: [VLRSETUPREQ_THRESHOLD - setupReqRetryLimit, VLRSETUPREQ_THRESHOLD-1], we'll send a setupReq
        if (retryCount < setupReqRetryLimit /*|| (retryCount >= (VLRSETUPREQ_THRESHOLD - setupReqRetryLimit) && retryCount < VLRSETUPREQ_THRESHOLD)*/) {
            resendSetupReq = true;

            if (retryCount > 0)
                EV_INFO << "WaitSetupReqIntTimer retryCount > 0! Processing wait setupReq timer to node " << targetVid << ", retryCount = " << retryCount << endl;
            
            // Commented out bc checking retryCount when schedule setupReq timer for pendingVset in processFillVsetTimer()
            // if (reqVnei && vset.size() < 2 * vsetHalfCardinality && retryCount > 0) {    // retryCount >= setupReqRetryLimit / 2  vset unfull and targetVid is a potential vnei that I'm currently looking for, also look for the next closest potential vnei in pendingVset
            //     VlrRingVID alterPendingVnei_old = setupReqTimer->getAlterPendingVnei();
            //     auto pendingItr = pendingVset.end();
            //     char targetVidFound = findAlterPendingVneiOf(/*referenceVid=*/targetVid, pendingItr);
            //     if (pendingItr != pendingVset.end()) {  // found alterPendingVnei for targetVid
            //         // Commented out for simplicity, each pendingVnei can schedule setupReq to its current alterPendingVnei with reqVnei=true, then that pendingVnei can find further alterPendingVnei, this can set reqVnei=true for nodes that don't belong to vset and they will be added when vset is unfull
            //         // // if I've sent SetupReq(reqVnei=true) to selected alterPendingVnei_old and it has been added to vset, but if alterPendingVnei_old is farther than alterPendingVnei_new, schedule setupReq to pendingVnei btw targetVid and alterPendingVnei_old
            //         // bool alterPendingVnei_old_isInvalid = false;
            //         // VlrRingVID alterPendingVnei_new = pendingItr->first;
            //         // if (alterPendingVnei_old != VLRRINGVID_NULL && alterPendingVnei_old != alterPendingVnei_new && vset.find(alterPendingVnei_old) != vset.end()) {  // neglect alterPendingVnei_old if alterPendingVnei_new is even closer to me
            //         //     if (targetVidFound == 1 && getVid_CW_Distance(vid, alterPendingVnei_old) > getVid_CW_Distance(vid, alterPendingVnei_new))    // targetVid is a cw pendingVnei, alterPendingVnei_new is also likely a cw pendingVnei
            //         //         alterPendingVnei_old_isInvalid = true;
            //         //     if (targetVidFound == 2 && getVid_CCW_Distance(vid, alterPendingVnei_old) > getVid_CCW_Distance(vid, alterPendingVnei_new))    // targetVid is a ccw pendingVnei, alterPendingVnei_new is also likely a ccw pendingVnei
            //         //         alterPendingVnei_old_isInvalid = true;
            //         // }
            //         // if (alterPendingVnei_old == VLRRINGVID_NULL || vset.find(alterPendingVnei_old) == vset.end() || alterPendingVnei_old_isInvalid) {   // schedule setupReq to alterPendingVnei_new
            //         // VlrRingVID referenceVid = targetVid;    // alterPendingVnei will be the closest pendingVnei just farther than referenceVid
            //         // VlrRingVID alterVneiPreclude = VLRRINGVID_NULL;     // avoid setting alterVneiPreclude as alterPendingVnei
            //         // if (alterPendingVnei != VLRRINGVID_NULL) {
            //         //     auto pendingItr = pendingVset.find(alterPendingVnei);
            //         //     if (pendingItr != pendingVset.end() && pendingItr->second.setupReqTimer && pendingItr->second.setupReqTimer->isScheduled() && pendingItr->second.setupReqTimer->getRetryCount() > 1) {
            //         //         // if setupReq has been sent and retried for alterPendingVnei, perhaps it's not available, find the next closest pendingVnei just farther than alterPendingVnei
            //         //         referenceVid = alterPendingVnei;
            //         //         alterVneiPreclude = pendingItr->second.setupReqTimer->getAlterPendingVnei();    // avoid setting alterPendingVnei of my current alterPendingVnei as my new alterPendingVnei
            //         //     }
            //         // }
            //         // auto pendingItr = pendingVset.end();
            //         // char referenceVidFound = findAlterPendingVneiOf(/*referenceVid=*/referenceVid, pendingItr);
            //         // if (pendingItr != pendingVset.end() && pendingItr->first != alterVneiPreclude) {
            //         EV_INFO << "Selected next potential vnei (alterPendingVnei) " << pendingItr->first << " for pendingVnei " << targetVid << " (retryCount=" << retryCount << ") at node " << vid << endl;
            //         setupReqTimer->setAlterPendingVnei(pendingItr->first);  // recorded selected alterPendingVnei
            //         int closeCount = 2;
            //         setRouteToPendingVsetItr(/*pendingItr=*/pendingItr, /*sendSetupReq=*/true, /*isCCW=*/false, closeCount, /*tryNonEssRoute=*/false, /*expiredPendingVneisPtr=*/nullptr);
            //     }                
            // }

            bool setupReqSent = false;
            bool recordMessageAndReschedule = false;
            bool msgInfoRecordTrace = false;
            bool msgInfoRouteWithTrace = false;
            
            // Commented out bc we want to add random delay only for the first setupReq(repairRoute=true) to targetVid to avoid jamming the failure area, that delay is added when we first schedule WaitSetupReqIntTimer to repair broken pathid in removeEndpointOnTeardown()
            // double delay = (repairRoute) ? uniform(0, patchedRouteRepairSetupReqMaxDelay) : 0;  // if this setupReq is repairing a patched route, add a random delay to avoid jamming the failure area
            double delay = 0;
            // resend setupReq to targetVid

            // if repairRoute=false, see if there's an overheard trace to targetVid
            auto overheardItr = overheardTraces.find(targetVid);
            if (!repairRoute && overheardItr != overheardTraces.end()) {    // overheard trace found
                std::vector<VlrRingVID>& pathToDst = overheardItr->second.first;    // pathToDst: [me, .., dst]
                if (pathToDst.front() != vid)    // ensure path includes me as first hop
                    pathToDst.insert(pathToDst.begin(), vid);
                // std::reverse(pathToDst.begin(), pathToDst.end());   // overheardItr->second: [me, .., dst]

                // send setupReq to dst using overheard trace
                unsigned int nextHopIndex = getNextHopIndexInTraceForSetupReq(pathToDst, /*myIndexInTrace=*/0);
                // reqOutgoing->getTraceVec(): [src, .., me, .., dst]
                if (nextHopIndex >= pathToDst.size()) {
                    EV_WARN << "No next hop found to send SetupReq with overheard trace at me = " << vid << " to node " << targetVid << " in pendingVset" << endl;
                } else {    // nexthop found for SetupReq with trace
                    setupReqSent = true;
                    recordMessageAndReschedule = true;
                    auto setupReq = createSetupReq(targetVid, /*proxy=*/VLRRINGVID_NULL, /*reqDispatch=*/false);
                    setupReq->setRepairRoute(repairRoute);  // repairRoute=false
                    setupReq->setReqVnei(reqVnei);
                    setupReq->setRecordTrace(false);
                    setupReq->setTraceVec(pathToDst);
                    setupReq->setIndexInTrace(nextHopIndex);     // set indexInTrace to index of next hop
                    VlrRingVID nextHopVid = pathToDst.at(nextHopIndex);
                    EV_INFO << "Sending SetupReq to node " << targetVid << " with overheard trace = " << pathToDst << ", nexthop: " << nextHopVid << endl;
                    sendCreatedSetupReq(setupReq, /*outGateIndex=*/psetTable.getPneiGateIndex(nextHopVid), /*computeChunkLength=*/true, delay);
                    msgInfoRouteWithTrace = true;
                }
                overheardTraces.erase(overheardItr);    // remove overheard trace to targetVid after it has been used to build a vroute
            }
            else {  // no overheard trace to targetVid
                const VlrRingVID& heardFrom = pendingVset[targetVid].heardFrom;     // targetVid must exist in pendingVset if WaitSetupReqIntTimer to targetVid has been allocated
                VlrRingVID transferNode_potential = heardFrom;      // transferNode to be used if transferNode allowed (!repairRoute && retryCount >= 1) and transferNode_potential isn't null
                if ((retryCount > setupReqRetryLimit / 2 || transferNode_potential == VLRRINGVID_NULL && retryCount == setupReqRetryLimit / 2) && uniform(0, 1) < 0.5) {    // give some probability for setupReq to use a random transfer node
                    do {    // loop in case selected transferNode_potential is myself or targetVid itself
                        transferNode_potential = intuniform(0, 60000);
                    } while (transferNode_potential == vid || transferNode_potential == targetVid);
                }
                // if repairRoute, I should have an existing vroute to dst with temporary route and shouldn't need transferNode
                // I won't use heardFrom as transferNode on the first trial to targetVid (retryCount == 0)
                VlrRingVID vlrOptionDst = (transferNode_potential == VLRRINGVID_NULL || repairRoute || retryCount < 1) ? targetVid : transferNode_potential;
                // VlrIntOption *vlrOption = createVlrOption(vlrOptionDst);
                VlrIntOption vlrOption;
                initializeVlrOption(vlrOption, /*dstVid=*/vlrOptionDst);

                // find next hop to dst with findNextHop() to utilize existing vroutes in routing table
                // VlrIntOption *vlrOption = createVlrOption(targetVid);     // create VlrOption to be set in IP header in datagramLocalOutHook()
                // L3Address nextHopAddr = findNextHop(vlrOption, /*prevHopAddrPtr=*/nullptr, /*excludeVid=*/VLRRINGVID_NULL, /*allowTempRoute=*/true);    // no need to specify excludeVid bc findNextHop() won't return myself anyway
                VlrRingVID nextHopVid = VLRRINGVID_NULL;
                if (repairRoute) {  // use the patched route to be repaired to reach targetVid if possible  NOTE if patchedRoute is broken, we should receive Teardown to remove it from vlrRoutingTable, and repairRoute will be set to false when patchedRoute isn't found
                    vlrOption.setCurrentPathid(setupReqTimer->getPatchedRoute());
                    vlrOption.setTowardVid(targetVid);
                    nextHopVid = findNextHopForSetupReq(vlrOption, /*prevHopVid=*/VLRRINGVID_NULL, /*dstVid=*/targetVid, /*newnode=*/VLRRINGVID_NULL, /*allowTempRoute=*/true);  // no need to specify excludeVid bc findNextHop() won't return myself anyway
                    // Commented out bc we only try to route to targetVid w/o patchedRoute if reqVnei=true
                    // if (nextHopAddr.isUnspecified()) {  // patchedRoute isn't available in vlrRoutingTable
                    //     EV_DETAIL << "PatchedRoute = " << setupReqTimer->getPatchedRoute() << " to dst = " << targetVid << " isn't available in vlrRoutingTable, setting repairRoute=false in wait setupReq timer" << endl;
                    //     repairRoute = false;
                    //     setupReqTimer->setRepairRoute(false);
                    // }
                } else {    // repairRoute=false
                // if (nextHopAddr.isUnspecified()) {  // repairRoute=false or repairRoute=true but patchedRoute isn't available in vlrRoutingTable, in latter case vlrOptionDst = targetVid
                    // vlrOption->setCurrentPathid(VLRPATHID_INVALID);
                    nextHopVid = findNextHopForSetupReq(vlrOption, /*prevHopVid=*/VLRRINGVID_NULL, /*dstVid=*/targetVid, /*newnode=*/VLRRINGVID_NULL, /*allowTempRoute=*/true);  // no need to specify excludeVid bc findNextHop() won't return myself anyway
                }
                
                if (nextHopVid == VLRRINGVID_NULL) {  // no pnei or vroute endpoint closer than me to dst
                    // delete vlrOption;
                    EV_DETAIL << "No greedy next hop found to send setupReq to dst = " << targetVid << endl;

                    if (vlrOptionDst == heardFrom)  // no next hop to heardFrom, don't use heardFrom as transferNode next time
                        pendingVset[targetVid].heardFrom = VLRRINGVID_NULL;

                    // if repairRoute, I should have an existing vroute to dst with temporary route
                    // only use transferNode if I can route to it, otherwise discard transferNode
                    if (!repairRoute) {  // send to any LINKED && inNetwork pnei and hope it can forward this msg to dst
                        VlrRingVID proxy = getProxyForSetupReq(/*checkInNetwork=*/true);
                        // VlrRingVID proxy = getClosestPneiTo(targetVid, /*excludeVid=*/vid, /*checkInNetwork=*/true).first;     // get LINKED && inNetwork pnei that's closest to targetVid, as long as there's a LINKED && inNetwork pnei, this shouldn't return NULL
                        if (proxy == VLRRINGVID_NULL && reqVnei && vset.size() > 0 && retryCount > 0) {    // retryCount >= setupReqRetryLimit / 2  vset isn't empty, no LINKED && inNetwork pnei to be used as proxy, perhaps my pneis are repairing routes to their vneis, try a pnei not inNetwork to utilize remaining routes
                            proxy = getProxyForSetupReq(/*checkInNetwork=*/false);
                        }
                        if (proxy != VLRRINGVID_NULL) {
                            EV_DETAIL << "Sending setupReq to dst = " << targetVid << ", proxy = " << proxy << endl;
                            setupReqSent = true;
                            // will reschedule this setupReqTimer and increment retryCount
                            sendSetupReq(targetVid, proxy, /*reqDispatch=*/false,  /*recordTrace=*/recordTraceForDirectedSetupReq, /*reqVnei=*/reqVnei, setupReqTimer);
                        }
                    } else {    // repairRoute=true but the patched route to be repaired isn't available, don't try to repair it the next time
                        setupReqTimer->setRepairRoute(false);
                    }
                } else {    // resend setupReq with traceVec
                    VlrRingVID transferNode = (vlrOption.getDstVid() == targetVid) ? VLRRINGVID_NULL : transferNode_potential;
                    VlrPathID patchedRouteToRepair = (repairRoute) ? setupReqTimer->getPatchedRoute() : VLRPATHID_INVALID;
                    // record trace if using repairLink and selected currentPathid goes to targetVid directly (may be a patched vroute)
                    bool reqRecordTrace = (repairRoute || transferNode != VLRRINGVID_NULL || recordTraceForDirectedSetupReq || vlrOption.getCurrentPathid() == VLRPATHID_REPPATH || (setupTempRoute && !sendRepairLocalNoTemp && vlrOption.getTowardVid() == targetVid));
                    // if (vlrOption.getCurrentPathid() == VLRPATHID_REPPATH)
                    //     EV_INFO << "ohno" << endl;
                    // if setupReq not recording trace, but I'm using rep-path to targetVid, it's possible that overlay is partitioned and setupReply can't be greedy routed back, thus add trace
                    // if (!reqRecordTrace && (selfInNetwork || !vset.empty()) && reqVnei) {  //  && !repairRoute
                    //     bool otherEndIsRep = false;
                    //     if (representativeFixed) {
                    //         otherEndIsRep = (targetVid == representative.vid && representative.heardfromvid != VLRRINGVID_NULL && repSeqExpirationTimer->isScheduled());
                    //     } else {
                    //         auto repMapItr = representativeMap.find(targetVid);
                    //         simtime_t expiredLastheard = simTime() - repSeqValidityInterval;
                    //         otherEndIsRep = (repMapItr != representativeMap.end() && repMapItr->second.heardfromvid != VLRRINGVID_NULL /*&& repMapItr->second.inNetwork*/ && repMapItr->second.lastHeard > expiredLastheard);   // NOTE if targetVid in representativeMap, we can use its rep-path no matter it's inNetwork or not
                    //     }
                    //     if (otherEndIsRep) {
                    //         reqRecordTrace = true;
                    //         // Commented out sending setupReply to targetVid using rep-path directly bc if targetVid isn't my vnei and tear down the vroute, I'll be no longer inNetwork, also, targetVid 
                    //         // VlrRingVID newnode = targetVid;
                    //         // VlrIntVidSet emptySet;         // knownSet only needed to determine neisToForward, since we don't care neisToForward, create dummy knownSet for shouldAddVnei(..)
                    //         // auto addResult = shouldAddVnei(newnode, emptySet, /*findNeisToForward=*/false);
                    //         // bool& shouldAdd = std::get<0>(addResult);  // access first element in tuple
                    //         // std::vector<VlrRingVID>& removedNeis = std::get<1>(addResult);
                            
                    //         // if (shouldAdd) {
                    //         //     EV_WARN << "Trying to add representative to vset, checking rep path to send SetupReply at me = " << vid << " to rep = " << newnode << endl;
                    //         //     // ensure it's possible for me send SetupReply to rep via rep path
                    //         //     VlrIntOption vlrOptionOut;
                    //         //     initializeVlrOption(vlrOptionOut, /*dstVid=*/newnode);
                    //         //     VlrRingVID nextHopVid = findNextHopForSetupReply(vlrOptionOut, /*prevHopVid=*/VLRRINGVID_NULL, /*newnode=*/newnode);
                    //         //     if (nextHopVid == VLRRINGVID_NULL) {
                    //         //         // delete vlrOptionOut;
                    //         //         EV_WARN << "No next hop found use rep path to send SetupReply at me = " << vid << " to rep = " << newnode << endl;
                    //         //     } else {
                    //         //         // send SetupReply to rep and add it to vset
                    //         //         sendGreedySetupReplyAddVnei(newnode, /*proxy=*/newnode, vlrOptionOut, nextHopVid, removedNeis);
                    //         //         return;
                    //         //     }
                    //         // }
                    //     }
                    // }
                    VlrRingVID proxy = (reqRecordTrace) ? VLRRINGVID_NULL : nextHopVid;
                    EV_DETAIL << "Found greedy first hop for setupReq to dst = " << targetVid << ", transferNode = " << transferNode << ", patchedRoute = " << patchedRouteToRepair << endl;
                    setupReqSent = true;
                    recordMessageAndReschedule = true;

                    auto setupReq = createSetupReq(targetVid, /*proxy=*/proxy, /*reqDispatch=*/false);
                    setupReq->setVlrOption(vlrOption);
                    setupReq->setRepairRoute(repairRoute);
                    setupReq->setPatchedRoute(patchedRouteToRepair);
                    setupReq->setReqVnei(reqVnei);
                    setupReq->setRecordTrace(reqRecordTrace);
                    setupReq->setTransferNode(transferNode);
                    if (reqRecordTrace)
                        setupReq->getTraceVecForUpdate().push_back(vid);  // put myself in traceVec
                    msgInfoRecordTrace = reqRecordTrace;
                    EV_INFO << "Sending SetupReq to node " << targetVid << " via greedy routing, nexthop: " << nextHopVid << endl;
                    sendCreatedSetupReq(setupReq, /*outGateIndex=*/psetTable.getPneiGateIndex(nextHopVid), /*computeChunkLength=*/true, delay);
                }
            }

            if (recordMessageAndReschedule) {
                if (recordStatsToFile) { // record sent message
                    std::ostringstream s;
                    if (msgInfoRecordTrace)
                        s << "recordTrace ";
                    if (msgInfoRouteWithTrace)
                        s << "routeWithTrace ";
                    s << "vset=" << printVsetToString(/*printVpath=*/true) << " pendingVset=" << printPendingVsetToString();
                    recordMessageRecord(/*action=*/0, /*src=*/vid, /*dst=*/targetVid, "SetupReq", /*msgId=*/0, /*hopcount=*/0, /*chunkByteLength=*/0, /*infoStr=*/s.str().c_str());   // unused params (msgId, hopcount, chunkByteLength) are just assigned to 0
                }
                // reschedule this setupReqTimer and increment retryCount
                setupReqTimer->setRetryCount(retryCount+1);
                // reschedule the wait setupReq timer w/o backoff
                // Commented out bc already added randomness of setupReq to different pendingVneis when first scheduling the wait setupReq timer
                // delay = (delay == 0 && retryCount > 0) ? uniform(0, 0.2) * routeSetupReqWaitTime : delay;  // add a random jitter to avoid multiple nodes sending SetupReq to find targetVid at same time
                scheduleAt(simTime() + delay + routeSetupReqWaitTime, setupReqTimer);
                EV_DETAIL << "Rescheduling setupReq timer: dst = " << setupReqTimer->getDst() << ", retryCount = " << setupReqTimer->getRetryCount() << endl;
            }

            if (!setupReqSent && reqVnei) /* && retryCount > setupReqRetryLimit */ {     // if setupReq can't be sent, increment retryCount so we may send a setupReqTrace later
                // int nextRetryCount = (retryCount+1 > VLRSETUPREQ_THRESHOLD) ? VLRSETUPREQ_THRESHOLD : (retryCount+1);
                int nextRetryCount = retryCount + 1;
                setupReqTimer->setRetryCount(nextRetryCount);
                EV_WARN << "No next hop found to send setupReq at me = " << vid << " on WaitSetupReqTimer retryCount = " << retryCount << ", rescheduling setupReq timer to dst = " << targetVid << ", new retryCount = " << nextRetryCount << endl;
                if (/*nextRetryCount == VLRSETUPREQ_THRESHOLD || allowSetupReqTrace ||*/ repairRoute)   // if didn't send setupReq bc repairRoute=true but patchedRoute no longer available, try greedy routing to find targetVid right away bc we didn't try this time
                    scheduleAt(simTime() + beaconInterval, setupReqTimer);
                else    // setupReqTrace won't be sent so give more time before another setupReq trial
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
            EV_WARN << "SetupReq attempts for node " << targetVid << " reached setupReqRetryLimit=" << setupReqRetryLimit << " limit. Stop sending setupReq." << endl;
            // delete setupReqTimer;    // don't delete because we need to record number of setupReq sent to targetVid
            
            // delete targetVid from pendingVset as it's not reachable
            EV_WARN << "Deleting node " << targetVid << " from pendingVset" << endl;
            pendingVsetErase(targetVid);    // this setupReqTimer will be cancelAndDelete()
        }
    }
    // else if (timerType == 1) {    // targetVid is in vset, this setupReq is to build a secondary vset-route to targetVid
    //     auto vneiItr = vset.find(targetVid);

    //     if (retryCount < setupReqRetryLimit) {

    //         if (!IsLinkedPneiOrOverheardMPnei(targetVid)) {   // targetVid is linked pnei or in overheardMPneis, no need to build second vset route to it any more
    //             EV_INFO << "Vset node " << targetVid << " is a linked pnei or mpnei in overheardMPneis, no need to setup second vset route, deleting setupReq timer" << endl;
    //             if (vneiItr != vset.end() && vneiItr->second.setupReqTimer) {    // should be true since setupReqTimer was allocated and scheduled
    //                 cancelAndDelete(vneiItr->second.setupReqTimer);
    //                 vneiItr->second.setupReqTimer = nullptr;
    //             }
    //             return;
    //         }

    //         bool recordMessageAndReschedule = false;
    //         // Commented out bc we want to add random delay only for the first setupReq(repairRoute=true) to targetVid to avoid jamming the failure area, that delay is added when we first schedule WaitSetupReqIntTimer to repair broken pathid in removeEndpointOnTeardown()
    //         // double delay = (repairRoute) ? uniform(0, patchedRouteRepairSetupReqMaxDelay) : 0;  // if this setupReq is repairing a patched route, add a random delay to avoid jamming the failure area
    //         double delay = 0;

    //         if (repairRoute) {
    //             // find next hop to dst with findNextHop() to utilize existing vroutes in routing table
    //             VlrIntOption *vlrOption = createVlrOption(targetVid);
    //             vlrOption->setCurrentPathid(setupReqTimer->getPatchedRoute());
    //             vlrOption->setTowardVid(targetVid);
    //             L3Address nextHopAddr = findNextHopForSetupReq(vlrOption, /*prevHopAddrPtr=*/nullptr, /*dstVid=*/targetVid, /*newnode=*/VLRRINGVID_NULL, /*allowTempRoute=*/true);  // no need to specify excludeVid bc findNextHop() won't return myself anyway

    //             if (nextHopAddr.isUnspecified()) {  // patched route isn't available
    //                 delete vlrOption;
    //                 EV_DETAIL << "No next hop found to send SetupReq(repairRoute=true) at me = " << vid << " to dst = " << targetVid << " to repair patched vroute " << setupReqTimer->getPatchedRoute() << endl;

    //             } else {    // resend setupReq with traceVec
    //                 resendSetupReq = true;
    //                 recordMessageAndReschedule = true;
    //                 auto setupReq = createSetupReq(targetVid, /*proxy=*/VLRRINGVID_NULL, /*reqDispatch=*/false);
    //                 setupReq->setRepairRoute(repairRoute);
    //                 setupReq->setPatchedRoute(setupReqTimer->getPatchedRoute());
    //                 setupReq->setReqVnei(reqVnei);
    //                 setupReq->setRecordTrace(true);
    //                 setupReq->getTraceVecForUpdate().push_back(vid);  // put myself in traceVec
    //                 EV_INFO << "Sending SetupReq to node " << targetVid << " via greedy routing, nexthop: " << nextHopAddr << endl;
    //                 sendCreatedSetupReq(setupReq, nextHopAddr, vlrOption, /*computeChunkLength=*/true, delay);
    //             }
    //         }
    //         else {  // build a second vset route to targetVid via proxy2
    //             VlrRingVID proxy = getProxy2ForSecondRoute(/*currRoutes=*/vneiItr->second.vsetRoutes, /*towardVid=*/targetVid);      // exclude next hop of current vset routes to vnei
    //             // VlrRingVID proxy = getProxy2ForSecondRoute(/*currRoutes=*/vlrRoutingTable.endpointToRoutesMap.at(vnei), /*towardVid=*/targetVid);    // exclude next hop of all existing vroutes to vnei
    //             if (proxy != VLRRINGVID_NULL) {
    //                 EV_DETAIL << "Sending setupReq to dst = " << targetVid << ", proxy = " << proxy << endl;
    //                 resendSetupReq = true;
    //                 // will reschedule this setupReqTimer and increment retryCount
    //                 sendSetupReq(targetVid, proxy, /*reqDispatch=*/false,  /*recordTrace=*/false, /*reqVnei=*/reqVnei, setupReqTimer);
    //             }
    //         }

    //         if (recordMessageAndReschedule) {
    //             if (recordStatsToFile) { // record sent message
    //                 recordMessageRecord(/*action=*/0, /*src=*/vid, /*dst=*/targetVid, "SetupReq", /*msgId=*/0, /*hopcount=*/0, /*chunkByteLength=*/0);   // unused params (msgId, hopcount, chunkByteLength) are just assigned to 0
    //             }
    //             // reschedule this setupReqTimer and increment retryCount
    //             setupReqTimer->setRetryCount(retryCount+1);
    //             // reschedule the wait setupReq timer w/o backoff
    //             // Commented out bc already added randomness of setupReq to different pendingVneis when first scheduling the wait setupReq timer
    //             // delay = (delay == 0 && retryCount > 0) ? uniform(0, 0.2) * routeSetupReqWaitTime : delay;  // add a random jitter to avoid multiple nodes sending SetupReq to find targetVid at same time
    //             scheduleAt(simTime() + delay + routeSetupReqWaitTime, setupReqTimer);
    //             EV_DETAIL << "Rescheduling setupReq timer: dst = " << setupReqTimer->getDst() << ", retryCount = " << setupReqTimer->getRetryCount() << endl;
    //         }
    //     }
        
    //     if (!resendSetupReq) {
    //         EV_WARN << "SetupReq failed for current vnei " << targetVid << " or it reached setupReqRetryLimit=" << setupReqRetryLimit << " limit. Stop sending setupReq." << endl;
    //         // delete setupReqTimer;    // don't delete because we need to record number of setupReq sent to targetVid
            
    //         // delete targetVid from vset as we failed to build a second vset route
    //         EV_WARN << "Deleting setupReqTimer of vnei " << targetVid << endl;
    //         if (vneiItr != vset.end() && vneiItr->second.setupReqTimer) {    // should be true since setupReqTimer was allocated and scheduled
    //             cancelAndDelete(vneiItr->second.setupReqTimer);
    //             vneiItr->second.setupReqTimer = nullptr;
    //         }
    //     }
    // }
}

// send SetupReply to newnode w/o trace (using vlrOptionOut.dst=proxy) and add it to vset
void Vlr::sendGreedySetupReplyAddVnei(const VlrRingVID& newnode, const VlrRingVID& proxy, const VlrIntOption& vlrOptionOut, const VlrRingVID& nextHopVid, const std::vector<VlrRingVID>& removedNeis)
{
    bool addedRoute = false;
    VlrPathID newPathid;
    do {    // loop in case generated newPathid isn't unique in my vlrRoutingTable
        newPathid = genPathID(newnode);
        addedRoute = vlrRoutingTable.addRoute(newPathid, vid, newnode, /*prevhopVid=*/VLRRINGVID_NULL, /*nexthopVid=*/nextHopVid, /*isVsetRoute=*/true).second;
    } while (!addedRoute);
    
    SetupReplyInt* replyOutgoing = createSetupReply(/*newnode=*/newnode, /*proxy=*/proxy, newPathid);    // srcVset = oldVset (doesn't include newnode) in the created SetupReply
    replyOutgoing->setVlrOption(vlrOptionOut);
    EV_INFO << "Sending SetupReply to rep = " << newnode << " using rep path, src = " << vid << ", pathid = " << newPathid << ", proxy = " << replyOutgoing->getProxy() << ", nexthop: " << nextHopVid << endl;
    sendCreatedSetupReply(replyOutgoing, /*outGateIndex=*/psetTable.getPneiGateIndex(nextHopVid));

    if (recordStatsToFile) {   // record sent message
        std::ostringstream s;
        // if (repairRoute)    // record "reqRepairRoute=1" to infoStr field if repairRoute=true
        s << "reqRepairRoute=" << false;    // repairRoute is false bc we are sending SetupReply w/o trace
        recordMessageRecord(/*action=*/0, /*src=*/vid, /*dst=*/newnode, "SetupReply", /*msgId=*/0, /*hopcount=*/0, /*chunkByteLength=*/0, /*infoStr=*/s.str().c_str());   // unused params (msgId, hopcount, chunkByteLength) are just assigned to 0
    }

    vsetInsertRmPending(newnode);       // targetVid will be removed from pendingVset here!
    vset.at(newnode).vsetRoutes.insert(newPathid);
    // add newnode to recentSetupReqFrom
    recentSetupReqFrom[newnode] = simTime() + recentSetupReqExpiration;
    
    // remove vneis that no longer belong to my vset bc newnode joining
    for (const VlrRingVID& oldNei : removedNeis) {
        // delayRouteTeardown() must be called before oldNei is removed from vset
        delayRouteTeardown(oldNei, vset.at(oldNei).vsetRoutes);     // vroute btw me and oldNei will be torn down later
        vsetEraseAddPending(oldNei);
    }

    // modify selfInNetwork or inNetworkWarmupTimer, recordNodeStatsRecord() to record if vset full and correct
    recordNewVnei(newnode);
}

// after adding a new vnei (and removing old vnei), modify selfInNetwork or inNetworkWarmupTimer, recordNodeStatsRecord() to record if vset full and correct
void Vlr::recordNewVnei(const VlrRingVID& newVnei)
{
    // set inNetwork = true after wait time 
    if (!selfInNetwork) {
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
            s << "vsetFull: inNetwork=" << selfInNetwork << " vset=" << printVsetToString(/*printVpath=*/true);
            recordNodeStatsRecord(/*infoStr=*/s.str().c_str());   // unused params (stage)
            
            if (convertVsetToSet() == vidRingRegVset) {
                nodesVsetCorrect.insert(vid);

                std::ostringstream s;
                s << "vsetCorrect: inNetwork=" << selfInNetwork << " vset=" << printVsetToString(/*printVpath=*/true) << " numNodesVsetCorrect=" << nodesVsetCorrect.size();
                recordNodeStatsRecord(/*infoStr=*/s.str().c_str());
            }
        }
    }
}

// get the closest ccw vnei and cw vnei (wrap-around search) to me
std::pair<VlrRingVID, VlrRingVID> Vlr::getClosestVneisInVset() const
{
    ASSERT(vset.size() >= 2);
    auto itrup = vset.lower_bound(vid);  // itr type: std::set<...>::const_iterator bc this const function can't change vset
    auto itrlow = itrup;
    if (itrup == vset.end()) {
        itrlow--;
        itrup = vset.begin();
    } else if (itrup == vset.begin())
        itrlow = --vset.end();
    else
        itrlow--;
        
    // return std::make_pair(*itrlow, *itrup);
    return std::make_pair(itrlow->first, itrup->first);
}

// get the farthest ccw vnei and cw vnei (wrap-around search in vset) to me
std::pair<VlrRingVID, VlrRingVID> Vlr::getFarthestVneisInFullVset() const
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

// check if targetVid is my closest virNei using vset
bool Vlr::isClosestVneiTo(VlrRingVID targetVid) const
{
    ASSERT(targetVid != vid);
    if (vset.size() < 2)    // if I'm the only node (or one of the only two nodes) inNetwork, any node can be my closest vnei
        return true;
    auto vneiPair = getClosestVneisInVset();    // my vset.size() >= 2
    if (getVid_CCW_Distance(vid, targetVid) < getVid_CCW_Distance(vid, vneiPair.first))
        return true;
    if (getVid_CW_Distance(vid, targetVid) < getVid_CW_Distance(vid, vneiPair.second))
        return true;
    return false;
}

// if isNewnodeCCW=true, newnode is my ccw vnei, else, newnode is my cw vnei
void Vlr::getVneisForwardInFullVsetTo(const VlrRingVID& newnode, const VlrIntVidSet& knownSet, bool isNewnodeCCW, std::set<VlrRingVID>& neisToForward) const
{
    auto itr = vset.lower_bound(vid);               // closest cw vnei of me
    auto itrNewnode = vset.lower_bound(newnode);    // closest cw vnei of newnode
    if (itr == vset.end())
        itr = vset.begin();
    if (itrNewnode == vset.end())
        itrNewnode = vset.begin();
    auto itrCopy = itr;
    int proximityNewnode = vsetHalfCardinality -1;  // smaller value means newnode is farther from me; if newnode is closest ccw/cw vnei of me, proximity = vsetHalfCardinality-1, if it's just closer than farthest, proximity = 0
    bool foundNewnode = false;
    std::set<VlrRingVID> neisToForwardSet;

    if (isNewnodeCCW) {
        for (int i = 0; i < vsetHalfCardinality; ++i) { 
            if (!foundNewnode && itrNewnode != itr)
                proximityNewnode--;
            else
                foundNewnode = true;
            
            advanceIteratorWrapAround(itr, -1, vset.begin(), vset.end());
            // neisToForwardSet.insert(*itr);  // every vnei on the same half of vset with newnode should be in neisToForward
            neisToForwardSet.insert(itr->first);  // every vnei on the same half of vset with newnode should be in neisToForward
        }
        
        for (int i = 0; i < proximityNewnode; ++i) {
            // neisToForwardSet.insert(*itrCopy);  // vnei on the opposite half of vset compared to newnode should be added to neisToForward based on newnode's proximity to me
            neisToForwardSet.insert(itrCopy->first);  // vnei on the opposite half of vset compared to newnode should be added to neisToForward based on newnode's proximity to me
            advanceIteratorWrapAround(itrCopy, 1, vset.begin(), vset.end());
        }
    }
    else {  // newnode is my cw vnei
        // neisToForwardSet.insert(*itr);      // insert my closest cw vnei in vset to neisToForward
        neisToForwardSet.insert(itr->first);      // insert my closest cw vnei in vset to neisToForward
        for (int i = 0; i < vsetHalfCardinality-1; ++i) {
            if (!foundNewnode && itrNewnode != itr)
                proximityNewnode--;
            else
                foundNewnode = true;
                
            advanceIteratorWrapAround(itr, 1, vset.begin(), vset.end());
            // neisToForwardSet.insert(*itr);  // insert my other cw vneis to neisToForward
            neisToForwardSet.insert(itr->first);  // insert my other cw vneis to neisToForward
        }
        
        for (int i = 0; i < proximityNewnode; ++i) {
            advanceIteratorWrapAround(itrCopy, -1, vset.begin(), vset.end());
            // neisToForwardSet.insert(*itrCopy);  // insert my ccw vneis to neisToForward based on newnode's proximity
            neisToForwardSet.insert(itrCopy->first);  // insert my ccw vneis to neisToForward based on newnode's proximity
        }
    }
    // select nodes in neisToForwardSet but not in knownSet, put them in neisToForward
    std::set_difference(neisToForwardSet.begin(), neisToForwardSet.end(), knownSet.begin(), knownSet.end(), std::inserter(neisToForward, neisToForward.begin()));
}

// return shouldAdd, removedNeis, neisToForward
// if findNeisToForward=false, returned neisToForward is an empty set
std::tuple<bool, std::vector<VlrRingVID>, std::set<VlrRingVID>> Vlr::shouldAddVnei(const VlrRingVID& newnode, const VlrIntVidSet& knownSet, bool findNeisToForward) const
{
    ASSERT(vset.find(newnode) == vset.end());   // assert newnode not in vset already
    std::tuple<bool, std::vector<VlrRingVID>, std::set<VlrRingVID>> result;
    EV_DEBUG << "Deciding whether newnode=" << newnode << " can be added to vset" << endl;
    bool& shouldAdd = std::get<0>(result);  // access first element in tuple
    std::vector<VlrRingVID>& removedNeis = std::get<1>(result);
    std::set<VlrRingVID>& neisToForward = std::get<2>(result);

    shouldAdd = (vset.size() < 2 * vsetHalfCardinality);    // if vset not full, shouldAdd = true
    if (shouldAdd) {
        if (findNeisToForward) {
            // convert vset to a set of vids
            std::vector<VlrRingVID> vsetVec;
            for (const auto& vnei : vset)
                vsetVec.push_back(vnei.first);
            // select nodes in vset but not in knownSet, put them in neisToForward
            std::set_difference(vsetVec.begin(), vsetVec.end(), knownSet.begin(), knownSet.end(), std::inserter(neisToForward, neisToForward.begin()));
            // std::set_difference(vset.begin(), vset.end(), knownSet.begin(), knownSet.end(), std::inserter(neisToForward, neisToForward.begin()));
        }
        EV_DEBUG << "Vset not full, can add newnode=" << newnode << " to vset, neisToForward: " << neisToForward << endl;

    } else {
        // vset full, check if newnode is closer than the farthest ccw or cw vnei
        auto vneiPair = getFarthestVneisInFullVset();
        if (getVid_CCW_Distance(vid, newnode) < getVid_CCW_Distance(vid, vneiPair.first)) { // newnode closer than farthest ccw vnei
            shouldAdd = true;
            removedNeis.push_back(vneiPair.first);
            if (findNeisToForward)
                getVneisForwardInFullVsetTo(newnode, knownSet, /*isNewnodeCCW=*/true, neisToForward);
            EV_DEBUG << "Vset full, can add newnode=" << newnode << " to vset, replaces farthest ccw vnei: " << removedNeis[0] << ", neisToForward: " << neisToForward << endl;
        }
        else if (getVid_CW_Distance(vid, newnode) < getVid_CW_Distance(vid, vneiPair.second)) {  // newnode closer than farthest cw vnei
            shouldAdd = true;
            removedNeis.push_back(vneiPair.second);
            if (findNeisToForward)
                getVneisForwardInFullVsetTo(newnode, knownSet, /*isNewnodeCCW=*/false, neisToForward);
            EV_DEBUG << "Vset full, can add newnode=" << newnode << " to vset, replaces farthest cw vnei: " << removedNeis[0] << ", neisToForward: " << neisToForward << endl;
        }
        else
            EV_DEBUG << "Vset full, should not add newnode=" << newnode << " to vset: " << printVsetToString() << endl;
    }
    return result;
}

void Vlr::initializeVlrOption(VlrIntOption& vlrOption, const VlrRingVID& dstVid/*=VLRRINGVID_NULL*/) const
{
    vlrOption.setDstVid(dstVid);                  // vidByteLength
    vlrOption.setTowardVid(VLRRINGVID_NULL);           // vidByteLength
    vlrOption.setCurrentPathid(VLRPATHID_INVALID);    // VLRPATHID_BYTELEN

    vlrOption.setTempTowardVid(VLRRINGVID_NULL);     // vidByteLength
    vlrOption.setTempPathid(VLRPATHID_INVALID);      // VLRPATHID_BYTELEN

    vlrOption.setPrevHopVid(VLRRINGVID_NULL);      // vidByteLength
}

unsigned int Vlr::setupPacketSrcVset(VlrIntSetupPacket *setupPacket) const
{
    std::vector<VlrRingVID> availablePendingVset;
    for (auto& elem : pendingVset) {
        if (IsLinkedPneiOrAvailableRouteEnd(elem.first))
            availablePendingVset.push_back(elem.first);
    }
    setupPacket->setSrcVsetArraySize(vset.size() + availablePendingVset.size());
    // setupPacket->setSrcVsetArraySize(vset.size());
    unsigned int k = 0;
    for (const auto& vnei : vset)     // audo& will be const reference bc this function is const
        setupPacket->setSrcVset(k++, vnei.first);

    for (auto& vnei : availablePendingVset)
        setupPacket->setSrcVset(k++, vnei);
    
    return k;
}

int Vlr::getVlrUniPacketByteLength() const
{
    // unsigned int messageId;  // for statistics only, size ignored
    // VlrIntOption vlrOption;
        // unsigned int dstVid;
        // unsigned int towardVid;
        // VlrPathID currentPathid;
        // unsigned int tempTowardVid;
        // VlrPathID tempPathid;
        // unsigned int prevHopVid;
    return 4 * VLRRINGVID_BYTELEN + 2 * VLRPATHID_BYTELEN;
}

int Vlr::computeSetupReqByteLength(SetupReqInt* setupReq) const
{
    // unsigned int dst, newnode, proxy, /*removedNei,*/ transferNode;
    int chunkByteLength = VLRRINGVID_BYTELEN * 4;
    // unsigned int srcVset[]
    chunkByteLength += setupReq->getSrcVsetArraySize() * VLRRINGVID_BYTELEN;
    // std::set<unsigned int> knownSet
    chunkByteLength += setupReq->getKnownSet().size() * VLRRINGVID_BYTELEN;
    // bool reqDispatch, repairRoute, recordTrace, reqVnei;
    // unsigned int indexInTrace;
    chunkByteLength += 1 + 3;
    // std::vector<unsigned int> traceVec
    chunkByteLength += setupReq->getTraceVec().size() * VLRRINGVID_BYTELEN;

    // for VlrIntUniPacket
    chunkByteLength += getVlrUniPacketByteLength();

    return chunkByteLength;
}

SetupReqInt* Vlr::createSetupReq(const VlrRingVID& dst, const VlrRingVID& proxy, bool reqDispatch)
{
    SetupReqInt *setupReq = new SetupReqInt(/*name=*/"SetupReq");
    setupReq->setNewnode(vid);      // vidByteLength
    setupReq->setDst(dst);
    setupReq->setProxy(proxy);
    // setupReq->setRemovedNei(VLRRINGVID_NULL);
    setupReq->setTransferNode(VLRRINGVID_NULL);
    setupReq->setReqDispatch(reqDispatch);
    setupReq->setRepairRoute(false);
    setupReq->setRecordTrace(false);
    setupReq->setIndexInTrace(0);
    setupReq->setReqVnei(true);

    setupPacketSrcVset(setupReq);
    setupReq->setMessageId(++allSendMessageId);
    setupReq->setHopcount(0);         // number of nodes this message traversed, including the starting and ending nodes, will be incremented to 1 in sendCreatedSetupReq()

    initializeVlrOption(setupReq->getVlrOptionForUpdate());

    return setupReq;
}

// if computeChunkLength = true, compute chunk length because chunk was just created with createSetupReq() or modified after dupShared() from another chunk
// else, no need to compute chunk length because chunk was dupShared() (not modified) from another chunk that has chunkLength set
void Vlr::sendCreatedSetupReq(SetupReqInt *setupReq, const int& outGateIndex, bool computeChunkLength/*=true*/, double delay/*=0*/)
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
void Vlr::sendSetupReq(const VlrRingVID& dst, const VlrRingVID& proxy, bool reqDispatch, bool recordTrace, bool reqVnei, WaitSetupReqIntTimer *setupReqTimer)
{
    auto setupReq = createSetupReq(dst, proxy, reqDispatch);
    int proxyGateIndex = psetTable.getPneiGateIndex(proxy);
    setupReq->getVlrOptionForUpdate().setDstVid(dst);

    if (recordTrace) {
        setupReq->getTraceVecForUpdate().push_back(vid);  // put myself in traceVec
        setupReq->setRecordTrace(true);
    }
    setupReq->setReqVnei(reqVnei);  // request to add dst as vnei

    sendCreatedSetupReq(setupReq, proxyGateIndex);
    
    if (recordStatsToFile) { // record sent message
        std::string recordMessageInfoStr = (recordTrace) ? "reqRecordTrace=1" : "";
        recordMessageRecord(/*action=*/0, /*src=*/vid, /*dst=*/dst, "SetupReq", /*msgId=*/0, /*hopcount=*/0, /*chunkByteLength=*/0, /*infoStr=*/recordMessageInfoStr.c_str());   // unused params (msgId, hopcount, chunkByteLength) are just assigned to 0
    }

    if (setupReqTimer) {    // if setupReqTimer != nullptr
        int retryCount = setupReqTimer->getRetryCount() + 1;
        setupReqTimer->setRetryCount(retryCount);
        // reschedule the wait setupReq timer w/o backoff
        // int backoffCount = (retryCount <= setupReqRetryLimit) ? retryCount : 1;     // total backoff time: [1, setupReqRetryLimit] * routeSetupReqWaitTime, if retryCount not <= setupReqRetryLimit, it's not considered a proper retry (retryCount is probably set manually), thus wait time is constant w/o backoff
        double randomCoefficient = (reqVnei) ? 1 : uniform(1, 1.2);  // add random delay for setupReq(reqVnei=false) to avoid sending multiple setupReq at same time when tryNonEssRoute becomes true
        scheduleAt(simTime() + randomCoefficient * routeSetupReqWaitTime, setupReqTimer);
        EV_DETAIL << "Rescheduling setupReq timer: dst = " << setupReq->getDst() << ", retryCount = " << retryCount << endl;
    }
}

void Vlr::processSetupReq(SetupReqInt* reqIncoming, bool& pktForwarded)
{
    EV_DEBUG << "Received SetupReq" << endl;

    // // IP.SourceAddress: address of the node from which the packet was received
    // VlrRingVID msgPrevHopVid = reqIncoming->getVlrOption().getPrevHopVid();
    // if (msgPrevHopVid == VLRRINGVID_NULL)
    //     throw cRuntimeError("Received SetupReq with vlrOption.prevHopVid = null");

    VlrRingVID newnode = reqIncoming->getNewnode();
    VlrRingVID dstVid = reqIncoming->getDst();        // can also get dst using vlrOptionIn->getDstVid()
    VlrRingVID proxy = reqIncoming->getProxy();
    bool recordTrace = reqIncoming->getRecordTrace();
    bool hasTrace = (reqIncoming->getTraceVec().size() > 0);
    bool withTrace = (hasTrace && !recordTrace);    // if true, get to dst via trace rather than perform greedy routing
    const VlrIntOption& vlrOptionIn = reqIncoming->getVlrOptionForUpdate();
    
    EV_INFO << "Processing SetupReq: dst = " << dstVid << ", newnode = " << newnode << ", proxy = " << proxy << ", recordTrace = " << recordTrace << endl;
    
    if (recordStatsToFile && recordReceivedMsg) {   // record received message
        recordMessageRecord(/*action=*/2, /*src=*/newnode, /*dst=*/dstVid, "SetupReq", /*msgId=*/reqIncoming->getMessageId(), /*hopcount=*/reqIncoming->getHopcount()+1, /*chunkByteLength=*/reqIncoming->getByteLength());   // unimportant params (msgId, hopcount)
    }
    bool pktRecorded = false;      // set to true if this msg is recorded with recordMessageRecord()

    bool reqForMe = false;
    if (dstVid == vid && newnode != vid) {
        reqForMe = true;
        // if (reqIncoming->getRemovedNei() != VLRRINGVID_NULL) {  // removedNei is set, try to remove it from my vset
        //     auto vneiItr = vset.find(reqIncoming->getRemovedNei());
        //     if (vneiItr != vset.end()) {
        //         // add vroute btw me and removedNei (already removed from vset) to nonEssRoutes to be torn down later
        //         delayRouteTeardown(reqIncoming->getRemovedNei(), vneiItr->second.vsetRoutes);
        //         vsetEraseAddPending(reqIncoming->getRemovedNei());
        //     }
        // }
    }
    else if (newnode == dstVid && newnode != vid) {    // this setupReq is finding the closest vnei to newnode, dst isn't specified
        // if previous hop selected a vroute not to me, meaning it had a better dst for this setupReq (if I'm as good as towardVid, previous hop would select its pnei--me, unless I'm in inNetwork warmup (inNetwork=false)), then this setupReq isn't for me
        // if I'm not inNetwork, I should't handle this setupReq where newnode is joining the network
        if (vlrOptionIn.getCurrentPathid() != VLRPATHID_INVALID && vlrOptionIn.getTowardVid() != vid /*&& selfInNetwork*/)
            reqForMe = false;
        else if (selfInNetwork && isClosestVneiTo(newnode))  // check if I have a vnei closer to newnode
            reqForMe = true;     // I'm closest ccw or cw vnei to newnode, process the req
    }

    if (representativeFixed && representative.heardfromvid == VLRRINGVID_NULL) {        // if I don't have a valid rep yet, I won't process or forward this message
        EV_WARN << "No valid rep heard: " << representative << ", cannot accept overlay message" << endl;
        
        if (recordStatsToFile && recordDroppedMsg && !pktRecorded) {   // record dropped message
            recordMessageRecord(/*action=*/4, /*src=*/newnode, /*dst=*/dstVid, "SetupReq", /*msgId=*/reqIncoming->getMessageId(), /*hopcount=*/reqIncoming->getHopcount()+1, /*chunkByteLength=*/reqIncoming->getByteLength(), /*infoStr=*/"no valid rep");
            pktRecorded = true;
        }
        return;
    }
    if (reqForMe && recentSetupReqFrom.find(newnode) != recentSetupReqFrom.end()) {  // I've just added newnode as vnei, ignore this setupReq
        EV_WARN << "SetupReq from newnode = " << newnode << " was recently received and accepted" << endl;    // ignoring this SetupReq
        // return;      // Commented out bc now allow multiple paths built to the same vnei
    }


    if (reqForMe) {     // checked I'm inNetwork

        const VlrIntVidSet& knownSetIn = reqIncoming->getKnownSet();
        bool reqDispatch = reqIncoming->getReqDispatch();
        bool repairRoute = reqIncoming->getRepairRoute();
        VlrPathID patchedRouteToRepair = reqIncoming->getPatchedRoute();
        bool reqVnei = reqIncoming->getReqVnei();
        EV_INFO << "Handling setupReq for me: dst = " << dstVid << ", newnode = " << newnode << ", proxy = " << proxy << ", knownSet = " << knownSetIn << ", reqDispatch = " << reqDispatch << ", repairRoute = " << repairRoute << " (patchedRoute=" << patchedRouteToRepair << ")"
                << ", reqVnei = " << reqVnei << ", srcVset = [";
        for (size_t i = 0; i < reqIncoming->getSrcVsetArraySize(); i++)
            EV_INFO << reqIncoming->getSrcVset(i) << " ";
        EV_INFO << "]" << endl;

        // checked I have valid rep, if I'm not inNetwork, isClosestVneiTo(newnode) won't return true, but reqForMe=true, which means dstVid is me, this setupReq was probably forwarded to me by my vnei, should accept, or newnode probably heard about me from my vnei
            // if I'm not inNetwork but my vset isn't empty, meaning I'm in inNetwork warmup wait time
            // if dispatched to me by my vnei A, meaning A thinks newnode should also be my vnei, newnode may help fill up my vset and skip inNetwork warmup
            // if directed to me and reqDispatch=false, newnode sent this setupReq just to add me as vnei, not to join the overlay, maybe bc my existing vroute to newnode is broken, I should always accept as long as I have valid rep
        if (!selfInNetwork && vset.empty() && knownSetIn.empty() && reqDispatch) {
            EV_WARN << "SetupReq (dst = " << dstVid << ", newnode = " << newnode << ", reqDispatch = " << reqDispatch << ") destined for me (not dispatched) but I'm not inNetwork, cannot add vnei, dropping SetupReq" << endl;
            
            if (recordStatsToFile && recordDroppedMsg && !pktRecorded) {   // record arrived message
                recordMessageRecord(/*action=*/1, /*src=*/newnode, /*dst=*/dstVid, "SetupReq", /*msgId=*/reqIncoming->getMessageId(), /*hopcount=*/reqIncoming->getHopcount()+1, /*chunkByteLength=*/reqIncoming->getByteLength(), /*infoStr=*/"dispatched not inNetwork");
                pktRecorded = true;
            }
            return;
        }

        bool processReq = false;
        bool removedNewnodeFromVset = false;
        // char newnodeOldVneiDist;   // if newnode existed in vset, record its vneiDist before removing it, valid only if removedNewnodeFromVset=true
        // VlrPathID oldPathid;       // if newnode existed in vset, record its vset-route before setting isVsetRoute to false, valid only if removedNewnodeFromVset=true
        std::set<VlrPathID> oldVsetRoutes;  // if newnode existed in vset, record its vset-routes before setting isVsetRoute to false, valid only if removedNewnodeFromVset=true
        bool removedNewnodeVsetRoute = false;

        auto vneiItr = vset.find(newnode);
        
        if (repairRoute) {  // newnode sent this setupReq to repair vroute btw itself and me
            auto vrouteItr = vlrRoutingTable.vlrRoutesMap.find(patchedRouteToRepair);
            if (vrouteItr != vlrRoutingTable.vlrRoutesMap.end()) {  // patchedRouteToRepair found in vlrRoutingTable
                // patchedRouteToRepair contains temporary route bc link failure, expire later as vroute is probably still availble with temporary route
                if (vrouteItr->second.isVsetRoute && vneiItr != vset.end()) {
                    auto vsetRouteItr = vneiItr->second.vsetRoutes.find(patchedRouteToRepair);
                    if (vsetRouteItr != vneiItr->second.vsetRoutes.end()) {     // patchedRouteToRepair was a vset-route to newnode, remove it from vsetRoutes
                        removedNewnodeVsetRoute = true;
                        vneiItr->second.vsetRoutes.erase(vsetRouteItr);
                    }
                }
                // oldPathidWasWanted=true if patchedRoute wasn't already in nonEssUnwantedRoutes
                bool oldPathidWasWanted = nonEssUnwantedRoutes.insert(patchedRouteToRepair).second;     // patched vroute should be torn down after patchedRouteExpiration
                vrouteItr->second.isVsetRoute = false;
                auto itr_bool = nonEssRoutes.insert({patchedRouteToRepair, simTime() + patchedRouteExpiration});   // if patchedRoute was a vset route it won't be in nonEssRoutes
                if (!itr_bool.second && oldPathidWasWanted) {  // patchedRoute already in nonEssRoutes but wasn't unwanted, I just learnt that it's patched
                    if (itr_bool.first->second != 1)
                        itr_bool.first->second = simTime() + patchedRouteExpiration;   // patchedRoute was nonEss, extend its expiration time, torn down after patchedRouteExpiration
                }
                EV_INFO << "Received SetupReq(repairRoute=true),  will tear down patched vroute to newnode " << newnode << ": pathid = " << patchedRouteToRepair << endl;
            }
        }

        if (reqVnei) {
            if (vneiItr == vset.end())     // newnode not in my vset
                processReq = true;
            else if (knownSetIn.empty()) {  // newnode sent setupReq to me directly but I already have newnode in vset, inconsistent
                // // Commented out so even if old vsetRoutes to newnode may be broken, I don't remove it, just build a new vset-route to newnode
                // // since newnode not in recentSetupReqFrom, I have waited long enough for my setupReply to reach newnode, so this is a broken vroute
                // // remove it from vset to call shouldAddVnei() properly, tear down path to newnode (later to avoid sending too many messages at once)
                
                // // get VlrPathID btw me and newnode     NOTE since newnode was in my vset, there must be a vset route to newnode in vlrRoutingTable
                // // oldPathid = vlrRoutingTable.getVsetRouteToVnei(newnode, vid);
                // // ASSERT(oldPathid != VLRPATHID_INVALID);
                // // newnodeOldVneiDist = vsetErase(newnode).second;     // newnode is in vset so vneiDis will be returned
                // oldVsetRoutes = vneiItr->second.vsetRoutes;
                // // make all vset-routes to newnode nonEss
                // // vneiItr->second.vsetRoutes.clear();
                // for (const VlrPathID& oldPathid : oldVsetRoutes) {
                //     vlrRoutingTable.vlrRoutesMap.at(oldPathid).isVsetRoute = false;
                //     if (repairRoute) {     // newnode was my vnei, sent this setupReq to repair vroute btw itself and me
                //         // vset-route contains temporary route bc link failure, expire later as vroute is probably still availble with temporary route
                //         nonEssRoutes[oldPathid] = simTime() + patchedRouteExpiration;   // newnode was in vset, oldPathid must be a vset route, hence not in nonEssRoutes
                //         nonEssUnwantedRoutes.insert(oldPathid);     // patched vroute should be torn down after patchedRouteExpiration
                //         EV_INFO << "Received SetupReq(repairRoute=true) and newnode already in my vset,  will tear down vroute to newnode " << newnode << ": pathid = " << oldPathid << endl;
                //     } else {           
                //         // put in nonEssRoutes, expire as soon as possible (it's broken, can tear down right now)
                //         nonEssRoutes[oldPathid] = 1;    // set expireTime to 1 to ensure it'll be torn down
                //         EV_WARN << "Newnode of SetupReq already in my vset, inconsistent, will tear down vroute to newnode " << newnode << ": pathid = " << oldPathid << endl;
                //     }
                // }
                // vsetEraseAddPending(newnode);    // newnode should be my vnei
                // removedNewnodeFromVset = true;

                // skip shouldAddVnei() since newnode is already in vset
                reqVnei = false;
                // process setupReq from newnode as normal 
                processReq = true;
            }
        } else { // newnode isn't requesting to add me as a vnei, just trying to build a vroute btw us
            processReq = true;
        }

        if (processReq) {     // newnode not in my vset or has been removed from my vset
            bool shouldAdd = false;

            if (reqVnei) {
                auto addResult = shouldAddVnei(newnode, knownSetIn, /*findNeisToForward=*/reqDispatch);
                shouldAdd = std::get<0>(addResult);  // access first element in tuple
                std::vector<VlrRingVID>& removedNeis = std::get<1>(addResult);
                std::set<VlrRingVID>& neisToForwardSet = std::get<2>(addResult);
            
                if (shouldAdd) {
                    // ensure it's possible for me send SetupReply to newnode via proxy
                    // VlrIntOption *vlrOptionOut = nullptr;
                    // L3Address nextHopAddr;
                    VlrIntOption vlrOptionOut;
                    initializeVlrOption(vlrOptionOut, /*dstVid=*/proxy);
                    VlrRingVID nextHopVid = VLRRINGVID_NULL;
                    std::vector<VlrRingVID> trace;
                    if (!hasTrace) {     // find next hop to proxy or newnode directly
                        nextHopVid = findNextHopForSetupReply(vlrOptionOut, /*prevHopVid=*/VLRRINGVID_NULL, /*newnode=*/newnode);
                    } else {    // use setupReq->traceVec to compute path for setupReply
                        trace = removeLoopInTrace(reqIncoming->getTraceVec());  // trace: [newnode, .., previous node before me] contains no loop
                        // ensure it's possible for me send SetupReply to newnode via trace
                        unsigned int nextHopIndex = getNextHopIndexInTrace(trace);
                        if (nextHopIndex < trace.size())    // trace[nextHopIndex] is a pnei of me that's closest to newnode in trace
                            nextHopVid = trace[nextHopIndex];
                    }

                    if (nextHopVid == VLRRINGVID_NULL) {
                        // if (!hasTrace)
                        //     delete vlrOptionOut;
                        EV_WARN << "No next hop found to send SetupReply at me = " << vid << ": newnode = " << newnode << ", original dst = " << dstVid << ", proxy = " << proxy << ", dropping SetupReq" << endl;
                        // Commented out bc reqVnei is set to false when newnode is already in vset
                        // if (removedNewnodeVsetRoute) {   // newnode was in vset and removed a vset-route patchedRouteToRepair (i.e. repairRoute=true), but reqVnei=false, we assume we'll build another vset-route to newnode, now no new vset-route is added, we need to handle lost of vset-route to newnode properly
                        //     // patchedRouteToRepair was a vset-route to newnode, add it back to call removeEndpointOnTeardown() properly
                        //     vneiItr->second.vsetRoutes.insert(patchedRouteToRepair);
                        //     removeEndpointOnTeardown(/*pathid=*/patchedRouteToRepair, /*towardVid=*/newnode, /*pathIsVsetRoute=*/true, /*pathIsTemporary=*/0, /*reqRepairRoute=*/repairRoute);      // vset route cannot be temporary
                        // }

                    } else {    // next hop found to send SetupReply to newnode
                        std::vector<VlrRingVID> neisToForward;
                        if (reqDispatch) {  // I'll forward this setupReq to my vneis, if reqDispatch=false, neisToForward should be empty
                            // Commented out bc srcVset not only include vneis of node, but also available pendingVneis  if a node has vneis it shouldn't send setupReq(reqDispatch=true)
                            // exclude srcVset from neisToForward
                            std::set<VlrRingVID> srcVsetSet;    // srcVsetSet is sorted
                            // int oVsetSize = reqIncoming->getSrcVsetArraySize();
                            // for (int k = 0; k < oVsetSize; ++k)
                            //     srcVsetSet.insert(reqIncoming->getSrcVset(k));
                            // select nodes in neisToForwardSet but not in srcVset, put them in neisToForward
                            std::set_difference(neisToForwardSet.begin(), neisToForwardSet.end(), srcVsetSet.begin(), srcVsetSet.end(), std::inserter(neisToForward, neisToForward.begin()));
                            EV_DETAIL << "shouldAdd(newnode=" << newnode << ") returns: shouldAdd = " << shouldAdd << ", removedNeis = " << removedNeis << ", neisToForward excluding srcVset = " << neisToForward << endl;
                            
                            // NOTE if reqDispatch = false, setupReq isn't dispatched to more vneis, thus knownSetIn always empty
                            // for (const VlrRingVID& node : knownSetIn)
                            //     pendingVsetAdd(node);
                        }

                        // forward SetupReq to other potential vneis of newnode
                        for (const VlrRingVID& vnei : neisToForward) {
                            // NOTE VlrCreationTimeTag will be copied as Chunk is constructed - Chunk::Chunk(const Chunk& other) - in dupShared()
                            auto reqOutgoing = reqIncoming->dup();    // create SetupReq to forward
                            VlrIntVidSet& knownSetOut = reqOutgoing->getKnownSetForUpdate();
                            // insert neisToForward into knownSet
                            knownSetOut.insert(neisToForward.begin(), neisToForward.end());
                            knownSetOut.insert(vid);   // insert myself into knownSet

                            reqOutgoing->setDst(vnei);
                            // // if vnei is removed from my vset (bc newnode joining), set reqOutgoing.removedNei = me to let vnei remove me from its vset upon receiving this SetupReq
                            // VlrRingVID removingNei = (std::find(removedNeis.begin(), removedNeis.end(), vnei) != removedNeis.end()) ? vid : VLRRINGVID_NULL;
                            // reqOutgoing->setRemovedNei(removingNei);

                            VlrIntOption& vlrOptionOut = reqOutgoing->getVlrOptionForUpdate();
                            initializeVlrOption(vlrOptionOut, /*dstVid=*/vnei);
                            VlrRingVID nextHopVid_dispatch = findNextHop(vlrOptionOut, /*prevHopVid=*/VLRRINGVID_NULL, /*excludeVid=*/newnode, /*allowTempRoute=*/true);
                            if (nextHopVid_dispatch == VLRRINGVID_NULL) {
                                delete reqOutgoing;
                                EV_WARN << "No next hop found to forward SetupReq from me = " << vid << " to new dst = " << vnei << ", newnode = " << newnode << ", original dst = " << dstVid << ", proxy = " << proxy << endl;
                            } else {
                                EV_INFO << "Forwarding SetupReq to new dst = " << vnei << ", newnode = " << newnode << ", proxy = " << reqOutgoing->getProxy() << ", nexthop: " << nextHopVid_dispatch << endl;
                                sendCreatedSetupReq(reqOutgoing, /*outGateIndex=*/psetTable.getPneiGateIndex(nextHopVid_dispatch), /*computeChunkLength=*/true);
                            }
                        }

                        // send SetupReply to newnode, add newnode to vset and vlrRoutingTable
                        bool addedRoute = false;
                        VlrPathID newPathid;
                        do {    // loop in case generated newPathid isn't unique in my vlrRoutingTable
                            newPathid = genPathID(newnode);
                            addedRoute = vlrRoutingTable.addRoute(newPathid, vid, newnode, /*prevhopVid=*/VLRRINGVID_NULL, /*nexthopVid=*/nextHopVid, /*isVsetRoute=*/true).second;
                        } while (!addedRoute);
                        
                        if (!hasTrace) {
                            SetupReplyInt* replyOutgoing = createSetupReply(newnode, proxy, newPathid);    // srcVset = oldVset (doesn't include newnode) in the created SetupReply
                            replyOutgoing->setVlrOption(vlrOptionOut);
                            EV_INFO << "Sending SetupReply to newnode = " << newnode << ", src = " << vid << ", pathid = " << newPathid << ", proxy = " << replyOutgoing->getProxy() << ", nexthop: " << nextHopVid << endl;
                            sendCreatedSetupReply(replyOutgoing, /*outGateIndex=*/psetTable.getPneiGateIndex(nextHopVid));
                        } else {
                            auto replyOutgoing = createSetupReply(newnode, /*proxy=*/VLRRINGVID_NULL, newPathid);    // srcVset = oldVset (doesn't include newnode) in the created SetupReply
                            // replyOutgoing->setVlrOption(vlrOptionOut);
                            replyOutgoing->setTrace(trace);
                            EV_INFO << "Sending SetupReply to newnode = " << newnode << ", src = " << vid << ", pathid = " << newPathid << ", trace = " << replyOutgoing->getTrace() << ", nexthop: " << nextHopVid << endl;
                            sendCreatedSetupReply(replyOutgoing, /*outGateIndex=*/psetTable.getPneiGateIndex(nextHopVid));
                        }

                        if (recordStatsToFile) {   // record sent message
                            std::ostringstream s;
                            s << "pathid=" << newPathid;
                            if (repairRoute)    // record "reqRepairRoute=1" to infoStr field if repairRoute=true
                                s << "reqRepairRoute=" << repairRoute;
                            recordMessageRecord(/*action=*/0, /*src=*/vid, /*dst=*/newnode, "SetupReply", /*msgId=*/0, /*hopcount=*/0, /*chunkByteLength=*/0, /*infoStr=*/s.str().c_str());   // unused params (msgId, hopcount, chunkByteLength) are just assigned to 0
                        }

                        vsetInsertRmPending(newnode);
                        vset.at(newnode).vsetRoutes.insert(newPathid);
                        // add newnode to recentSetupReqFrom
                        recentSetupReqFrom[newnode] = simTime() + recentSetupReqExpiration;

                        // // set inNetwork = true after wait time 
                        // if (!selfInNetwork) {
                        //     if (!inNetworkWarmupTimer->isScheduled()) {
                        //         scheduleAt(simTime() + inNetworkWarmupTime, inNetworkWarmupTimer);
                        //         EV_DETAIL << "inNetwork scheduled to become true at " << inNetworkWarmupTimer->getArrivalTime() << endl;
                        //     }
                        //     else if (vset.size() >= 2 * vsetHalfCardinality) {  // vset is full
                        //         cancelEvent(inNetworkWarmupTimer);
                        //         selfInNetwork = true;

                        //         lastTimeNodeJoinedInNetwork = simTime().dbl();
                        //     }
                        //     else if (vset.size() == 1) { // vset was empty but inNetworkWarmupTimer was scheduled, this means I was a rep that lost all my vneis, now that I've joined an existing overlay, cancel the original inNetworkWarmupTimer to schedule a new one to wait for other vneis
                        //         cancelEvent(inNetworkWarmupTimer);
                        //         scheduleAt(simTime() + inNetworkWarmupTime, inNetworkWarmupTimer);
                        //         EV_DETAIL << "inNetwork re-scheduled to become true at " << inNetworkWarmupTimer->getArrivalTime() << endl;
                        //     }
                        // }
                        
                        // convert vset to a set of vids for EV_INFO purpose
                        // std::set<VlrRingVID> vsetneis;
                        // auto setItr = vsetneis.end();    // hint for the position where the element can be inserted in vsetneis
                        // for (auto& elem : vset)
                        //     setItr = vsetneis.insert(setItr, elem.first);
                        // EV_INFO << "Adding newnode = " << newnode << " to vset = " << vsetneis << ", inNetwork = " << selfInNetwork << endl;
                        // EV_INFO << "Adding newnode = " << newnode << " to vset = " << printVsetToString() << ", inNetwork = " << selfInNetwork << endl;


                        // remove vneis that no longer belong to my vset bc newnode joining
                        for (const VlrRingVID& oldNei : removedNeis) {
                            // delayRouteTeardown() must be called before oldNei is removed from vset
                            delayRouteTeardown(oldNei, vset.at(oldNei).vsetRoutes, /*sendNotifyVset=*/!reqDispatch);     // vroute btw me and oldNei will be torn down later
                            vsetEraseAddPending(oldNei);
                        }
                        // if (dstVid == vid && reqIncoming->getRemovedNei() != VLRRINGVID_NULL) {  // removedNei is set, ensure it's removed from my vset
                        //     auto vneiItr = vset.find(reqIncoming->getRemovedNei());
                        //     if (vneiItr != vset.end()) {
                        //         // add vroute btw me and removedNei (already removed from vset) to nonEssRoutes to be torn down later
                        //         delayRouteTeardown(reqIncoming->getRemovedNei(), vneiItr->second.vsetRoutes);
                        //         vsetEraseAddPending(reqIncoming->getRemovedNei());
                        //     }
                        // }

                        // if (recordStatsToFile) { // write node status update
                        //     if (vset.size() >= 2 * vsetHalfCardinality) {  // vset is full
                        //         std::ostringstream s;
                        //         s << "vsetFull: inNetwork=" << selfInNetwork << " vset=" << printVsetToString();
                        //         recordNodeStatsRecord(/*infoStr=*/s.str().c_str());   // unused params (stage)
                                
                        //         if (convertVsetToSet() == vidRingRegVset) {
                        //             std::ostringstream s;
                        //             s << "vsetCorrect: inNetwork=" << selfInNetwork << " vset=" << printVsetToString();
                        //             recordNodeStatsRecord(/*infoStr=*/s.str().c_str());
                        //         }
                        //     }
                        // }

                        // modify selfInNetwork or inNetworkWarmupTimer, recordNodeStatsRecord() to record if vset full and correct
                        recordNewVnei(newnode);

                        // process srcVset in received SetupReq
                        processOtherVset(reqIncoming, /*srcVid=*/newnode);
                    }
                } 
            }
            if (!shouldAdd) {    // shouldAdd == false
                // if newnode was in vset and removed b4 processing this setupReq, shouldAdd(newnode) should return true bc newnode was in vset, we assume newnode will be added back, otherwise we need to handle lost of vnei (newnode) properly
                // Commented out bc removedNewnodeFromVset=true when newnode was a vnei and sent setupReq to repair a vset-route patchedRouteToRepair, but reqVnei=false
                // ASSERT(!removedNewnodeFromVset);     // removedNewnodeFromVset isn't used bc newnode isn't removed from vset if it already exists, removedNewnodeVsetRoute is used instead to indicate removed a patched vset-route to newnode
                pendingVsetAdd(newnode);
                // if (knownSetIn.empty()) {  // newnode sent setupReq to me directly but I don't think it belongs to my vset, inconsistent
                // NOTE reqOutgoing is a shared_ptr (implemented in IntrusivePtr.h), no need to delete reqOutgoing;

                // ensure it's possible for me send reply to newnode via proxy or trace
                // if !hasTrace
                    // if !reqVnei && newnode in vset, newnode sent this setupReq to build a second vset-route to me, send SetupReply to it via greedy routing
                    // if reqVnei, newnode sent this setupReq add me as vnei but I don't think I shouldAdd, send SetupFail to newnode, notify it of my vset
                // if hasTrace, msg contains trace to newnode
                    // if newnode is in vset && repairRoute && removedNewnodeVsetRoute, newnode sent this setupReq to repair a vset-route to me, send SetupReply with trace
                    // if newnode is in vset && repairRoute && !removedNewnodeVsetRoute, newnode sent this setupReq to repair a vset-route to me but that route is no longer a vset-route, send SetupFail to newnode
                        // NOTE if newnode shouldn't send setupReq to repair the same vset-route twice, also if I've received RepairRoute for patchedRouteToRepair that newnode is trying to repair, I should've removed newnode from my vset, but I may have received setupReply of a new vset-route and added newnode back
                    // if newnode isn't in vset && newnode is a LINKED pnei or there exists a wanted vroute btw us, send SetupFail to newnode, notify it of my vset
                    // otherwise, send AddRoute to newnode, notify it of my vset
                bool newnodeInVset = (vneiItr != vset.end());
                if (newnodeInVset && repairRoute && !removedNewnodeVsetRoute)
                    newnodeInVset = false;
                bool useGreedySetupFail = (!hasTrace && reqVnei);
                bool useGreedyAddRoute = (!hasTrace && !reqVnei && !newnodeInVset && !repairRoute);
                bool useGreedySetupReply = (!hasTrace && (!reqVnei && newnodeInVset));
                bool useSetupFailWithTrace = (hasTrace && !newnodeInVset && IsLinkedPneiOrAvailableWantedRouteEnd(newnode, /*routeBtwUs=*/true));   // NOTE routeBtwUs=true bc this setupReq may be repairing a nonEss vroute bc newnode received RepairRoute and it doesn't have other vroute to me, it isn't good for me to reject it just bc I have other vroute (likely patched but I don't know) to newnode
                VlrIntOption vlrOptionOut;
                VlrRingVID nextHopVid = VLRRINGVID_NULL;
                std::vector<VlrRingVID> trace;
                
                // if newnode is requesting me as a vnei (but shouldAdd == false), I send setupFail to it via greedy routing
                // if newnode isn't requesting me as a vnei but setupReq doesn't include trace (shouldn't happen), I don't send anything
                if (useGreedySetupFail) {     // find next hop to proxy or newnode directly
                    // send SetupFail to newnode, notify it of my vset
                    // vlrOptionOut = createVlrOption(proxy);
                    initializeVlrOption(vlrOptionOut, /*dstVid=*/proxy);
                    nextHopVid = findNextHopForSetupReply(vlrOptionOut, /*prevHopVid=*/VLRRINGVID_NULL, /*newnode=*/newnode, /*allowTempRoute=*/true);
                } else if (useGreedySetupReply) {
                    initializeVlrOption(vlrOptionOut, /*dstVid=*/proxy);
                    nextHopVid = findNextHopForSetupReply(vlrOptionOut, /*prevHopVid=*/VLRRINGVID_NULL, /*newnode=*/newnode, /*allowTempRoute=*/false);
                } else if (useGreedyAddRoute) {
                    VlrRingVID vlrOptionDst = (proxy == VLRRINGVID_NULL || proxy == vid) ? newnode : proxy;
                    initializeVlrOption(vlrOptionOut, /*dstVid=*/vlrOptionDst);
                    nextHopVid = findNextHopForSetupReq(vlrOptionOut, /*prevHopVid=*/VLRRINGVID_NULL, /*dstVid=*/newnode, /*newnode=*/VLRRINGVID_NULL, /*allowTempRoute=*/false);
                }
                else if (hasTrace) {    // use setupReq->traceVec to compute path for reply
                    trace = removeLoopInTrace(reqIncoming->getTraceVec());  // trace: [newnode, .., previous node before me] contains no loop
                    // ensure it's possible for me send SetupReply to newnode via trace
                    unsigned int nextHopIndex = getNextHopIndexInTrace(trace);
                    if (nextHopIndex < trace.size())    // trace[nextHopIndex] is a pnei of me that's closest to newnode in trace
                        nextHopVid = trace[nextHopIndex];
                }

                if (nextHopVid == VLRRINGVID_NULL) {
                    // if (useGreedySetupFail || useGreedySetupReply)
                    //     delete vlrOptionOut;
                    EV_WARN << "No next hop found to send SetupFail/AddRoute at me = " << vid << ": newnode = " << newnode << ", original dst = " << dstVid << ", proxy = " << proxy << endl;
                    
                    if (removedNewnodeVsetRoute) {   // newnode was in vset and removed a vset-route patchedRouteToRepair (i.e. repairRoute=true), but reqVnei=false, we assume we'll build another vset-route to newnode, now no new vset-route is added, we need to handle lost of vset-route to newnode properly
                        // patchedRouteToRepair was a vset-route to newnode, add it back to call removeEndpointOnTeardown() properly
                        vneiItr->second.vsetRoutes.insert(patchedRouteToRepair);
                        removeEndpointOnTeardown(/*pathid=*/patchedRouteToRepair, /*towardVid=*/newnode, /*pathIsVsetRoute=*/true, /*pathIsTemporary=*/0, /*reqRepairRoute=*/repairRoute);      // vset route cannot be temporary
                        // setupReq(repairRoute=true) will be sent with a random delay in processWaitSetupReqTimer()
                    }
                } else {
                    if (useGreedySetupFail || useSetupFailWithTrace) {
                        if (useGreedySetupFail) {
                            auto replyOutgoing = createSetupFail(newnode, proxy);    // srcVset = vset, newnode isn't added to my vset
                            replyOutgoing->setVlrOption(vlrOptionOut);
                            EV_INFO << "Sending SetupFail to newnode = " << newnode << ", proxy = " << replyOutgoing->getProxy() << ", nexthop: " << nextHopVid << endl;
                            sendCreatedSetupFail(replyOutgoing, /*outGateIndex=*/psetTable.getPneiGateIndex(nextHopVid), /*computeChunkLength=*/true);
                        } else {    // useSetupFailWithTrace
                            auto replyOutgoing = createSetupFail(newnode, /*proxy=*/VLRRINGVID_NULL);    // srcVset = vset, newnode isn't added to my vset
                            replyOutgoing->setTrace(trace);
                            sendCreatedSetupFail(replyOutgoing, /*outGateIndex=*/psetTable.getPneiGateIndex(nextHopVid));
                            EV_INFO << "Sending SetupFail to newnode = " << newnode << ", trace = " << replyOutgoing->getTrace() << ", nexthop: " << nextHopVid << endl;
                        }
                        
                        if (recordStatsToFile) {   // record sent message
                            std::ostringstream s;
                            s << "vset=" << printVsetToString() << " pendingVset=" << printPendingVsetToString();
                            recordMessageRecord(/*action=*/0, /*src=*/vid, /*dst=*/newnode, "SetupFail", /*msgId=*/0, /*hopcount=*/0, /*chunkByteLength=*/0, /*infoStr=*/s.str().c_str());   // unused params (msgId, hopcount, chunkByteLength) are just assigned to 0
                        }
                    }
                    else if (hasTrace || useGreedySetupReply || useGreedyAddRoute) {
                        // setup a non-essential vroute to src
                        bool addedRoute = false;
                        VlrPathID newPathid;
                        do {    // loop in case generated newPathid isn't unique in my vlrRoutingTable
                            newPathid = genPathID(newnode);
                            addedRoute = vlrRoutingTable.addRoute(newPathid, vid, newnode, /*prevhopVid=*/VLRRINGVID_NULL, /*nexthopVid=*/nextHopVid, /*isVsetRoute=*/false).second;
                        } while (!addedRoute);

                        if (newnodeInVset) {    // newnode was already in vset
                            vneiItr->second.vsetRoutes.insert(newPathid);
                            // cancelAndDelete(vneiItr->second.setupReqTimer);
                            // vneiItr->second.setupReqTimer = nullptr;
                            vlrRoutingTable.vlrRoutesMap.at(newPathid).isVsetRoute = true;

                            if (useGreedySetupReply) {
                                auto replyOutgoing = createSetupReply(newnode, proxy, newPathid);    // srcVset = oldVset (doesn't include newnode) in the created SetupReply
                                // replyOutgoing->setReqVnei(false);
                                replyOutgoing->setVlrOption(vlrOptionOut);
                                EV_INFO << "Sending SetupReply(reqVnei=false) to newnode = " << newnode << ", src = " << vid << ", pathid = " << newPathid << ", proxy = " << replyOutgoing->getProxy() << ", nexthop: " << nextHopVid << endl;
                                sendCreatedSetupReply(replyOutgoing, /*outGateIndex=*/psetTable.getPneiGateIndex(nextHopVid));
                            } else {    // hasTrace = true, send SetupReply with trace
                                auto replyOutgoing = createSetupReply(newnode, /*proxy=*/VLRRINGVID_NULL, newPathid);    // srcVset = oldVset (doesn't include newnode) in the created SetupReply
                                replyOutgoing->setTrace(trace);
                                EV_INFO << "Sending SetupReply to newnode = " << newnode << ", src = " << vid << ", pathid = " << newPathid << ", trace = " << replyOutgoing->getTrace() << ", nexthop: " << nextHopVid << endl;
                                sendCreatedSetupReply(replyOutgoing, /*outGateIndex=*/psetTable.getPneiGateIndex(nextHopVid));
                            }

                            if (recordStatsToFile) {   // record sent message
                                std::ostringstream s;
                                s << "pathid=" << newPathid;
                                if (repairRoute)    // record "reqRepairRoute=1" to infoStr field if repairRoute=true
                                    s << "reqRepairRoute=" << repairRoute;
                                recordMessageRecord(/*action=*/0, /*src=*/vid, /*dst=*/newnode, "SetupReply", /*msgId=*/0, /*hopcount=*/0, /*chunkByteLength=*/0, /*infoStr=*/s.str().c_str());   // unused params (msgId, hopcount, chunkByteLength) are just assigned to 0
                            }

                        }
                        else if (hasTrace || useGreedyAddRoute) {    // newnode wasn't in vset or added to vset, probably a pendingVnei requesting a nonEss vroute
                            nonEssRoutes[newPathid] = simTime() + nonEssRouteExpiration;

                            if (useGreedyAddRoute) {
                                auto replyOutgoing = createAddRoute(newnode, newPathid);    // srcVset = vset, newnode isn't added to my vset
                                replyOutgoing->setProxy(vlrOptionOut.getDstVid());
                                replyOutgoing->setVlrOption(vlrOptionOut);
                                EV_INFO << "Sending AddRoute greedy to newnode = " << newnode << ", src = " << vid << ", pathid = " << newPathid << ", proxy = " << replyOutgoing->getProxy() << ", nexthop: " << nextHopVid << endl;
                                sendCreatedAddRoute(replyOutgoing, /*outGateIndex=*/psetTable.getPneiGateIndex(nextHopVid));
                            } else {    // hasTrace = true, send AddRoute with trace
                                auto replyOutgoing = createAddRoute(newnode, newPathid);    // srcVset = vset, newnode isn't added to my vset
                                replyOutgoing->setTrace(trace);
                                EV_INFO << "Sending AddRoute to newnode = " << newnode << ", src = " << vid << ", pathid = " << newPathid << ", trace = " << replyOutgoing->getTrace() << ", nexthop: " << nextHopVid << endl;
                                sendCreatedAddRoute(replyOutgoing, /*outGateIndex=*/psetTable.getPneiGateIndex(nextHopVid));
                            }

                            // cancelAndDelete setupReq timer to newnode in pendingVset
                            auto pendingItr = pendingVset.find(newnode);
                            if (pendingItr != pendingVset.end() && pendingItr->second.setupReqTimer) {
                                cancelAndDelete(pendingItr->second.setupReqTimer);
                                pendingItr->second.setupReqTimer = nullptr;
                            }

                            if (recordStatsToFile) {   // record sent message
                                std::ostringstream s;
                                s << "pathid=" << newPathid;
                                if (repairRoute)    // record "reqRepairRoute=1" to infoStr field if repairRoute=true
                                    s << "reqRepairRoute=" << repairRoute;
                                    s << " vset=" << printVsetToString() << " pendingVset=" << printPendingVsetToString();
                                recordMessageRecord(/*action=*/0, /*src=*/vid, /*dst=*/newnode, "AddRoute", /*msgId=*/0, /*hopcount=*/0, /*chunkByteLength=*/0, /*infoStr=*/s.str().c_str());   // unused params (msgId, hopcount, chunkByteLength) are just assigned to 0
                            }
                        }
                    }
                }
                // process srcVset in received SetupReq whether or not I built a vroute to newnode
                processOtherVset(reqIncoming, /*srcVid=*/newnode);
            }
        }
    }
    else {  // forward this req with findNextHop(exclude=newnode)
        // auto reqOutgoing = staticPtrCast<SetupReqInt>(reqIncoming->dupShared());
        forwardSetupReq(reqIncoming, pktForwarded, withTrace, recordTrace);

        // utilize msg trace to record path to nodes close to me
        if (hasTrace && checkOverHeardTraces && reqIncoming->getTraceVec().size() >= overHeardTraceMinCheckLen) {
            if (recordTrace)    // trace: [src, .., node before me]
                addCloseNodesFromTrace(/*numHalf=*/vsetAndPendingHalfCardinality, reqIncoming->getTraceVec(), /*removeLoop=*/recordTrace, /*traceEndIndex=*/reqIncoming->getTraceVec().size()-1);
            else    // trace: [src, .., me, .., dst], only consider traversed nodes (before me in trace)
                addCloseNodesFromTrace(/*numHalf=*/vsetAndPendingHalfCardinality, reqIncoming->getTraceVec(), /*removeLoop=*/recordTrace, /*traceEndIndex=*/reqIncoming->getIndexInTrace());
        } else if (checkOverHeardTraces) {    // see if src of msg belongs to pendingVset
            pendingVsetAdd(newnode, /*heardFrom=*/VLRRINGVID_NULL, /*addToEmpty=*/false);
        }
    }

    if (recordStatsToFile && recordDroppedMsg && !pktRecorded) {   // record message
        if (reqForMe)
            recordMessageRecord(/*action=*/1, /*src=*/newnode, /*dst=*/dstVid, "SetupReq", /*msgId=*/reqIncoming->getMessageId(), /*hopcount=*/reqIncoming->getHopcount()+1, /*chunkByteLength=*/reqIncoming->getByteLength());
        else if (!pktForwarded)
            recordMessageRecord(/*action=*/4, /*src=*/newnode, /*dst=*/dstVid, "SetupReq", /*msgId=*/reqIncoming->getMessageId(), /*hopcount=*/reqIncoming->getHopcount()+1, /*chunkByteLength=*/reqIncoming->getByteLength());
    }
}

void Vlr::forwardSetupReq(SetupReqInt* reqOutgoing, bool& pktForwarded, bool withTrace, bool recordTrace) {
    VlrRingVID newnode = reqOutgoing->getNewnode();
    VlrRingVID dstVid = reqOutgoing->getDst();        // can also get dst using vlrOptionIn->getDstVid()
    VlrRingVID proxy = reqOutgoing->getProxy();
    VlrRingVID transferNode = reqOutgoing->getTransferNode();

    VlrIntOption& vlrOptionOut = reqOutgoing->getVlrOptionForUpdate();
    // IP.SourceAddress: address of the node from which the packet was received
    VlrRingVID msgPrevHopVid = vlrOptionOut.getPrevHopVid();
    if (msgPrevHopVid == VLRRINGVID_NULL)  // assert prevHopVid isn't myself (shouldn't happen)
        throw cRuntimeError("Received SetupReq with vlrOption.prevHopVid = null");

    if (!withTrace) {   // forward this SetupReq with findNextHop()
        if (transferNode == vid) {  // transferNode is reached
            reqOutgoing->setTransferNode(VLRRINGVID_NULL);
            if (vlrOptionOut.getDstVid() != dstVid) {
                vlrOptionOut.setDstVid(dstVid);
                vlrOptionOut.setCurrentPathid(VLRPATHID_INVALID);
                vlrOptionOut.setTempPathid(VLRPATHID_INVALID);
            }
        }
        // L3Address nextHopAddr = findNextHop(vlrOptionOut, /*prevHopAddrPtr=*/&prevHopAddr, /*excludeVid=*/newnode, /*allowTempRoute=*/true);
        VlrRingVID nextHopVid = findNextHopForSetupReq(vlrOptionOut, /*prevHopVid=*/msgPrevHopVid, /*dstVid=*/dstVid, /*newnode=*/newnode, /*allowTempRoute=*/true);
        
        if (nextHopVid == VLRRINGVID_NULL) {
            // delete vlrOptionOut;
            EV_WARN << "No next hop found for SetupReq received at me = " << vid << ", dropping packet: newnode = " << newnode << ", dst = " << dstVid << ", proxy = " << proxy << ", transferNode = " << transferNode << endl;
            // if (displayBubbles && hasGUI())
            //     getContainingNode(host)->bubble("No next hop found, dropping packet");
            // send setupFail to src of dropped setupReq to notify it of my vset
            if (sendNotifyVsetOnDroppedSetupReq && newnode != vid && dstVid != vid) {
                // bool reqDispatch = reqOutgoing->getReqDispatch();
                bool repairRoute = reqOutgoing->getRepairRoute();
                bool reqVnei = reqOutgoing->getReqVnei();
                // std::set<VlrRingVID> closePendingVneis = getCloseNodesInMapToSet(/*numHalf=*/vsetAndBackupHalfCardinality, /*vidMap=*/pendingVset);
                if (reqVnei && !repairRoute && (vset.find(newnode) != vset.end() /*|| pendingVset.find(newnode) != pendingVset.end()*/)) {
                    std::set<VlrRingVID> srcVsetSet;    // srcVsetSet is sorted bc it's a set
                    int oVsetSize = reqOutgoing->getSrcVsetArraySize();
                    for (int k = 0; k < oVsetSize; k++)
                        srcVsetSet.insert(reqOutgoing->getSrcVset(k));
                    // select nodes in my vset but not in srcVset, put them in notifyVneis
                    std::vector<VlrRingVID> notifyVneis;
                    std::vector<VlrRingVID> vsetVec;
                    for (const auto& vnei : vset)
                        vsetVec.push_back(vnei.first);
                    std::set_difference(vsetVec.begin(), vsetVec.end(), srcVsetSet.begin(), srcVsetSet.end(), std::inserter(notifyVneis, notifyVneis.begin()));
                    // std::set_difference(vset.begin(), vset.end(), srcVsetSet.begin(), srcVsetSet.end(), std::inserter(notifyVneis, notifyVneis.begin()));
                    if (!notifyVneis.empty()) {
                        bool hasTrace = (reqOutgoing->getTraceVec().size() > 0);
                        // send NotifyVset back to src
                        bool notifymsgSent = false;
                        if (!hasTrace) {
                            // send NotifyVset to newnode, notify it of my vset
                            VlrIntOption vlrOptionNew;
                            VlrRingVID vlrOptionDst = (proxy == VLRRINGVID_NULL) ? newnode : proxy;
                            initializeVlrOption(vlrOptionNew, /*dstVid=*/vlrOptionDst);
                            VlrRingVID nextHopVid = findNextHopForSetupReq(vlrOptionNew, /*prevHopVid=*/VLRRINGVID_NULL, /*dstVid=*/newnode, /*newnode=*/VLRRINGVID_NULL, /*allowTempRoute=*/true);
                            if (nextHopVid == VLRRINGVID_NULL) {
                                // delete vlrOptionNew;
                                EV_WARN << "No next hop found to send NotifyVset back to newnode = " << newnode << " at me=" << vid << ", dropping SetupReq to dst = " << dstVid << endl;
                            } else {
                                // auto replyOutgoing = createSetupFail(newnode, proxy);    // srcVset = vset, newnode isn't added to my vset
                                // replyOutgoing->setNotifyVsetOnly(true);
                                auto replyOutgoing = createNotifyVset(/*dstVid=*/newnode, /*toVnei=*/false);
                                replyOutgoing->setProxy(proxy);
                                replyOutgoing->setVlrOption(vlrOptionNew);
                                EV_INFO << "Sending NotifyVset to newnode = " << newnode << ", proxy = " << replyOutgoing->getProxy() << ", nexthop: " << nextHopVid << endl;
                                sendCreatedNotifyVset(replyOutgoing, /*outGateIndex=*/psetTable.getPneiGateIndex(nextHopVid), /*computeChunkLength=*/true);
                                notifymsgSent = true;
                            }

                        } else {    // hasTrace = true, send this NotifyVset with trace
                            std::vector<VlrRingVID> trace = removeLoopInTrace(reqOutgoing->getTraceVec());  // trace: [newnode, .., previous node before me] contains no loop
                            // ensure it's possible for me send SetupReply to newnode via trace
                            unsigned int nextHopIndex = getNextHopIndexInTrace(trace);
                            if (nextHopIndex >= trace.size()) {
                                EV_WARN << "No next hop found to send NotifyVset with trace back to newnode = " << newnode << " at me=" << vid << ", dropping SetupReq to dst = " << dstVid << ", trace = " << trace << endl;
                            } else {    // trace[nextHopIndex] is a pnei of me that's closest to newnode in trace
                                VlrRingVID nextHopVid = trace.at(nextHopIndex);
                                auto replyOutgoing = createNotifyVset(/*dstVid=*/newnode, /*toVnei=*/false);
                                replyOutgoing->setTrace(trace);
                                sendCreatedNotifyVset(replyOutgoing, /*outGateIndex=*/psetTable.getPneiGateIndex(nextHopVid));
                                EV_INFO << "Sending NotifyVset to newnode = " << newnode << ", trace = " << replyOutgoing->getTrace() << ", nexthop: " << nextHopVid << endl;
                                notifymsgSent = true;
                            }
                        }
                        if (notifymsgSent && recordStatsToFile) {  // record sent message
                            std::ostringstream s;
                            s << "no next hop for SetupReq to dst=" << dstVid;
                            recordMessageRecord(/*action=*/0, /*src=*/vid, /*dst=*/newnode, "NotifyVset", /*msgId=*/0, /*hopcount=*/0, /*chunkByteLength=*/0, /*infoStr=*/s.str().c_str());   // unused params (msgId, hopcount, chunkByteLength) are just assigned to 0
                        }
                    }
                }
            }
        } else {
            if (recordTrace)
                reqOutgoing->getTraceVecForUpdate().push_back(vid);  // put myself in traceVec
            sendCreatedSetupReq(reqOutgoing, /*outGateIndex=*/psetTable.getPneiGateIndex(nextHopVid), /*computeChunkLength=*/recordTrace);   // recompute chunk length if traceVec is modified
            pktForwarded = true;
        }
    } else { // forward this SetupReq with trace
        unsigned int nextHopIndex = getNextHopIndexInTraceForSetupReq(reqOutgoing->getTraceVec(), /*myIndexInTrace=*/reqOutgoing->getIndexInTrace());
        // reqOutgoing->getTraceVec(): [src, .., me, .., dst]
        if (nextHopIndex >= reqOutgoing->getTraceVec().size()) {
            EV_WARN << "No next hop found for SetupReq with trace received at me = " << vid << " because no node in trace is a LINKED pnei, dropping packet: newnode = " << newnode << ", dst = " << dstVid << ", trace = " << reqOutgoing->getTraceVec() << endl;
            // if (displayBubbles && hasGUI())
            //     getContainingNode(host)->bubble("No next hop found, dropping packet");
            // try greedy routing this setupReq to dst instead of using trace
            // VlrIntOption *vlrOption = createVlrOption(dstVid);     // create VlrOption to be set in IP header in datagramLocalOutHook()
            initializeVlrOption(vlrOptionOut, /*dstVid=*/dstVid);
            vlrOptionOut.setPrevHopVid(msgPrevHopVid);      // NOTE we can't erase vlrOptionOut.prevHopVid bc can't forward when msgPrevHopVid == VLRRINGVID_NULL

            reqOutgoing->setRecordTrace(true);
            VlrIntVidVec& trace = reqOutgoing->getTraceVecForUpdate();
            trace.erase(trace.begin() + reqOutgoing->getIndexInTrace(), trace.end());      // erase nodes after and including myself from trace
            forwardSetupReq(reqOutgoing, pktForwarded, /*withTrace=*/false, /*recordTrace=*/true);
        } else {    // nexthop found for SetupReq with trace
            reqOutgoing->setIndexInTrace(nextHopIndex);     // set indexInTrace to index of next hop
            VlrRingVID nextHopVid = reqOutgoing->getTraceVec().at(nextHopIndex);

            EV_INFO << "Sending SetupReq to dst = " << dstVid << ", newnode = " << newnode << ", trace = " << reqOutgoing->getTraceVec() << ", nexthopVid: " << nextHopVid << endl;
            sendCreatedSetupReq(reqOutgoing, /*outGateIndex=*/psetTable.getPneiGateIndex(nextHopVid), /*computeChunkLength=*/false);  // trace in setupReq isn't modified
            pktForwarded = true;
        }
    }
}

// for routing SetupReq, check if any node along the trace after myself is a LINKED pnei, if so return its index in trace
// trace is loop-free, contains [src, .., me, nextHop, .., dst]
// if myIndexInTrace < trace.size()-1, first check if next hop after me is a a LINKED pnei, if so return its index as directly --> skip checking every node after me in trace
unsigned int Vlr::getNextHopIndexInTraceForSetupReq(const VlrIntVidVec& trace, unsigned int myIndexInTrace) const
{
    EV_DEBUG << "Finding nexthop for SetupReq with trace: " << trace << endl;
    std::set<VlrRingVID> linkedPneis = psetTable.getPneisLinkedSet();
    unsigned int nextHopIndex = trace.size();
    if (nextHopIndex > 0) {     // trace shouldn't be empty, otherwise hasTrace would be false
        if (myIndexInTrace < nextHopIndex-1) {  // I'm not the last node in trace
            if (trace[myIndexInTrace] == vid && linkedPneis.find(trace[myIndexInTrace+1]) != linkedPneis.end())    // next node after me in trace is a LINKED pnei
                return (myIndexInTrace + 1);
        }
        // next node after me in trace isn't a LINKED pnei, check if any node after me in trace is a LINKED pnei
        unsigned int i;
        for (i = nextHopIndex-1; i > 0; i--) {   // check every node after src (index 0) in trace in reverse order
            if (trace[i] == vid)        // trace[i] is myself      NOTE myself must be in trace
                break;
            else if (linkedPneis.find(trace[i]) != linkedPneis.end()) {    // trace[i] is a LINKED pnei
                nextHopIndex = i;
                break;
            }
        }
    }
    return nextHopIndex;
}

// trace: [src, ..., (me, ...)]     traceEndIndex: index of the node before me in trace, I'll only consider nodes upto (including) trace[traceEndIndex], traceEndIndex < trace.size()
// if removeLoop=true, trace may contain loops that need to be removed before recording
void Vlr::addCloseNodesFromTrace(int numHalf, const VlrIntVidVec& trace, bool removeLoop, unsigned int traceEndIndex)
{
    if (pendingVset.size() >= 2) {  // don't try to add nodes to pendingVset if I don't already have enough nodes to determine if a node is close to me, bc it's likely I haven't joined overlay
        std::set<VlrRingVID> traceSet(trace.begin(), trace.begin() + traceEndIndex + 1);  // convert nodes in trace to a set
        std::vector<VlrRingVID> closeVec = getCloseNodesInSet(/*numHalf=*/numHalf, traceSet);    // numHalf ccw/cw nodes close to me in traceSet
        // bool lastNodeAddedInPendingVset = true;     // Commented out bc it's possible a node isn't added to pendingVset bc it's my vnei -- if two nodes can't be added in pendingVset back to back, no need to check further in closeVec
        for (const auto& node : closeVec) {
            pendingVsetAdd(node, /*heardFrom=*/VLRRINGVID_NULL, /*addToEmpty=*/false);
            auto nodeItrPending = pendingVset.find(node);
            if (nodeItrPending == pendingVset.end()) {  // node not added in pendingVset
                // if (!lastNodeAddedInPendingVset)    // last node in closeVec wasn't added in pendingVset either
                //     break;
                // else
                //     lastNodeAddedInPendingVset = false;
            } 
            else {  // node is now in pendingVset
                // lastNodeAddedInPendingVset = true;
                if (!IsLinkedPneiOrAvailableRouteEnd(node) && overheardTraces.find(node) == overheardTraces.end()) {    // node isn't linked pnei or existing vroute endpoint in vlrRoutingTable, and not exists in overheardTraces
                    std::set<VlrRingVID> pathNodes;     // nodes in recorded trace to node
                    overheardTraces[node] = {{}, simTime() + overHeardTraceExpiration};    // initialize expireTime of overheard trace to node
                    std::vector<VlrRingVID>& overheardTrace = overheardTraces[node].first;
                    if (trace[traceEndIndex] != vid) {  // assume trace[traceEndIndex] is a pnei of me
                        // ensure I'm the first node in overheardTrace
                        overheardTrace.push_back(vid);
                        pathNodes.insert(vid);
                    }

                    for (int i = traceEndIndex; i >= 0; i--) {
                        VlrRingVID currnode = trace[i];
                        if (!removeLoop) {
                            overheardTrace.push_back(currnode);
                        } else {    // check for loops in trace
                            if (pathNodes.find(currnode) != pathNodes.end()) {  // if currnode already exists in newTrace
                                // delete the nodes between the 2 duplicate currnode, excluding the previous currnode
                                auto tailItr = --overheardTrace.end();  // iterator to last node in newTrace
                                while (*tailItr != currnode) {
                                    pathNodes.erase(*tailItr);
                                    overheardTrace.erase(tailItr--);
                                }
                            } else {    // if currnode hasn't appeared in newTrace
                                overheardTrace.push_back(currnode);
                                pathNodes.insert(currnode);
                            }
                        }
                        if (currnode == node)
                            break;
                    }
                    // // overheardTrace: [me, ..., node], convert it to [node, ..., me]
                    // std::reverse(overheardTrace.begin(), overheardTrace.end());
                    EV_DETAIL << "Added overheard trace from me=" << vid << " to node=" << node << ": " << overheardTrace << ", using trace " << trace << ", removeLoop=" << removeLoop << ", traceEndIndex=" << traceEndIndex << endl;
                }
            }
        }
    }
}

void Vlr::processRecentReplacedVneiTimer()
{
    // send NotifyVset to every node in recentReplacedVneis
    for (auto it = recentReplacedVneis.begin(); it != recentReplacedVneis.end(); ++it) {
        const VlrRingVID& oldVnei = *it;

        VlrIntOption vlrOptionOut;
        initializeVlrOption(vlrOptionOut, /*dstVid=*/oldVnei);
        VlrRingVID nextHopVid = findNextHop(vlrOptionOut, /*prevHopVid=*/VLRRINGVID_NULL, /*excludeVid=*/VLRRINGVID_NULL, /*allowTempRoute=*/true);
        if (nextHopVid == VLRRINGVID_NULL) {
            // delete vlrOptionOut;
            EV_WARN << "No next hop found to send NotifyVset at me = " << vid << " to old vnei = " << oldVnei << ", shouldn't happen because if old vset-route exists" << endl;
        } else {
            auto msgOutgoing = createNotifyVset(/*dstVid=*/oldVnei, /*toVnei=*/false);
            msgOutgoing->setVlrOption(vlrOptionOut);

            if (recordStatsToFile) {   // record sent message
                std::ostringstream s;
                s << "replaced oldVnei=" << oldVnei << " vset=" << printVsetToString();
                recordMessageRecord(/*action=*/0, /*src=*/vid, /*dst=*/oldVnei, "NotifyVset", /*msgId=*/0, /*hopcount=*/0, /*chunkByteLength=*/0, /*infoStr=*/s.str().c_str());   // unused params (msgId, hopcount, chunkByteLength) are just assigned to 0
            }
            EV_INFO << "Sending NotifyVset to old vnei = " << oldVnei << ", nexthop: " << nextHopVid << endl;
            sendCreatedNotifyVset(msgOutgoing, /*outGateIndex=*/psetTable.getPneiGateIndex(nextHopVid));
        }
    }
    recentReplacedVneis.clear();
}

int Vlr::computeSetupReplyByteLength(SetupReplyInt* msg) const
{
    // unsigned int proxy, newnode, src;
    int chunkByteLength = VLRRINGVID_BYTELEN * 3;
    // VlrPathID pathid
    chunkByteLength += VLRPATHID_BYTELEN;
    // unsigned int srcVset[]
    chunkByteLength += msg->getSrcVsetArraySize() * VLRRINGVID_BYTELEN;
    // std::vector<unsigned int> trace
    chunkByteLength += msg->getTrace().size() * VLRRINGVID_BYTELEN;
    // std::vector<unsigned int> prevhopVids
    // unsigned int oldestPrevhopIndex
    chunkByteLength += (routePrevhopVidsSize-1) * VLRRINGVID_BYTELEN + 2;   // routePrevhopVidsSize-1 bc size of prevHopVid included in vlrOption
    // // bool reqVnei
    // chunkByteLength += 1;

    // for VlrIntUniPacket
    chunkByteLength += getVlrUniPacketByteLength();

    return chunkByteLength;
}

SetupReplyInt* Vlr::createSetupReply(const VlrRingVID& newnode, const VlrRingVID& proxy, const VlrPathID& pathid)
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

    setupPacketSrcVset(msg);
    msg->setMessageId(++allSendMessageId);

    initializeVlrOption(msg->getVlrOptionForUpdate());

    return msg;
}

// if computeChunkLength = true, compute chunk length because chunk was just created with createSetupReply() or modified after dupShared() from another chunk
// else, no need to compute chunk length because chunk was dupShared() (not modified) from another chunk that has chunkLength set
void Vlr::sendCreatedSetupReply(SetupReplyInt *setupReply, const int& outGateIndex, bool computeChunkLength/*=true*/, double delay/*=0*/)
{
    if (computeChunkLength)
        setupReply->setByteLength(computeSetupReplyByteLength(setupReply));
    EV_DEBUG << "Sending setupReply: src = " << setupReply->getSrc() << ", newnode = " << setupReply->getNewnode() << ", proxy = " << setupReply->getProxy() << endl;

    setupReply->getVlrOptionForUpdate().setPrevHopVid(vid);    // set packet prevHopVid to myself
    setupReply->setHopcount(setupReply->getHopcount() +1);    // increment packet hopcount
    
    // NOTE addTag should be executed after chunkLength has been set, and chunkLength shouldn't be changed before findTag/getTag

    // all multihop VLR packets (setupReq, setupReply, etc) L3 dst are set to a pnei, greedy routing at L3 in routeDatagram() isn't needed, but we do greedy routing and deal with VlrOption at L4 (in processSetupReq() for example) 
    // udpPacket->addTagIfAbsent<VlrIntOptionReq>()->setVlrOption(vlrOption);      // VlrOption to be set in IP header in datagramLocalOutHook()
    
    sendCreatedPacket(setupReply, /*unicast=*/true, /*outGateIndex=*/outGateIndex, /*delay=*/delay, /*checkFail=*/true);
}

void Vlr::processSetupReply(SetupReplyInt *replyIncoming, bool& pktForwarded)
{
    EV_DEBUG << "Received SetupReply" << endl;
    
    // // VlrIntOption *vlrOptionIn = nullptr;
    // VlrIntOption& vlrOptionIn = replyIncoming->getVlrOptionForUpdate();
    VlrRingVID msgPrevHopVid = replyIncoming->getVlrOption().getPrevHopVid();
    if (msgPrevHopVid == VLRRINGVID_NULL)
        throw cRuntimeError("Received SetupReply with vlrOption.prevHopVid = null");

    bool withTrace = (replyIncoming->getTrace().size() > 0);
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

    auto vrouteItr = vlrRoutingTable.vlrRoutesMap.find(newPathid);
    if (vrouteItr != vlrRoutingTable.vlrRoutesMap.end()) {  // newPathid already in vlrRoutingTable
        // EV_WARN << "The new pathid " << newPathid << " of setupReply is already in my routing table, tearing down both paths with the same pathid" << endl;
        // // tear down path recorded in routing table (pathid duplicate, something is wrong, can be a loop)
        // std::vector<L3Address> sendTeardownToAddrs = {vrouteItr->second.prevhopAddr, vrouteItr->second.nexthopAddr};
        // // get 2-bit isUnavailable of prevhop and nexthop
        // std::vector<char> nextHopStates = {vlrRoutingTable.getPrevNextIsUnavailable(vrouteItr->second.isUnavailable, /*getPrev=*/true), vlrRoutingTable.getPrevNextIsUnavailable(vrouteItr->second.isUnavailable, /*getPrev=*/false)};
        // if (prevHopAddr != vrouteItr->second.prevhopAddr && prevHopAddr != vrouteItr->second.nexthopAddr) {
        //     // tear down path received from prevHopAddr
        //     sendTeardownToAddrs.push_back(prevHopAddr);
        // }
        // // check if I'm an endpoint of pathid, if so this may be a vset route, or maybe in nonEssRoutes
        // if (vrouteItr->second.fromVid == vid)
        //     removeEndpointOnTeardown(newPathid, /*towardVid=*/vrouteItr->second.toVid, /*pathIsVsetRoute=*/vrouteItr->second.isVsetRoute, /*pathIsTemporary=*/vrouteItr->second.isUnavailable);
        // else if (vrouteItr->second.toVid == vid)
        //     removeEndpointOnTeardown(newPathid, /*towardVid=*/vrouteItr->second.fromVid, /*pathIsVsetRoute=*/vrouteItr->second.isVsetRoute, /*pathIsTemporary=*/vrouteItr->second.isUnavailable);
        
        // vlrRoutingTable.removeRouteByPathID(newPathid);
        
        // EV_DETAIL << "Sending Teardown (pathid = " << newPathid << ") to [";
        // for (const auto& nextHopAddr : sendTeardownToAddrs)
        //     if (!nextHopAddr.isUnspecified())
        //         EV_DETAIL << nextHopAddr << ' ';
        // EV_DETAIL << ']' << endl;

        // for (size_t i = 0; i < sendTeardownToAddrs.size(); ++i) {
        //     const L3Address& nextHopAddr = sendTeardownToAddrs[i];
        //     if (!nextHopAddr.isUnspecified()) {  // I'm not an endpoint of the vroute
        //         if (i < 2) {    // send Teardown to prevhop and nexthop of the existing vroute
        //             const char& nextHopIsUnavailable = nextHopStates[i];
        //             if (nextHopIsUnavailable != 1) {  // next hop isn't unavailable
        //                 const auto& teardownOut = createTeardownOnePathid(newPathid, /*addSrcVset=*/false, /*rebuild=*/true);
        //                 sendCreatedTeardownToNextHop(teardownOut, nextHopAddr, nextHopIsUnavailable);
        //             } else {  // next hop is unavailable, remove this vroute that have been torn down from lostPneis.brokenVroutes
        //                 removeRouteFromLostPneiBrokenVroutes(newPathid, /*lostPneiAddr=*/nextHopAddr);
        //             }
        //         } else {    // send Teardown to prevHopAddr
        //             const auto& teardownOut = createTeardownOnePathid(newPathid, /*addSrcVset=*/false, /*rebuild=*/true);
        //             sendCreatedTeardown(teardownOut, nextHopAddr, /*vlrOption=*/nullptr);
        //         }
        //     }
        // }
        EV_WARN << "The new pathid " << newPathid << " of setupReply is already in my routing table, tearing down the new path" << endl;
        // pathid duplicate, something is wrong, can be a loop, if indeed a loop, I send Teardown to prevHopAddr and Teardown will reach me again to remove the existing erroneous vlrRoutingTable record of newPathid
        // tear down newPathid, i.e. send Teardown to prevHopAddr
        TeardownInt* teardownOut = createTeardownOnePathid(newPathid, /*addSrcVset=*/false, /*rebuild=*/true);
        sendCreatedTeardown(teardownOut, /*nextHopPnei=*/msgPrevHopVid);

        if (recordStatsToFile) {   // record sent message
            recordMessageRecord(/*action=*/0, /*src=*/vid, /*dst=*/VLRRINGVID_NULL, "Teardown", /*msgId=*/0, /*hopcount=*/0, /*chunkByteLength=*/0, /*infoStr=*/"setupReply: new path already in routing table");   // unused params (msgId, hopcount, chunkByteLength) are just assigned to 0
        }
    }
    else {    // checked newPathid not in vlrRoutingTable
        // VlrRingVID prevHopVid = getVidFromAddressInRegistry(prevHopAddr);    // map prevHopAddr L3Address to VlrRingVID
        if (!psetTable.pneiIsLinked(msgPrevHopVid)) {        // if prevHopAddr isn't a LINKED pnei
            EV_WARN << "Previous hop " << msgPrevHopVid << " of setupReply is not a LINKED pnei, tearing down pathid " << newPathid << endl;
            // tear down newPathid, i.e. send Teardown to prevHopAddr
            TeardownInt* teardownOut = createTeardownOnePathid(newPathid, /*addSrcVset=*/false, /*rebuild=*/true);
            sendCreatedTeardown(teardownOut, /*nextHopPnei=*/msgPrevHopVid);

            if (recordStatsToFile) {   // record sent message
                recordMessageRecord(/*action=*/0, /*src=*/vid, /*dst=*/VLRRINGVID_NULL, "Teardown", /*msgId=*/0, /*hopcount=*/0, /*chunkByteLength=*/0, /*infoStr=*/"setupReply: previous hop not a LINKED pnei");   // unused params (msgId, hopcount, chunkByteLength) are just assigned to 0
            }
        }
        // checked SetupReply received from a LINKED pnei
        else if (representativeFixed && representative.heardfromvid == VLRRINGVID_NULL) {        // if I don't have a valid rep yet, I won't process or forward this message
            EV_WARN << "No valid rep heard: " << representative << ", cannot accept overlay message, tearing down pathid " << newPathid << endl;
            // tear down newPathid, i.e. send Teardown to prevHopAddr
            TeardownInt* teardownOut = createTeardownOnePathid(newPathid, /*addSrcVset=*/false, /*rebuild=*/true);
            sendCreatedTeardown(teardownOut, /*nextHopPnei=*/msgPrevHopVid);

            if (recordStatsToFile) {   // record sent message
                recordMessageRecord(/*action=*/0, /*src=*/vid, /*dst=*/VLRRINGVID_NULL, "Teardown", /*msgId=*/0, /*hopcount=*/0, /*chunkByteLength=*/0, /*infoStr=*/"setupReply: no rep");   // unused params (msgId, hopcount, chunkByteLength) are just assigned to 0
            }
        }
        // checked I have a valid rep
        else if (newnode == vid) {      // this SetupReply is destined for me
            // bool reqVnei = replyIncoming->getReqVnei();
            pktForMe = true;

            EV_INFO << "Handling setupReply to me: src = " << srcVid << ", newnode = " << newnode << ", proxy = " << proxy << ", pathid = " << newPathid 
                    << ", srcVset = [";
            for (size_t i = 0; i < replyIncoming->getSrcVsetArraySize(); i++)
                EV_INFO << replyIncoming->getSrcVset(i) << " ";
            EV_INFO << "]" << endl;

            if (vset.find(srcVid) == vset.end()) {     // srcVid not in my vset
                VlrIntVidSet emptySet;         // knownSet only needed to determine neisToForward, since we don't care neisToForward, create dummy knownSet for shouldAddVnei(..)
                auto addResult = shouldAddVnei(srcVid, /*knownSet=*/emptySet, /*findNeisToForward=*/false);
                bool& shouldAdd = std::get<0>(addResult);  // access first element in tuple
                std::vector<VlrRingVID>& removedNeis = std::get<1>(addResult);
                EV_DETAIL << "shouldAdd(newnode=" << newnode << ") returns: shouldAdd = " << shouldAdd << ", removedNeis = " << removedNeis << endl;
                if (shouldAdd) {
                    if (recordStatsToFile) {   // record received setupReply for me that indeed adds newPathid to vlrRoutingTable
                        recordMessageRecord(/*action=*/1, /*src=*/srcVid, /*dst=*/vid, "SetupReply", /*msgId=*/newPathid, /*hopcount=*/msgHopCount, /*chunkByteLength=*/replyIncoming->getByteLength());    // unimportant params (msgId)
                        pktRecorded = true;
                    }

                    // add srcVid to vset and vlrRoutingTable
                    auto itr_bool = vlrRoutingTable.addRoute(newPathid, srcVid, newnode, /*prevhopVid=*/msgPrevHopVid, /*nexthopVid=*/VLRRINGVID_NULL, /*isVsetRoute=*/true);
                    itr_bool.first->second.hopcount = msgHopCount;
                    std::vector<VlrRingVID>& routePrevhopVids = itr_bool.first->second.prevhopVids;
                    setRoutePrevhopVids(routePrevhopVids, replyIncoming->getPrevhopVids(), replyIncoming->getOldestPrevhopIndex());
            
                    vsetInsertRmPending(srcVid);    // add to vset, remove from pendingVset (cancel WaitSetupReqIntTimer)
                    vset.at(srcVid).vsetRoutes.insert(newPathid);

                    // emit(routeAddedSignal, msgHopCount);
                    // // vrouteLenVector.record(msgHopCount);
                    // // vrouteLenPSquare.collect(msgHopCount);

                    // // set InNetwork = true after wait time 
                    // if (!selfInNetwork) {
                    //     if (!inNetworkWarmupTimer->isScheduled()) {
                    //         scheduleAt(simTime() + inNetworkWarmupTime, inNetworkWarmupTimer);
                    //         EV_DETAIL << "inNetwork scheduled to become true at " << inNetworkWarmupTimer->getArrivalTime() << endl;
                    //     }
                    //     else if (vset.size() >= 2 * vsetHalfCardinality) {
                    //         cancelEvent(inNetworkWarmupTimer);
                    //         selfInNetwork = true;

                    //         lastTimeNodeJoinedInNetwork = simTime().dbl();
                    //     }
                    //     else if (vset.size() == 1) { // vset was empty but inNetworkWarmupTimer was scheduled, this means I was a rep that lost all my vneis, now that I've joined an existing overlay, cancel the original inNetworkWarmupTimer to schedule a new one to wait for other vneis
                    //         cancelEvent(inNetworkWarmupTimer);
                    //         scheduleAt(simTime() + inNetworkWarmupTime, inNetworkWarmupTimer);
                    //         EV_DETAIL << "inNetwork re-scheduled to become true at " << inNetworkWarmupTimer->getArrivalTime() << endl;
                    //     }
                    // }

                    // convert vset to a set of vids for EV_INFO purpose
                    // std::set<VlrRingVID> vsetneis;
                    // auto setItr = vsetneis.end();    // hint for the position where the element can be inserted in vsetneis
                    // for (auto& elem : vset)
                    //     setItr = vsetneis.insert(setItr, elem.first);
                    // EV_INFO << "Adding src = " << srcVid << " to vset = " << vsetneis << ", inNetwork = " << selfInNetwork << endl;
                    EV_INFO << "Adding src = " << srcVid << " to vset = " << printVsetToString() << ", inNetwork = " << selfInNetwork << endl;

                    // remove vneis that no longer belong to my vset bc newnode joining
                    for (const VlrRingVID& oldNei : removedNeis) {
                        // delayRouteTeardown() must be called before oldNei is removed from vset
                        delayRouteTeardown(oldNei, vset.at(oldNei).vsetRoutes);     // vroute btw me and oldNei will be torn down later
                        vsetEraseAddPending(oldNei);
                    }
                    // process srcVset in received SetupReply
                    processOtherVset(replyIncoming, /*srcVid=*/srcVid);

                    // if (recordStatsToFile) { // write node status update
                    //     if (vset.size() >= 2 * vsetHalfCardinality) {  // vset is full
                    //         std::ostringstream s;
                    //         s << "vsetFull: inNetwork=" << selfInNetwork << " vset=" << printVsetToString();
                    //         recordNodeStatsRecord(/*infoStr=*/s.str().c_str());
                            
                    //         if (convertVsetToSet() == vidRingRegVset) {
                    //             std::ostringstream s;
                    //             s << "vsetCorrect: inNetwork=" << selfInNetwork << " vset=" << printVsetToString();
                    //             recordNodeStatsRecord(/*infoStr=*/s.str().c_str());
                    //         }
                    //     }
                    // }

                    // modify selfInNetwork or inNetworkWarmupTimer, recordNodeStatsRecord() to record if vset full and correct
                    recordNewVnei(srcVid);

                } else {    // srcVid doesn't belong to my vset
                    EV_DEBUG << "SrcVid = " << srcVid << " of setupReply should not be added to my vset, tearing down the new pathid " << newPathid << endl;
                    // tear down newPathid, i.e. send Teardown to prevHopAddr
                    TeardownInt* teardownOut = createTeardownOnePathid(newPathid, /*addSrcVset=*/true, /*rebuild=*/false);
                    sendCreatedTeardown(teardownOut, /*nextHopPnei=*/msgPrevHopVid);

                    if (recordStatsToFile) {   // record sent message
                        recordMessageRecord(/*action=*/0, /*src=*/vid, /*dst=*/VLRRINGVID_NULL, "Teardown", /*msgId=*/0, /*hopcount=*/0, /*chunkByteLength=*/0, /*infoStr=*/"setupReply: shouldAdd=false");   // unused params (msgId, hopcount, chunkByteLength) are just assigned to 0
                    }
                }
                
            } else {    // srcVid sent setup to me but it's already in my vset, inconsistent
                // if (reqVnei) {
                //     // newPathid isn't in routing table, maybe there's an old vroute in routing table correlated with srcVid
                //     VlrPathID oldPathid = vlrRoutingTable.getVsetRouteToVnei(srcVid, vid);    // get vset route btw srcVid and me
                //     ASSERT(oldPathid != VLRPATHID_INVALID);     // there should always be a vset route in routing table corresponding to vnei in vset
                //     const auto& vroute = vlrRoutingTable.vlrRoutesMap.at(oldPathid);
                //     // vroute.isVsetRoute = true, so it can't be in nonEssRoutes

                //     // if old path was broken, I should've received Teardown to remove srcVid from vset, thus it's likely I just added srcVid in vset and sent setupReply to it, meanwhile srcVid has added me and its setupReply just arrived at me
                //     // NOTE it's also possible that old path was broken, Teardown arrived at srcVid and srcVid sent a setupReqTrace to me, which progressed in the network faster than the Teardown of old path to me
                //     // if old path initiated by me, and src > me, keep old path as vset route, else, use new path as vset route

                //     if (vroute.fromVid == vid && srcVid > vid) {     // if SetupReply of old path was initiated by me, this path was initiated by src, possibly we sent setupReq to each other at same time, assume old path isn't broken, keep the old path if src > me
                //         // tear down new path, keep old path as vset route, assuming old path isn't broken
                //         EV_WARN << "Src = " << srcVid << " of setupReply already in my vset, tearing down newPathid " << newPathid << ", keeping the old pathid " << oldPathid << " " << vroute << endl;
                //         // tear down newPathid, i.e. send Teardown to prevHopAddr
                //         const auto& teardownOut = createTeardownOnePathid(newPathid, /*addSrcVset=*/true, /*rebuild=*/false);
                //         sendCreatedTeardown(teardownOut, prevHopAddr, /*vlrOption=*/nullptr);
                //     } else {    // Either the old path was also initiated by src, old path must be broken, accept the new path  Or old path was initiated by me and src < me
                //         // tear down old path, accept new path as vset route
                //         EV_WARN << "Src = " << srcVid << " of setupReply already in my vset, tearing down the old pathid " << oldPathid << " " << vroute << endl;

                //         bool isFromPrevhop = (srcVid == vroute.fromVid);    // if from prevhop, send Teardown to prevhop
                //         const L3Address& nextHopAddr = (isFromPrevhop) ? vroute.prevhopAddr : vroute.nexthopAddr;
                //         // tear down old path with oldPathid
                //         char nextHopIsUnavailable = vlrRoutingTable.getPrevNextIsUnavailable(vroute.isUnavailable, /*getPrev=*/isFromPrevhop);
                //         if (nextHopIsUnavailable != 1) {  // next hop isn't unavailable
                //             const auto& teardownOut = createTeardownOnePathid(oldPathid, /*addSrcVset=*/true, /*rebuild=*/false);
                //             sendCreatedTeardownToNextHop(teardownOut, nextHopAddr, nextHopIsUnavailable);
                //         } else {  // next hop is unavailable, remove this vroute that have been torn down from lostPneis.brokenVroutes
                //             removeRouteFromLostPneiBrokenVroutes(oldPathid, /*lostPneiAddr=*/nextHopAddr);
                //         }

                //         // since newPathid != oldPathid, just accpet this SetupReply? since srcVid is already in my vset there shouldn't be any vneis removed bc of srcVid, and InNetwork shouldn't change
                //         // remove oldPathid, add newPathid to vlrRoutingTable
                //         vlrRoutingTable.removeRouteByPathID(oldPathid);
                //         auto itr_bool = vlrRoutingTable.addRoute(newPathid, srcVid, newnode, prevHopAddr, L3Address(), /*isVsetRoute=*/true);
                //         std::vector<VlrRingVID>& routePrevhopVids = itr_bool.first->second.prevhopVids;
                //         setRoutePrevhopVids(routePrevhopVids, replyIncoming->getPrevhopVids(), replyIncoming->getOldestPrevhopIndex());

                //         if (recordStatsToFile) {   // record received setupReply for me that indeed adds newPathid to vlrRoutingTable
                //             recordMessageRecord(/*action=*/1, /*src=*/srcVid, /*dst=*/vid, "SetupReply", /*msgId=*/newPathid, /*hopcount=*/msgHopCount, /*chunkByteLength=*/B(replyIncoming->getChunkLength()).get());    // unimportant params (msgId)
                //             pktRecorded = true;
                //         }
                //     } 
                // } else {    // reqVnei = false, src sent this setupReply to build a second vset-route
                // add newPathid to vlrRoutingTable as a vset-route
                auto itr_bool = vlrRoutingTable.addRoute(newPathid, srcVid, newnode, /*prevhopVid=*/msgPrevHopVid, /*nexthopVid=*/VLRRINGVID_NULL, /*isVsetRoute=*/true);
                itr_bool.first->second.hopcount = msgHopCount;
                std::vector<VlrRingVID>& routePrevhopVids = itr_bool.first->second.prevhopVids;
                setRoutePrevhopVids(routePrevhopVids, replyIncoming->getPrevhopVids(), replyIncoming->getOldestPrevhopIndex());

                auto vneiItr = vset.find(srcVid);
                vneiItr->second.vsetRoutes.insert(newPathid);
                // cancelAndDelete(vneiItr->second.setupReqTimer);
                // vneiItr->second.setupReqTimer = nullptr;
                EV_INFO << "Adding vset-route pathid" << newPathid << " to src = " << srcVid << " in vset, vsetRoutes size=" << vneiItr->second.vsetRoutes.size() << endl;

                if (recordStatsToFile) {   // record received setupReply for me that indeed adds newPathid to vlrRoutingTable
                    recordMessageRecord(/*action=*/1, /*src=*/srcVid, /*dst=*/vid, "SetupReply", /*msgId=*/newPathid, /*hopcount=*/msgHopCount, /*chunkByteLength=*/replyIncoming->getByteLength());    // unimportant params (msgId)
                    pktRecorded = true;
                }
                
                // process srcVset in received SetupReply
                processOtherVset(replyIncoming, /*srcVid=*/srcVid);
            }
        }
        else {  // this SetupReply isn't destined for me
            // auto replyOutgoing = staticPtrCast<SetupReplyInt>(replyIncoming->dupShared());
            forwardSetupReply(replyIncoming, pktForwarded, newPathid, msgPrevHopVid, withTrace);

            if (checkOverHeardTraces) {
                // see if src of msg belongs to pendingVset
                pendingVsetAdd(srcVid, /*heardFrom=*/VLRRINGVID_NULL, /*addToEmpty=*/false);
                // assuming msg is always sent as a response of req, see if src of req belongs to pendingVset
                pendingVsetAdd(newnode, /*heardFrom=*/VLRRINGVID_NULL, /*addToEmpty=*/false);
                if (proxy != VLRRINGVID_NULL && proxy != newnode)
                    pendingVsetAdd(proxy, /*heardFrom=*/VLRRINGVID_NULL, /*addToEmpty=*/false);
            }
        }
    }

    if (recordStatsToFile && recordDroppedMsg && !pktRecorded) {   // record message
        if (pktForMe)
            recordMessageRecord(/*action=*/1, /*src=*/srcVid, /*dst=*/newnode, "SetupReply", /*msgId=*/replyIncoming->getMessageId(), /*hopcount=*/msgHopCount, /*chunkByteLength=*/replyIncoming->getByteLength());
        else if (!pktForwarded)
            recordMessageRecord(/*action=*/4, /*src=*/srcVid, /*dst=*/newnode, "SetupReply", /*msgId=*/replyIncoming->getMessageId(), /*hopcount=*/msgHopCount, /*chunkByteLength=*/replyIncoming->getByteLength());
    }
}

void Vlr::forwardSetupReply(SetupReplyInt* replyOutgoing, bool& pktForwarded, const VlrPathID& newPathid, VlrRingVID msgPrevHopVid, bool withTrace)
{
    VlrRingVID newnode = replyOutgoing->getNewnode();
    VlrRingVID srcVid = replyOutgoing->getSrc();        
    VlrRingVID proxy = replyOutgoing->getProxy();       // can also get proxy using vlrOptionIn->getDstVid()
    // unsigned int msgHopCount = replyOutgoing->getHopcount() +1;

    if (!withTrace) {
        // forward this SetupReply with findNextHopForSetupReply()
        VlrIntOption& vlrOptionOut = replyOutgoing->getVlrOptionForUpdate();
        VlrRingVID nextHopVid = findNextHopForSetupReply(vlrOptionOut, /*prevHopVid=*/msgPrevHopVid, /*newnode=*/newnode);
        if (nextHopVid == VLRRINGVID_NULL) {
            // delete vlrOptionOut;
            EV_WARN << "No next hop found for SetupReply received at me = " << vid << ", tearing down vroute: pathid = " << newPathid << ", newnode = " << newnode << ", src = " << srcVid << ", proxy = " << proxy << endl;
            // tear down newPathid, i.e. send Teardown to prevHopAddr
            TeardownInt* teardownOut = createTeardownOnePathid(newPathid, /*addSrcVset=*/false, /*rebuild=*/true);
            sendCreatedTeardown(teardownOut, /*nextHopPnei=*/msgPrevHopVid);

            if (recordStatsToFile) {   // record sent message
                std::ostringstream s;
                s << "setupReply: no next hop found w/o trace src=" << srcVid << " newnode=" << newnode << " proxy=" << proxy;
                recordMessageRecord(/*action=*/0, /*src=*/vid, /*dst=*/VLRRINGVID_NULL, "Teardown", /*msgId=*/0, /*hopcount=*/0, /*chunkByteLength=*/0, /*infoStr=*/s.str().c_str());   // unused params (msgId, hopcount, chunkByteLength) are just assigned to 0
            }

            // if (displayBubbles && hasGUI())
            //     getContainingNode(host)->bubble("No next hop found for SetupReply");
        } else {    // nexthop found for SetupReply
            // we've checked newPathid not in vlrRoutingTable
            auto itr_bool = vlrRoutingTable.addRoute(newPathid, srcVid, newnode, /*prevhopVid=*/msgPrevHopVid, /*nexthopVid=*/nextHopVid, /*isVsetRoute=*/false);
            std::vector<VlrRingVID>& routePrevhopVids = itr_bool.first->second.prevhopVids;
            unsigned int oldestPrevhopIndex = setRoutePrevhopVidsFromMessage(routePrevhopVids, replyOutgoing->getPrevhopVidsForUpdate(), replyOutgoing->getOldestPrevhopIndex());
            replyOutgoing->setOldestPrevhopIndex(oldestPrevhopIndex);
            
            EV_INFO << "Added new vroute: newPathid = " << newPathid << " " << vlrRoutingTable.vlrRoutesMap.at(newPathid) << endl;
            // replyOutgoing->setHopcount(msgHopCount);     // Commented out bc will increment in sendCreatedSetupReply()
            sendCreatedSetupReply(replyOutgoing, /*outGateIndex=*/psetTable.getPneiGateIndex(nextHopVid), /*computeChunkLength=*/false);
            pktForwarded = true;
        }
    } else { // forward this SetupReply with trace
        // replyOutgoing->getTraceForUpdate(): [start node, .., parent node, (me, skipped nodes)]
        unsigned int nextHopIndex = getNextHopIndexInTrace(replyOutgoing->getTraceForUpdate());
        // replyOutgoing->getTrace(): [start node, .., parent node]
        if (nextHopIndex >= replyOutgoing->getTrace().size()) {
            EV_WARN << "No next hop found for SetupReply with trace received at me = " << vid << " because no node in trace is a LINKED pnei, tearing down vroute: pathid = " << newPathid << ", newnode = " << newnode << ", src = " << srcVid << ", trace = " << replyOutgoing->getTrace() << endl;
            // tear down newPathid, i.e. send Teardown to prevHopAddr
            TeardownInt *teardownOut = createTeardownOnePathid(newPathid, /*addSrcVset=*/false, /*rebuild=*/true);
            sendCreatedTeardown(teardownOut, /*nextHopPnei=*/msgPrevHopVid);

            if (recordStatsToFile) {   // record sent message
                recordMessageRecord(/*action=*/0, /*src=*/vid, /*dst=*/VLRRINGVID_NULL, "Teardown", /*msgId=*/0, /*hopcount=*/0, /*chunkByteLength=*/0, /*infoStr=*/"setupReply: no next hop found in trace");   // unused params (msgId, hopcount, chunkByteLength) are just assigned to 0
            }

        } else {    // nexthop found for SetupReply with trace
            // replyOutgoing->setHopcount(msgHopCount);
            VlrRingVID nextHopVid = replyOutgoing->getTrace().at(nextHopIndex);

            // we've checked newPathid not in vlrRoutingTable
            auto itr_bool = vlrRoutingTable.addRoute(newPathid, srcVid, newnode, /*prevhopVid=*/msgPrevHopVid, /*nexthopVid=*/nextHopVid, /*isVsetRoute=*/false);
            std::vector<VlrRingVID>& routePrevhopVids = itr_bool.first->second.prevhopVids;
            unsigned int oldestPrevhopIndex = setRoutePrevhopVidsFromMessage(routePrevhopVids, replyOutgoing->getPrevhopVidsForUpdate(), replyOutgoing->getOldestPrevhopIndex());
            replyOutgoing->setOldestPrevhopIndex(oldestPrevhopIndex);

            EV_INFO << "Sending SetupReply to newnode = " << newnode << ", src = " << srcVid << ", pathid = " << newPathid << ", trace = " << replyOutgoing->getTrace() << ", nexthop: " << nextHopVid << endl;
            sendCreatedSetupReply(replyOutgoing, /*outGateIndex=*/psetTable.getPneiGateIndex(nextHopVid), /*computeChunkLength=*/true);
            pktForwarded = true;
        }
    }
}

int Vlr::computeSetupFailByteLength(SetupFailInt* msg) const
{
    // unsigned int src, newnode, proxy;
    int chunkByteLength = VLRRINGVID_BYTELEN * 3;
    // unsigned int srcVset[]
    chunkByteLength += msg->getSrcVsetArraySize() * VLRRINGVID_BYTELEN;
    // // bool notifyVsetOnly
    // chunkByteLength += 1;
    // std::vector<unsigned int> trace
    chunkByteLength += msg->getTrace().size() * VLRRINGVID_BYTELEN;

    // for VlrIntUniPacket
    chunkByteLength += getVlrUniPacketByteLength();

    return chunkByteLength;
}

SetupFailInt* Vlr::createSetupFail(const VlrRingVID& newnode, const VlrRingVID& proxy)
{
    SetupFailInt *msg = new SetupFailInt(/*name=*/"SetupFail");
    msg->setNewnode(newnode);      // vidByteLength
    msg->setProxy(proxy);
    msg->setSrc(vid);
    // msg->setNotifyVsetOnly(false);

    setupPacketSrcVset(msg);
    msg->setMessageId(++allSendMessageId);
    msg->setHopcount(0);         // number of nodes this message traversed, including the starting and ending nodes

    initializeVlrOption(msg->getVlrOptionForUpdate());

    return msg;
}

// if computeChunkLength = true, compute chunk length because chunk was just created with createSetupFail() or modified after dupShared() from another chunk
// else, no need to compute chunk length because chunk was dupShared() (not modified) from another chunk that has chunkLength set
void Vlr::sendCreatedSetupFail(SetupFailInt *msg, const int& outGateIndex, bool computeChunkLength/*=true*/, double delay/*=0*/)
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

void Vlr::processSetupFail(SetupFailInt *msgIncoming, bool& pktForwarded)
{
    EV_DEBUG << "Received SetupFail" << endl;

    VlrRingVID msgPrevHopVid = msgIncoming->getVlrOption().getPrevHopVid();
    if (msgPrevHopVid == VLRRINGVID_NULL)
        throw cRuntimeError("Received SetupFail with vlrOption.prevHopVid = null");
    
    bool withTrace = (msgIncoming->getTrace().size() > 0);
    VlrRingVID newnode = msgIncoming->getNewnode();
    VlrRingVID srcVid = msgIncoming->getSrc();        
    VlrRingVID proxy = msgIncoming->getProxy();       // can also get proxy using vlrOptionIn->getDstVid()
    EV_INFO << "Processing SetupFail: proxy = " << proxy << ", newnode = " << newnode << ", src = " << srcVid << ", prevhop: " << msgPrevHopVid << endl;

    if (recordStatsToFile && recordReceivedMsg) {   // record received message
        recordMessageRecord(/*action=*/2, /*src=*/srcVid, /*dst=*/newnode, "SetupFail", /*msgId=*/msgIncoming->getMessageId(), /*hopcount=*/msgIncoming->getHopcount()+1, /*chunkByteLength=*/msgIncoming->getByteLength());  // unimportant params (msgId, hopcount)
    }
    bool pktForMe = false;      // set to true if this msg is directed to me or I processed it as its dst
    bool pktRecorded = false;      // set to true if this msg is recorded with recordMessageRecord()

    if (representativeFixed && representative.heardfromvid == VLRRINGVID_NULL) {        // if I don't have a valid rep yet, I won't process or forward this message
        EV_WARN << "No valid rep heard: " << representative << ", cannot accept overlay message" << endl;
        
        if (recordStatsToFile && recordDroppedMsg && !pktRecorded) {   // record dropped message
            recordMessageRecord(/*action=*/4, /*src=*/srcVid, /*dst=*/newnode, "SetupFail", /*msgId=*/msgIncoming->getMessageId(), /*hopcount=*/msgIncoming->getHopcount()+1, /*chunkByteLength=*/msgIncoming->getByteLength(), /*infoStr=*/"no valid rep");
            pktRecorded = true;
        }
        return;
    }
    // checked I have a valid rep
    else if (newnode == vid) {
        // bool notifyVsetOnly = msgIncoming->getNotifyVsetOnly();
        pktForMe = true;
        EV_INFO << "Handling setupFail to me (processing oVset): src = " << srcVid << ", newnode = " << newnode << ", proxy = " << proxy << ", srcVset = [";
        for (size_t i = 0; i < msgIncoming->getSrcVsetArraySize(); i++)
            EV_INFO << msgIncoming->getSrcVset(i) << " ";
        EV_INFO << "]" << endl;

        bool srcInPending = false;
        // if (!notifyVsetOnly) {  // if notifyVsetOnly=true, don't cancel WaitSetupReqIntTimer for srcVid in pendingVset
        // cancel WaitSetupReqIntTimer for srcVid in pendingVset (received this setupFail bc I've sent setupReq to srcVid, so srcVid (and its WaitSetupReqIntTimer) should be in my pendingVset, unless it's crowded out by other nodes closer to me)
        auto itr = pendingVset.find(srcVid);
        if (itr != pendingVset.end() && itr->second.setupReqTimer) { // if node exists in pendingVset and WaitSetupReqIntTimer != nullptr
            srcInPending = true;
            int retryCount = itr->second.setupReqTimer->getRetryCount();
            EV_DETAIL << "Canceling wait setupReq timer to src " << srcVid << ", retryCount: " << retryCount << endl;
            
            itr->second.lastHeard = simTime();  // update lastHeard time

            // NOTE same condition as in processWaitSetupReqTimer()
            if (retryCount < setupReqRetryLimit /*|| (retryCount >= (VLRSETUPREQ_THRESHOLD - setupReqRetryLimit) && retryCount < VLRSETUPREQ_THRESHOLD)*/) {
                // cancelEvent(itr->second.setupReqTimer);
                cancelAndDelete(itr->second.setupReqTimer);
                itr->second.setupReqTimer = nullptr;
            }
            // else if (retryCount >= VLRSETUPREQ_THRESHOLD && (retryCount - VLRSETUPREQ_THRESHOLD) < setupReqTraceRetryLimit) {
            //     // cancelEvent(itr->second.setupReqTimer);
            //     cancelAndDelete(itr->second.setupReqTimer);
            //     itr->second.setupReqTimer = nullptr;
            // }
            else {
                EV_WARN << "SetupReq attempts for node " << srcVid << " reached setupReqRetryLimit=" << setupReqRetryLimit << " limit, deleting from pendingVset." << endl;
                pendingVsetErase(srcVid);    // this setupReqTimer will be cancelAndDelete()
            }
        } // else  // src not in pendingVset
        
        if (!srcInPending)
            pendingVsetAdd(srcVid);

        // process srcVset, also add srcVid to pendingVset if relevant (though it just refused me as vnei)
        processOtherVset(msgIncoming, /*srcVid=*/srcVid);

    }
    else {  // this SetupFail isn't destined for me
        // auto replyOutgoing = staticPtrCast<SetupFailInt>(msgIncoming->dupShared());
        forwardSetupFail(msgIncoming, pktForwarded, msgPrevHopVid, withTrace);

        if (checkOverHeardTraces) {
            // see if src of msg belongs to pendingVset
            pendingVsetAdd(srcVid, /*heardFrom=*/VLRRINGVID_NULL, /*addToEmpty=*/false);
            // assuming msg is always sent as a response of req, see if src of req belongs to pendingVset
            pendingVsetAdd(newnode, /*heardFrom=*/VLRRINGVID_NULL, /*addToEmpty=*/false);
            if (proxy != VLRRINGVID_NULL && proxy != newnode)
                pendingVsetAdd(proxy, /*heardFrom=*/VLRRINGVID_NULL, /*addToEmpty=*/false);
        }
    }

    if (recordStatsToFile && recordDroppedMsg && !pktRecorded) {   // record message
        if (pktForMe)
            recordMessageRecord(/*action=*/1, /*src=*/srcVid, /*dst=*/newnode, "SetupFail", /*msgId=*/msgIncoming->getMessageId(), /*hopcount=*/msgIncoming->getHopcount()+1, /*chunkByteLength=*/msgIncoming->getByteLength());
        else if (!pktForwarded)
            recordMessageRecord(/*action=*/4, /*src=*/srcVid, /*dst=*/newnode, "SetupFail", /*msgId=*/msgIncoming->getMessageId(), /*hopcount=*/msgIncoming->getHopcount()+1, /*chunkByteLength=*/msgIncoming->getByteLength());
    }
}

void Vlr::forwardSetupFail(SetupFailInt *replyOutgoing, bool& pktForwarded, VlrRingVID msgPrevHopVid, bool withTrace)
{
    VlrRingVID newnode = replyOutgoing->getNewnode();
    VlrRingVID srcVid = replyOutgoing->getSrc();        
    VlrRingVID proxy = replyOutgoing->getProxy();       // can also get proxy using vlrOptionIn->getDstVid()

    if (!withTrace) {
        // forward this SetupFail with findNextHopForSetupReply()
        VlrIntOption& vlrOptionOut = replyOutgoing->getVlrOptionForUpdate();
        VlrIntOption vlrOptionInCopy = vlrOptionOut;
        VlrRingVID nextHopVid = findNextHopForSetupReply(vlrOptionOut, /*prevHopVid=*/msgPrevHopVid, /*newnode=*/newnode, /*allowTempRoute=*/true);
        if (nextHopVid == VLRRINGVID_NULL) {
            // delete vlrOptionOut;
            EV_WARN << "No next hop found for SetupFail received at me = " << vid << ", dropping packet: newnode = " << newnode << ", src = " << srcVid << ", proxy = " << proxy << endl;
            if (recordStatsToFile && !recordDroppedMsg) {   // record dropped message
                std::ostringstream s;
                s << "no next hop: proxy=" << proxy << " vlrOptionIn=" << printVlrOptionToString(vlrOptionInCopy) << " vlrOptionOut=" << printVlrOptionToString(vlrOptionOut) << " vset=" << printVsetToString() << " pendingVset=" << printPendingVsetToString();            
                recordMessageRecord(/*action=*/4, /*src=*/srcVid, /*dst=*/newnode, "SetupFail", /*msgId=*/replyOutgoing->getMessageId(), /*hopcount=*/replyOutgoing->getHopcount()+1, /*chunkByteLength=*/replyOutgoing->getByteLength(), /*infoStr=*/s.str().c_str());   // unimportant params (msgId, hopcount)
            }
            // if (displayBubbles && hasGUI())
            //     getContainingNode(host)->bubble("No next hop found for SetupFail");
        } else {    // nexthop found to forward SetupFail
            sendCreatedSetupFail(replyOutgoing, /*outGateIndex=*/psetTable.getPneiGateIndex(nextHopVid), /*computeChunkLength=*/false);
            pktForwarded = true;
        }
    } else {    // forward this SetupFail with trace
        unsigned int nextHopIndex = getNextHopIndexInTrace(replyOutgoing->getTraceForUpdate());    // replyOutgoing->getTraceForUpdate(): [start node, .., parent node, (me, skipped nodes)]
        if (nextHopIndex >= replyOutgoing->getTrace().size()) {
            EV_WARN << "No next hop found for SetupFail with trace received at me = " << vid << " because no node in trace is a LINKED pnei, dropping packet: newnode = " << newnode << ", src = " << srcVid << ", trace = " << replyOutgoing->getTrace() << endl;
        } else {    // nexthop found for SetupReply with trace
            VlrRingVID nextHopVid = replyOutgoing->getTrace().at(nextHopIndex);
            EV_INFO << "Sending SetupFail to newnode = " << newnode << ", src = " << srcVid << ", trace = " << replyOutgoing->getTrace() << ", nexthop: " << nextHopVid << endl;
            sendCreatedSetupFail(replyOutgoing, /*outGateIndex=*/psetTable.getPneiGateIndex(nextHopVid), /*computeChunkLength=*/true);
            pktForwarded = true;
        }
    }
}

int Vlr::computeAddRouteByteLength(AddRouteInt* msg) const
{
    // unsigned int dst, src;
    int chunkByteLength = VLRRINGVID_BYTELEN * 2;
    // VlrPathID pathid
    chunkByteLength += VLRPATHID_BYTELEN;
    // unsigned int srcVset[]
    chunkByteLength += msg->getSrcVsetArraySize() * VLRRINGVID_BYTELEN;
    // std::vector<unsigned int> trace
    chunkByteLength += msg->getTrace().size() * VLRRINGVID_BYTELEN;
    // std::vector<unsigned int> prevhopVids
    // unsigned int oldestPrevhopIndex
    chunkByteLength += (routePrevhopVidsSize-1) * VLRRINGVID_BYTELEN + 2;   // routePrevhopVidsSize-1 bc size of prevHopVid included in vlrOption

    // for VlrIntUniPacket
    chunkByteLength += getVlrUniPacketByteLength();

    return chunkByteLength;
}

AddRouteInt* Vlr::createAddRoute(const VlrRingVID& dst, const VlrPathID& pathid)
{
    AddRouteInt *msg = new AddRouteInt(/*name=*/"AddRoute");
    msg->setDst(dst);     // vidByteLength
    msg->setSrc(vid);
    msg->setProxy(VLRRINGVID_NULL);

    msg->getPathidForUpdate() = pathid;

    msg->getPrevhopVidsForUpdate().push_back(vid);
    msg->setOldestPrevhopIndex(0);

    msg->setHopcount(0);         // number of nodes this message traversed, including the starting and ending nodes

    setupPacketSrcVset(msg);
    msg->setMessageId(++allSendMessageId);

    initializeVlrOption(msg->getVlrOptionForUpdate());

    return msg;
}

// if computeChunkLength = true, compute chunk length because chunk was just created with createSetupReply() or modified after dupShared() from another chunk
// else, no need to compute chunk length because chunk was dupShared() (not modified) from another chunk that has chunkLength set
void Vlr::sendCreatedAddRoute(AddRouteInt *msg, const int& outGateIndex, bool computeChunkLength/*=true*/, double delay/*=0*/)
{
    if (computeChunkLength)
        msg->setByteLength(computeAddRouteByteLength(msg));
    EV_DEBUG << "Sending AddRoute: src = " << msg->getSrc() << ", dst = " << msg->getDst() << endl;

    msg->getVlrOptionForUpdate().setPrevHopVid(vid);    // set packet prevHopVid to myself
    msg->setHopcount(msg->getHopcount() +1);    // increment packet hopcount
    
    // NOTE addTag should be executed after chunkLength has been set, and chunkLength shouldn't be changed before findTag/getTag

    // all multihop VLR packets (setupReq, setupReply, etc) L3 dst are set to a pnei, greedy routing at L3 in routeDatagram() isn't needed, but we do greedy routing and deal with VlrOption at L4 (in processSetupReq() for example) 
    // udpPacket->addTagIfAbsent<VlrIntOptionReq>()->setVlrOption(vlrOption);      // VlrOption to be set in IP header in datagramLocalOutHook()
    
    sendCreatedPacket(msg, /*unicast=*/true, /*outGateIndex=*/outGateIndex, /*delay=*/delay, /*checkFail=*/true);
}

void Vlr::processAddRoute(AddRouteInt *msgIncoming, bool& pktForwarded)
{
    EV_DEBUG << "Received AddRoute" << endl;
    
    // // VlrIntOption *vlrOptionIn = nullptr;
    // VlrIntOption& vlrOptionIn = msgIncoming->getVlrOptionForUpdate();
    VlrRingVID msgPrevHopVid = msgIncoming->getVlrOption().getPrevHopVid();
    if (msgPrevHopVid == VLRRINGVID_NULL)
        throw cRuntimeError("Received AddRoute with vlrOption.prevHopVid = null");

    bool withTrace = (msgIncoming->getTrace().size() > 0);
    VlrRingVID dstVid = msgIncoming->getDst();   // can also get proxy using vlrOptionIn->getDstVid()
    VlrRingVID srcVid = msgIncoming->getSrc();        
    const VlrPathID& newPathid = msgIncoming->getPathid();
    unsigned int msgHopCount = msgIncoming->getHopcount() +1;

    EV_INFO << "Processing AddRoute: dst = " << dstVid << ", src = " << srcVid << ", pathid: " << newPathid << ", hopcount: " << msgHopCount << ", prevhop: " << msgPrevHopVid << endl;
    
    if (recordStatsToFile && recordReceivedMsg) {   // record received message
        recordMessageRecord(/*action=*/2, /*src=*/srcVid, /*dst=*/dstVid, "AddRoute", /*msgId=*/msgIncoming->getMessageId(), /*hopcount=*/msgHopCount, /*chunkByteLength=*/msgIncoming->getByteLength());    // unimportant params (msgId)
    }
    bool pktForMe = false;      // set to true if this msg is directed to me or I processed it as its dst
    bool pktRecorded = false;      // set to true if this msg is recorded with recordMessageRecord()

    auto vrouteItr = vlrRoutingTable.vlrRoutesMap.find(newPathid);
    if (vrouteItr != vlrRoutingTable.vlrRoutesMap.end()) {  // newPathid already in vlrRoutingTable
        EV_WARN << "The new pathid " << newPathid << " of AddRoute is already in my routing table, tearing down the new path" << endl;
        // pathid duplicate, something is wrong, can be a loop, if indeed a loop, I send Teardown to prevHopAddr and Teardown will reach me again to remove the existing erroneous vlrRoutingTable record of newPathid
        // tear down newPathid, i.e. send Teardown to prevHopAddr
        TeardownInt* teardownOut = createTeardownOnePathid(newPathid, /*addSrcVset=*/false, /*rebuild=*/true);
        sendCreatedTeardown(teardownOut, /*nextHopPnei=*/msgPrevHopVid);

        if (recordStatsToFile) {   // record sent message
            recordMessageRecord(/*action=*/0, /*src=*/vid, /*dst=*/VLRRINGVID_NULL, "Teardown", /*msgId=*/0, /*hopcount=*/0, /*chunkByteLength=*/0, /*infoStr=*/"addRoute: new path already in routing table");   // unused params (msgId, hopcount, chunkByteLength) are just assigned to 0
        }
    }
    else {    // checked newPathid not in vlrRoutingTable
        // VlrRingVID prevHopVid = getVidFromAddressInRegistry(prevHopAddr);    // map prevHopAddr L3Address to VlrRingVID
        if (!psetTable.pneiIsLinked(msgPrevHopVid)) {        // if prevHopAddr isn't a LINKED pnei
            EV_WARN << "Previous hop " << msgPrevHopVid << " of AddRoute is not a LINKED pnei, tearing down pathid " << newPathid << endl;
            // tear down newPathid, i.e. send Teardown to prevHopAddr
            TeardownInt* teardownOut = createTeardownOnePathid(newPathid, /*addSrcVset=*/false, /*rebuild=*/true);
            sendCreatedTeardown(teardownOut, /*nextHopPnei=*/msgPrevHopVid);

            if (recordStatsToFile) {   // record sent message
                recordMessageRecord(/*action=*/0, /*src=*/vid, /*dst=*/VLRRINGVID_NULL, "Teardown", /*msgId=*/0, /*hopcount=*/0, /*chunkByteLength=*/0, /*infoStr=*/"addRoute: previous hop not a LINKED pnei");   // unused params (msgId, hopcount, chunkByteLength) are just assigned to 0
            }
        }
        // checked AddRoute received from a LINKED pnei
        else if (representativeFixed && representative.heardfromvid == VLRRINGVID_NULL) {        // if I don't have a valid rep yet, I won't process or forward this message
            EV_WARN << "No valid rep heard: " << representative << ", cannot accept overlay message, tearing down pathid " << newPathid << endl;
            // tear down newPathid, i.e. send Teardown to prevHopAddr
            TeardownInt* teardownOut = createTeardownOnePathid(newPathid, /*addSrcVset=*/false, /*rebuild=*/true);
            sendCreatedTeardown(teardownOut, /*nextHopPnei=*/msgPrevHopVid);

            if (recordStatsToFile) {   // record sent message
                recordMessageRecord(/*action=*/0, /*src=*/vid, /*dst=*/VLRRINGVID_NULL, "Teardown", /*msgId=*/0, /*hopcount=*/0, /*chunkByteLength=*/0, /*infoStr=*/"addRoute: no rep");   // unused params (msgId, hopcount, chunkByteLength) are just assigned to 0
            }
        }
        // checked I have a valid rep
        else if (dstVid == vid) {      // this AddRoute is destined for me
            pktForMe = true;
            EV_INFO << "Handling AddRoute to me: src = " << srcVid << ", dst = " << dstVid << ", pathid = " << newPathid << ", srcVset = [";
            for (size_t i = 0; i < msgIncoming->getSrcVsetArraySize(); i++)
                EV_INFO << msgIncoming->getSrcVset(i) << " ";
            EV_INFO << "]" << endl;

            if (recordStatsToFile) {   // record received AddRoute for me that indeed adds newPathid to vlrRoutingTable
                recordMessageRecord(/*action=*/1, /*src=*/srcVid, /*dst=*/vid, "AddRoute", /*msgId=*/newPathid, /*hopcount=*/msgHopCount, /*chunkByteLength=*/msgIncoming->getByteLength());    // unimportant params (msgId)
                pktRecorded = true;
            }
            // add non-essential vroute to vlrRoutingTable
            
            auto itr_bool = vlrRoutingTable.addRoute(newPathid, srcVid, dstVid, /*prevhopVid=*/msgPrevHopVid, /*nexthopVid=*/VLRRINGVID_NULL, /*isVsetRoute=*/false);
            itr_bool.first->second.hopcount = msgHopCount;
            std::vector<VlrRingVID>& routePrevhopVids = itr_bool.first->second.prevhopVids;
            setRoutePrevhopVids(routePrevhopVids, msgIncoming->getPrevhopVids(), msgIncoming->getOldestPrevhopIndex());

            nonEssRoutes[newPathid] = simTime() + nonEssRouteExpiration;

            // emit(routeAddedSignal, msgHopCount);
            // // vrouteLenVector.record(msgHopCount);
            // // vrouteLenPSquare.collect(msgHopCount);

            // NOTE src sent AddRoute to me bc it doesn't think I'm a vnei, otherwise it would have sent me setupReq/setupReply
            // cancel WaitSetupReqIntTimer for srcVid if it's in pendingVset
            auto itr = pendingVset.find(srcVid);
            if (itr != pendingVset.end()) { // if node exists in pendingVset
                itr->second.lastHeard = simTime();  // update lastHeard time

                if (itr->second.setupReqTimer /*&& itr->second.setupReqTimer->isScheduled()*/) {      // if WaitSetupReqIntTimer != nullptr and setupReq timer is scheduled (i.e. I've sent a setupReq to src and waiting for reply)
                    int retryCount = itr->second.setupReqTimer->getRetryCount();
                    EV_DETAIL << "Canceling wait setupReq timer to src " << srcVid << ", retryCount: " << retryCount << endl;

                    // NOTE same condition as in processWaitSetupReqTimer()
                    if (retryCount < setupReqRetryLimit /*|| (retryCount >= (VLRSETUPREQ_THRESHOLD - setupReqRetryLimit) && retryCount < VLRSETUPREQ_THRESHOLD)*/) {
                        // cancelEvent(itr->second.setupReqTimer);
                        cancelAndDelete(itr->second.setupReqTimer);
                        itr->second.setupReqTimer = nullptr;
                    }
                    // else if (retryCount >= VLRSETUPREQ_THRESHOLD && (retryCount - VLRSETUPREQ_THRESHOLD) < setupReqTraceRetryLimit) {
                    //     // cancelEvent(itr->second.setupReqTimer);
                    //     cancelAndDelete(itr->second.setupReqTimer);
                    //     itr->second.setupReqTimer = nullptr;
                    // }
                    else {
                        EV_WARN << "SetupReq attempts for node " << srcVid << " reached setupReqRetryLimit=" << setupReqRetryLimit << " limit, deleting from pendingVset." << endl;
                        pendingVsetErase(srcVid);    // this setupReqTimer will be cancelAndDelete()
                    }
                }
            } else  // src not in pendingVset
                pendingVsetAdd(srcVid);

            // process srcVset in received AddRoute
            processOtherVset(msgIncoming, /*srcVid=*/srcVid);
        }
        else {  // this AddRoute isn't destined for me
            // auto msgOutgoing = staticPtrCast<AddRouteInt>(msgIncoming->dupShared());
            forwardAddRoute(msgIncoming, pktForwarded, newPathid, msgPrevHopVid, withTrace);

            if (checkOverHeardTraces) {
                // see if src of msg belongs to pendingVset
                pendingVsetAdd(srcVid, /*heardFrom=*/VLRRINGVID_NULL, /*addToEmpty=*/false);
                // assuming msg is always sent as a response of req, see if src of req belongs to pendingVset
                pendingVsetAdd(dstVid, /*heardFrom=*/VLRRINGVID_NULL, /*addToEmpty=*/false);
            }
        }
    }

    if (recordStatsToFile && recordDroppedMsg && !pktRecorded) {   // record message
        if (pktForMe)
            recordMessageRecord(/*action=*/1, /*src=*/srcVid, /*dst=*/dstVid, "AddRoute", /*msgId=*/newPathid, /*hopcount=*/msgHopCount, /*chunkByteLength=*/msgIncoming->getByteLength());
        else if (!pktForwarded)
            recordMessageRecord(/*action=*/4, /*src=*/srcVid, /*dst=*/dstVid, "AddRoute", /*msgId=*/newPathid, /*hopcount=*/msgHopCount, /*chunkByteLength=*/msgIncoming->getByteLength());
    }
}

// prevHopAddr.isUnspecified() if forwardAddRoute() is called from processFailedPacket() and I am the src who initiated this addRoute
void Vlr::forwardAddRoute(AddRouteInt *msgOutgoing, bool& pktForwarded, const VlrPathID& newPathid, VlrRingVID msgPrevHopVid, bool withTrace)
{
    VlrRingVID dstVid = msgOutgoing->getDst();
    VlrRingVID srcVid = msgOutgoing->getSrc();        
    VlrRingVID proxy = msgOutgoing->getProxy();       // could be null
    // unsigned int msgHopCount = msgOutgoing->getHopcount() +1;

    if (!withTrace) {
        // forward this AddRoute with findNextHop()
        VlrIntOption& vlrOptionOut = msgOutgoing->getVlrOptionForUpdate();
        VlrRingVID nextHopVid = VLRRINGVID_NULL;
        // if (proxy == VLRRINGVID_NULL)
        //     nextHopVid = findNextHop(vlrOptionOut, /*prevHopVid=*/msgPrevHopVid, /*excludeVid=*/VLRRINGVID_NULL, /*allowTempRoute=*/false);
        // else    // proxy should be vlrOptionOut->getDstVid()
        //     nextHopVid = findNextHopForSetupReply(vlrOptionOut, /*prevHopVid=*/msgPrevHopVid, /*newnode=*/dstVid);
        if (proxy == vid) {  // proxy is reached
            msgOutgoing->setProxy(VLRRINGVID_NULL);
            if (vlrOptionOut.getDstVid() != dstVid) {
                vlrOptionOut.setDstVid(dstVid);
                vlrOptionOut.setCurrentPathid(VLRPATHID_INVALID);
                vlrOptionOut.setTempPathid(VLRPATHID_INVALID);
            }
        }
        nextHopVid = findNextHopForSetupReq(vlrOptionOut, /*prevHopVid=*/msgPrevHopVid, /*dstVid=*/dstVid, /*newnode=*/VLRRINGVID_NULL, /*allowTempRoute=*/false);
        
        if (nextHopVid == VLRRINGVID_NULL) {
            // delete vlrOptionOut;
            EV_WARN << "No next hop found for AddRoute received at me = " << vid << ", tearing down vroute: pathid = " << newPathid << ", dst = " << dstVid << ", src = " << srcVid << endl;
            // tear down newPathid, i.e. send Teardown to prevHopAddr
            // if (prevHopAddr.isUnspecified()) {
            TeardownInt* teardownOut = createTeardownOnePathid(newPathid, /*addSrcVset=*/false, /*rebuild=*/true);
            sendCreatedTeardown(teardownOut, /*nextHopPnei=*/msgPrevHopVid);

            if (recordStatsToFile) {   // record sent message
                recordMessageRecord(/*action=*/0, /*src=*/vid, /*dst=*/VLRRINGVID_NULL, "Teardown", /*msgId=*/0, /*hopcount=*/0, /*chunkByteLength=*/0, /*infoStr=*/"addRoute: no next hop found w/o trace");   // unused params (msgId, hopcount, chunkByteLength) are just assigned to 0
            }
            
            // if (displayBubbles && hasGUI())
            //     getContainingNode(host)->bubble("No next hop found for AddRoute");
        } else {    // nexthop found for AddRoute
            // we've checked newPathid not in vlrRoutingTable
            auto itr_bool = vlrRoutingTable.addRoute(newPathid, srcVid, dstVid, /*prevhopVid=*/msgPrevHopVid, /*nexthopVid=*/nextHopVid, /*isVsetRoute=*/false);
            std::vector<VlrRingVID>& routePrevhopVids = itr_bool.first->second.prevhopVids;
            unsigned int oldestPrevhopIndex = setRoutePrevhopVidsFromMessage(routePrevhopVids, msgOutgoing->getPrevhopVidsForUpdate(), msgOutgoing->getOldestPrevhopIndex());
            msgOutgoing->setOldestPrevhopIndex(oldestPrevhopIndex);
            
            EV_INFO << "Added new non-essential vroute: newPathid = " << newPathid << " " << vlrRoutingTable.vlrRoutesMap.at(newPathid) << endl;
            // msgOutgoing->setHopcount(msgHopCount);
            sendCreatedAddRoute(msgOutgoing, /*outGateIndex=*/psetTable.getPneiGateIndex(nextHopVid), /*computeChunkLength=*/false);
            pktForwarded = true;
        }
    } else { // forward this AddRoute with trace
        // msgOutgoing->getTraceForUpdate(): [start node, .., parent node, (me, skipped nodes)]
        unsigned int nextHopIndex = getNextHopIndexInTrace(msgOutgoing->getTraceForUpdate());
        // msgOutgoing->getTrace(): [start node, .., parent node]
        if (nextHopIndex >= msgOutgoing->getTrace().size()) {
            EV_WARN << "No next hop found for AddRoute with trace received at me = " << vid << " because no node in trace is a LINKED pnei, tearing down vroute: pathid = " << newPathid << ", dst = " << dstVid << ", src = " << srcVid << ", trace = " << msgOutgoing->getTrace() << endl;
            // tear down newPathid, i.e. send Teardown to prevHopAddr
            // if (prevHopAddr.isUnspecified()) {
            TeardownInt* teardownOut = createTeardownOnePathid(newPathid, /*addSrcVset=*/false, /*rebuild=*/true);
            sendCreatedTeardown(teardownOut, /*nextHopPnei=*/msgPrevHopVid);

            if (recordStatsToFile) {   // record sent message
                recordMessageRecord(/*action=*/0, /*src=*/vid, /*dst=*/VLRRINGVID_NULL, "Teardown", /*msgId=*/0, /*hopcount=*/0, /*chunkByteLength=*/0, /*infoStr=*/"addRoute: no next hop found in trace");   // unused params (msgId, hopcount, chunkByteLength) are just assigned to 0
            }

        } else {    // nexthop found for AddRoute with trace
            // msgOutgoing->setHopcount(msgHopCount);
            VlrRingVID nextHopVid = msgOutgoing->getTrace().at(nextHopIndex);

            // we've checked newPathid not in vlrRoutingTable
            auto itr_bool = vlrRoutingTable.addRoute(newPathid, srcVid, dstVid, /*prevhopVid=*/msgPrevHopVid, /*nexthopVid=*/nextHopVid, /*isVsetRoute=*/false);
            std::vector<VlrRingVID>& routePrevhopVids = itr_bool.first->second.prevhopVids;
            unsigned int oldestPrevhopIndex = setRoutePrevhopVidsFromMessage(routePrevhopVids, msgOutgoing->getPrevhopVidsForUpdate(), msgOutgoing->getOldestPrevhopIndex());
            msgOutgoing->setOldestPrevhopIndex(oldestPrevhopIndex);

            EV_INFO << "Sending AddRoute to dst = " << dstVid << ", src = " << srcVid << ", pathid = " << newPathid << ", trace = " << msgOutgoing->getTrace() << ", nexthop: " << nextHopVid << endl;
            sendCreatedAddRoute(msgOutgoing, /*outGateIndex=*/psetTable.getPneiGateIndex(nextHopVid), /*computeChunkLength=*/true);
            pktForwarded = true;
        }
    }
}

// create TeardownInt with one pathid and set its chunk length
TeardownInt* Vlr::createTeardownOnePathid(const VlrPathID& pathid, bool addSrcVset, bool rebuild)
{
    std::vector<VlrPathID> pathids {pathid};
    auto msg = createTeardown(pathids, addSrcVset, rebuild, /*dismantled=*/false);
    return msg;
}

// create TeardownInt with multiple pathids and set its chunk length
TeardownInt* Vlr::createTeardown(const std::vector<VlrPathID>& pathids, bool addSrcVset, bool rebuild, bool dismantled)
{
    TeardownInt *msg = new TeardownInt(/*name=*/"Teardown");
    msg->setSrc(vid);          // vidByteLength
    msg->setPathidsArraySize(pathids.size());
    msg->setRebuild(rebuild);
    msg->setDismantled(dismantled);
    unsigned int k = 0;
    for (const auto& pathid : pathids)
        msg->setPathids(k++, pathid);      // VLRPATHID_BYTELEN
    // VlrPathID pathids[]
    // unsigned int src
    // bool rebuild;
    // bool dismantled;
    int chunkByteLength = k * VLRPATHID_BYTELEN + VLRRINGVID_BYTELEN + 1;

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

// set pathids in TeardownInt and recompuate its chunk length
void Vlr::setTeardownPathids(TeardownInt *teardown, const std::vector<VlrPathID>& pathids) const
{
    teardown->setPathidsArraySize(pathids.size());
    unsigned int k = 0;
    for (const auto& pathid : pathids)
        teardown->setPathids(k++, pathid);      // VLRPATHID_BYTELEN
    // VlrPathID pathids[]
    // unsigned int src
    // bool rebuild;
    // bool dismantled;
    int chunkByteLength = k * VLRPATHID_BYTELEN + VLRRINGVID_BYTELEN + 1;
    // unsigned int srcVset[];
    chunkByteLength += teardown->getSrcVsetArraySize() * VLRRINGVID_BYTELEN;

    // for VlrIntUniPacket
    chunkByteLength += getVlrUniPacketByteLength();
    
    teardown->setByteLength(chunkByteLength);
}

// nextHopIsUnavailable: 00 (available) or 11 (endpoint unavailable but next hop linked) or 01 (unavailable) or 10 (using temp route); if 01, no Teardown is sent as nextHop is unavailable, if 02, Teardown is sent on temporary route with VlrIntOption
void Vlr::sendCreatedTeardownToNextHop(TeardownInt *msg, VlrRingVID nextHopVid, char nextHopIsUnavailable)
{
    if (nextHopIsUnavailable == 0 || nextHopIsUnavailable == 3) {    // next hop is a linked pnei
        msg->getVlrOptionForUpdate().setTempPathid(VLRPATHID_INVALID);  // ensure tempPathid is null
        sendCreatedTeardown(msg, /*nextHopPnei=*/nextHopVid);
    }
    else if (nextHopIsUnavailable == 2) { // next hop is a lost pnei connected via temporary route
        // VlrRingVID lostPneiVid = getVidFromAddressInRegistry(nextHopAddr);    // map nextHopAddr L3Address to VlrRingVID
        VlrRingVID lostPneiVid = nextHopVid;
        auto lostPneiItr = lostPneis.find(lostPneiVid);
        if (lostPneiItr != lostPneis.end()) {
            // remove vroutes to tear down from lostPneis[lostPneiVid].brokenVroutes
            for (size_t k = 0; k < msg->getPathidsArraySize(); ++k) {
                const VlrPathID& oldPathid = msg->getPathids(k);
                lostPneiItr->second.brokenVroutes.erase(oldPathid);
            }
            // send Teardown along temporary route
            const VlrPathID& tempPathid = *(lostPneiItr->second.tempVlinks.begin());
            // const L3Address& nextHopAddr_new = vlrRoutingTable.getRouteNextHop(tempPathid, lostPneiVid);
            VlrRingVID nextHopVid_new = vlrRoutingTable.getRouteNextHop(tempPathid, lostPneiVid);
            VlrIntOption& vlrOption = msg->getVlrOptionForUpdate();
            initializeVlrOption(vlrOption, /*dstVid=*/VLRRINGVID_NULL);
            vlrOption.setTempPathid(tempPathid);
            vlrOption.setTempTowardVid(lostPneiVid);
            EV_INFO << "Sending Teardown to temporary route = " << tempPathid << ", nexthop: " << nextHopVid_new << endl;
            sendCreatedTeardown(msg, /*nextHopPnei=*/nextHopVid_new);

            if (lostPneiItr->second.brokenVroutes.empty()) {    // if lostPnei's brokenVroutes empty after removing pathid
                delayLostPneiTempVlinksTeardown(lostPneiItr->second.tempVlinks, /*delay=*/0);   // notify lostPnei that my brokenVroutes now empty
                cancelAndDelete(lostPneiItr->second.timer);
                lostPneis.erase(lostPneiItr);
            }
        }
        
    } // else, next hop is a lost pnei that can't be connected
}

void Vlr::sendCreatedTeardown(TeardownInt *msg, VlrRingVID nextHopPnei, double delay/*=0*/)
{
    msg->getVlrOptionForUpdate().setPrevHopVid(vid);    // set packet prevHopVid to myself
    msg->setHopcount(msg->getHopcount() +1);    // increment packet hopcount

    // Teardown may be sent to msgPrevHopVid which may not be a pnei in my pset
    int nextHopGateIndex = psetTable.findPneiGateIndex(nextHopPnei);
    if (nextHopGateIndex == -1) {
        nextHopGateIndex = psetTable.findRecvNeiGateIndex(nextHopPnei);
        if (nextHopGateIndex == -1)
            throw cRuntimeError("sendCreatedTeardown(nextHopPnei=%d) at me=%d, cannot find gateIndex associated with nextHopPnei in psetTable.vidToStateMap or psetTable.recvVidToGateIndexMap", nextHopPnei, vid);
    }
    sendCreatedPacket(msg, /*unicast=*/true, /*outGateIndex=*/nextHopGateIndex, /*delay=*/delay, /*checkFail=*/true);
}

// add oldVsetRoutes (nonEss vroutes btw me and oldVnei) to nonEssRoutes     # NOTE oldVnei is removed because of a new vnei joining, so removing oldVnei shouldn't leave vset empty
// sendNotifyVset=false when setupReq from newnode is dispatched to oldVnei
void Vlr::delayRouteTeardown(const VlrRingVID& oldVnei, const std::set<VlrPathID>& oldVsetRoutes, bool sendNotifyVset/*=true*/)
{
    // get VlrPathID btw me and oldVnei
    // VlrPathID oldPathid = vlrRoutingTable.getVsetRouteToVnei(oldVnei, vid);
    // ASSERT(oldPathid != VLRPATHID_INVALID);
    for (const VlrPathID& oldPathid : oldVsetRoutes) {
        vlrRoutingTable.vlrRoutesMap.at(oldPathid).isVsetRoute = false;
        // put in nonEssRoutes with expiration time
        nonEssRoutes[oldPathid] = simTime() + nonEssRouteExpiration;
    }

    if (sendNotifyVsetToReplacedVnei && sendNotifyVset) {     // send NotifyVset to oldVnei
        recentReplacedVneis.insert(oldVnei);
        if (!recentReplacedVneiTimer->isScheduled())
            scheduleAt(simTime(), recentReplacedVneiTimer);
    }
        
}

void Vlr::processTeardown(TeardownInt* msgIncoming, bool& pktForwarded)
{
    EV_DEBUG << "Received Teardown" << endl;
    // IP.SourceAddress: address of the node from which the packet was received
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

    if (checkOverHeardTraces)
        // see if src of msg belongs to pendingVset
        pendingVsetAdd(msgIncoming->getSrc(), /*heardFrom=*/VLRRINGVID_NULL, /*addToEmpty=*/false);

    // const auto& networkHeader = getNetworkProtocolHeader(packet);
    // VlrIntOption *vlrOptionIn = const_cast<VlrIntOption *>(findVlrOptionInNetworkDatagram(networkHeader));  // may be nullptr if no VlrOption is provided in IP header, i.e. not traversing a temporary route
    
    // if traversing the temporary route portion of the vroute to tear down
    VlrPathID tempPathid = vlrOptionIn.getTempPathid();
    if (tempPathid != VLRPATHID_INVALID) {
        auto tempPathItr = vlrRoutingTable.vlrRoutesMap.find(tempPathid);
        if (tempPathItr != vlrRoutingTable.vlrRoutesMap.end()) {   // tempPathid is in vlrRoutingTable
            VlrRingVID tempTowardVid = vlrOptionIn.getTempTowardVid();
            if (tempTowardVid == vid) {     // reached the end of temporary route portion
                VlrRingVID otherEnd = (vid == tempPathItr->second.fromVid) ? tempPathItr->second.toVid : tempPathItr->second.fromVid;
                auto lostPneiItr = lostPneis.find(otherEnd);
                if (lostPneiItr != lostPneis.end()) {
                    msgPrevHopVid = lostPneiItr->first;  // as if this Teardown is received directly from lost pnei
                    
                    // received Teardown for oldPathid over temporary route, remove oldPathid from brokenVroutes as it will be removed from vlrRoutingTable
                    for (size_t k = 0; k < numOfPathids; ++k) {
                        const VlrPathID& oldPathid = msgIncoming->getPathids(k);
                        lostPneiItr->second.brokenVroutes.erase(oldPathid);
                    }
                    if (lostPneiItr->second.brokenVroutes.empty()) {    // if lostPneis[lostPnei].brokenVroutes empty after removing oldPathid
                        delayLostPneiTempVlinksTeardown(lostPneiItr->second.tempVlinks, /*delay=*/0);
                        cancelAndDelete(lostPneiItr->second.timer);
                        lostPneis.erase(lostPneiItr);
                    }
                } else {    // otherEnd not in lostPneis, this means vroutes broken by otherEnd have been torn down and no longer exist in vlrRoutingTable
                    EV_WARN << "Teardown received from previous hop " << msgPrevHopVid << " via temporary route pathid " << tempPathid << ", but otherEnd = " << otherEnd << " of temporary route not found in lostPneis" << endl;
                    msgPrevHopVid = otherEnd;
                }
            } else {    // send to next hop in temporary route
                VlrRingVID nextHopVid = vlrRoutingTable.getRouteItrNextHop(tempPathItr, tempTowardVid);
                // forward Teardown with vlrOption
                // VlrIntOption* vlrOptionOut = vlrOptionIn->dup();
                // auto teardownOut = staticPtrCast<TeardownInt>(msgIncoming->dupShared());
                EV_INFO << "Sending Teardown along temporary route = " << tempPathid << ", nexthop: " << nextHopVid << endl;
                sendCreatedTeardown(msgIncoming, /*nextHopPnei=*/nextHopVid);
                pktForwarded = true;
                return;
            }
        } else {
            EV_WARN << "The temporary route pathid " << tempPathid << " of Teardown is not in my routing table, dropping the Teardown message and sending Teardown to previous hop " << msgPrevHopVid << endl;
            
            // tear down tempPathid, i.e. send Teardown to prevHopAddr
            const auto& teardownOut = createTeardownOnePathid(tempPathid, /*addSrcVset=*/false, /*rebuild=*/true);
            sendCreatedTeardown(teardownOut, /*nextHopPnei=*/msgPrevHopVid);

            if (recordStatsToFile) {   // record sent message
                recordMessageRecord(/*action=*/0, /*src=*/vid, /*dst=*/VLRRINGVID_NULL, "Teardown", /*msgId=*/0, /*hopcount=*/0, /*chunkByteLength=*/0, /*infoStr=*/"processTeardown: using temporary route but not found");   // unused params (msgId, hopcount, chunkByteLength) are just assigned to 0
                
                if (recordDroppedMsg && !pktRecorded) {     // record dropped message
                    recordMessageRecord(/*action=*/4, /*src=*/msgIncoming->getSrc(), /*dst=*/vid, "Teardown", /*msgId=*/msgIncoming->getPathids(0), /*hopcount=*/msgIncoming->getHopcount()+1, /*chunkByteLength=*/msgIncoming->getByteLength());
                    pktRecorded = true;
                }
            }
            return;
        }
    }
    

    std::map<VlrRingVID, std::pair<std::vector<VlrPathID>, char>> nextHopToPathidsMap;    // for Teardown to send, map next hop address to (pathids, next hop 2-bit isUnavailable) pair
    for (size_t k = 0; k < numOfPathids; ++k) {
        const VlrPathID& oldPathid = msgIncoming->getPathids(k);

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
                const VlrRingVID& nextHopVid = (isToNexthop) ? vrouteItr->second.nexthopVid : vrouteItr->second.prevhopVid;
                char nextHopIsUnavailable = vlrRoutingTable.getPrevNextIsUnavailable(vrouteItr->second.isUnavailable, /*getPrev=*/!isToNexthop);     // if isToNexthop, next hop is nexthop, getPrev=false
                bool isTemporaryRoute = vlrRoutingTable.getIsTemporaryRoute(vrouteItr->second.isUnavailable);
                bool isDismantledRoute = vlrRoutingTable.getIsDismantledRoute(vrouteItr->second.isUnavailable);
                EV_INFO << "The pathid " << oldPathid << " of Teardown is found in routing table, nextHopAddr: " << nextHopVid << " (specified=" << (nextHopVid!=VLRRINGVID_NULL) << ", isUnavailable=" << (int)nextHopIsUnavailable << ")" << endl;

                if (nextHopVid == VLRRINGVID_NULL) {  // I'm an endpoint of the vroute
                    pktForMe = true;
                    const VlrRingVID& otherEnd = (isToNexthop) ? vrouteItr->second.fromVid : vrouteItr->second.toVid;
                    bool rebuildTemp = (msgIncoming->getSrc() == otherEnd && msgIncoming->getRebuild() == false) ? false : true;    // don't rebuild route if otherEnd initiated this Teardown with rebuild=false
                    
                    processTeardownAtEndpoint(oldPathid, otherEnd, msgIncoming, rebuildTemp);

                    vlrRoutingTable.removeRouteByPathID(oldPathid);
                    
                }
                else {  // I'm not an endpoint of this vroute
                    bool removeFromRoutingTable = (isTemporaryRoute || !keepDismantledRoute || !msgIncoming->getDismantled() || isDismantledRoute);

                    if (nextHopIsUnavailable != 1) {
                        bool forwardTeardown = true;
                        if (isDismantledRoute) {
                            dismantledRoutes.erase(oldPathid);
                            if (nextHopIsUnavailable == 2)  // next hop is lostPnei and vroute is dismantled, shouldn't happen but if happens, no need to forward Teardown bc lostPnei will tear down dismantled vroutes with patched prev/next hop
                                forwardTeardown = false;
                            else {  // if next hop is the endpoint, no need to forward Teardown bc endpoint doesn't keep this dismantled vroute
                                VlrRingVID towardEnd = (isToNexthop) ? vrouteItr->second.toVid : vrouteItr->second.fromVid;
                                // auto pneiItr = psetTable.vidToStateMap.find(towardEnd);
                                // if (pneiItr != psetTable.vidToStateMap.end() && pneiItr->second.address == nextHopAddr) {
                                if (towardEnd == nextHopVid) {
                                    EV_DETAIL << "Teardown of dismantled vroute (pathid = " << oldPathid << ") won't be sent to next hop " << nextHopVid << " which is an endpoint node " << towardEnd << endl;
                                    forwardTeardown = false;
                                }
                            }
                        }
                        if (forwardTeardown) {
                            auto nextHopItr = nextHopToPathidsMap.find(nextHopVid);
                            if (nextHopItr == nextHopToPathidsMap.end()) 
                                nextHopToPathidsMap[nextHopVid] = {{oldPathid}, nextHopIsUnavailable};
                            else
                                nextHopItr->second.first.push_back(oldPathid);
                        }
                    } // NOTE if next hop is unavailable, this vroute can be regular or dismantled
                    else if (!isDismantledRoute) {  // next hop unavailable and regular vroute, remove this vroute from lostPneis.brokenVroutes
                        removeRouteFromLostPneiBrokenVroutes(oldPathid, /*lostPneiVid=*/nextHopVid);
                    } else {   // next hop unavailable and vroute is dismantled
                        dismantledRoutes.erase(oldPathid);
                    }
                    
                    if (removeFromRoutingTable) {
                        EV_DETAIL << "Pathid " << oldPathid << " is removed from routing table, isTemporaryRoute=" << isTemporaryRoute << ", isDismantledRoute=" << isDismantledRoute << ", keepDismantledRoute=" << keepDismantledRoute << endl;
                        vlrRoutingTable.removeRouteByPathID(oldPathid);
                    }
                    else {
                        // set previous hop blocked
                        char prevhopState_new = 3;      // prevhop is LINKED
                        char prevHopIsUnavailable = vlrRoutingTable.getPrevNextIsUnavailable(vrouteItr->second.isUnavailable, /*getPrev=*/isToNexthop);
                        if (tempPathid != VLRPATHID_INVALID && prevHopIsUnavailable == 2)  // prevhop connected via temporary route    NOTE this dismantled vroute has been removed from lostPneis[otherEnd of tempPathid].brokenVroutes when changing msgPrevHopVid
                            prevhopState_new = 1;       // prevhop isn't LINKED
                        vlrRoutingTable.setRouteItrPrevNextIsUnavailable(vrouteItr, /*setPrev=*/isToNexthop, /*value=*/prevhopState_new);
                        // set vroute as dismantled
                        vlrRoutingTable.setRouteItrIsDismantled(vrouteItr, /*setDismantled=*/true, /*setPrev=*/isToNexthop);
                        // remove endpoint connected by previous hop in endpointToRoutesMap
                        VlrRingVID lostEnd = (isToNexthop) ? vrouteItr->second.fromVid : vrouteItr->second.toVid;
                        EV_DETAIL << "Pathid " << oldPathid << " is kept in routing table and added to dismantledRoutes, endpoint " << lostEnd << " becomes unavailable" << endl;

                        vlrRoutingTable.removeRouteEndFromEndpointMap(oldPathid, lostEnd);

                        if (nextHopIsUnavailable != 0)   // should've kept this as a dismantled vroute, i.e. previous hops have probably kept this as dismantled, but next hop isn't directly linked, i.e. can't reach remaining endpoint, should notify previous hops to remove this dismantled vroute
                            dismantledRoutes.insert({oldPathid, simTime()});    // expire it now
                        else
                            dismantledRoutes.insert({oldPathid, simTime() + dismantledRouteExpiration});  // let node who initiated this Teardown to expire the dismantled vroute first
                    }
                }
            }
        }
    }
    int numSent = 0;
    for (const auto& mappair : nextHopToPathidsMap) {
        const auto& pathids = mappair.second.first;
        const auto& nextHopIsUnavailable = mappair.second.second;
        // auto teardownOut = staticPtrCast<TeardownInt>(msgIncoming->dupShared());
        if (numSent++ == 0)     // numSent == 0, haven't forwarded the Teardown I received
            pktForwarded = true;
        else    // numSent > 0, already sent the Teardown I received
            msgIncoming = msgIncoming->dup();
        if (pathids.size() < numOfPathids)      // pathids in Teardown should be modified
            setTeardownPathids(msgIncoming, pathids);   // set pathids in teardownOut and recompute chunk length
        sendCreatedTeardownToNextHop(msgIncoming, /*nextHopVid=*/mappair.first, nextHopIsUnavailable);
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
// rebuildTemp=false if oldPathid is a temporary route and Teardown is initiated by otherEnd which wants me to tear down all broken vroutes instead of rebuild this temporary route
void Vlr::processTeardownAtEndpoint(const VlrPathID& oldPathid, const VlrRingVID& otherEnd, const TeardownInt* msgIncoming, bool rebuildTemp)
{
    auto vrouteItr = vlrRoutingTable.vlrRoutesMap.find(oldPathid);
    ASSERT(vrouteItr != vlrRoutingTable.vlrRoutesMap.end());    // since we should've checked that I'm an endpoint
    bool isTemporaryRoute = vlrRoutingTable.getIsTemporaryRoute(vrouteItr->second.isUnavailable);

    if (isTemporaryRoute) {
        // NOTE for temporary route, it's not likely that the other end initiated Teardown of temporary route bc it wants to remove broken vroutes, as long as temporary route is valid, it shouldn't remove broken vroutes, unless all broken vroutes have been repaired, in this case all broken vroutes should've been removed on my end as well
        if (rebuildTemp == false) {  // otherEnd initiated Teardown of temporary route bc it has torn down all broken vroutes, in this case I should tear down all broken vroutes w/o scheduling another repairLinkReq
            auto lostPneiItr = lostPneis.find(otherEnd);
            if (lostPneiItr != lostPneis.end()) {
                cancelAndDelete(lostPneiItr->second.timer);
                lostPneiItr->second.tempVlinks.erase(oldPathid);
                // Commented out bc timer is scheduled to tear down these vroutes right away, lost pnei is assumed to be unavailable and no Teardown will be sent toward it, so setting its 2-bit isUnavailable in every broken vroute isn't necessary
                // setLostPneiBrokenVroutes(lostPneiItr, /*setBroken=*/true);       // set vroutes broken by lost pnei unavailable

                // schedule a timer to tear down all broken vroutes now
                char timerName[40] = "WaitRepairLinkIntTimer:";
                lostPneiItr->second.timer = new WaitRepairLinkIntTimer(strcat(timerName, std::to_string(otherEnd).c_str()));
                lostPneiItr->second.timer->setDst(otherEnd);
                lostPneiItr->second.timer->setRetryCount(repairLinkReqRetryLimit);
                scheduleAt(simTime(), lostPneiItr->second.timer);
                EV_WARN << "Received Teardown(rebuild=false) of temporary route " << oldPathid << " from lost pnei " << otherEnd << ", scheduling a WaitRepairLinkIntTimer now to tear down broken vroutes" << endl;
            } 
        } else {
            // set lostPneis[otherEnd].brokenVroutes unavailable, schedule or wait for repairLinkReq to repair temporary route to otherEnd
            removeEndpointOnTeardown(oldPathid, /*towardVid=*/otherEnd, /*pathIsVsetRoute=*/vrouteItr->second.isVsetRoute, /*pathIsTemporary=*/16);
        }
    }
    else {   // for regular vroute remove otherEnd from vset or nonEssRoutes
        removeEndpointOnTeardown(oldPathid, /*towardVid=*/otherEnd, /*pathIsVsetRoute=*/vrouteItr->second.isVsetRoute, /*pathIsTemporary=*/vrouteItr->second.isUnavailable);
        // otherEnd should be in pendingVset now if it was once in my vset
        if (msgIncoming != nullptr && msgIncoming->getSrc() == otherEnd && msgIncoming->getSrcVsetArraySize() > 0) {  // otherEnd initiated this Teardown and sent its srcVset, probably it no longer needs me as vnei
            processOtherVset(msgIncoming, /*srcVid=*/otherEnd);      // process srcVset
            
            // Commented out bc when no longer sending SetupReply to rep instead of SetupReq
            // lost vnei is rep, WaitSetupReqIntTimer may send SetupReply to towardVid instead of SetupReq, ensure I schedule setupReq to relevant pendingVneis in srcVset from rep b4 sending another SetupReply when WaitSetupReqIntTimer triggers
            // if (vrouteItr->second.isVsetRoute && !recordTraceForDirectedSetupReq) {
            //     bool otherEndIsRep = false;
            //     if (representativeFixed) {
            //         otherEndIsRep = (otherEnd == representative.vid && representative.heardfromvid != VLRRINGVID_NULL && repSeqExpirationTimer->isScheduled());
            //     } else {
            //         auto repMapItr = representativeMap.find(otherEnd);
            //         simtime_t expiredLastheard = simTime() - repSeqValidityInterval;
            //         otherEndIsRep = (repMapItr != representativeMap.end() && repMapItr->second.heardfromvid != VLRRINGVID_NULL && repMapItr->second.inNetwork && repMapItr->second.lastHeard > expiredLastheard);
            //     }
            //     if (otherEndIsRep) {
            //         ASSERT(fillVsetTimer->isScheduled());   // fillVsetTimer should always be scheduled, especially now that vset isn't empty

            //         double maxDelayToNextTimer = beaconInterval;
            //         if (fillVsetTimer->getArrivalTime() - simTime() >= maxDelayToNextTimer) {    // only reschedule fillVsetTimer if it won't come within maxDelayToNextTimer
            //             cancelEvent(fillVsetTimer);
            //             scheduleFillVsetTimer(/*firstTimer=*/false, /*maxDelay=*/maxDelayToNextTimer);
            //         }
            //     }
            // }

        } // else, send setupReq to otherEnd?
        pendingVsetAdd(otherEnd, /*heardFrom=*/VLRRINGVID_NULL, /*addToEmpty=*/true, /*nodeAlive=*/false);
    }
}

// check if other end of pathid (towardVid) is in my vset, if so remove; also check if pathid is a non-essential vroute in nonEssRoutes scheduled to be torn down (i.e. other end was in my vset), if so remove
// check if pathid is temporary route and other end (towardVid) is in my lostPneis, if so schedule another repairLinkReq
// if I removed towardVid from vset because its vset-route (pathid) is patched (using temporary route), reqRepairRoute = true, I'll schedule a setupReq to towardVid right away; NOTE reqRepairRoute can only be true only when pathIsVsetRoute is true
void Vlr::removeEndpointOnTeardown(const VlrPathID& pathid, const VlrRingVID& towardVid, bool pathIsVsetRoute, char pathIsTemporary, bool reqRepairRoute/*=false*/)
{
    if (pathIsVsetRoute) {
        // remove towardVid from vset
        EV_DETAIL << "Removing endpoint " << towardVid << " of vset route " << pathid << ", reqRepairRoute = " << reqRepairRoute << endl;

        auto vneiItr = vset.find(towardVid);
        // ASSERT(vneiItr != vset.end());  // towardVid should be in vset if pathIsVsetRoute=true
        if (vneiItr != vset.end()) {    // towardVid in my vset
            vneiItr->second.vsetRoutes.erase(pathid);
            if (vneiItr->second.vsetRoutes.empty()) {   // no more vset-routes to towardVid
                EV_DETAIL << "Removing other endpoint = " << towardVid << " from vset, reqRepairRoute = " << reqRepairRoute << endl;

                bool removedFromVset = vsetEraseAddPending(towardVid);  // if true, towardVid was removed from vset and added to pendingVset

                nodesVsetCorrect.erase(vid);
                
                bool keepSelfInNetwork = false;    // keep myself inNetwork if I have a cw/ccw vnei closer to me than towardVid
                bool wasClosestVnei = false;
                // if vsetHalfCardinality=1, I only put two closest vneis in vset, after removing towardVid I only have 1 vnei left
                if (selfInNetwork && vsetHalfCardinality > 1 && vset.size() >= 2) {      // I still have two or more vneis in vset
                    auto vneiPair = getClosestVneisInVset();    // my vset.size() >= 2
                    // if (oldVneiDist < 0 && getVid_CCW_Distance(vid, towardVid) > getVid_CCW_Distance(vid, vneiPair.first))  // removed vnei (towardVid) was my ccw vnei but I have a closer ccw vnei to me
                    //     keepSelfInNetwork = true;
                    // else if (oldVneiDist > 0 && getVid_CW_Distance(vid, towardVid) > getVid_CW_Distance(vid, vneiPair.second))  // removed vnei (towardVid) was my cw vnei but I have a closer cw vnei to me
                    //     keepSelfInNetwork = true;
                    if (getVid_CCW_Distance(vid, towardVid) > getVid_CCW_Distance(vid, vneiPair.first) && getVid_CW_Distance(vid, towardVid) > getVid_CW_Distance(vid, vneiPair.second))   // removed vnei (towardVid) isn't my closest ccw/cw vnei
                        keepSelfInNetwork = true;
                }
                bool repChecked = (representativeFixed && representative.heardfromvid == VLRRINGVID_NULL) ? false : true;
                if (repChecked) {        // initiate a setupReqTrace only if I don't have a valid rep yet
                    // if me > towardVid, I initiate a setupReqTrace now, else, I wait for other end to initiate a setupReqTrace (I only wait for finite time in case other end is dead)
                    auto towardItrPending = pendingVset.find(towardVid);
                    if (towardItrPending != pendingVset.end() && towardItrPending->second.setupReqTimer == nullptr) { // should be true if towardVid was added to pendingVset above
                        WaitSetupReqIntTimer*& setupReqTimer = towardItrPending->second.setupReqTimer;
                        char timerName[40] = "WaitSetupReqIntTimer:";
                        setupReqTimer = new WaitSetupReqIntTimer(strcat(timerName, std::to_string(towardVid).c_str()));
                        setupReqTimer->setDst(towardVid);
                        setupReqTimer->setTimerType(0);
                        setupReqTimer->setRepairRoute(reqRepairRoute);
                        // setupReqTimer->setAllowSetupReqTrace(wasClosestVnei);
                        setupReqTimer->setReqVnei(true);
                        setupReqTimer->setAlterPendingVnei(VLRRINGVID_NULL);
                        ASSERT(towardItrPending->second.setupReqTimer == setupReqTimer);
                        // Commented out to avoid sending setupReqTrace
                        // retryCount = VLRSETUPREQ_THRESHOLD: send a setupReqTrace at scheduled time (haven't sent any setupReqTrace to towardVid yet)
                        // retryCount = VLRSETUPREQ_THRESHOLD-1: send a setupReq at scheduled time, if no reply received when setupReq timer timeout, send a setupReqTrace then
                        // int retryCount = (wasClosestVnei) ? (VLRSETUPREQ_THRESHOLD - 1) : 0;    // if wasClosestVnei, we'll send setupReqTrace, so we don't need as many setupReq trials
                        // setupReqTimer->setRetryCount(retryCount);
                        setupReqTimer->setRetryCount(0);     // don't send setupReqTrace
                        if (!reqRepairRoute) {      // removing towardVid from vset bc received Teardown of its vset route
                            if (vid > towardVid) {
                                // schedule a setupReqTrace to send now rather than calling sendSetupReqTrace() bc there may be more pneis/vroutes to remove after this one, remove all of them before finding a next hop for setupReqTrace
                                scheduleAt(simTime() + uniform(0.01, 0.1) * routeSetupReqWaitTime, setupReqTimer);
                                EV_DETAIL << "Scheduling a SetupReq/SetupReqTrace to removed vnei " << towardVid << " right away" << endl;
                            } else {    // schedule a setupReqTrace to send later
                                // scheduleAt(simTime() + uniform(1, 3) * routeSetupReqTraceWaitTime, setupReqTimer);
                                scheduleAt(simTime() + uniform(0.1, 0.3) * routeSetupReqWaitTime, setupReqTimer);    // if retryCount set within [0, VLRSETUPREQ_THRESHOLD-1], i.e. send a setupReq first
                                EV_DETAIL << "Scheduling a SetupReq/SetupReqTrace to removed vnei " << towardVid << " at " << setupReqTimer->getArrivalTime() << endl;
                            }
                        } else {    // removing towardVid from vset bc received RepairRoute of its vset route
                            setupReqTimer->setPatchedRoute(pathid);
                            // Commented out to avoid sending setupReqTrace
                            // retryCount = (wasClosestVnei) ? (VLRSETUPREQ_THRESHOLD - 2) : 0;    // if wasClosestVnei, we'll send setupReqTrace, so we don't need as many setupReq trials
                            // if (retryCount < 0)
                            //     retryCount = 0;
                            // setupReqTimer->setRetryCount(retryCount);
                            setupReqTimer->setRetryCount(0);
                            scheduleAt(simTime() + uniform(0, patchedRouteRepairSetupReqMaxDelay), setupReqTimer);  // add random delay b4 sending first setupReq(repairRoute=true) to repair pathid
                            EV_DETAIL << "Scheduling a SetupReq(repairRoute=true) to removed vnei " << towardVid << " at " << setupReqTimer->getArrivalTime() << endl;
                        }
                    }
                }

                // NOTE if there was only one vnei in vset, one of closest ccw/cw vneis must be unspecified, hence keepSelfInNetwork won't be true; if there were two vneis in vset (if both closest ccw/cw specified, removed vnei should be one of them), keepSelfInNetwork won't be true either
                // NOTE inNetworkWarmupTimer shouldn't be scheduled when I'm inNetwork
                if (selfInNetwork && vset.size() < 2 * vsetHalfCardinality) { // if I was inNetwork but vset no longer full after removing towardVid
                    if (!keepSelfInNetwork) {
                        // set inNetwork = false (bc I'm probably not fully connected in overlay) so during inNetwork warmup when I try to recover vroute to towardVid, my pnei doesn't use me as proxy to join overlay
                        selfInNetwork = false;
                        // set inNetwork = true after expected wait time for vroute to be repaired
                        // scheduleAt(simTime() + routeSetupReqTraceWaitTime, inNetworkWarmupTimer);
                        scheduleAt(simTime() + setupReqRetryLimit * routeSetupReqWaitTime, inNetworkWarmupTimer);   // I'm not planning to send setupReqTrace
                        // EV_DETAIL << "Vset = " << printVsetToString() << " after removing endpoint = " << towardVid << ", setting inNetwork = false, inNetwork scheduled to become true at " << inNetworkWarmupTimer->getArrivalTime() << endl;
                        EV_DETAIL << "Vset = " << printVsetToString(/*printVpath=*/true) << " after removing endpoint = " << towardVid << ", setting inNetwork = false, inNetwork scheduled to become true at " << inNetworkWarmupTimer->getArrivalTime() << endl;
                    }
                }
                else if (vset.empty()) {     // if vset empty after removing towardVid
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
                    s << "vsetUnfull: inNetwork=" << selfInNetwork << " vset=" << printVsetToString(/*printVpath=*/true) << " vsetSize=" << vset.size() << " (reqRepairRoute=" << reqRepairRoute << ")";
                    recordNodeStatsRecord(/*infoStr=*/s.str().c_str());   // unused params (stage)
                }
            }
            else {  // didn't remove towardVid from vset bc there are still vset-routes to towardVid after removing pathid
                // Commented out when not repairing second vset route
                // if (reqRepairRoute) {    // received RepairRoute of vset route pathid
                //     WaitSetupReqIntTimer*& setupReqTimer = vneiItr->second.setupReqTimer;
                //     if (vneiItr->second.setupReqTimer == nullptr) { // should be true if towardVid was added to pendingVset above
                //         char timerName[40] = "WaitSetupReqIntTimer:";
                //         setupReqTimer = new WaitSetupReqIntTimer(strcat(timerName, std::to_string(towardVid).c_str()));
                //         setupReqTimer->setDst(towardVid);
                //         setupReqTimer->setTimerType(1);
                //         setupReqTimer->setAllowSetupReqTrace(false);
                //         setupReqTimer->setReqVnei(false);
                //         ASSERT(vneiItr->second.setupReqTimer == setupReqTimer);
                //         setupReqTimer->setRetryCount(0);     // don't send setupReqTrace
                //     }
                //     if (!vneiItr->second.setupReqTimer->isScheduled()) {
                //         setupReqTimer->setPatchedRoute(pathid);
                //         setupReqTimer->setRepairRoute(reqRepairRoute);
                //         scheduleAt(simTime() + uniform(0, patchedRouteRepairSetupReqMaxDelay), setupReqTimer);  // add random delay b4 sending first setupReq(repairRoute=true) to repair pathid
                //         EV_DETAIL << "Scheduling a SetupReq(repairRoute=true) to current vnei " << towardVid << " for patched vroute " << pathid << endl;
                //     }
                // }
            }
        }
    }
    else {  // if pathid in nonEssRoutes, its isVsetRoute should have been set to false
        auto nonEssItr = nonEssRoutes.find(pathid);
        if (nonEssItr != nonEssRoutes.end()) {
            if (reqRepairRoute) {   // if reqRepairRoute = true, we aren't removing pathid (probably patched) from vlrRoutingTable, if pathid isn't a vset route, don't remove it from nonEssRoutes, if towardVid is a vnei or pendingVnei, pathid is a secondary vroute, schedule setupReq(reqVnei=false) to repair vroute
                // char isPendingVnei = 0;     // towardVid is 0: neither pendingVnei nor current vnei, 1: pendingVnei, 2: current vnei, if 1 or 2, I'll schedule setupReq(reqVnei=false) to repair the patched nonEss vroute
                auto pendingItr = pendingVset.find(towardVid);
                if (pendingItr != pendingVset.end() && (pendingItr->second.setupReqTimer == nullptr || !pendingItr->second.setupReqTimer->isScheduled() || !pendingItr->second.setupReqTimer->getRepairRoute()) && !IsLinkedPneiOrAvailableWantedRouteEnd(towardVid)) {
                    // isPendingVnei = 1;
                    // I don't have direct link or vroute to towardVid (excluding the patched route), schedule setupReq(reqVnei=false) to repair the patched nonEss vroute to towardVid     NOTE existing vroute to towardVid may also be a patched route but I haven't received RepairRoute yet, it's ok bc I can utilize the patched route for now if I need to reach towardVid
                    if (pendingItr->second.setupReqTimer == nullptr) {
                        char timerName[40] = "WaitSetupReqIntTimer:";
                        pendingItr->second.setupReqTimer = new WaitSetupReqIntTimer(strcat(timerName, std::to_string(pendingItr->first).c_str()));
                        pendingItr->second.setupReqTimer->setDst(pendingItr->first);
                        pendingItr->second.setupReqTimer->setTimerType(0);
                        pendingItr->second.setupReqTimer->setRetryCount(0);
                    }  // else, pendingItr->second is allocated but isn't scheduled, probably canceled
                    pendingItr->second.setupReqTimer->setRepairRoute(true);
                    pendingItr->second.setupReqTimer->setPatchedRoute(pathid);
                    // pendingItr->second.setupReqTimer->setAllowSetupReqTrace(false);
                    // pendingItr->second.setupReqTimer->setReqVnei(false);   // setupReq is to repair a nonEss vroute, not a vset-route
                    pendingItr->second.setupReqTimer->setAlterPendingVnei(VLRRINGVID_NULL);

                    if (!pendingItr->second.setupReqTimer->isScheduled())
                        scheduleAt(simTime() + uniform(0, patchedRouteRepairSetupReqMaxDelay), pendingItr->second.setupReqTimer);  // add random delay b4 sending first setupReq(repairRoute=true) to repair pathid
                }
                // else if (vset.find(towardVid) != vset.end()) {    // towardVid is current vnei
                //     isPendingVnei = 2;
                // }
                // if (isPendingVnei > 0) {
                //     // WaitSetupReqIntTimer *setupReqTimer = allocate new WaitSetupReqIntTimer
                // }
            } else {    // if reqRepairRoute = false
                EV_DETAIL << "Removing pathid = " << pathid << " from nonEssRoutes before removing it from routing table because of teardown" << endl;
                nonEssRoutes.erase(nonEssItr);
                // pathid is removed from nonEssRoutes, remove it from nonEssUnwantedRoutes if it's also in there (i.e. patched nonEss vroute)
                auto nonEssUnwantedRouteItr = nonEssUnwantedRoutes.find(pathid);
                if (nonEssUnwantedRouteItr != nonEssUnwantedRoutes.end())
                    nonEssUnwantedRoutes.erase(nonEssUnwantedRouteItr);
                
                // if scheduled setupReq(repairRoute=true) to towardVid to repair pathid but pathid is now torn down, set repairRoute=false in WaitSetupReqIntTimer
                auto pendingItr = pendingVset.find(towardVid);
                if (pendingItr != pendingVset.end() && pendingItr->second.setupReqTimer && pendingItr->second.setupReqTimer->isScheduled() && pendingItr->second.setupReqTimer->getRepairRoute() && pendingItr->second.setupReqTimer->getPatchedRoute() == pathid) {
                    pendingItr->second.setupReqTimer->setRepairRoute(false);    // pathid no longer exists
                    
                    double maxDelayToNextTimer = 0.2 * routeSetupReqWaitTime;
                    if (pendingItr->second.setupReqTimer->getArrivalTime() - simTime() >= maxDelayToNextTimer) {    // reschedule setupReqTimer to towardVid if it won't come within 0.5 * routeSetupReqWaitTime
                        cancelEvent(pendingItr->second.setupReqTimer);
                        scheduleAt(simTime() + uniform(0, maxDelayToNextTimer), pendingItr->second.setupReqTimer);  // add random delay b4 sending first setupReq(repairRoute=true) to repair pathid
                    }
                }
            }
        }   
        else if (reqRepairRoute) {  // pathid isn't a vset-route or nonEss route
            return;
        }
        else if (vlrRoutingTable.getIsTemporaryRoute(pathIsTemporary)) {    // pathid is a temporary route
            // NOTE RepairRoute won't be sent for a temporary route, hence reqRepairRoute must be false here
            auto lostPneiItr = lostPneis.find(towardVid);
            if (lostPneiItr != lostPneis.end()) {
                cancelAndDelete(lostPneiItr->second.timer);
                lostPneiItr->second.timer = nullptr;

                lostPneiItr->second.tempVlinks.erase(pathid);
                if (lostPneiItr->second.tempVlinks.empty()) {   // no more temporary route to lost pnei
                    bool brokenVroutesExist = false;

                    if (!lostPneiItr->second.brokenVroutes.empty()) {   // there are still vroutes broken by lost pnei towardVid, check if they are indeed broken
                        // set vroutes broken by lost pnei unavailable
                        setLostPneiBrokenVroutes(lostPneiItr, /*setBroken=*/true);

                        if (!lostPneiItr->second.brokenVroutes.empty()) {
                            brokenVroutesExist = true;
                            
                            // schedule another repairLinkReq to repair link to lost pnei towardVid
                            EV_WARN << "Endpoint " << towardVid << " of removed tempPathid = " << pathid << " is a lost pnei with broken vroutes = [";
                            for (const auto& pathid : lostPneiItr->second.brokenVroutes) 
                                EV_WARN << pathid << " ";
                            EV_WARN << "], scheduling RepairLinkReq to rebuild temporary route to lost pnei" << endl;

                            // schedule WaitRepairLinkIntTimer to towardVid
                            char timerName[40] = "WaitRepairLinkIntTimer:";
                            lostPneiItr->second.timer = new WaitRepairLinkIntTimer(strcat(timerName, std::to_string(towardVid).c_str()));
                            lostPneiItr->second.timer->setDst(towardVid);
                            lostPneiItr->second.timer->setRetryCount(0);
                            double delay = 0.01;    // add minimal delay to ensure I check delayedRepairLinkReq before sending repairLinkReq, bc even if I'm the larger link end, I may have detected link failure too late that the smaller end has already sent repairLinkReq to me
                            if (vid > towardVid) {
                                // schedule a repairLinkReq to send now
                                scheduleAt(simTime() +delay, lostPneiItr->second.timer);
                                EV_WARN << "Scheduling a RepairLinkReq to lost pnei " << towardVid << " right away at " << lostPneiItr->second.timer->getArrivalTime() << endl;
                            }
                            else {    
                                // schedule a repairLinkReq to send later
                                // NOTE the larger link end will wait for repairLinkReqWaitTime + repairLinkReqSmallEndSendDelay before tearing down the broken vroutes, in case its nexthop has failed and its nextnexthop is the smaller link end of the failed link (nexthop, nextnexthop)
                                scheduleAt(simTime() +delay + repairLinkReqSmallEndSendDelay, lostPneiItr->second.timer);
                                EV_WARN << "Scheduling a RepairLinkReq to lost pnei " << towardVid << " at " << lostPneiItr->second.timer->getArrivalTime() << endl;
                                
                                // schedule a timer to tear down broken vroutes if they aren't repaired in time
                                // lostPneiItr->second.timer->setRetryCount(repairLinkReqRetryLimit);
                                // scheduleAt(simTime() + 1.2 * repairLinkReqWaitTime, lostPneiItr->second.timer);
                                // EV_DETAIL << "Scheduling a WaitRepairLinkIntTimer to tear down broken vroutes because of lost pnei " << towardVid << endl;
                            }

                            // check delayedRepairLinkReq to see if I can reply to brokenVroutes in received repairLinkReq after new link breakage detected
                            if (!delayedRepairReqTimer->isScheduled())
                                scheduleAt(simTime(), delayedRepairReqTimer);
                        }
                    }
                    if (!brokenVroutesExist) {  // no broken vroutes still require a temporary route to lost pnei towardVid
                        EV_WARN << "Endpoint " << towardVid << " of removed tempPathid = " << pathid << " is a lost pnei but no brokenVroutes, removing it from lostPneis, tempVlinks = ";
                        for (const auto& vlinkid : lostPneiItr->second.tempVlinks)
                            EV_WARN << vlinkid << " ";
                        EV_WARN << "]" << endl;
                        lostPneis.erase(lostPneiItr);
                    }
                } else {    // after removing pathid, there exists another temporary route to lost pnei
                    // check if the temporary route removed is associated with brokenVroutes whose nexthopVid have been changed to towardVid recently
                    auto recentTempItr = recentTempRouteSent.find(pathid);
                    if (recentTempItr != recentTempRouteSent.end()) {
                        if (recentTempItr->second.second > simTime()) { // recentTempRouteSent record hasn't expired
                            EV_WARN << "Removing tempPathid = " << pathid << " to endpoint " << towardVid << " found in recentTempRouteSent and was recently built by me" << endl;
                            for (const auto& brokenPathidPair : recentTempItr->second.first) {
                                const VlrPathID& brokenPathid = brokenPathidPair.first;
                                const VlrRingVID& oldNexthopVid = brokenPathidPair.second;
                                auto vrouteItr = vlrRoutingTable.vlrRoutesMap.find(brokenPathid);
                                if (vrouteItr != vlrRoutingTable.vlrRoutesMap.end()) {  // pathid still in vlrRoutingTable
                                    if (vlrRoutingTable.getPrevNextIsUnavailable(vrouteItr->second.isUnavailable, /*getPrev=*/false) == 2 && vrouteItr->second.nexthopVid == towardVid) { // route nexthop is using temporary route which was recently built but now torn down
                                        // set nexthop of brokenPathid broken
                                        vlrRoutingTable.setRouteItrPrevNextIsUnavailable(vrouteItr, /*setPrev=*/false, /*value=*/1);     // set route nexthop state
                                        vlrRoutingTable.removeRouteEndFromEndpointMap(brokenPathid, vrouteItr->second.toVid);     // remove route toVid in endpointToRoutesMap
                                        // schedule WaitRepairLinkIntTimer to tear down broken vroutes if they aren't repaired in time
                                        auto lostPneiItr = lostPneis.find(towardVid);
                                        if (lostPneiItr != lostPneis.end()) {
                                            lostPneiItr->second.brokenVroutes.erase(brokenPathid);
                                        }
                                        // if oldNexthopVid not in lostPneis or lostPneis[oldNexthopVid].timer is still scheduled, associate brokenPathid with oldNexthopVid again
                                        // lostPneis[oldNexthopVid].timer isn't scheduled, meaning temporary route has been built to oldNexthopVid but doesn't include brokenPathid, need to associate brokenPathid with some dummy nexthop in lostPneis
                                        VlrRingVID newNexthopVid = oldNexthopVid;
                                        lostPneiItr = lostPneis.find(oldNexthopVid);
                                        if (lostPneiItr != lostPneis.end()) {
                                            if (lostPneiItr->second.timer == nullptr || !lostPneiItr->second.timer->isScheduled()) {    // can't associate brokenPathid with lostPneis[oldNexthopVid].brokenVroutes
                                                // lostPneis[VLRRINGVID_DUMMY] may already exist
                                                // NOTE if lostPneis[oldNexthopVid].timer should always be scheduled if VLRRINGVID_DUMMY in lostPneis, bc it's just a timer to tear down brokenVroutes
                                                auto itr_bool = lostPneis.insert({VLRRINGVID_DUMMY, {{/*brokenVroutes*/}, nullptr, {}}});  // std::pair<iterator, bool>: iterator (to the inserted element), bool (whether the element is inserted)
                                                lostPneiItr = itr_bool.first;
                                                newNexthopVid = VLRRINGVID_DUMMY;
                                                // add brokenPathid to lostPneis[VLRRINGVID_DUMMY].brokenVroutes
                                                lostPneiItr->second.brokenVroutes.insert(brokenPathid);
                                            } else {    // lostPneis[oldNexthopVid].timer is scheduled
                                                // add brokenPathid to lostPneis[oldNexthopVid].brokenVroutes
                                                lostPneiItr->second.brokenVroutes.insert(brokenPathid);
                                            }
                                        } else {    // oldNexthopVid not in lostPneis
                                            auto itr_bool = lostPneis.insert({oldNexthopVid, {{brokenPathid}, nullptr, {}}});  // std::pair<iterator, bool>: iterator (to the inserted element), bool (whether the element is inserted)
                                            lostPneiItr = itr_bool.first;
                                        }
                                        ASSERT(lostPneiItr->first == newNexthopVid);
                                        vrouteItr->second.nexthopVid = newNexthopVid;
                                        EV_WARN << "Changing route nexthop: pathid = " << vrouteItr->first << " nexthop changed from " << towardVid << " to " << newNexthopVid << endl;

                                        if (lostPneiItr->second.timer == nullptr) {   // newNexthopVid was just added to lostPneis, schedule WaitRepairLinkIntTimer to tear down broken vroutes if they aren't repaired in time
                                            // vroute nexthop unavailable for brokenPathid, hence I need to wait for repairLinkReq from the other link end rather than sending repairLinkReq
                                            char timerName[40] = "WaitRepairLinkIntTimer:";
                                            lostPneiItr->second.timer = new WaitRepairLinkIntTimer(strcat(timerName, std::to_string(newNexthopVid).c_str()));
                                            lostPneiItr->second.timer->setDst(newNexthopVid);
                                            lostPneiItr->second.timer->setRetryCount(repairLinkReqRetryLimit);
                                            double reqWaitTime = (vid > newNexthopVid) ? (repairLinkReqWaitTime + repairLinkReqSmallEndSendDelay) : repairLinkReqWaitTime;  // if I'm the larger link end of the failed link, I wait longer bc smaller link ends wait longer before the first repairLinkReq timer rings 
                                            scheduleAt(simTime() + reqWaitTime, lostPneiItr->second.timer);
                                            EV_WARN << "Scheduling a WaitRepairLinkIntTimer at " << lostPneiItr->second.timer->getArrivalTime() << " to tear down brokenPathid that are newly associated with newNexthopVid=" << newNexthopVid << " of RepairLinkReq" << endl;
                                        }
                                    }
                                }
                            }
                        }
                        recentTempRouteSent.erase(recentTempItr);
                    }
                }
            }
        }
    }
}

// put tempVlinks into nonEssRoutes to tear down later as they are no longer necessary for broken vroutes
void Vlr::delayLostPneiTempVlinksTeardown(const std::set<VlrPathID>& tempVlinks, double delay)
{
    for (const auto& vlinkid : tempVlinks) {
        if (delay == 0)
            nonEssRoutes[vlinkid] = 1;      // set expireTime to 1 to ensure it'll be torn down
        else
            nonEssRoutes[vlinkid] = simTime() + delay;
    }
}

// remove pathid from lostPneis[lostPnei].brokenVroutes
void Vlr::removeRouteFromLostPneiBrokenVroutes(const VlrPathID& pathid, const VlrRingVID& lostPneiVid)
{
    auto lostPneiItr = lostPneis.find(lostPneiVid);
    if (lostPneiItr != lostPneis.end()) {
        lostPneiItr->second.brokenVroutes.erase(pathid);
        
        if (lostPneiItr->second.brokenVroutes.empty()) {    // if lostPneis[lostPnei].brokenVroutes empty after removing pathid
            delayLostPneiTempVlinksTeardown(lostPneiItr->second.tempVlinks, /*delay=*/0);   // notify lostPnei asap that my brokenVroutes now empty
            cancelAndDelete(lostPneiItr->second.timer);
            lostPneis.erase(lostPneiItr);
        }
    }
}

void Vlr::processTestPacketTimer()
{
    EV_DEBUG << "Processing TestPacket timer at node " << vid << endl;
    if (sendTestPacketStart) {
        bool repChecked = (representativeFixed && representative.heardfromvid == VLRRINGVID_NULL) ? false : true;
        if (repChecked) {   // I have valid rep
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
                VlrRingVID nextHopVid = findNextHop(vlrOptionOut, /*prevHopVid=*/VLRRINGVID_NULL, /*excludeVid=*/VLRRINGVID_NULL, /*allowTempRoute=*/true);
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
    }
    // sendTestPacketStart not yet true
    else if (lastTimeNodeJoinedInNetwork + sendTestPacketOverlayWaitTime <= simTime().dbl()) {
        sendTestPacketStart = true;
    }
    scheduleTestPacketTimer();
}

VlrIntTestPacket* Vlr::createTestPacket(const VlrRingVID& dstVid) const
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
void Vlr::sendCreatedTestPacket(VlrIntTestPacket *msg, const int& outGateIndex, double delay/*=0*/)
{
    EV_DEBUG << "Sending TestPacket: dst = " << msg->getDst() << ", src = " << msg->getSrc() << endl;

    msg->getVlrOptionForUpdate().setPrevHopVid(vid);    // set packet prevHopVid to myself
    msg->setHopcount(msg->getHopcount() +1);    // increment packet hopcount
    
    // NOTE addTag should be executed after chunkLength has been set, and chunkLength shouldn't be changed before findTag/getTag

    // all multihop VLR packets (setupReq, setupReply, etc) L3 dst are set to a pnei, greedy routing at L3 in routeDatagram() isn't needed, but we do greedy routing and deal with VlrOption at L4 (in processSetupReq() for example) 
    // udpPacket->addTagIfAbsent<VlrIntOptionReq>()->setVlrOption(vlrOption);      // VlrOption to be set in IP header in datagramLocalOutHook()
    
    sendCreatedPacket(msg, /*unicast=*/true, /*outGateIndex=*/outGateIndex, /*delay=*/delay, /*checkFail=*/true);
}

void Vlr::processTestPacket(VlrIntTestPacket *msgIncoming, bool& pktForwarded)
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

    // Commented out for TestPacket, no harm to try delivery
    // if (representative.heardfromvid == VLRRINGVID_NULL) {        // if I don't have a valid rep yet, I won't process or forward this message
    //     EV_WARN << "No valid rep heard: " << representative << ", cannot accept overlay message" << endl;
    //     return;
    // }
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
    
        // forward this TestPacket with findNextHop()
        // VlrIntOption* vlrOptionOut = vlrOptionIn->dup();
        VlrIntOption vlrOptionInCopy = vlrOptionIn;
        VlrRingVID nextHopVid = findNextHop(vlrOptionIn, /*prevHopVid=*/msgPrevHopVid, /*excludeVid=*/VLRRINGVID_NULL, /*allowTempRoute=*/true);
        if (nextHopVid == VLRRINGVID_NULL) {
            // delete vlrOptionOut;
            EV_WARN << "No next hop found for TestPacket received at me = " << vid << ", dropping packet: src = " << srcVid << ", dst = " << dstVid << ", hopCount = " << msgHopCount << endl;
            if (recordStatsToFile /*&& recordDroppedMsg*/) {   // record dropped message
                std::ostringstream s;
                s << "no next hop: vlrOptionIn=" << printVlrOptionToString(vlrOptionInCopy) << " vlrOptionOut=" << printVlrOptionToString(vlrOptionIn);            
                recordMessageRecord(/*action=*/4, /*src=*/srcVid, /*dst=*/dstVid, "TestPacket", /*msgId=*/msgIncoming->getMessageId(), /*hopcount=*/msgIncoming->getHopcount()+1, /*chunkByteLength=*/msgIncoming->getByteLength(), /*infoStr=*/s.str().c_str());   // unimportant params (msgId, hopcount)
            }
            // if (displayBubbles && hasGUI())
            //     getContainingNode(host)->bubble("No next hop found for TestPacket");
        } else {    // nexthop found to forward TestPacket
            // auto msgOutgoing = staticPtrCast<VlrIntTestPacket>(msgIncoming->dupShared());
            // msgIncoming->setHopcount(msgHopCount);
            sendCreatedTestPacket(msgIncoming, /*outGateIndex=*/psetTable.getPneiGateIndex(nextHopVid));    // computeChunkLength=false
            pktForwarded = true;
        }
    }
    // if (checkOverHeardTraces) {    // see if src of msg belongs to pendingVset
    //     pendingVsetAdd(srcVid, /*heardFrom=*/VLRRINGVID_NULL, /*addToEmpty=*/false);
    // }
}

void Vlr::processWaitRepairLinkTimer(WaitRepairLinkIntTimer *repairLinkTimer)
{
    // WaitRepairLinkIntTimer *repairLinkTimer = check_and_cast<WaitRepairLinkIntTimer *>(message);
    VlrRingVID targetVid = repairLinkTimer->getDst();
    int retryCount = repairLinkTimer->getRetryCount();
    EV_WARN << "Processing wait repairLinkReq timer timeout (useRepairLocal=" << sendRepairLocalNoTemp << ") of lost pnei " << targetVid << ", retryCount: " << retryCount << endl;

    if (representativeFixed && representative.heardfromvid == VLRRINGVID_NULL) {        // if I don't have a valid rep, I shouldn't have any vroutes
        EV_WARN << "No valid rep heard: " << representative << ", cannot send repairLinkReq message, scheduling repSeqExpirationTimer to tear down all vroutes" << endl;
        scheduleAt(simTime(), repSeqExpirationTimer);
        return;
    }

    if (retryCount < repairLinkReqRetryLimit) {
        
        auto lostPneiItr = lostPneis.find(targetVid);
        ASSERT(lostPneiItr != lostPneis.end());    //  targetVid must be in lostPneis
        
        std::set<VlrPathID> brokenPathidsReq;   // brokenPathids in dstToPathidsMap, i.e. route prevhop == lost pnei

        VlrIntVidToPathidSetMap dstToPathidsMap;    // std::map<VlrRingVID, std::vector<VlrPathID>>
        // send RepairLinkReq for brokenVroutes whose prevhop is lost
        for (const auto& brokenPathid : lostPneiItr->second.brokenVroutes) {
            auto vrouteItr = vlrRoutingTable.vlrRoutesMap.find(brokenPathid);
            if (vrouteItr != vlrRoutingTable.vlrRoutesMap.end()) {  // pathid should be in vlrRoutingTable
                // NOTE temporary route or dismantled vroute shouldn't be in brokenVroutes
                // one of both of prevhop and nexthop is the lost pnei
                if (lostPneiItr->first == vrouteItr->second.prevhopVid) { // route prevhop == lost pnei
                    brokenPathidsReq.insert(brokenPathid);
                    for (const VlrRingVID& prevhopVid : vrouteItr->second.prevhopVids)  // prevhopVids include vid of prevhop, prevprevhop, ..
                        dstToPathidsMap[prevhopVid].insert(brokenPathid);
                    dstToPathidsMap[vrouteItr->second.fromVid].insert(brokenPathid);
                    
                    if (sendRepairLocalNoTemp) {
                        // ensure recentRepairLocalBrokenVroutes[brokenPathid] recorded when last time brokenPathid's prevhop was lost is cleared
                        auto recentBrokenItr = recentRepairLocalBrokenVroutes.find(brokenPathid);
                        if (recentBrokenItr != recentRepairLocalBrokenVroutes.end())
                            recentRepairLocalBrokenVroutes.erase(recentBrokenItr);
                    }
                }   // else, route nexthop == lost pnei
            }
        }
        
        if (!dstToPathidsMap.empty()) {
            if (!sendRepairLocalNoTemp) {
                // send repairLinkReq
                const auto& repairReq = createRepairLinkReqFlood();
                repairReq->setDstToPathidsMap(dstToPathidsMap);
                sendCreatedRepairLinkReqFlood(repairReq, /*computeChunkLength=*/true, /*delay=*/uniform(0, maxFloodJitter));     // add random delay bc in case of a node failure, all pneis of the failed node whose vid larger than the failed node may detect link breakage and initiate repairLinkReqFlood at same time
                
                EV_WARN << "Sending RepairLinkReq from me = " << vid << " to lost pnei = " << targetVid << " and prevhopVids of brokenPathids (prevhop==lostpnei) : {";
                for (const auto& brokenPathid : brokenPathidsReq)
                    EV_WARN << brokenPathid << " ";
                EV_WARN << "}" << endl;

                if (recordStatsToFile) {   // record sent message
                    recordMessageRecord(/*action=*/0, /*src=*/vid, /*dst=*/targetVid, "RepairLinkReqFlood", /*msgId=*/0, /*hopcount=*/0, /*chunkByteLength=*/0);   // unused params (msgId, hopcount, chunkByteLength) are just assigned to 0
                }
            } else {
                // send repairLocalReq
                const auto& repairReq = createRepairLocalReqFlood();
                repairReq->setDstToPathidsMap(dstToPathidsMap);
                repairReq->setBrokenPathids(brokenPathidsReq);
                sendCreatedRepairLocalReqFlood(repairReq, /*computeChunkLength=*/true, /*delay=*/uniform(0, maxFloodJitter));     // add random delay bc in case of a node failure, all pneis of the failed node may detect link breakage and initiate repairLocalReqFlood at same time

                EV_WARN << "Sending RepairLocalReq from me = " << vid << " to lost pnei = " << targetVid << " and prevhopVids of brokenPathids (prevhop==lostpnei) : {";
                for (const auto& brokenPathid : brokenPathidsReq)
                    EV_WARN << brokenPathid << " ";
                EV_WARN << "}" << endl;

                if (recordStatsToFile) {   // record sent message
                    recordMessageRecord(/*action=*/0, /*src=*/vid, /*dst=*/targetVid, "RepairLocalReqFlood", /*msgId=*/0, /*hopcount=*/0, /*chunkByteLength=*/0);   // unused params (msgId, hopcount, chunkByteLength) are just assigned to 0
                }
            }
            
        }

        // reschedule the wait repairLink timer
        int retryCount = repairLinkTimer->getRetryCount();
        repairLinkTimer->setRetryCount(++retryCount);
        double reqWaitTime = (vid > targetVid) ? (repairLinkReqWaitTime + repairLinkReqSmallEndSendDelay) : repairLinkReqWaitTime;  // if I'm the larger link end of the failed link, I wait longer bc smaller link ends wait longer before the first repairLinkReq timer rings
        if (lostPneiItr->second.brokenVroutes.empty())  // no more brokenVroutes associated lostPneis[targetVid], can remove right now
            reqWaitTime = 0;
        scheduleAt(simTime() + reqWaitTime, repairLinkTimer);
        EV_DETAIL << "Rescheduling repairLinkReq timer: dst = " << targetVid << ", retryCount = " << retryCount << endl;
    }
    else {  // tear down all vroutes broken by lost pnei targetVid that haven't been repaired, delete targetVid from lostPneis
        auto lostPneiItr = lostPneis.find(targetVid);
        ASSERT(lostPneiItr != lostPneis.end());
        // const L3Address& pneiAddr = lostPneiItr->second.address;
        EV_WARN << "Delete node " << targetVid << " from lostPneis, temp routes = [";
        for (const auto& vlinkid : lostPneiItr->second.tempVlinks)
            EV_WARN << vlinkid << " ";
        EV_WARN << "], broken vroutes = [";
        for (const auto& pathid : lostPneiItr->second.brokenVroutes) 
            EV_WARN << pathid << " ";
        EV_WARN << "]" << endl;

        if (!psetTable.pneiIsLinked(targetVid)) {   // confirm targetVid is no longer LINKED
            
            std::map<VlrRingVID, std::pair<std::vector<VlrPathID>, char>> nextHopToPathidsMap;    // for Teardown to send, map next hop address to (pathids, next hop 2-bit isUnavailable) pair
            for (const auto& oldPathid : lostPneiItr->second.brokenVroutes) {
                auto vrouteItr = vlrRoutingTable.vlrRoutesMap.find(oldPathid);
                if (vrouteItr != vlrRoutingTable.vlrRoutesMap.end()) {  // pathid still in vlrRoutingTable
                    if (vrouteItr->second.isUnavailable != 0 && !vlrRoutingTable.getIsDismantledRoute(vrouteItr->second.isUnavailable)) {   // this is a regular vroute, unavailable or patched (using temporary route)
                        // send Teardown to prevhop/nexthop that's still LINKED, bc pnei can't be reached (sending a futile Teardown to it can cause collision, other nodes along oldPathid won't process the unicast Teardown not directed to themselves)
                        bool isToNexthop = (vrouteItr->second.prevhopVid == targetVid);
                        const VlrRingVID& nextHopVid = (isToNexthop) ? vrouteItr->second.nexthopVid : vrouteItr->second.prevhopVid;
                        char nextHopIsUnavailable = vlrRoutingTable.getPrevNextIsUnavailable(vrouteItr->second.isUnavailable, /*getPrev=*/!isToNexthop);

                        if (nextHopVid != VLRRINGVID_NULL) {     // I'm not an endpoint of the vroute
                            // remove regular route from vlrRoutingTable if remaining next hop is blocked/patched/unavailable, as dismantled vroute shouldn't involve temporary route
                            bool removeFromRoutingTable = (!keepDismantledRoute || nextHopIsUnavailable != 0);
                            if (nextHopIsUnavailable != 1) {
                                EV_DETAIL << "Teardown (pathid = " << oldPathid << ") will be sent to nexthop " << nextHopVid << endl;
                                auto nextHopItr = nextHopToPathidsMap.find(nextHopVid);
                                if (nextHopItr == nextHopToPathidsMap.end())
                                    nextHopToPathidsMap[nextHopVid] = {{oldPathid}, nextHopIsUnavailable};
                                else
                                    nextHopItr->second.first.push_back(oldPathid);
                            } else  // next hop unavailable and regular vroute, remove this vroute from lostPneis.brokenVroutes
                                removeRouteFromLostPneiBrokenVroutes(oldPathid, /*lostPneiVid=*/nextHopVid);

                            if (removeFromRoutingTable) {
                                EV_DETAIL << "Pathid " << oldPathid << " is removed from vlrRoutingTable, nextHopIsUnavailable = " << (int)nextHopIsUnavailable << ", keepDismantledRoute=" << keepDismantledRoute << endl;
                                // delete route in endpointToRoutesMap
                                vlrRoutingTable.removeRouteEndsFromEndpointMap(oldPathid, vrouteItr->second);
                                // delete route in vlrRoutesMap
                                vlrRoutingTable.vlrRoutesMap.erase(vrouteItr);
                            } else {    // endpoint connected by the remaining next hop is still available
                                // set lost pnei in route unavailable
                                vlrRoutingTable.setRouteItrPrevNextIsUnavailable(vrouteItr, /*setPrev=*/isToNexthop, /*value=*/1);
                                // set vroute as dismantled
                                vlrRoutingTable.setRouteItrIsDismantled(vrouteItr, /*setDismantled=*/true, /*setPrev=*/isToNexthop);
                                // remove endpoint connected by lost pnei in endpointToRoutesMap
                                VlrRingVID lostEnd = (isToNexthop) ? vrouteItr->second.fromVid : vrouteItr->second.toVid;
                                EV_DETAIL << "Pathid " << oldPathid << " is torn down but kept in vlrRoutingTable and added to dismantledRoutes, endpoint " << lostEnd << " becomes unavailable" << endl;
                                vlrRoutingTable.removeRouteEndFromEndpointMap(oldPathid, lostEnd);

                                dismantledRoutes.insert({oldPathid, simTime() + dismantledRouteExpiration});
                            }

                        } else {
                            // check if I'm an endpoint of pathid, if so this may be a vset route, or maybe in nonEssRoutes (brokenVroutes shouldn't be temporary or dismantled)
                            if (vrouteItr->second.fromVid == vid)
                                removeEndpointOnTeardown(oldPathid, /*towardVid=*/vrouteItr->second.toVid, /*pathIsVsetRoute=*/vrouteItr->second.isVsetRoute, /*pathIsTemporary=*/vrouteItr->second.isUnavailable);
                            else if (vrouteItr->second.toVid == vid)
                                removeEndpointOnTeardown(oldPathid, /*towardVid=*/vrouteItr->second.fromVid, /*pathIsVsetRoute=*/vrouteItr->second.isVsetRoute, /*pathIsTemporary=*/vrouteItr->second.isUnavailable);
                        
                            // delete route in endpointToRoutesMap
                            vlrRoutingTable.removeRouteEndsFromEndpointMap(oldPathid, vrouteItr->second);
                            // delete route in vlrRoutesMap
                            vlrRoutingTable.vlrRoutesMap.erase(vrouteItr);
                        }
                        
                    }
                }
            }
            for (const auto& mappair : nextHopToPathidsMap) {
                const auto& teardownOut = createTeardown(/*pathids=*/mappair.second.first, /*addSrcVset=*/false, /*rebuild=*/true, /*dismantled=*/true);
                // checked that nextHopIsUnavailable != 1
                sendCreatedTeardownToNextHop(teardownOut, /*nextHopAddr=*/mappair.first, /*nextHopIsUnavailable=*/mappair.second.second);

                if (recordStatsToFile) {   // record sent message
                    recordMessageRecord(/*action=*/0, /*src=*/vid, /*dst=*/VLRRINGVID_NULL, "Teardown", /*msgId=*/0, /*hopcount=*/0, /*chunkByteLength=*/0, /*infoStr=*/"repairLinkReq retry limit");   // unused params (msgId, hopcount, chunkByteLength) are just assigned to 0
                }
            }
        }
        // NOTE when there is a valid temporary route to lost pnei, this WaitRepairLinkTimer shouldn't timeout, so tempVlinks is likely empty
        // put temporary vlinks to lost pnei in nonEssRoutes as they are no longer necessary for broken vroutes
        // tear down the temporary route to notify the lost pnei of the removal of these broken vroutes
        delayLostPneiTempVlinksTeardown(lostPneiItr->second.tempVlinks, /*delay=*/0);
        
        cancelAndDelete(lostPneiItr->second.timer);
        lostPneis.erase(lostPneiItr);
    }
}

int Vlr::computeRepairLinkReqFloodByteLength(RepairLinkReqFloodInt* repairReq) const
{
    // unsigned int ttl;    // 2 byte
    // unsigned int floodSeqnum;    // 2 byte
    int chunkByteLength = 4;
    // std::map<unsigned int, std::vector<VlrPathID>> dstToPathidsMap;
    const auto& dstToPathidsMap = repairReq->getDstToPathidsMap();
    for (auto it = dstToPathidsMap.begin(); it != dstToPathidsMap.end(); ++it)
        chunkByteLength += VLRRINGVID_BYTELEN + it->second.size() * VLRPATHID_BYTELEN;
    // std::vector<unsigned int> linkTrace
    chunkByteLength += repairReq->getLinkTrace().size() * VLRRINGVID_BYTELEN;
    // // L3Address srcAddress;
    // chunkByteLength += addressByteLength;

    return chunkByteLength;
}

RepairLinkReqFloodInt* Vlr::createRepairLinkReqFlood()
{
    RepairLinkReqFloodInt *repairReq = new RepairLinkReqFloodInt(/*name=*/"RepairLinkReqFlood");
    repairReq->setTtl(repairLinkReqFloodTTL);
    repairReq->setFloodSeqnum(++floodSeqnum);

    repairReq->getLinkTraceForUpdate().push_back(vid);  // put myself in linkTrace
    // repairReq->setSrcAddress(getSelfAddress());

    return repairReq;
}

// if computeChunkLength = true, compute chunk length because chunk was just created with createSetupReq() or modified after dupShared() from another chunk
// else, no need to compute chunk length because chunk was dupShared() (not modified) from another chunk that has chunkLength set
void Vlr::sendCreatedRepairLinkReqFlood(RepairLinkReqFloodInt *repairReq, bool computeChunkLength/*=true*/, double delay/*=0*/)
{
    if (computeChunkLength)
        repairReq->setByteLength(computeRepairLinkReqFloodByteLength(repairReq));
    EV_DEBUG << "Sending repairLinkReqFlood: src = " << repairReq->getLinkTrace().at(0) << ", ttl = " << repairReq->getTtl() << ", floodSeqnum = " << repairReq->getFloodSeqnum() << endl;

    // repairReq->getVlrOptionForUpdate().setPrevHopVid(vid);    // set packet prevHopVid to myself
    
    // NOTE addTag should be executed after chunkLength has been set, and chunkLength shouldn't be changed before findTag/getTag

    // all multihop VLR packets (setupReq, setupReply, etc) L3 dst are set to a pnei, greedy routing at L3 in routeDatagram() isn't needed, but we do greedy routing and deal with VlrOption at L4 (in processSetupReq() for example) 
    // udpPacket->addTagIfAbsent<VlrIntOptionReq>()->setVlrOption(vlrOption);      // VlrOption to be set in IP header in datagramLocalOutHook()
    
    sendCreatedPacket(repairReq, /*unicast=*/false, /*outGateIndex=*/-1, /*delay=*/delay, /*checkFail=*/true);
}

void Vlr::processRepairLinkReqFlood(RepairLinkReqFloodInt *reqIncoming, bool& pktForwarded)
{
    EV_DEBUG << "Received RepairLinkReqFlood" << endl;
    
    const VlrIntVidToPathidSetMap& dstToPathidsMap = reqIncoming->getDstToPathidsMap();
    VlrRingVID srcVid = reqIncoming->getLinkTrace().at(0);
    unsigned int floodTtl = reqIncoming->getTtl();
    unsigned int floodSeqnum = reqIncoming->getFloodSeqnum();
    EV_INFO << "Processing RepairLinkReqFlood: src = " << srcVid << ", flood seqnum = " << floodSeqnum << ", ttl = " << floodTtl << ", dst = {";
    for (auto it = dstToPathidsMap.begin(); it != dstToPathidsMap.end(); ++it)
        EV_INFO << it->first << ' ';
    EV_INFO << "}" << endl;

    if (recordStatsToFile && recordReceivedMsg) {   // record received message
        // RepairLinkReqFlood doesn't have <VlrCreationTimeTag> tag
        recordMessageRecord(/*action=*/2, /*src=*/srcVid, /*dst=*/vid, "RepairLinkReqFlood", /*msgId=*/floodSeqnum, /*hopcount=*/reqIncoming->getLinkTrace().size()+1, /*chunkByteLength=*/reqIncoming->getByteLength());  // unimportant params (msgId)
    }
    bool pktForMe = false;      // set to true if this msg is directed to me or I processed it as its dst
    bool pktRecorded = false;      // set to true if this msg is recorded with recordMessageRecord()

    if (representativeFixed && representative.heardfromvid == VLRRINGVID_NULL) {        // if I don't have a valid rep yet, I won't process or forward this message
        EV_WARN << "No valid rep heard: " << representative << ", cannot accept overlay message" << endl;
        
        if (recordStatsToFile && recordDroppedMsg && !pktRecorded) {   // record dropped message
            recordMessageRecord(/*action=*/4, /*src=*/srcVid, /*dst=*/vid, "RepairLinkReqFlood", /*msgId=*/floodSeqnum, /*hopcount=*/reqIncoming->getLinkTrace().size()+1, /*chunkByteLength=*/reqIncoming->getByteLength(), /*infoStr=*/"no valid rep");
            pktRecorded = true;
        }
        return;
    }
    if (srcVid == vid) {
        EV_INFO << "Received RepairLinkReqFlood sent by me with flood seqnum = " << floodSeqnum << ", ignoring this flood request" << endl;

        if (recordStatsToFile && recordDroppedMsg && !pktRecorded) {   // record dropped message
            recordMessageRecord(/*action=*/4, /*src=*/srcVid, /*dst=*/vid, "RepairLinkReqFlood", /*msgId=*/floodSeqnum, /*hopcount=*/reqIncoming->getLinkTrace().size()+1, /*chunkByteLength=*/reqIncoming->getByteLength(), /*infoStr=*/"flood sent from me");
            pktRecorded = true;
        }
        return;
    }
    bool reqProcessed = false;      // if this flood request has been processed
    auto dstItr = dstToPathidsMap.find(vid);
    bool dstHasMe = (dstItr != dstToPathidsMap.end());
    // check if floodSeqnum is larger than previous one from src, i.e. if I've processed this flood request
    auto recentFloodItr = recentReqFloodFrom.find(srcVid);
    if (recentFloodItr != recentReqFloodFrom.end()) {       // if I've recently received flood request from src
        if (floodSeqnum <= recentFloodItr->second.first) {
            // NOTE flood request whose dst contains myself may still be flooded to pneis
            if (dstHasMe && floodSeqnum == recentFloodItr->second.first) { // dst received flood request from src that has been processed, probably via another trace, see if I can repair any brokenPathids with this trace bc the previous trace may be broken
                reqProcessed = true;
            } else {
                EV_INFO << "Recently received RepairLinkReqFlood with flood seqnum = " << floodSeqnum << " from src = " << srcVid << ", ignoring this flood request" << endl;

                if (recordStatsToFile && recordDroppedMsg && !pktRecorded) {   // record dropped message
                    recordMessageRecord(/*action=*/4, /*src=*/srcVid, /*dst=*/vid, "RepairLinkReqFlood", /*msgId=*/floodSeqnum, /*hopcount=*/reqIncoming->getLinkTrace().size()+1, /*chunkByteLength=*/reqIncoming->getByteLength(), /*infoStr=*/"flood already received");
                    pktRecorded = true;
                }
                return;
            }
        }
    } else {    // add src in recentReqFloodFrom
        auto itr_bool = recentReqFloodFrom.insert({srcVid, {}});  // std::pair<iterator, bool>: iterator (to the inserted element), bool (whether the element is inserted)
        recentFloodItr = itr_bool.first;
    }
    if (!reqProcessed) {
        // update floodSeqnum of src in recentReqFloodFrom
        recentFloodItr->second.first = floodSeqnum;
        recentFloodItr->second.second = simTime() + recentReqFloodExpiration;
    }
    ASSERT(std::find(reqIncoming->getLinkTrace().begin(), reqIncoming->getLinkTrace().end(), vid) == reqIncoming->getLinkTrace().end());   // my vid shouldn't be in linkTrace bc I would remove myself from dstToPathidsMap when forwarding it for the first time
    if (dstHasMe) {
        pktForMe = true;
        // const L3Address& srcAddr = reqIncoming->getSrcAddress();
        // std::vector<VlrPathID> brokenPathids (dstItr->second.begin(), dstItr->second.end());    // convert set of VlrPathID (dstItr->second) to vector of VlrPathID (brokenPathids)
        std::set<VlrPathID> brokenPathids;
        auto reqInfoItr = delayedRepairLinkReq.find(srcVid);
        if (reqInfoItr != delayedRepairLinkReq.end()) {
            // get brokenPathids in dstItr->second that are not already in delayedRepairLinkReq[srcVid], add to brokenPathids
            std::set_difference(dstItr->second.begin(), dstItr->second.end(), reqInfoItr->second.brokenPathids.begin(), reqInfoItr->second.brokenPathids.end(), std::inserter(brokenPathids, brokenPathids.end()));
        } else {
            brokenPathids = dstItr->second;
        }
        EV_INFO << "Handling RepairLinkReqFlood to me: src = " << srcVid << ", trace = " << reqIncoming->getLinkTrace() << ", brokenPathids = " << brokenPathids << endl;
        // linkTrace (I haven't added myself) is [src], meaning src and I are still pneis, no repair is needed, if src in lostPneis, will remove it in handleLinkedPnei()

        std::set<VlrPathID> delayedBrokenPathids;  // pathids (from brokenPathids) found in vlrRoutingTable but no nexthop breakage detected yet

        processRepairLinkReqInfo(srcVid, brokenPathids, reqIncoming->getLinkTrace(), delayedBrokenPathids);
        
        if (!delayedBrokenPathids.empty()) {
            auto reqInfoItr = delayedRepairLinkReq.find(srcVid);
            if (reqInfoItr == delayedRepairLinkReq.end()) {    // src not yet in delayedRepairLinkReq
                auto itr_bool = delayedRepairLinkReq.insert({srcVid, {delayedBrokenPathids, reqIncoming->getLinkTrace(), simTime() + delayedRepairLinkReqExpireTimeCoefficient * beaconInterval}});  // std::pair<iterator, bool>: iterator (to the inserted element), bool (whether the element is inserted)
                reqInfoItr = itr_bool.first;
            } else {    // src already in delayedRepairLinkReq
                reqInfoItr->second.brokenPathids.insert(delayedBrokenPathids.begin(), delayedBrokenPathids.end());
            }
        }
        
    }
    if (reqProcessed) {
        EV_INFO << "Already processed RepairLinkReqFlood with from flood ttl = " << floodTtl << " from src = " << srcVid << ", reqProcessed=true, dropping this flood request" << endl;

        if (recordStatsToFile && recordDroppedMsg && !pktRecorded) {   // record dropped message
            recordMessageRecord(/*action=*/4, /*src=*/srcVid, /*dst=*/vid, "RepairLinkReqFlood", /*msgId=*/floodSeqnum, /*hopcount=*/reqIncoming->getLinkTrace().size()+1, /*chunkByteLength=*/reqIncoming->getByteLength(), /*infoStr=*/"flood already received");
            pktRecorded = true;
        }
        return;
    }
    if (floodTtl <= 1) {
        EV_INFO << "Received RepairLinkReqFlood with from flood ttl = " << floodTtl << " from src = " << srcVid << ", dropping this flood request" << endl;
        // return;      // proceed to utilize linkTrace
    }
    else {  // broadcast RepairLinkReqFlood
        // NOTE I broadcast even if dstHasMe bc dstToPathidsMap has multiple dst, even if I've issued RepairLinkReply for brokenPathids in dstToPathidsMap[me], I'm not sure if there are other brokenPathids that can only be repaired by other dst
        VlrIntVidToPathidSetMap& dstToPathidsMapForUpdate = reqIncoming->getDstToPathidsMapForUpdate();
        dstToPathidsMapForUpdate.erase(vid);    // remove myself from dstToPathidsMap since I've processed RepairLinkReq for myself, now just need to broadcast to other dst
        if (!dstToPathidsMapForUpdate.empty()) {    // there're more dst in this RepairLinkReqFlood
            // auto reqOutgoing = staticPtrCast<RepairLinkReqFloodInt>(reqIncoming->dupShared());
            auto reqOutgoing = reqIncoming;
            reqOutgoing->getLinkTraceForUpdate().push_back(vid);  // put myself in linkTrace
            reqOutgoing->setTtl(floodTtl-1);    // decrement ttl
            // Commented out bc multiple dst contained in this RepairLinkReqFlood
            // double maxFloodJitterToDst = (psetTable.pneiIsLinked(dstVid)) ? (maxFloodJitter/8) : maxFloodJitter;    // reduce forward delay if dst is linked pnei
            double maxFloodJitterToDst = maxFloodJitter;
            sendCreatedRepairLinkReqFlood(reqOutgoing, /*computeChunkLength=*/true, /*delay=*/uniform(0, maxFloodJitterToDst));
            pktForwarded = true;
        }
    }

    // utilize msg trace to record path to nodes close to me
    if (checkOverHeardTraces && reqIncoming->getLinkTrace().size() >= overHeardTraceMinCheckLen) {
        // trace: [src, .., node before me]
        addCloseNodesFromTrace(/*numHalf=*/vsetAndPendingHalfCardinality, reqIncoming->getLinkTrace(), /*removeLoop=*/false, /*traceEndIndex=*/reqIncoming->getLinkTrace().size()-1);
    } else if (checkOverHeardTraces) {
        // see if src of msg belongs to pendingVset
        pendingVsetAdd(srcVid, /*heardFrom=*/VLRRINGVID_NULL, /*addToEmpty=*/false);
    }
    // Commented out bc aren't sure if nodes in linkTrace satisfies greedy property
    // if (checkOverHeardMPneis && reqIncoming->getLinkTrace().size() >= 2) {
    //     const VlrIntVidVec& trace = reqIncoming->getLinkTrace();
    //     const VlrRingVID& prevhopVid = trace.back();
    //     if (psetTable.pneiIsLinked(prevhopVid)) {
    //         // trace: [src, .., node before me], go over every node in trace except node just before me
    //         for (int i = 0; i < trace.size()-1; i++)
    //             overheardMPneis[trace[i]] = std::make_pair(prevhopVid, simTime() + neighborValidityInterval);   // initialize expireTime of overheard trace to node
    //     }
    // }
    if (recordStatsToFile && recordDroppedMsg && !pktRecorded) {   // record message
        if (pktForMe)
            recordMessageRecord(/*action=*/1, /*src=*/srcVid, /*dst=*/vid, "RepairLinkReqFlood", /*msgId=*/floodSeqnum, /*hopcount=*/reqIncoming->getLinkTrace().size()+1, /*chunkByteLength=*/reqIncoming->getByteLength());
        else if (!pktForwarded)
            recordMessageRecord(/*action=*/4, /*src=*/srcVid, /*dst=*/vid, "RepairLinkReqFlood", /*msgId=*/floodSeqnum, /*hopcount=*/reqIncoming->getLinkTrace().size()+1, /*chunkByteLength=*/reqIncoming->getByteLength());
    }
}

// set corresponding prevhop/nexthop of each vroute in lostPneiItr->second.brokenVroutes to unavailable if setBroken=true, or available w/ temporary route if setBroken=false
// if vroute has become available, remove it from lostPneiItr->second.brokenVroutes
// if std::vector<VlrPathID> *sendRepairRouteVroutesPtr given (not nullptr), add vroutes whose prevhop==lostPnei and also vroutes whose prevhop/nexthop state was updated to patched (01 -> 10) in this function
void Vlr::setLostPneiBrokenVroutes(std::map<VlrRingVID, LostPneiInfo>::iterator lostPneiItr, bool setBroken, std::vector<VlrPathID> *sendRepairRouteVroutesPtr/*=nullptr*/)
{
    char hopState = (setBroken) ? 1 : 2;    // 01 (unavailable) or 10 (using temp route)

    if (lostPneiItr != lostPneis.end()) {
        for (auto brokenItr = lostPneiItr->second.brokenVroutes.begin(); brokenItr != lostPneiItr->second.brokenVroutes.end(); ) {
            const VlrPathID& brokenPathid = *brokenItr;
            auto vrouteItr = vlrRoutingTable.vlrRoutesMap.find(brokenPathid);
            if (vrouteItr != vlrRoutingTable.vlrRoutesMap.end()) {  // pathid still in vlrRoutingTable
                if (vrouteItr->second.isUnavailable == 0 || vlrRoutingTable.getIsDismantledRoute(vrouteItr->second.isUnavailable))  // broken vroute is no longer broken somehow  NOTE temporary route shouldn't be in brokenVroutes
                    lostPneiItr->second.brokenVroutes.erase(brokenItr++);
                // one of both of prevhop and nexthop is a lost pnei
                else if (lostPneiItr->first == vrouteItr->second.prevhopVid) { // route prevhop == lost pnei
                    // if (updatedVroutesPtr && vlrRoutingTable.getPrevNextIsUnavailable(vrouteItr->second.isUnavailable, /*getPrev=*/true) == hopState)    // route prevhop state doesn't need to be changed
                    //     updatedVroutesPtr->push_back(brokenPathid);
                    if (sendRepairRouteVroutesPtr)
                        sendRepairRouteVroutesPtr->push_back(brokenPathid);

                    vlrRoutingTable.setRouteItrPrevNextIsUnavailable(vrouteItr, /*setPrev=*/true, /*value=*/hopState);     // set route prevhop state
                    if (setBroken)
                        vlrRoutingTable.removeRouteEndFromEndpointMap(brokenPathid, vrouteItr->second.fromVid);     // remove route fromVid in endpointToRoutesMap
                    else 
                        vlrRoutingTable.addRouteEndInEndpointMap(brokenPathid, vrouteItr->second.fromVid);     // add back route fromVid in endpointToRoutesMap
                    
                    brokenItr++;
                } else { // route nexthop == lost pnei
                    if (sendRepairRouteVroutesPtr && !setBroken && vlrRoutingTable.getPrevNextIsUnavailable(vrouteItr->second.isUnavailable, /*getPrev=*/false) != hopState)    // route nexthop state needs to be changed
                        sendRepairRouteVroutesPtr->push_back(brokenPathid);

                    vlrRoutingTable.setRouteItrPrevNextIsUnavailable(vrouteItr, /*setPrev=*/false, /*value=*/hopState);     // set route nexthop state
                    if (setBroken)
                        vlrRoutingTable.removeRouteEndFromEndpointMap(brokenPathid, vrouteItr->second.toVid);     // remove route toVid in endpointToRoutesMap
                    else 
                        vlrRoutingTable.addRouteEndInEndpointMap(brokenPathid, vrouteItr->second.toVid);     // add back route toVid in endpointToRoutesMap
                    
                    brokenItr++;
                }
            } else  // broken vroute no longer in vlrRoutingTable somehow
                lostPneiItr->second.brokenVroutes.erase(brokenItr++);
        }
    }
}

// process all repairLinkReqInfo in delayedRepairLinkReq after new breakage detected
void Vlr::processDelayedRepairLinkReq()
{
    for (auto reqInfoItr = delayedRepairLinkReq.begin(); reqInfoItr != delayedRepairLinkReq.end(); ) {
        if (reqInfoItr->second.expireTime <= simTime())    // repairLinkReqInfo has expired
            delayedRepairLinkReq.erase(reqInfoItr++);     // step1: it_tobe = it+1; step2: erase(it); step3: it = it_tobe;
        else {  // process repairLinkReqInfo as if I just received the repairLinkReq from src
            std::set<VlrPathID> delayedBrokenPathids;  // pathids (from brokenPathids) whose nexthop still not broken yet
            processRepairLinkReqInfo(/*srcVid=*/reqInfoItr->first, reqInfoItr->second.brokenPathids, reqInfoItr->second.linkTrace, delayedBrokenPathids);
            
            if (!delayedBrokenPathids.empty()) {    // these pathids should remain in delayedRepairLinkReq[src] in case their nexthop breakage isn't detected yet
                reqInfoItr->second.brokenPathids = delayedBrokenPathids;
                reqInfoItr++;
            } else {    // all brokenPathids received in this repairLinkReq are processed, no need to keep this repairLinkReqInfo
                delayedRepairLinkReq.erase(reqInfoItr++);
                // continue;
            }
        }
    }
}

// delayedBrokenPathids: pathids (from brokenPathids) found in vlrRoutingTable but no nexthop breakage detected yet, will be added to delayedRepairLinkReq if this function is called from processRepairLinkReqFlood()
void Vlr::processRepairLinkReqInfo(const VlrRingVID& srcVid, const std::set<VlrPathID>& brokenPathids, const VlrIntVidVec& linkTrace, std::set<VlrPathID>& delayedBrokenPathids)
{
    std::vector<VlrPathID> replyBrokenPathids;  // brokenPathids that are indeed broken (nexthop unavailable), will be added to the RepairLinkReply sent to src
    std::vector<VlrPathID> srcBrokenPathids;    // brokenPathids that are newly associated with lostPneis[src], need to be added to lostPneis[src].brokenVroutes
    std::map<VlrPathID, VlrRingVID> srcBrokenPathidOldNexthopMap;   // brokenPathids that are newly associated with lostPneis[src], and their old nexthop

    EV_WARN << "processRepairLinkReqInfo to me=" << vid << ": src = " << srcVid << ", trace = " << linkTrace << ", brokenPathids = " << brokenPathids << endl;
    // check if brokenPathids are indeed broken (nexthop unavailable)
    for (const VlrPathID& brokenPathid : brokenPathids) {
        auto vrouteItr = vlrRoutingTable.vlrRoutesMap.find(brokenPathid);
        if (vrouteItr != vlrRoutingTable.vlrRoutesMap.end()) {  // pathid still in vlrRoutingTable
            if (vlrRoutingTable.getPrevNextIsUnavailable(vrouteItr->second.isUnavailable, /*getPrev=*/false) == 1) { // route nexthop is unavailable
                replyBrokenPathids.push_back(brokenPathid);
                // set route nexthop to srcAddr
                if (vrouteItr->second.nexthopVid != srcVid) {     // current nexthop (lost pnei) isn't src, probably current nexthop has failed, associate this broken vroute with src as lost pnei
                    VlrRingVID nexthopVid = vrouteItr->second.nexthopVid;    // map nexthopAddr L3Address (lost pnei) to VlrRingVID
                    EV_WARN << "Changing route nexthop: pathid = " << vrouteItr->first << " nexthop changed from " << nexthopVid << " to " << srcVid << endl;
                    auto lostPneiItr = lostPneis.find(nexthopVid);
                    if (lostPneiItr != lostPneis.end()) {
                        lostPneiItr->second.brokenVroutes.erase(brokenPathid);
                    }
                    vrouteItr->second.nexthopVid = srcVid;
                    srcBrokenPathids.push_back(brokenPathid);
                    srcBrokenPathidOldNexthopMap.insert({brokenPathid, nexthopVid});
                }
            } else {    // maybe lost of nexthop hasn't been detected, record repairLinkReq received
                delayedBrokenPathids.insert(brokenPathid);
            }
        }
    }

    if (!replyBrokenPathids.empty()) {  // there exists broken vroutes (nexthop unavailable) that can be repaired by temporary route to src
        // if replyBrokenPathids aren't newly associated with lostPneis[src] in srcBrokenPathids, they should already be in lostPneis[src].brokenVroutes
        auto lostPneiItr = lostPneis.find(srcVid);
        if (lostPneiItr == lostPneis.end()) {    // src not yet in lostPneis
            // lostPneis[srcVid] = {srcAddr, {srcBrokenPathids.begin(), srcBrokenPathids.end()}, nullptr, {}};
            auto itr_bool = lostPneis.insert({srcVid, {{srcBrokenPathids.begin(), srcBrokenPathids.end()}, nullptr, {}}});  // std::pair<iterator, bool>: iterator (to the inserted element), bool (whether the element is inserted)
            lostPneiItr = itr_bool.first;
        } else {    // src already in lostPneis
            // add brokenPathids that are newly associated with src to lostPneis[src].brokenVroutes
            lostPneiItr->second.brokenVroutes.insert(srcBrokenPathids.begin(), srcBrokenPathids.end());
        }

        // send RepairLinkReply to setup temporary route to src using linkTrace
        std::vector<VlrRingVID> trace = linkTrace;   // trace: [start node, .., previous node]
        unsigned int nextHopIndex = getNextHopIndexInTrace(trace, /*preferShort=*/false);
        // trace: [start node, .., previous node]
        if (nextHopIndex >= trace.size()) {
            EV_WARN << "No next hop found to send RepairLinkReply at me = " << vid << " to src = " << srcVid << " because no node in linkTrace " << trace << " is a LINKED pnei, dropping RepairLinkReq" << endl;
            if (lostPneiItr->second.timer == nullptr) {   // src was just added to lostPneis, schedule WaitRepairLinkIntTimer to tear down broken vroutes if they aren't repaired in time
                // vroute nexthop unavailable for all replyBrokenPathids, hence I need to wait for repairLinkReq from the other link end rather than sending repairLinkReq
                char timerName[40] = "WaitRepairLinkIntTimer:";
                lostPneiItr->second.timer = new WaitRepairLinkIntTimer(strcat(timerName, std::to_string(srcVid).c_str()));
                lostPneiItr->second.timer->setDst(srcVid);
                lostPneiItr->second.timer->setRetryCount(repairLinkReqRetryLimit);
                double reqWaitTime = (vid > srcVid) ? (repairLinkReqWaitTime + repairLinkReqSmallEndSendDelay) : repairLinkReqWaitTime;  // if I'm the larger link end of the failed link, I wait longer bc smaller link ends wait longer before the first repairLinkReq timer rings 
                scheduleAt(simTime() + reqWaitTime, lostPneiItr->second.timer);
                EV_WARN << "Scheduling a WaitRepairLinkIntTimer at " << lostPneiItr->second.timer->getArrivalTime() << " to tear down brokenPathids that are newly associated with srcVid=" << srcVid << " of RepairLinkReq" << endl;
            }
        } else {    // next hop in linkTrace valid
            // if src was a lost pnei, lostPneis[src].brokenVroutes may include broken vroutes whose prevhop=src, other than replyBrokenPathids whose nexthop=src
            cancelAndDelete(lostPneiItr->second.timer);
            lostPneiItr->second.timer = nullptr;
            // set vroutes broken by lost pnei available with temporary route
            // std::vector<VlrPathID> brokenPrevPathids;   // broken vroutes for which I should send repairLinkReq
            // std::vector<VlrPathID> brokenNextPathids;   // broken vroutes for which I should receive repairLinkReq
            // setLostPneiBrokenVroutes(lostPneiItr, /*setBroken=*/false, /*brokenPrevPtr=*/&brokenPrevPathids, /*brokenNextPtr=*/&brokenNextPathids);
            setLostPneiBrokenVroutes(lostPneiItr, /*setBroken=*/false);
            
            EV_WARN << "setLostPneiBrokenVroutes(setBroken=false) of RepairLinkReq srcVid = " << srcVid << " brokenVroutes = [";
            for (const auto& brokenVroute : lostPneiItr->second.brokenVroutes)
                EV_WARN << brokenVroute << " ";
            EV_WARN << "]" << endl;

            if (!lostPneiItr->second.brokenVroutes.empty()) {   // there are vroutes broken by lost pnei src
                // if (!lostPneiItr->second.tempVlinks.empty()) {  // there is an existing temporary route to lost pnei src
                //     if (reqProcessed && trace.size() >= lostPneiItr->second.tempVlinkHopcount-2) {    // the new trace from src is longer than than existing temporary route
                //         // NOTE reqIncoming->getLinkTrace().size() doesn't include me yet
                //         EV_INFO << "Recently processed RepairLinkReqFlood to me with flood seqnum = " << floodSeqnum << " from src = " << srcVid << ", trace length isn't much shorter than previous one, ignoring this flood request" << endl;
                //         return;
                //     }
                //     // if reqProcessed=false, since src has sent a new repairLinkReq that I haven't processed, existing temporary route to src is probably broken
                //     // if reqProcessed=true, new trace from src is shorter than existing temporary route, put existing one in nonEssRoutes and setup a new one
                //     auto tempVlinkItr = lostPneiItr->second.tempVlinks.begin();
                //     nonEssRoutes[*tempVlinkItr] = (reqProcessed) ? (simTime() + nonEssRouteExpiration) : simTime();    // put the existing temporary route in nonEssRoutes to tear down later
                //     lostPneiItr->second.tempVlinks.erase(tempVlinkItr);
                // }
                // send RepairLinkReply to src using linkTrace to setup temporary route
                VlrRingVID nextHopVid = trace[nextHopIndex];
                bool addedRoute = false;
                VlrPathID newPathid;
                do {    // loop in case generated newPathid isn't unique in my vlrRoutingTable
                    newPathid = genPathID(srcVid);
                    addedRoute = vlrRoutingTable.addRoute(newPathid, vid, srcVid, /*prevhopVid=*/VLRRINGVID_NULL, /*nexthopVid=*/nextHopVid, /*isVsetRoute=*/false, /*isUnavailable=*/16).second;
                } while (!addedRoute);
                RepairLinkReplyInt *replyOutgoing = new RepairLinkReplyInt(/*name=*/"RepairLinkReply");
                replyOutgoing->setSrc(vid);
                // replyOutgoing->setSrcAddress(getSelfAddress());
                replyOutgoing->setBrokenPathids(replyBrokenPathids);
                // replyOutgoing->setBrokenPathids(brokenNextPathids);
                // replyOutgoing->setBrokenPathids2(brokenPrevPathids);
                replyOutgoing->setTempPathid(newPathid);
                replyOutgoing->setLinkTrace(trace);
                replyOutgoing->setHopcount(0);
                replyOutgoing->setMessageId(++allSendMessageId);
                EV_INFO << "Sending RepairLinkReply to src " << srcVid << " of RepairLinkReq, pathid = " << newPathid << ", linkTrace = " << replyOutgoing->getLinkTrace() << ", brokenPathids (nexthop=src) = " << replyOutgoing->getBrokenPathids() 
                        << ", nexthop: " << nextHopVid << endl;
                sendCreatedRepairLinkReply(replyOutgoing, /*outGateIndex=*/psetTable.getPneiGateIndex(nextHopVid), /*computeChunkLength=*/true);

                if (recordStatsToFile) {   // record sent message
                    recordMessageRecord(/*action=*/0, /*src=*/vid, /*dst=*/srcVid, "RepairLinkReply", /*msgId=*/0, /*hopcount=*/0, /*chunkByteLength=*/0);   // unused params (msgId, hopcount, chunkByteLength) are just assigned to 0
                }

                // record newly created temporary route pathid in lostPneis[src]
                lostPneiItr->second.tempVlinks.insert(newPathid);
                // lostPneiItr->second.tempVlinkHopcount = trace.size() + 1;   // trace doesn't contain me, but tempVlinkHopcount should include me

                // record newly created temporary route and srcBrokenPathids in case it gets torn down before reaching the otherEnd
                if (!srcBrokenPathidOldNexthopMap.empty())
                    recentTempRouteSent.insert({newPathid, {srcBrokenPathidOldNexthopMap, simTime() + 0.2 * trace.size()}});

                if (checkOverHeardTraces)
                    // see if src of msg belongs to pendingVset
                    pendingVsetAdd(srcVid, /*heardFrom=*/VLRRINGVID_NULL, /*addToEmpty=*/false);

            } else {  // no broken vroutes still require a temporary route to lost pnei towardVid
                EV_WARN << "SrcVid = " << srcVid << " of repairLinkReq found in lostPneis but no brokenVroutes, removing srcVid from lostPneis, tempVlinks = ";
                for (const auto& vlinkid : lostPneiItr->second.tempVlinks)
                    EV_WARN << vlinkid << " ";
                EV_WARN << "]" << endl;
                // put temporary vlinks to lost pnei in nonEssRoutes as they are no longer necessary for broken vroutes
                // tear down the temporary route to notify the lost pnei of the removal of these broken vroutes
                delayLostPneiTempVlinksTeardown(lostPneiItr->second.tempVlinks, /*delay=*/0);

                lostPneis.erase(lostPneiItr);
            }
        }
    }
}

int Vlr::computeRepairLinkReplyByteLength(RepairLinkReplyInt* msg) const
{
    // unsigned int src;
    int chunkByteLength = VLRRINGVID_BYTELEN;
    // // L3Address srcAddress;
    // chunkByteLength += addressByteLength;
    // std::vector<VlrPathID> brokenPathids
    chunkByteLength += msg->getBrokenPathids().size() * VLRPATHID_BYTELEN;
    // // std::vector<VlrPathID> brokenPathids2
    // chunkByteLength += msg->getBrokenPathids2().size() * VLRPATHID_BYTELEN;
    // VlrPathID tempPathid;
    chunkByteLength += VLRPATHID_BYTELEN;
    // std::vector<unsigned int> linkTrace
    chunkByteLength += msg->getLinkTrace().size() * VLRRINGVID_BYTELEN;

    return chunkByteLength;
}

// if computeChunkLength = true, compute chunk length because chunk was just created with createSetupReply() or modified after dupShared() from another chunk
// else, no need to compute chunk length because chunk was dupShared() (not modified) from another chunk that has chunkLength set
void Vlr::sendCreatedRepairLinkReply(RepairLinkReplyInt *msg, const int& outGateIndex, bool computeChunkLength/*=true*/, double delay/*=0*/)
{
    if (computeChunkLength)
        msg->setByteLength(computeRepairLinkReplyByteLength(msg));
    EV_INFO << "Sending repairLinkReply: src = " << msg->getSrc() << ", tempPathid = " << msg->getTempPathid() << ", linkTrace = " << msg->getLinkTrace() << endl;

    msg->getVlrOptionForUpdate().setPrevHopVid(vid);    // set packet prevHopVid to myself
    msg->setHopcount(msg->getHopcount() +1);    // increment packet hopcount
    
    // NOTE addTag should be executed after chunkLength has been set, and chunkLength shouldn't be changed before findTag/getTag

    // all multihop VLR packets (setupReq, setupReply, etc) L3 dst are set to a pnei, greedy routing at L3 in routeDatagram() isn't needed, but we do greedy routing and deal with VlrOption at L4 (in processSetupReq() for example) 
    // udpPacket->addTagIfAbsent<VlrIntOptionReq>()->setVlrOption(vlrOption);      // VlrOption to be set in IP header in datagramLocalOutHook()
    
    sendCreatedPacket(msg, /*unicast=*/true, /*outGateIndex=*/outGateIndex, /*delay=*/delay, /*checkFail=*/true);
}

void Vlr::processRepairLinkReply(RepairLinkReplyInt *replyIncoming, bool& pktForwarded)
{
    EV_DEBUG << "Received RepairLinkReply" << endl;
    // // VlrIntOption *vlrOptionIn = nullptr;
    // VlrIntOption& vlrOptionIn = replyIncoming->getVlrOptionForUpdate();
    VlrRingVID msgPrevHopVid = replyIncoming->getVlrOption().getPrevHopVid();
    if (msgPrevHopVid == VLRRINGVID_NULL)
        throw cRuntimeError("Received RepairLinkReply with vlrOption.prevHopVid = null");
        
    VlrRingVID srcVid = replyIncoming->getSrc();
    const VlrPathID& newPathid = replyIncoming->getTempPathid();
    VlrRingVID dstVid = replyIncoming->getLinkTrace().at(0);

    EV_INFO << "Processing RepairLinkReply: src = " << srcVid << ", dstVid = " << dstVid << ", tempPathid = " << newPathid << ", prevhop: " << msgPrevHopVid << endl;

    if (recordStatsToFile && recordReceivedMsg) {   // record received message
        recordMessageRecord(/*action=*/2, /*src=*/srcVid, /*dst=*/dstVid, "RepairLinkReply", /*msgId=*/newPathid, /*hopcount=*/replyIncoming->getHopcount()+1, /*chunkByteLength=*/replyIncoming->getByteLength());    // unimportant params (msgId)
    }
    bool pktForMe = false;      // set to true if this msg is directed to me or I processed it as its dst
    bool pktRecorded = false;      // set to true if this msg is recorded with recordMessageRecord()

    auto vrouteItr = vlrRoutingTable.vlrRoutesMap.find(newPathid);
    if (vrouteItr != vlrRoutingTable.vlrRoutesMap.end()) {  // newPathid already in vlrRoutingTable
        EV_WARN << "The new temporary pathid " << newPathid << " of repairLinkReply is already in my routing table, tearing down the new path" << endl;
        // pathid duplicate, something is wrong, can be a loop, if indeed a loop, I send Teardown to prevHopAddr and Teardown will reach me again to remove the existing erroneous vlrRoutingTable record of newPathid
        // tear down newPathid, i.e. send Teardown to prevHopAddr
        TeardownInt* teardownOut = createTeardownOnePathid(newPathid, /*addSrcVset=*/false, /*rebuild=*/true);
        sendCreatedTeardown(teardownOut, /*nextHopPnei=*/msgPrevHopVid);

        if (recordStatsToFile) {   // record sent message
            recordMessageRecord(/*action=*/0, /*src=*/vid, /*dst=*/VLRRINGVID_NULL, "Teardown", /*msgId=*/0, /*hopcount=*/0, /*chunkByteLength=*/0, /*infoStr=*/"repairLinkReply: new path already in routing table");   // unused params (msgId, hopcount, chunkByteLength) are just assigned to 0
        }
    }
    else {    // checked newPathid not in vlrRoutingTable
        // VlrRingVID prevHopVid = getVidFromAddressInRegistry(prevHopAddr);    // map prevHopAddr L3Address to VlrRingVID
        if (!psetTable.pneiIsLinked(msgPrevHopVid)) {        // if prevHopAddr isn't a LINKED pnei
            EV_WARN << "Previous hop " << msgPrevHopVid << " of repairLinkReply is not a LINKED pnei, tearing down pathid " << newPathid << endl;
            // tear down newPathid, i.e. send Teardown to prevHopAddr
            TeardownInt* teardownOut = createTeardownOnePathid(newPathid, /*addSrcVset=*/false, /*rebuild=*/true);
            sendCreatedTeardown(teardownOut, /*nextHopPnei=*/msgPrevHopVid);

            if (recordStatsToFile) {   // record sent message
                recordMessageRecord(/*action=*/0, /*src=*/vid, /*dst=*/VLRRINGVID_NULL, "Teardown", /*msgId=*/0, /*hopcount=*/0, /*chunkByteLength=*/0, /*infoStr=*/"repairLinkReply: previous hop not a LINKED pnei");   // unused params (msgId, hopcount, chunkByteLength) are just assigned to 0
            }
        }
        // checked this message received from a LINKED pnei
        else if (representativeFixed && representative.heardfromvid == VLRRINGVID_NULL) {        // if I don't have a valid rep yet, I won't process or forward this message
            EV_WARN << "No valid rep heard: " << representative << ", cannot accept overlay message, tearing down pathid " << newPathid << endl;
            // tear down newPathid, i.e. send Teardown to prevHopAddr
            TeardownInt* teardownOut = createTeardownOnePathid(newPathid, /*addSrcVset=*/false, /*rebuild=*/true);
            sendCreatedTeardown(teardownOut, /*nextHopPnei=*/msgPrevHopVid);

            if (recordStatsToFile) {   // record sent message
                recordMessageRecord(/*action=*/0, /*src=*/vid, /*dst=*/VLRRINGVID_NULL, "Teardown", /*msgId=*/0, /*hopcount=*/0, /*chunkByteLength=*/0, /*infoStr=*/"repairLinkReply: no rep");   // unused params (msgId, hopcount, chunkByteLength) are just assigned to 0
            }
        }
        // checked I have a valid rep
        else if (dstVid == vid) {      // this RepairLinkReply is destined for me
            pktForMe = true;
            // const L3Address& srcAddr = replyIncoming->getSrcAddress();
            unsigned int msgHopCount = replyIncoming->getHopcount() +1;
            const std::vector<VlrPathID>& brokenPathids = replyIncoming->getBrokenPathids();    // broken vroutes that are patched by the new temporary route
            EV_WARN << "Handling repairLinkReply to me: src = " << srcVid << ", temporary pathid = " << newPathid << ", hopcount = " << msgHopCount
                    << ", brokenPathids (prevhop=src) = " << brokenPathids << endl;

            std::vector<VlrPathID> replyBrokenPathids;  // brokenPathids that are repaired by this RepairLinkReply
            std::vector<VlrPathID> srcBrokenPathids;  // brokenPathids that are newly associated with lostPneis[src]

            // brokenPathids (prevhop unavailable or already patched), if prevhop unavailable or already patched and src isn't prevhop, set prevhop to src and remove any nodes before src from prevhopVids only if but src is farther to me in prevhopVids
            for (const VlrPathID& brokenPathid : brokenPathids) {
                auto vrouteItr = vlrRoutingTable.vlrRoutesMap.find(brokenPathid);
                if (vrouteItr != vlrRoutingTable.vlrRoutesMap.end()) {  // pathid still in vlrRoutingTable
                    char prevhopIsUnavailable = vlrRoutingTable.getPrevNextIsUnavailable(vrouteItr->second.isUnavailable, /*getPrev=*/true);
                    // NOTE I receive repairLinkReply from src bc I sent repairLinkReq when prevhop of this vroute is broken (prevhopIsUnavailable = 01), now I may have received repairLinkReply from another node P in prevhopVids (prevhopIsUnavailable = 02), and I may have realized that node P is be a LINKED pnei of me (prevhopIsUnavailable = 00)
                    if (prevhopIsUnavailable == 1 || prevhopIsUnavailable == 2 || prevhopIsUnavailable == 0) { // route prevhop is unavailable or already patched
                        EV_WARN << "Checking route (from=" << vrouteItr->second.fromVid << ", to=" << vrouteItr->second.toVid << ", prevhop=" << vrouteItr->second.prevhopVid << " (isUnavailable=" << (int)prevhopIsUnavailable << "), nexthop=" << vrouteItr->second.nexthopVid << "): pathid = " << vrouteItr->first << ", prevhopVids = " << vrouteItr->second.prevhopVids << endl;
                        // if (brokenPathid == 888034778 && vid == 49308)
                        //     EV_INFO << "ohno";
                        // set route prevhop to srcAddr
                        if (vrouteItr->second.prevhopVid != srcVid) {     // current prevhop (lost pnei) isn't src, probably current prevhop has failed, associate this broken vroute with src as lost pnei
                            // VlrRingVID lostPneiVid = getVidFromAddressInRegistry(vrouteItr->second.prevhopAddr);    // map current prevhopAddr L3Address (lost pnei) to VlrRingVID
                            VlrRingVID lostPneiVid = vrouteItr->second.prevhopVid;    // map current prevhopAddr L3Address (lost pnei) to VlrRingVID
                            unsigned int lostPneiIndex = 0, srcIndex = 0;
                            std::vector<VlrRingVID>& routePrevhopVids = vrouteItr->second.prevhopVids;
                            if (vrouteItr->second.fromVid == srcVid) {
                                if (std::find(routePrevhopVids.begin(), routePrevhopVids.end(), srcVid) == routePrevhopVids.end())  // repairLinkReply sent by fromVid, but fromVid isn't in routePrevhopVids
                                    routePrevhopVids.push_back(srcVid);     // srcVid will replace current prevhopVid for sure bc it's the farthest in routePrevhopVids, then every node will be removed from routePrevhopVids except srcVid, hence routePrevhopVids won't be oversized
                            }
                            for (unsigned int i = 0; i < routePrevhopVids.size(); i++) {
                                if (routePrevhopVids[i] == lostPneiVid)
                                    lostPneiIndex = i;
                                else if (routePrevhopVids[i] == srcVid)
                                    srcIndex = i;
                            }
                            if (srcIndex > lostPneiIndex) {     // src is farther to me than current prevhop, update prevhop to src
                                auto lostPneiItr = lostPneis.find(lostPneiVid);
                                if (lostPneiItr != lostPneis.end()) {
                                    EV_INFO << "srcVid=" << srcVid << " may replace current prevhop=" << lostPneiVid << "), removing brokenPathid=" << brokenPathid << " from lostPneis[prevhop].brokenVroutes=" << lostPneiItr->second.brokenVroutes << endl;
                                    lostPneiItr->second.brokenVroutes.erase(brokenPathid);
                                }
                                if (prevhopIsUnavailable == 0) {     // current vrouteItr->second.prevhopVid is a LINKED pnei of me, since I'm changing prevhopVid to srcVid (using temp route), set prevhopIsUnavailable to 2, otherwise setLostPneiBrokenVroutes() ignores non-broken vroutes
                                    vlrRoutingTable.setRouteItrPrevNextIsUnavailable(vrouteItr, /*setPrev=*/true, /*value=*/2);     // set route nexthop state
                                    char prevhopIsUnavailable_new = vlrRoutingTable.getPrevNextIsUnavailable(vrouteItr->second.isUnavailable, /*getPrev=*/true);
                                    EV_INFO << "srcVid=" << srcVid << " may replace current prevhop=" << lostPneiVid << "), pathid=" << vrouteItr->first << " is updated: prevhop isUnavailable=" << (int)prevhopIsUnavailable_new << endl;
                                }
                                vrouteItr->second.prevhopVid = srcVid;
                                srcBrokenPathids.push_back(brokenPathid);
                                replyBrokenPathids.push_back(brokenPathid);     // since I received this repairLinkReply, current nexthop at src is me, and now I updated my prevhop (lost pnei) to src, consistent
                                // remove from prevhopVids any nodes before src, as prevhop is now set to src
                                routePrevhopVids.erase(routePrevhopVids.begin(), routePrevhopVids.begin()+srcIndex);
                                ASSERT(routePrevhopVids.size() <= routePrevhopVidsSize);
                                EV_INFO << "Updated route prevhop to " << vrouteItr->second.prevhopVid << ", pathid = " << vrouteItr->first << ", prevhopVids = " << vrouteItr->second.prevhopVids << endl;
                            }
                        } else {    // current prevhop (lost pnei) is src, and since I received this repairLinkReply, current nexthop at src is me, consistent
                            replyBrokenPathids.push_back(brokenPathid);
                        }
                    }
                }
            }

            if (!replyBrokenPathids.empty()) { // some broken vroutes that src wants to repair with this RepairLinkReply are repaired (prevhop was broken) or updated (prevhop changed to src) or remain consistent (prevhop was src)

                auto lostPneiItr = lostPneis.find(srcVid);
                if (!srcBrokenPathids.empty()) {    // there are brokenPathids newly associated with lostPneis[src]
                    if (lostPneiItr == lostPneis.end()) {    // src not yet in lostPneis
                        auto itr_bool = lostPneis.insert({srcVid, {{srcBrokenPathids.begin(), srcBrokenPathids.end()}, nullptr, {}}});  // std::pair<iterator, bool>: iterator (to the inserted element), bool (whether the element is inserted)
                        lostPneiItr = itr_bool.first;
                    } else {    // src already in lostPneis
                        // add brokenPathids that are newly associated with src to lostPneis[src].brokenVroutes
                        lostPneiItr->second.brokenVroutes.insert(srcBrokenPathids.begin(), srcBrokenPathids.end());
                    }
                }
                if (lostPneiItr != lostPneis.end()) {
                    cancelAndDelete(lostPneiItr->second.timer);
                    lostPneiItr->second.timer = nullptr;

                    // // add brokenPathids that are newly associated with src to lostPneis[src].brokenVroutes
                    // lostPneiItr->second.brokenVroutes.insert(srcBrokenPathids.begin(), srcBrokenPathids.end());
                    
                    // set vroutes broken by lost pnei available with temporary route
                    // NOTE if two link ends send repairLinkReq then receive repairLinkReq from each other, they would both setup a temporary route toward the other end and set all brokenPathids to patched, then when they receive repairLinkReply from the other end, they won't send repairRoute bc brokenPathids were already patched
                    // I'll send RepairRoute to vroutes whose prevhop is src (I should send repairLinkReq and receive repairLinkReply) and also vroutes whose prevhop/nexthop status is changed (from broken to patched) after setting src available with temporary route
                        // then even if (in case of link failure) both ends have sent out repairLinkReply and set every brokenVroute to patched, both ends will send out RepairRoute for brokenVroutes whose prevhop==lostPnei; (in case of node failure) both ends have sent out repairLinkReply and set brokenVroute whose nexthop==lostPnei to patched, both ends will still send out RepairRoute only for brokenVroutes whose prevhop==lostPnei
                    std::vector<VlrPathID> sendRepairRouteVroutes;  // vroutes for which I'll send RepairRoute
                    setLostPneiBrokenVroutes(lostPneiItr, /*setBroken=*/false, /*updatedVroutesPtr=*/&sendRepairRouteVroutes);
                    lostPneiItr->second.tempVlinks.insert(newPathid);
                    // add new temporary route to vlrRoutingTable
                    vlrRoutingTable.addRoute(newPathid, srcVid, vid, /*prevhopVid=*/msgPrevHopVid, /*nexthopVid=*/VLRRINGVID_NULL, /*isVsetRoute=*/false, /*isUnavailable=*/16);     // set temporary route

                    EV_WARN << "setLostPneiBrokenVroutes(setBroken=false) of RepairLinkReply srcVid = " << srcVid << " brokenVroutes = [";
                    for (const auto& brokenVroute : lostPneiItr->second.brokenVroutes)
                        EV_WARN << brokenVroute << " ";
                    EV_WARN << "]" << endl;

                    if (checkOverHeardTraces)
                        // see if src of msg belongs to pendingVset
                        pendingVsetAdd(srcVid, /*heardFrom=*/VLRRINGVID_NULL, /*addToEmpty=*/false);

                    if (recordStatsToFile) {   // record received repairLinkReply for me that indeed adds newPathid to vlrRoutingTable
                        recordMessageRecord(/*action=*/1, /*src=*/srcVid, /*dst=*/vid, "RepairLinkReply", /*msgId=*/newPathid, /*hopcount=*/msgHopCount, /*chunkByteLength=*/replyIncoming->getByteLength());   // unimportant params (msgId)
                        pktRecorded = true;
                    }

                    // for every broken vroute, send RepairRoute to the reachable endpoint to remove loop in broken but now patched vroutes
                    if (!lostPneiItr->second.brokenVroutes.empty()) {   // there are vroutes broken by lost pnei src
                        // if (lostPneiItr->second.tempVlinks.size() == 1) {   // only send RepairRoute for brokenVroutes on receipt of first temporary route from src
                        // only send RepairRoute if patching with temporary route may result in loop in brokenVroutes, if temporary route is a direct link to src, i.e. src is my pnei, there won't be loop, lostPneis[src] will be removed in handleLinkedPnei()
                        if (msgPrevHopVid != srcVid) {   // if previous hop of temporary route is src, this temporary route is a direct link to src, bc src can't appear twice in temporary route (loop-free)
                            std::map<VlrRingVID, std::pair<std::vector<VlrPathID>, char>> nextHopToPathidsMap;    // for RepairRoute to send, map next hop address to (pathids, next hop 2-bit isUnavailable) pair
                            for (const auto& oldPathid : sendRepairRouteVroutes) {
                                auto vrouteItr = vlrRoutingTable.vlrRoutesMap.find(oldPathid);
                                if (vrouteItr != vlrRoutingTable.vlrRoutesMap.end()) {  // pathid still in vlrRoutingTable
                                    // NOTE available or dismantled vroute should have been removed from brokenVroutes in setLostPneiBrokenVroutes()
                                    // send RepairRoute toward the endpoint not affected by the link failure, i.e. sent to prevhop/nexthop that's not the src of this RepairLinkReply
                                    bool isToNexthop = (vrouteItr->second.prevhopVid == lostPneiItr->first);
                                    const VlrRingVID& nextHopVid = (isToNexthop) ? vrouteItr->second.nexthopVid : vrouteItr->second.prevhopVid;
                                    char nextHopIsUnavailable = vlrRoutingTable.getPrevNextIsUnavailable(vrouteItr->second.isUnavailable, /*getPrev=*/!isToNexthop);

                                    if (nextHopVid != VLRRINGVID_NULL) {  // I'm not the endpoint to notify
                                        if (nextHopIsUnavailable != 1) {     // next hop isn't unavailable
                                            if (!vrouteItr->second.prevhopRepairRouteSent) {
                                                if (isToNexthop)    // prevhop is patched, record that I've sent RepairRoute for this vroute
                                                    vrouteItr->second.prevhopRepairRouteSent = true;
                                                EV_DETAIL << "RepairRoute (pathid = " << oldPathid << ") will be sent to nextHop = " << nextHopVid << endl;
                                                auto nextHopItr = nextHopToPathidsMap.find(nextHopVid);
                                                if (nextHopItr == nextHopToPathidsMap.end())
                                                    nextHopToPathidsMap[nextHopVid] = {{oldPathid}, nextHopIsUnavailable};
                                                else
                                                    nextHopItr->second.first.push_back(oldPathid);
                                            } else  // prevhopRepairRouteSent = true, I've sent RepairRoute for this vroute
                                                EV_WARN << "Skipping RepairRoute for brokenPathid = " << oldPathid << " because I have sent RepairRoute for it before" << endl;
                                        }
                                    } else {    // I'm the endpoint, send setupReq(repairRoute) to otherEnd
                                        const VlrRingVID& otherEnd = (isToNexthop) ? vrouteItr->second.fromVid : vrouteItr->second.toVid;
                                        ASSERT(vrouteItr->second.isUnavailable != 16);  // shouldn't repair a temporary route, broken temporary route is torn down directly
                                        EV_DETAIL << "Handling RepairRoute for pathid = " << oldPathid << ", otherEnd = " << otherEnd << ", isVsetRoute = " << vrouteItr->second.isVsetRoute << endl;

                                        // oldPathidWasWanted=true if oldPathid wasn't already in nonEssUnwantedRoutes
                                        bool oldPathidWasWanted = nonEssUnwantedRoutes.insert(oldPathid).second;     // patched vroute should be torn down after patchedRouteExpiration
                                        // remove otherEnd from vset, schedule a setupReq recording trace to be sent right away
                                        removeEndpointOnTeardown(oldPathid, /*towardVid=*/otherEnd, /*pathIsVsetRoute=*/vrouteItr->second.isVsetRoute, /*pathIsTemporary=*/vrouteItr->second.isUnavailable, /*reqRepairRoute=*/true);
                                        
                                        // pathid contains temporary route bc link failure, expire later as vroute is needed to route setupReq
                                        vrouteItr->second.isVsetRoute = false;      // oldPathid is no longer a vset route as otherEnd is removed from vset                                  
                                        auto itr_bool = nonEssRoutes.insert({oldPathid, simTime() + patchedRouteExpiration});   // if it's already in nonEssRoutes, no need to reschedule its expiration time as otherEnd isn't a vnei
                                        if (!itr_bool.second && oldPathidWasWanted) {  // oldPathid already in nonEssRoutes but wasn't unwanted, I just learnt that it's patched
                                            if (itr_bool.first->second != 1)
                                                itr_bool.first->second = simTime() + patchedRouteExpiration;   // oldPathid may be a nonEss vroute to pendingVnei otherEnd, extend its expiration time bc we may want to repair it later
                                        }
                                    }
                                }
                            }
                            for (const auto& mappair : nextHopToPathidsMap) {
                                auto msgOut = createRepairRoute(/*pathids=*/mappair.second.first);
                                VlrRingVID resultNextHopVid = getVlrOptionToNextHop(/*msg=*/msgOut, /*nextHopVid=*/mappair.first, /*nextHopIsUnavailable=*/mappair.second.second);     // check if next hop is a linked or lost pnei
                                // checked that nextHopIsUnavailable != 1
                                if (resultNextHopVid != VLRRINGVID_NULL) {
                                    sendCreatedRepairRoute(msgOut, /*nextHopPnei=*/resultNextHopVid);

                                    if (recordStatsToFile) {   // record sent message
                                        std::ostringstream s;
                                        s << mappair.second.first;
                                        recordMessageRecord(/*action=*/0, /*src=*/vid, /*dst=*/VLRRINGVID_NULL, msgOut->getName(), /*msgId=*/0, /*hopcount=*/0, /*chunkByteLength=*/0, /*infoStr=*/s.str().c_str());   // unused params (msgId, hopcount, chunkByteLength) are just assigned to 0
                                    }
                                }
                            }
                        }

                        if (brokenPathids.size() > replyBrokenPathids.size()) {     // there exist brokenPathids in repairLinkReq that aren't added to lostPneis[src].brokenVroutes
                            std::vector<VlrPathID> brokenPathidsSort = brokenPathids;
                            std::sort(brokenPathidsSort.begin(), brokenPathidsSort.end());
                            std::sort(replyBrokenPathids.begin(), replyBrokenPathids.end());
                            std::vector<VlrPathID> brokenPathidsUnrepaired;     // brokenPathids in repairLinkReq that aren't added to replyBrokenPathids
                            std::set_difference(brokenPathidsSort.begin(), brokenPathidsSort.end(), replyBrokenPathids.begin(), replyBrokenPathids.end(), std::inserter(brokenPathidsUnrepaired, brokenPathidsUnrepaired.begin()));
                            VlrIntOption vlrOption;
                            initializeVlrOption(vlrOption, /*dstVid=*/srcVid);

                            // find next hop to src using temp route just built by this repairLinkReply  NOTE if temp route is broken, we should receive Teardown and all brokenVroutes associated with it may be set unavailable
                            VlrRingVID nextHopVid = VLRRINGVID_NULL;
                            vlrOption.setCurrentPathid(newPathid);
                            vlrOption.setTowardVid(srcVid);
                            nextHopVid = findNextHopForSetupReq(vlrOption, /*prevHopVid=*/VLRRINGVID_NULL, /*dstVid=*/srcVid, /*newnode=*/VLRRINGVID_NULL, /*allowTempRoute=*/true);  // no need to specify excludeVid bc findNextHop() won't return myself anyway
                            if (!brokenPathidsUnrepaired.empty() && nextHopVid != VLRRINGVID_NULL) {
                                RepairLinkFailInt *replyOutgoing = new RepairLinkFailInt(/*name=*/"RepairLinkFail");
                                replyOutgoing->setSrc(vid);
                                replyOutgoing->setDst(srcVid);
                                replyOutgoing->setBrokenPathids(brokenPathidsUnrepaired);
                                replyOutgoing->setTempPathid(newPathid);
                                replyOutgoing->setHopcount(0);
                                replyOutgoing->setMessageId(++allSendMessageId);
                                EV_INFO << "Sending RepairLinkFail to src " << srcVid << " of RepairLinkReply, tempPathid = " << newPathid << ", brokenPathids (prevhop=src) = " << replyOutgoing->getBrokenPathids() 
                                        << ", nexthop: " << nextHopVid << endl;
                                sendCreatedRepairLinkFail(replyOutgoing, /*outGateIndex=*/psetTable.getPneiGateIndex(nextHopVid), /*computeChunkLength=*/true);

                                if (recordStatsToFile) {   // record sent message
                                    recordMessageRecord(/*action=*/0, /*src=*/vid, /*dst=*/srcVid, "RepairLinkFail", /*msgId=*/0, /*hopcount=*/0, /*chunkByteLength=*/0);   // unused params (msgId, hopcount, chunkByteLength) are just assigned to 0
                                }
                            }

                        }
                        
                    } else {    // lostPneis[src].brokenVroutes is empty, remove src from lostPneis
                        EV_WARN << "SrcVid = " << srcVid << " of repairLinkReply found in lostPneis but no brokenVroutes, removing srcVid from lostPneis, tempVlinks = ";
                        for (const auto& vlinkid : lostPneiItr->second.tempVlinks)
                            EV_WARN << vlinkid << " ";
                        EV_WARN << "]" << endl;
                        // put temporary vlinks to lost pnei in nonEssRoutes as they are no longer necessary for broken vroutes
                        delayLostPneiTempVlinksTeardown(lostPneiItr->second.tempVlinks, /*delay=*/0);
                        lostPneis.erase(lostPneiItr);
                    }

                } else {    // src not in lostPneis, but I received this repairLinkReply bc I've sent out repairLinkReq to src, perhaps I've torn down broken vroutes
                    EV_WARN << "SrcVid = " << srcVid << " of repairLinkReply not found in lostPneis, tearing down temporary pathid " << newPathid << endl;
                    // tear down newPathid, i.e. send Teardown to prevHopAddr
                    TeardownInt* teardownOut = createTeardownOnePathid(newPathid, /*addSrcVset=*/false, /*rebuild=*/false);
                    sendCreatedTeardown(teardownOut, /*nextHopPnei=*/msgPrevHopVid);

                    if (recordStatsToFile) {   // record sent message
                        recordMessageRecord(/*action=*/0, /*src=*/vid, /*dst=*/VLRRINGVID_NULL, "Teardown", /*msgId=*/0, /*hopcount=*/0, /*chunkByteLength=*/0, /*infoStr=*/"repairLinkReply: src not found in lostPneis");   // unused params (msgId, hopcount, chunkByteLength) are just assigned to 0
                    }
                }
            } else {    // replyBrokenPathids.empty()   src isn't (and didn't become) prevhop of any brokenPathids in this repairLinkReply
                EV_WARN << "SrcVid = " << srcVid << " of repairLinkReply didn't become prevhop of any brokenPathids, tearing down temporary pathid " << newPathid << endl;
                // tear down newPathid, i.e. send Teardown to prevHopAddr
                TeardownInt* teardownOut = createTeardownOnePathid(newPathid, /*addSrcVset=*/false, /*rebuild=*/true);      // rebuild=true bc I didn't check if src already exists in my lostPneis
                sendCreatedTeardown(teardownOut, /*nextHopPnei=*/msgPrevHopVid);

                if (recordStatsToFile) {   // record sent message
                    recordMessageRecord(/*action=*/0, /*src=*/vid, /*dst=*/VLRRINGVID_NULL, "Teardown", /*msgId=*/0, /*hopcount=*/0, /*chunkByteLength=*/0, /*infoStr=*/"repairLinkReply: brokenPathids inconsistent");   // unused params (msgId, hopcount, chunkByteLength) are just assigned to 0
                }
            }
        }
        else {  // this repairLinkReply isn't destined for me
            // auto replyOutgoing = staticPtrCast<RepairLinkReplyInt>(replyIncoming->dupShared());
            forwardRepairLinkReply(replyIncoming, pktForwarded, newPathid, msgPrevHopVid);
            
            if (checkOverHeardTraces) {
                // see if src of msg belongs to pendingVset
                pendingVsetAdd(srcVid, /*heardFrom=*/VLRRINGVID_NULL, /*addToEmpty=*/false);
            }
        }
    }

    if (recordStatsToFile && recordDroppedMsg && !pktRecorded) {   // record message
        if (pktForMe)
            recordMessageRecord(/*action=*/1, /*src=*/srcVid, /*dst=*/dstVid, "RepairLinkReply", /*msgId=*/newPathid, /*hopcount=*/replyIncoming->getHopcount()+1, /*chunkByteLength=*/replyIncoming->getByteLength());
        else if (!pktForwarded)
            recordMessageRecord(/*action=*/4, /*src=*/srcVid, /*dst=*/dstVid, "RepairLinkReply", /*msgId=*/newPathid, /*hopcount=*/replyIncoming->getHopcount()+1, /*chunkByteLength=*/replyIncoming->getByteLength());
    }
}

void Vlr::forwardRepairLinkReply(RepairLinkReplyInt* replyOutgoing, bool& pktForwarded, const VlrPathID& newPathid, VlrRingVID msgPrevHopVid)
{
    VlrRingVID srcVid = replyOutgoing->getSrc();
    VlrRingVID dstVid = replyOutgoing->getLinkTrace().at(0);
    // unsigned int msgHopCount = replyOutgoing->getHopcount() +1;

    // forward this repairLinkReply with linkTrace
    // replyOutgoing->getTraceForUpdate(): [start node, .., parent node, (me, skipped nodes)]
    unsigned int nextHopIndex = getNextHopIndexInTrace(replyOutgoing->getLinkTraceForUpdate(), /*preferShort=*/false);
    // replyOutgoing->getTrace(): [start node, .., parent node]
    if (nextHopIndex >= replyOutgoing->getLinkTrace().size()) {
        EV_WARN << "No next hop found for RepairLinkReply with trace received at me = " << vid << " because no node in trace is a LINKED pnei, tearing down vroute: pathid = " << newPathid << ", src = " << srcVid << ", linkTrace = " << replyOutgoing->getLinkTrace() << endl;
        // tear down newPathid, i.e. send Teardown to prevHopAddr
        TeardownInt* teardownOut = createTeardownOnePathid(newPathid, /*addSrcVset=*/false, /*rebuild=*/true);
        sendCreatedTeardown(teardownOut, /*nextHopPnei=*/msgPrevHopVid);

        if (recordStatsToFile) {   // record sent message
            recordMessageRecord(/*action=*/0, /*src=*/vid, /*dst=*/VLRRINGVID_NULL, "Teardown", /*msgId=*/0, /*hopcount=*/0, /*chunkByteLength=*/0, /*infoStr=*/"repairLinkReply: no next hop found in trace");   // unused params (msgId, hopcount, chunkByteLength) are just assigned to 0
        }

    } else {    // nexthop found for repairLinkReply
        // replyOutgoing->setHopcount(msgHopCount);
        VlrRingVID nextHopVid = replyOutgoing->getLinkTrace().at(nextHopIndex);

        // we've checked newPathid not in vlrRoutingTable
        vlrRoutingTable.addRoute(newPathid, srcVid, dstVid, /*prevhopVid=*/msgPrevHopVid, /*nexthopVid=*/nextHopVid, /*isVsetRoute=*/false, /*isUnavailable=*/16);     // set temporary route

        EV_INFO << "Sending RepairLinkReply for temporary pathid = " << newPathid << ", linkTrace = " << replyOutgoing->getLinkTrace() << ", nexthop: " << nextHopVid << endl;
        sendCreatedRepairLinkReply(replyOutgoing, /*outGateIndex=*/psetTable.getPneiGateIndex(nextHopVid), /*computeChunkLength=*/true);
        pktForwarded = true;
    }
}

int Vlr::computeRepairLinkFailByteLength(RepairLinkFailInt* msg) const
{
    // unsigned int src, dst;
    int chunkByteLength = VLRRINGVID_BYTELEN * 2;
    // std::vector<VlrPathID> brokenPathids
    chunkByteLength += msg->getBrokenPathids().size() * VLRPATHID_BYTELEN;
    // VlrPathID tempPathid;
    chunkByteLength += VLRPATHID_BYTELEN;

    // for VlrIntUniPacket
    chunkByteLength += getVlrUniPacketByteLength();
    return chunkByteLength;
}

// if computeChunkLength = true, compute chunk length because chunk was just created with createSetupReply() or modified after dupShared() from another chunk
// else, no need to compute chunk length because chunk was dupShared() (not modified) from another chunk that has chunkLength set
void Vlr::sendCreatedRepairLinkFail(RepairLinkFailInt *msg, const int& outGateIndex, bool computeChunkLength/*=true*/, double delay/*=0*/)
{
    if (computeChunkLength)
        msg->setByteLength(computeRepairLinkFailByteLength(msg));
    EV_INFO << "Sending repairLinkFail: src = " << msg->getSrc() << ", dst = " << msg->getDst() << ", brokenPathids = " << msg->getBrokenPathids() << endl;

    msg->getVlrOptionForUpdate().setPrevHopVid(vid);    // set packet prevHopVid to myself
    msg->setHopcount(msg->getHopcount() +1);    // increment packet hopcount
    
    // NOTE addTag should be executed after chunkLength has been set, and chunkLength shouldn't be changed before findTag/getTag

    // all multihop VLR packets (setupReq, setupReply, etc) L3 dst are set to a pnei, greedy routing at L3 in routeDatagram() isn't needed, but we do greedy routing and deal with VlrOption at L4 (in processSetupReq() for example) 
    // udpPacket->addTagIfAbsent<VlrIntOptionReq>()->setVlrOption(vlrOption);      // VlrOption to be set in IP header in datagramLocalOutHook()
    
    sendCreatedPacket(msg, /*unicast=*/true, /*outGateIndex=*/outGateIndex, /*delay=*/delay, /*checkFail=*/true);
}

void Vlr::processRepairLinkFail(RepairLinkFailInt *msgIncoming, bool& pktForwarded)
{
    EV_DEBUG << "Received RepairLinkFail" << endl;

    VlrRingVID msgPrevHopVid = msgIncoming->getVlrOption().getPrevHopVid();
    if (msgPrevHopVid == VLRRINGVID_NULL)
        throw cRuntimeError("Received RepairLinkFail with vlrOption.prevHopVid = null");
    
    VlrRingVID dstVid = msgIncoming->getDst();
    VlrRingVID srcVid = msgIncoming->getSrc();        
    EV_INFO << "Processing RepairLinkFail: dst = " << dstVid << ", src = " << srcVid << ", prevhop: " << msgPrevHopVid << endl;

    if (recordStatsToFile && recordReceivedMsg) {   // record received message
        recordMessageRecord(/*action=*/2, /*src=*/srcVid, /*dst=*/dstVid, "RepairLinkFail", /*msgId=*/msgIncoming->getMessageId(), /*hopcount=*/msgIncoming->getHopcount()+1, /*chunkByteLength=*/msgIncoming->getByteLength());  // unimportant params (msgId, hopcount)
    }
    bool pktForMe = false;      // set to true if this msg is directed to me or I processed it as its dst
    bool pktRecorded = false;      // set to true if this msg is recorded with recordMessageRecord()

    if (representativeFixed && representative.heardfromvid == VLRRINGVID_NULL) {        // if I don't have a valid rep yet, I won't process or forward this message
        EV_WARN << "No valid rep heard: " << representative << ", cannot accept overlay message" << endl;
        
        if (recordStatsToFile && recordDroppedMsg && !pktRecorded) {   // record dropped message
            recordMessageRecord(/*action=*/4, /*src=*/srcVid, /*dst=*/dstVid, "RepairLinkFail", /*msgId=*/msgIncoming->getMessageId(), /*hopcount=*/msgIncoming->getHopcount()+1, /*chunkByteLength=*/msgIncoming->getByteLength(), /*infoStr=*/"no valid rep");
            pktRecorded = true;
        }
        return;
    }
    // checked I have a valid rep
    else if (dstVid == vid) {
        pktForMe = true;

        const VlrPathID& tempPathid = msgIncoming->getTempPathid();
        const std::vector<VlrPathID>& brokenPathids = msgIncoming->getBrokenPathids();    // broken vroutes that I thought could be patched by temporary route tempPathid to src, but refused by src
        EV_INFO << "Handling RepairLinkFail to me: src = " << srcVid << ", dst = " << dstVid << ", tempPathid = " << tempPathid << ", brokenPathids (nexthop=src) = " << brokenPathids << endl;

        // remove brokenPathids from lostPneis[src].brokenVroutes, and from recentTempRouteSent[tempPathid].first
        auto lostPneiItr = lostPneis.find(srcVid);
        if (lostPneiItr != lostPneis.end()) {
            
            bool tempPathidRecentSent = false;
            auto recentTempItr = recentTempRouteSent.find(tempPathid);
            if (recentTempItr != recentTempRouteSent.end()) {
                if (recentTempItr->second.second > simTime()) { // recentTempRouteSent record hasn't expired
                    EV_WARN << "Removing brokenPathids (nexthop=src) = " << brokenPathids << " previously patched by tempPathid = " << tempPathid << " to endpoint " << srcVid << " found in recentTempRouteSent and was recently built by me" << endl;
                    tempPathidRecentSent = true;
                }
            }
            
            for (const VlrPathID& brokenPathid : brokenPathids) {
                auto vrouteItr = vlrRoutingTable.vlrRoutesMap.find(brokenPathid);
                if (vrouteItr != vlrRoutingTable.vlrRoutesMap.end()) {  // pathid still in vlrRoutingTable
                    if (vlrRoutingTable.getPrevNextIsUnavailable(vrouteItr->second.isUnavailable, /*getPrev=*/false) == 2 && vrouteItr->second.nexthopVid == srcVid) { // route nexthop is using temporary route recently built from me to srcVid
                        // set nexthop of brokenPathid broken
                        vlrRoutingTable.setRouteItrPrevNextIsUnavailable(vrouteItr, /*setPrev=*/false, /*value=*/1);     // set route nexthop state
                        vlrRoutingTable.removeRouteEndFromEndpointMap(brokenPathid, vrouteItr->second.toVid);     // remove route toVid in endpointToRoutesMap
                        // schedule WaitRepairLinkIntTimer to tear down broken vroutes if they aren't repaired in time
                        lostPneiItr->second.brokenVroutes.erase(brokenPathid);
                        // if oldNexthopVid not in lostPneis or lostPneis[oldNexthopVid].timer is still scheduled, associate brokenPathid with oldNexthopVid again
                        // lostPneis[oldNexthopVid].timer isn't scheduled, meaning temporary route has been built to oldNexthopVid but doesn't include brokenPathid, need to associate brokenPathid with some dummy nexthop in lostPneis
                        VlrRingVID oldNexthopVid = srcVid;
                        if (tempPathidRecentSent) {
                            auto pathidOldNextItr = recentTempItr->second.first.find(brokenPathid);
                            if (pathidOldNextItr != recentTempItr->second.first.end()) {
                                oldNexthopVid = pathidOldNextItr->second;
                                recentTempItr->second.first.erase(pathidOldNextItr);
                            } // else, brokenPathid is associated with lostPneis[src] even before tempPathid was built by me
                        }
                        VlrRingVID newNexthopVid = oldNexthopVid;
                        auto lostPneiItr2 = lostPneis.find(oldNexthopVid);
                        if (lostPneiItr2 != lostPneis.end()) {
                            if (lostPneiItr2->second.timer == nullptr || !lostPneiItr2->second.timer->isScheduled()) {    // can't associate brokenPathid with lostPneis[oldNexthopVid].brokenVroutes
                                // NOTE if lostPneis[oldNexthopVid].timer should always be scheduled if VLRRINGVID_DUMMY in lostPneis, bc it's just a timer to tear down brokenVroutes
                                auto itr_bool = lostPneis.insert({VLRRINGVID_DUMMY, {{/*brokenVroutes*/}, nullptr, {}}});  // std::pair<iterator, bool>: iterator (to the inserted element), bool (whether the element is inserted)
                                lostPneiItr2 = itr_bool.first;
                                newNexthopVid = VLRRINGVID_DUMMY;
                                // add brokenPathid to lostPneis[VLRRINGVID_DUMMY].brokenVroutes
                                lostPneiItr2->second.brokenVroutes.insert(brokenPathid);
                            } else {    // lostPneis[oldNexthopVid].timer is scheduled
                                // add brokenPathid to lostPneis[oldNexthopVid].brokenVroutes
                                lostPneiItr2->second.brokenVroutes.insert(brokenPathid);
                            }
                        } else {    // oldNexthopVid not in lostPneis
                            auto itr_bool = lostPneis.insert({oldNexthopVid, {{brokenPathid}, nullptr, {}}});  // std::pair<iterator, bool>: iterator (to the inserted element), bool (whether the element is inserted)
                            lostPneiItr2 = itr_bool.first;
                        }
                        ASSERT(lostPneiItr2->first == newNexthopVid);
                        vrouteItr->second.nexthopVid = newNexthopVid;
                        EV_WARN << "Changing route nexthop: pathid = " << vrouteItr->first << " nexthop changed from " << srcVid << " to " << newNexthopVid << endl;

                        if (lostPneiItr2->second.timer == nullptr) {   // newNexthopVid was just added to lostPneis, schedule WaitRepairLinkIntTimer to tear down broken vroutes if they aren't repaired in time
                            // vroute nexthop unavailable for brokenPathid, hence I need to wait for repairLinkReq from the other link end rather than sending repairLinkReq
                            char timerName[40] = "WaitRepairLinkIntTimer:";
                            lostPneiItr2->second.timer = new WaitRepairLinkIntTimer(strcat(timerName, std::to_string(newNexthopVid).c_str()));
                            lostPneiItr2->second.timer->setDst(newNexthopVid);
                            lostPneiItr2->second.timer->setRetryCount(repairLinkReqRetryLimit);
                            double reqWaitTime = (vid > newNexthopVid) ? (repairLinkReqWaitTime + repairLinkReqSmallEndSendDelay) : repairLinkReqWaitTime;  // if I'm the larger link end of the failed link, I wait longer bc smaller link ends wait longer before the first repairLinkReq timer rings 
                            scheduleAt(simTime() + reqWaitTime, lostPneiItr2->second.timer);
                            EV_WARN << "Scheduling a WaitRepairLinkIntTimer at " << lostPneiItr2->second.timer->getArrivalTime() << " to tear down brokenPathid that are newly associated with newNexthopVid=" << newNexthopVid << endl;
                        }                        
                        
                    }
                }
            } 
        }

        if (checkOverHeardTraces)
            // see if src of msg belongs to pendingVset
            pendingVsetAdd(srcVid, /*heardFrom=*/VLRRINGVID_NULL, /*addToEmpty=*/false);

    }
    else {  // this RepairLinkFail isn't destined for me
        // auto replyOutgoing = staticPtrCast<SetupFailInt>(msgIncoming->dupShared());
        forwardRepairLinkFail(msgIncoming, pktForwarded, msgPrevHopVid);

        if (checkOverHeardTraces)
            // see if src of msg belongs to pendingVset
            pendingVsetAdd(srcVid, /*heardFrom=*/VLRRINGVID_NULL, /*addToEmpty=*/false);
    }

    if (recordStatsToFile && recordDroppedMsg && !pktRecorded) {   // record message
        if (pktForMe)
            recordMessageRecord(/*action=*/1, /*src=*/srcVid, /*dst=*/dstVid, "RepairLinkFail", /*msgId=*/msgIncoming->getMessageId(), /*hopcount=*/msgIncoming->getHopcount()+1, /*chunkByteLength=*/msgIncoming->getByteLength());
        else if (!pktForwarded)
            recordMessageRecord(/*action=*/4, /*src=*/srcVid, /*dst=*/dstVid, "RepairLinkFail", /*msgId=*/msgIncoming->getMessageId(), /*hopcount=*/msgIncoming->getHopcount()+1, /*chunkByteLength=*/msgIncoming->getByteLength());
    }
}

void Vlr::forwardRepairLinkFail(RepairLinkFailInt *replyOutgoing, bool& pktForwarded, VlrRingVID msgPrevHopVid)
{
    VlrRingVID dstVid = replyOutgoing->getDst();
    VlrRingVID srcVid = replyOutgoing->getSrc();        
    
    VlrIntOption& vlrOptionOut = replyOutgoing->getVlrOptionForUpdate();
    VlrRingVID nextHopVid = findNextHopForSetupReq(vlrOptionOut, /*prevHopVid=*/msgPrevHopVid, /*dstVid=*/dstVid, /*newnode=*/VLRRINGVID_NULL, /*allowTempRoute=*/true);
    if (nextHopVid == VLRRINGVID_NULL) {
        // delete vlrOptionOut;
        EV_WARN << "No next hop found for RepairLinkFail received at me = " << vid << ", dropping packet: dst = " << dstVid << ", src = " << srcVid << ", brokenPathids (nexthop=src) = " << replyOutgoing->getBrokenPathids() << endl;
        // if (displayBubbles && hasGUI())
        //     getContainingNode(host)->bubble("No next hop found for SetupFail");
    } else {    // nexthop found to forward RepairLinkFail
        sendCreatedRepairLinkFail(replyOutgoing, /*outGateIndex=*/psetTable.getPneiGateIndex(nextHopVid), /*computeChunkLength=*/false);
        pktForwarded = true;
    }
}


// create TeardownInt with multiple pathids and set its chunk length
RepairRouteInt* Vlr::createRepairRoute(const std::vector<VlrPathID>& pathids)
{
    RepairRouteInt *msg = new RepairRouteInt(/*name=*/"RepairLinkNotify");
    msg->setSrc(vid);                      // vidByteLength
    msg->setPathidsArraySize(pathids.size());
    unsigned int k = 0;
    for (const auto& pathid : pathids)
        msg->setPathids(k++, pathid);      // VLRPATHID_BYTELEN

    // VlrPathID pathids[]
    // // unsigned int src
    int chunkByteLength = k * VLRPATHID_BYTELEN;
    // chunkByteLength += vidByteLength;

    initializeVlrOption(msg->getVlrOptionForUpdate());

    msg->setMessageId(++allSendMessageId);
    msg->setHopcount(0);

    // for VlrIntUniPacket
    chunkByteLength += getVlrUniPacketByteLength();

    msg->setByteLength(chunkByteLength);
    return msg;
}

// set pathids in TeardownInt and recompuate its chunk length
void Vlr::setRepairRoutePathids(RepairRouteInt *msg, const std::vector<VlrPathID>& pathids) const
{
    msg->setPathidsArraySize(pathids.size());
    unsigned int k = 0;
    for (const auto& pathid : pathids)
        msg->setPathids(k++, pathid);      // VLRPATHID_BYTELEN
    // VlrPathID pathids[]
    // // unsigned int src
    int chunkByteLength = k * VLRPATHID_BYTELEN;

    // for VlrIntUniPacket
    chunkByteLength += getVlrUniPacketByteLength();
    
    msg->setByteLength(chunkByteLength);
}

// if next hop in lostPneis, initialize VlrIntOption to follow temporary route to next hop, return nextHopVid in temporary route
// if next hop is available, return nextHopVid; if next hop unavailable, return VLRRINGVID_NULL
// nextHopIsUnavailable: 00 (available) or 11 (endpoint unavailable but next hop linked) or 01 (uvavailable) or 10 (using temp route); if 01, no msg is sent as nextHop is unavailable, if 02, msg is sent on temporary route with VlrIntOption
VlrRingVID Vlr::getVlrOptionToNextHop(VlrIntUniPacket* msg, VlrRingVID nextHopVid, char nextHopIsUnavailable) const
{
    VlrRingVID resultVid = nextHopVid;
    if (nextHopIsUnavailable == 0 || nextHopIsUnavailable == 3) {    // next hop is a linked pnei
        msg->getVlrOptionForUpdate().setTempPathid(VLRPATHID_INVALID);  // ensure tempPathid is null
        return resultVid;
    }
    else if (nextHopIsUnavailable == 2) { // next hop is a lost pnei connected via temporary route
        // VlrRingVID lostPneiVid = getVidFromAddressInRegistry(nextHopAddr);    // map nextHopAddr L3Address to VlrRingVID
        VlrRingVID lostPneiVid = nextHopVid;
        auto lostPneiItr = lostPneis.find(lostPneiVid);
        if (lostPneiItr != lostPneis.end()) {
            // send msg along temporary route
            ASSERT(!lostPneiItr->second.tempVlinks.empty());
            const VlrPathID& tempPathid = *(lostPneiItr->second.tempVlinks.begin());
            VlrRingVID nextHopVid_new = vlrRoutingTable.getRouteNextHop(tempPathid, lostPneiVid);
            VlrIntOption& vlrOption = msg->getVlrOptionForUpdate();
            initializeVlrOption(vlrOption, /*dstVid=*/VLRRINGVID_NULL);     // vlrOption->dstVid isn't used
            vlrOption.setTempPathid(tempPathid);
            vlrOption.setTempTowardVid(lostPneiVid);
            EV_INFO << "Created VlrIntOption to nextHop (lost pnei) = " << nextHopVid << ", temporary route = " << tempPathid << ", new nextHop: " << nextHopVid_new << endl;
            resultVid = nextHopVid_new;
            return resultVid;
        }
    } // else, next hop is a lost pnei that can't be connected
    return VLRRINGVID_NULL;
}

// VlrIntOption is needed in RepairRoute bc it may need to traverse temporary routes to tear down broken vroutes
void Vlr::sendCreatedRepairRoute(RepairRouteInt *msg, VlrRingVID nextHopPnei, double delay/*=0*/)
{
    EV_INFO << "Sending RepairRoute: pathids = [";
    for (size_t i = 0; i < msg->getPathidsArraySize(); i++)
        EV_INFO << msg->getPathids(i) << " ";
    EV_INFO << "]" << ", nexthop: " << nextHopPnei << endl;

    msg->getVlrOptionForUpdate().setPrevHopVid(vid);    // set packet prevHopVid to myself
    msg->setHopcount(msg->getHopcount() +1);    // increment packet hopcount

    // if nextHopAddr can't be reached, RepairRoute won't be resent to an alternative next hop bc we should continue repair vroute (remove loop) after failed link in vroute has been repaired (setup temporary route)
    sendCreatedPacket(msg, /*unicast=*/true, /*outGateIndex=*/psetTable.getPneiGateIndex(nextHopPnei), /*delay=*/delay, /*checkFail=*/true);
}

void Vlr::processRepairRoute(RepairRouteInt *msgIncoming, bool& pktForwarded)
{
    EV_DEBUG << "Received RepairRoute" << endl;
    // IP.SourceAddress: address of the node from which the packet was received
    VlrIntOption& vlrOptionIn = msgIncoming->getVlrOptionForUpdate();
    VlrRingVID msgPrevHopVid = vlrOptionIn.getPrevHopVid();
    if (msgPrevHopVid == VLRRINGVID_NULL)
        throw cRuntimeError("Received RepairRoute with vlrOption.prevHopVid = null");

    size_t numOfPathids = msgIncoming->getPathidsArraySize();
    EV_INFO << "Processing RepairRoute: pathids = [";
    for (size_t i = 0; i < numOfPathids; i++)
        EV_INFO << msgIncoming->getPathids(i) << " ";
    EV_INFO << "]" << ", prevhop: " << msgPrevHopVid << endl;

    if (recordStatsToFile && recordReceivedMsg) {   // record received message
        // RepairRoute doesn't have <VlrCreationTimeTag> tag
        recordMessageRecord(/*action=*/2, /*src=*/msgIncoming->getSrc(), /*dst=*/vid, msgIncoming->getName(), /*msgId=*/msgIncoming->getPathids(0), /*hopcount=*/msgIncoming->getHopcount()+1, /*chunkByteLength=*/msgIncoming->getByteLength());  // unimportant params (msgId, hopcount)
    }
    bool pktForMe = false;      // set to true if this msg is directed to me or I processed it as its dst
    bool pktRecorded = false;      // set to true if this msg is recorded with recordMessageRecord()

    // const auto& networkHeader = getNetworkProtocolHeader(packet);
    // VlrIntOption *vlrOptionIn = const_cast<VlrIntOption *>(findVlrOptionInNetworkDatagram(networkHeader));  // may be nullptr if no VlrOption is provided in IP header, i.e. not traversing a temporary route
    
    // if traversing the temporary route portion of the vroute to forward RepairRoute
    VlrPathID tempPathid = VLRPATHID_INVALID;
    tempPathid = vlrOptionIn.getTempPathid();
    if (tempPathid != VLRPATHID_INVALID) {
        auto tempPathItr = vlrRoutingTable.vlrRoutesMap.find(tempPathid);
        if (tempPathItr != vlrRoutingTable.vlrRoutesMap.end()) {   // tempPathid is in vlrRoutingTable
            VlrRingVID tempTowardVid = vlrOptionIn.getTempTowardVid();
            if (tempTowardVid == vid) {     // reached the end of temporary route portion
                const VlrRingVID& otherEnd = (vid == tempPathItr->second.fromVid) ? tempPathItr->second.toVid : tempPathItr->second.fromVid;
                auto lostPneiItr = lostPneis.find(otherEnd);
                if (lostPneiItr != lostPneis.end()) {
                    msgPrevHopVid = lostPneiItr->first;  // as if this msg is received directly from lost pnei
                } else {// if otherEnd not in lostPneis, this means vroutes broken by otherEnd have been torn down and no longer exist in vlrRoutingTable, we no longer need to follow vroute to their endpoint
                    EV_WARN << "RepairRoute received from previous hop " << msgPrevHopVid << " via temporary route pathid " << tempPathid << ", but otherEnd = " << otherEnd << " of temporary route not found in lostPneis" << endl;
                    msgPrevHopVid = otherEnd;
                }
            } else {    // send to next hop in temporary route
                VlrRingVID nextHopVid = vlrRoutingTable.getRouteItrNextHop(tempPathItr, tempTowardVid);
                // forward msg with vlrOption
                // VlrIntOption* vlrOptionOut = vlrOptionIn->dup();
                // auto msgOut = staticPtrCast<RepairRouteInt>(msgIncoming->dupShared());
                EV_INFO << "Sending RepairRoute along temporary route = " << tempPathid << ", nexthop: " << nextHopVid << endl;
                sendCreatedRepairRoute(msgIncoming, /*nextHopPnei=*/nextHopVid);
                pktForwarded = true;
                return;
            }
        } else {
            EV_WARN << "The temporary route pathid " << tempPathid << " of RepairRoute is not in my routing table, dropping the RepairRoute message and sending Teardown to previous hop " << msgPrevHopVid << endl;
            
            // tear down tempPathid, i.e. send Teardown to prevHopAddr
            const auto& teardownOut = createTeardownOnePathid(tempPathid, /*addSrcVset=*/false, /*rebuild=*/true);
            sendCreatedTeardown(teardownOut, /*nextHopPnei=*/msgPrevHopVid);

            if (recordStatsToFile) {   // record sent message
                recordMessageRecord(/*action=*/0, /*src=*/vid, /*dst=*/VLRRINGVID_NULL, "Teardown", /*msgId=*/0, /*hopcount=*/0, /*chunkByteLength=*/0, /*infoStr=*/"processRepairRoute: using temporary route but not found");   // unused params (msgId, hopcount, chunkByteLength) are just assigned to 0

                if (recordDroppedMsg && !pktRecorded) {     // record dropped message
                    recordMessageRecord(/*action=*/4, /*src=*/msgIncoming->getSrc(), /*dst=*/vid, msgIncoming->getName(), /*msgId=*/msgIncoming->getPathids(0), /*hopcount=*/msgIncoming->getHopcount()+1, /*chunkByteLength=*/msgIncoming->getByteLength());
                    pktRecorded = true;
                }
            }
            return;
        }
    }
    

    std::map<VlrRingVID, std::pair<std::vector<VlrPathID>, char>> nextHopToPathidsMap;    // for RepairRoute to send, map next hop address to (pathids, next hop 2-bit isUnavailable) pair
    for (size_t k = 0; k < numOfPathids; ++k) {
        const VlrPathID& oldPathid = msgIncoming->getPathids(k);

        auto vrouteItr = vlrRoutingTable.vlrRoutesMap.find(oldPathid);
        if (vrouteItr == vlrRoutingTable.vlrRoutesMap.end()) {  // oldPathid not found in vlrRoutingTable
            EV_WARN << "The pathid " << oldPathid << " of RepairRoute is not in my routing table, dropping RepairRoute for this pathid and sending Teardown to previous hop " << msgPrevHopVid << endl;

            if (tempPathid != VLRPATHID_INVALID) {  // this message arrived through temporary route, I must be an endpoint of the temporary route, previous hop doesn't have oldPathid, need to notify other end of tempPathid
                auto tempPathItr = vlrRoutingTable.vlrRoutesMap.find(tempPathid);
                if (tempPathItr != vlrRoutingTable.vlrRoutesMap.end()) {
                    const VlrRingVID& otherEnd = (vid == tempPathItr->second.fromVid) ? tempPathItr->second.toVid : tempPathItr->second.fromVid;
                    auto lostPneiItr = lostPneis.find(otherEnd);
                    if (lostPneiItr != lostPneis.end()) {
                        const VlrRingVID& prevHopVid = lostPneiItr->first;  // send Teardown of oldPathid to other end of tempPathid
                        EV_WARN << "Sending Teardown for unfound pathid = " << oldPathid << " through tempPathid=" << tempPathid << " to previous hop vid=" << otherEnd << endl;
                        const auto& teardownOut = createTeardownOnePathid(oldPathid, /*addSrcVset=*/false, /*rebuild=*/true);
                        sendCreatedTeardownToNextHop(teardownOut, prevHopVid, /*nextHopIsUnavailable=*/2);

                        if (recordStatsToFile) {   // record sent message
                            recordMessageRecord(/*action=*/0, /*src=*/vid, /*dst=*/VLRRINGVID_NULL, "Teardown", /*msgId=*/0, /*hopcount=*/0, /*chunkByteLength=*/0, /*infoStr=*/"processRepairRoute: pathid not found");   // unused params (msgId, hopcount, chunkByteLength) are just assigned to 0
                        }
                    }
                }
            } else {    // previous hop is directly connected
                // tear down oldPathid, i.e. send Teardown to prevHopAddr
                const auto& teardownOut = createTeardownOnePathid(oldPathid, /*addSrcVset=*/false, /*rebuild=*/true);
                sendCreatedTeardown(teardownOut, /*nextHopPnei=*/msgPrevHopVid);

                if (recordStatsToFile) {   // record sent message
                    recordMessageRecord(/*action=*/0, /*src=*/vid, /*dst=*/VLRRINGVID_NULL, "Teardown", /*msgId=*/0, /*hopcount=*/0, /*chunkByteLength=*/0, /*infoStr=*/"processRepairRoute: pathid not found");   // unused params (msgId, hopcount, chunkByteLength) are just assigned to 0
                }
            }
        }
        else {  // checked oldPathid is in vlrRoutingTable
            if (msgPrevHopVid != vrouteItr->second.prevhopVid && msgPrevHopVid != vrouteItr->second.nexthopVid) {
                EV_WARN << "Sender " << msgPrevHopVid << " of RepairRoute is neither prevhop nor nexthop of pathid " << oldPathid << " to be repaired, dropping RepairRoute for this pathid" << endl;
            } else {    // sender of this RepairRoute is either prevhopAddr or nexthopAddr of the vroute
                bool isToNexthop = (msgPrevHopVid == vrouteItr->second.prevhopVid);
                const VlrRingVID& nextHopVid = (isToNexthop) ? vrouteItr->second.nexthopVid : vrouteItr->second.prevhopVid;
                char nextHopIsUnavailable = vlrRoutingTable.getPrevNextIsUnavailable(vrouteItr->second.isUnavailable, /*getPrev=*/!isToNexthop);     // if isToNexthop, next hop is nexthop, getPrev=false
                EV_INFO << "The pathid " << oldPathid << " of RepairRoute is found in routing table, nextHopVid: " << nextHopVid << " (specified=" << (nextHopVid!=VLRRINGVID_NULL) << ", isUnavailable=" << (int)nextHopIsUnavailable << ")" << endl;

                if (nextHopVid == VLRRINGVID_NULL) {  // I'm an endpoint of the vroute
                    pktForMe = true;
                    const VlrRingVID& otherEnd = (isToNexthop) ? vrouteItr->second.fromVid : vrouteItr->second.toVid;
                    ASSERT(vrouteItr->second.isUnavailable != 16);  // shouldn't repair a temporary route, broken temporary route is torn down directly
                    EV_DETAIL << "Handling RepairRoute for pathid = " << oldPathid << ", otherEnd = " << otherEnd << ", isVsetRoute = " << vrouteItr->second.isVsetRoute << endl;

                    // oldPathidWasWanted=true if oldPathid wasn't already in nonEssUnwantedRoutes
                    bool oldPathidWasWanted = nonEssUnwantedRoutes.insert(oldPathid).second;     // patched vroute should be torn down after patchedRouteExpiration
                    
                    if (vrouteItr->second.isVsetRoute) {    // if oldPathid is not the only vset-path to otherEnd, otherEnd wouldn't be removed from vset, hence setupReq wouldn't be scheduled to replace oldPathid
                        auto vneiItr = vset.find(otherEnd);
                        if (vneiItr != vset.end() && vneiItr->second.vsetRoutes.size() > 1) {
                            EV_DETAIL << "Pathid = " << oldPathid << " is a vset-path to otherEnd = " << otherEnd << ", vset-paths = " << vneiItr->second.vsetRoutes << endl;
                            if (oldPathid == *(vneiItr->second.vsetRoutes.begin())) {   // oldPathid is the smallest vset pathid in vneiItr->second.vsetRoutes
                                // remove all other vset-paths to otherEnd except oldPathid, which will be removed afterward in removeEndpointOnTeardown()
                                for (auto vsetRouteItr = vneiItr->second.vsetRoutes.begin(); vsetRouteItr != vneiItr->second.vsetRoutes.end(); ) {
                                    if (*vsetRouteItr != oldPathid) {
                                        const VlrPathID& vsetPathid = *vsetRouteItr;
                                        vlrRoutingTable.vlrRoutesMap.at(vsetPathid).isVsetRoute = false;
                                        // put in nonEssRoutes with expiration time
                                        nonEssRoutes[vsetPathid] = simTime() + patchedRouteExpiration;
                                        nonEssUnwantedRoutes.insert(vsetPathid);    // ensure vsetPathid will be torn down to ensure otherEnd no longer recognizes it as vset-path
                                        vneiItr->second.vsetRoutes.erase(vsetRouteItr++);
                                    } else
                                        vsetRouteItr++;
                                }
                                EV_DETAIL << "Pathid = " << oldPathid << " should be the only vset-path to otherEnd = " << otherEnd << ", vset-paths = " << vneiItr->second.vsetRoutes << endl;
                            }
                        }
                    }
                    // remove otherEnd from vset, schedule a setupReq recording trace to be sent right away
                    removeEndpointOnTeardown(oldPathid, /*towardVid=*/otherEnd, /*pathIsVsetRoute=*/vrouteItr->second.isVsetRoute, /*pathIsTemporary=*/vrouteItr->second.isUnavailable, /*reqRepairRoute=*/true);
                    
                    // pathid contains temporary route bc link failure, expire later as vroute is needed to route setupReq
                    vrouteItr->second.isVsetRoute = false;      // oldPathid is no longer a vset route as otherEnd is removed from vset
                    auto itr_bool = nonEssRoutes.insert({oldPathid, simTime() + patchedRouteExpiration});   // if it's already in nonEssRoutes, no need to reschedule its expiration time as otherEnd isn't a vnei
                    if (!itr_bool.second && oldPathidWasWanted) {  // oldPathid already in nonEssRoutes but wasn't unwanted, I just learnt that it's patched
                        if (itr_bool.first->second != 1)
                            itr_bool.first->second = simTime() + patchedRouteExpiration;   // oldPathid may be a nonEss vroute to pendingVnei otherEnd, extend its expiration time bc we may want to repair it later
                    }
                }
                else {  // I'm not an endpoint of this vroute
                    if (nextHopIsUnavailable != 1) {    // if next hop is connected (linked or with a temporary route)
                        // if (tempPathid != VLRPATHID_INVALID && isToNexthop && vrouteItr->second.prevhopRepairRouteSent)    // message arrived through temporary route, prevhop of this vroute is patched, and I've sent RepairRoute for this vroute to nexthop, no need to notify endpoint toVid again
                        //     EV_WARN << "Skipping RepairRoute for brokenPathid = " << oldPathid << " because I have sent RepairRoute for it before" << endl;
                        // else {
                        auto nextHopItr = nextHopToPathidsMap.find(nextHopVid);
                        if (nextHopItr == nextHopToPathidsMap.end()) 
                            nextHopToPathidsMap[nextHopVid] = {{oldPathid}, nextHopIsUnavailable};
                        else
                            nextHopItr->second.first.push_back(oldPathid);
                        // }
                    } // else, next hop is unavailable, cannot forward this msg bc vroute is broken
                }
            }
        }
    }
    int numSent = 0;
    for (const auto& mappair : nextHopToPathidsMap) {
        const auto& pathids = mappair.second.first;
        const auto& nextHopIsUnavailable = mappair.second.second;
        // auto msgOut = staticPtrCast<RepairRouteInt>(msgIncoming->dupShared());
        if (numSent++ == 0)     // numSent == 0, haven't forwarded the Teardown I received
            pktForwarded = true;
        else    // numSent > 0, already sent the Teardown I received
            msgIncoming = msgIncoming->dup();
        
        if (pathids.size() < numOfPathids)      // pathids in msg should be modified
            setRepairRoutePathids(msgIncoming, pathids);     // set pathids in msgOut and recompute chunk length
        
        VlrRingVID resultNextHopVid = getVlrOptionToNextHop(msgIncoming, /*nextHopVid=*/mappair.first, nextHopIsUnavailable);     // check if next hop is a linked or lost pnei
        
        // if (!nhResult.first.isUnspecified())
        if (resultNextHopVid != VLRRINGVID_NULL) // checked that nextHopIsUnavailable != 1
            sendCreatedRepairRoute(msgIncoming, /*nextHopPnei=*/resultNextHopVid);
    }

    if (recordStatsToFile && recordDroppedMsg && !pktRecorded) {   // record message
        if (pktForMe)
            recordMessageRecord(/*action=*/1, /*src=*/msgIncoming->getSrc(), /*dst=*/vid, msgIncoming->getName(), /*msgId=*/msgIncoming->getPathids(0), /*hopcount=*/msgIncoming->getHopcount()+1, /*chunkByteLength=*/msgIncoming->getByteLength());
        else if (!pktForwarded)
            recordMessageRecord(/*action=*/4, /*src=*/msgIncoming->getSrc(), /*dst=*/vid, msgIncoming->getName(), /*msgId=*/msgIncoming->getPathids(0), /*hopcount=*/msgIncoming->getHopcount()+1, /*chunkByteLength=*/msgIncoming->getByteLength());
    }
}

int Vlr::computeNotifyVsetByteLength(NotifyVsetInt* msg) const
{
    // unsigned int dst, src
    int chunkByteLength = VLRRINGVID_BYTELEN * 2;
    // // bool toVnei
    // chunkByteLength += 1;
    // unsigned int srcVset[]
    chunkByteLength += msg->getSrcVsetArraySize() * VLRRINGVID_BYTELEN;
    // std::vector<unsigned int> trace
    chunkByteLength += msg->getTrace().size() * VLRRINGVID_BYTELEN;

    // for VlrIntUniPacket
    chunkByteLength += getVlrUniPacketByteLength();

    return chunkByteLength;
}

NotifyVsetInt* Vlr::createNotifyVset(const VlrRingVID& dstVid, bool toVnei)
{
    NotifyVsetInt *msg = new NotifyVsetInt(/*name=*/"NotifyVset");
    msg->setSrc(vid);      // vidByteLength
    msg->setDst(dstVid);
    msg->setToVnei(toVnei);
    msg->setProxy(VLRRINGVID_NULL);

    unsigned int srcVsetSize = setupPacketSrcVset(msg);    // number of nodes added to srcVset

    msg->setMessageId(++allSendMessageId);
    msg->setHopcount(0);         // number of nodes this message traversed, including the starting and ending nodes, hence at the starting node hopcount = 1
    initializeVlrOption(msg->getVlrOptionForUpdate());

    return msg;
}

// if computeChunkLength = true, compute chunk length because chunk was just created with createSetupReq() or modified after dupShared() from another chunk
// else, no need to compute chunk length because chunk was dupShared() (not modified) from another chunk that has chunkLength set
void Vlr::sendCreatedNotifyVset(NotifyVsetInt *msg, const int& outGateIndex, bool computeChunkLength/*=true*/)  // double delay/*=0*/
{
    if (computeChunkLength)
        msg->setByteLength(computeNotifyVsetByteLength(msg));
    EV_DEBUG << "Sending NotifyVset: src = " << msg->getSrc() << ", dst = " << msg->getDst() << ", srcVset = [";
    for (size_t i = 0; i < msg->getSrcVsetArraySize(); i++)
        EV_DEBUG << msg->getSrcVset(i) << " ";
    EV_DEBUG << "]" << endl;

    msg->getVlrOptionForUpdate().setPrevHopVid(vid);    // set packet prevHopVid to myself
    msg->setHopcount(msg->getHopcount() +1);    // increment packet hopcount
    
    // NOTE addTag should be executed after chunkLength has been set, and chunkLength shouldn't be changed before findTag/getTag

    // all multihop VLR packets (setupReq, setupReply, etc) L3 dst are set to a pnei, greedy routing at L3 in routeDatagram() isn't needed, but we do greedy routing and deal with VlrOption at L4 (in processSetupReq() for example) 
    // udpPacket->addTagIfAbsent<VlrIntOptionReq>()->setVlrOption(vlrOption);      // VlrOption to be set in IP header in datagramLocalOutHook()
    
    sendCreatedPacket(msg, /*unicast=*/true, /*outGateIndex=*/outGateIndex, /*delay=*/0, /*checkFail=*/true);
}

void Vlr::processNotifyVset(NotifyVsetInt *msgIncoming, bool& pktForwarded)
{
    EV_DEBUG << "Received NotifyVset" << endl;
    VlrRingVID dstVid = msgIncoming->getDst();
    VlrRingVID srcVid = msgIncoming->getSrc();
    bool withTrace = (msgIncoming->getTrace().size() > 0);

    EV_INFO << "Processing NotifyVset: src = " << srcVid << ", dst = " << dstVid << endl;

    if (recordStatsToFile && recordReceivedMsg) {   // record received message
        recordMessageRecord(/*action=*/2, /*src=*/srcVid, /*dst=*/dstVid, "NotifyVset", /*msgId=*/msgIncoming->getMessageId(), /*hopcount=*/msgIncoming->getHopcount()+1, /*chunkByteLength=*/msgIncoming->getByteLength());   // unimportant params (msgId, hopcount)
    }
    bool pktForMe = false;      // set to true if this msg is directed to me or I processed it as its dst
    bool pktRecorded = false;      // set to true if this msg is recorded with recordMessageRecord()

    if (representativeFixed && representative.heardfromvid == VLRRINGVID_NULL) {        // if I don't have a valid rep yet, I won't process or forward this message
        EV_WARN << "No valid rep heard: " << representative << ", cannot accept overlay message" << endl;

        if (recordStatsToFile && recordDroppedMsg && !pktRecorded) {   // record dropped message
            recordMessageRecord(/*action=*/4, /*src=*/srcVid, /*dst=*/dstVid, "NotifyVset", /*msgId=*/msgIncoming->getMessageId(), /*hopcount=*/msgIncoming->getHopcount()+1, /*chunkByteLength=*/msgIncoming->getByteLength(), /*infoStr=*/"no valid rep");
            pktRecorded = true;
        }
        return;
    }
    // checked I have a valid rep
    else if (dstVid == vid) {
        pktForMe = true;
        bool isToVnei = msgIncoming->getToVnei();
        EV_INFO << "Received NotifyVset to me: src = " << srcVid << ", isToVnei = " << isToVnei << ", srcVset = [";
        for (size_t i = 0; i < msgIncoming->getSrcVsetArraySize(); i++)
            EV_INFO << msgIncoming->getSrcVset(i) << " ";
        EV_INFO << "]" << endl;

        if (isToVnei && vset.find(srcVid) == vset.end()) {
            EV_WARN << "Received NotifyVset from non-vnei = " << srcVid << ", sending NotifyVset reply" << endl;
            // if src thinks I'm its vnei but it's not in my vset, send NotifyVset to it if I have vnei that it doesn't have
            // std::vector<VlrRingVID> srcVsetVec;    // Commented out bc can't assuem srcVset is sorted    reqIncoming.srcVset is sorted since vset is a set, thus srcVsetVec is sorted if retrieve nodes in order
            std::set<VlrRingVID> srcVsetSet;    // srcVsetSet is sorted bc it's a set
            int oVsetSize = msgIncoming->getSrcVsetArraySize();
            for (int k = 0; k < oVsetSize; k++)
                srcVsetSet.insert(msgIncoming->getSrcVset(k));
            // select nodes in my vset but not in srcVset, put them in notifyVneis
            std::vector<VlrRingVID> notifyVneis;
            std::vector<VlrRingVID> vsetVec;
            for (const auto& vnei : vset)
                vsetVec.push_back(vnei.first);
            std::set_difference(vsetVec.begin(), vsetVec.end(), srcVsetSet.begin(), srcVsetSet.end(), std::inserter(notifyVneis, notifyVneis.begin()));
            // std::set_difference(vset.begin(), vset.end(), srcVsetSet.begin(), srcVsetSet.end(), std::inserter(notifyVneis, notifyVneis.begin()));
            if (!notifyVneis.empty()) {
                // send NotifyVset back to src
                VlrIntOption vlrOptionOut;
                initializeVlrOption(vlrOptionOut, /*dstVid=*/srcVid);
                VlrRingVID nextHopVid = findNextHop(vlrOptionOut, /*prevHopVid=*/VLRRINGVID_NULL, /*excludeVid=*/VLRRINGVID_NULL, /*allowTempRoute=*/true);
                if (nextHopVid == VLRRINGVID_NULL) {
                    // delete vlrOptionOut;
                    EV_WARN << "No next hop found to send NotifyVset back to non-vnei = " << srcVid << ", dropping NotifyVset reply" << endl;
                } else {
                    const auto& msgOutgoing = createNotifyVset(/*dstVid=*/srcVid, /*toVnei=*/false);
                    msgOutgoing->setVlrOption(vlrOptionOut);
                    if (recordStatsToFile) {   // record sent message
                        recordMessageRecord(/*action=*/0, /*src=*/vid, /*dst=*/srcVid, "NotifyVset", /*msgId=*/0, /*hopcount=*/0, /*chunkByteLength=*/0);   // unused params (msgId, hopcount, chunkByteLength) are just assigned to 0
                    }
                    EV_INFO << "Sending NotifyVset to non-vnei = " << srcVid << ", nexthop: " << nextHopVid << endl;
                    sendCreatedNotifyVset(msgOutgoing, /*outGateIndex=*/psetTable.getPneiGateIndex(nextHopVid));
                }
            }
        }

        pendingVsetAdd(srcVid);

        // process srcVset in received NotifyVset
        processOtherVset(msgIncoming, /*srcVid=*/srcVid);
    }
    else {  // this NotifyVset isn't destined for me
        // forward this NotifyVset with findNextHop()
        forwardNotifyVset(msgIncoming, pktForwarded, withTrace);

        if (checkOverHeardTraces)
            // see if src of msg belongs to pendingVset
            pendingVsetAdd(srcVid, /*heardFrom=*/VLRRINGVID_NULL, /*addToEmpty=*/false);
    }

    if (recordStatsToFile && recordDroppedMsg && !pktRecorded) {   // record message
        if (pktForMe)
            recordMessageRecord(/*action=*/1, /*src=*/srcVid, /*dst=*/dstVid, "NotifyVset", /*msgId=*/msgIncoming->getMessageId(), /*hopcount=*/msgIncoming->getHopcount()+1, /*chunkByteLength=*/msgIncoming->getByteLength());
        else if (!pktForwarded)
            recordMessageRecord(/*action=*/4, /*src=*/srcVid, /*dst=*/dstVid, "NotifyVset", /*msgId=*/msgIncoming->getMessageId(), /*hopcount=*/msgIncoming->getHopcount()+1, /*chunkByteLength=*/msgIncoming->getByteLength());
    }
}

void Vlr::forwardNotifyVset(NotifyVsetInt* msgIncoming, bool& pktForwarded, bool withTrace)
{
    VlrRingVID dstVid = msgIncoming->getDst();        // can also get dst using vlrOptionIn->getDstVid()
    VlrRingVID proxy = msgIncoming->getProxy();       // could be null

    if (!withTrace) {
        VlrIntOption& vlrOptionOut = msgIncoming->getVlrOptionForUpdate();
        // IP.SourceAddress: address of the node from which the packet was received
        VlrRingVID msgPrevHopVid = vlrOptionOut.getPrevHopVid();
        if (msgPrevHopVid == VLRRINGVID_NULL)  // assert prevHopVid isn't myself (shouldn't happen)
            throw cRuntimeError("Received NotifyVset with vlrOption.prevHopVid = null");
        
        if (proxy == vid) {  // proxy is reached
            msgIncoming->setProxy(VLRRINGVID_NULL);
            if (vlrOptionOut.getDstVid() != dstVid) {
                vlrOptionOut.setDstVid(dstVid);
                vlrOptionOut.setCurrentPathid(VLRPATHID_INVALID);
                vlrOptionOut.setTempPathid(VLRPATHID_INVALID);
            }
        }
        VlrRingVID nextHopVid = findNextHopForSetupReq(vlrOptionOut, /*prevHopVid=*/msgPrevHopVid, /*dstVid=*/dstVid, /*newnode=*/VLRRINGVID_NULL, /*allowTempRoute=*/true);
        if (nextHopVid == VLRRINGVID_NULL) {
            // delete vlrOptionOut;
            EV_WARN << "No next hop found for NotifyVset received at me = " << vid << ", dropping packet: src = " << msgIncoming->getSrc() << ", dst = " << dstVid << endl;
            // if (displayBubbles && hasGUI())
            //     getContainingNode(host)->bubble("No next hop found, dropping packet");
        } else {        // nexthop found to forward NotifyVset
            // auto msgOutgoing = staticPtrCast<NotifyVsetInt>(msgIncoming->dupShared());
            sendCreatedNotifyVset(msgIncoming, /*outGateIndex=*/psetTable.getPneiGateIndex(nextHopVid), /*computeChunkLength=*/false);
            pktForwarded = true;
        }
    } else { // forward this NotifyVset with trace
        unsigned int nextHopIndex = getNextHopIndexInTrace(msgIncoming->getTraceForUpdate());    // replyOutgoing->getTraceForUpdate(): [start node, .., parent node, (me, skipped nodes)]
        if (nextHopIndex >= msgIncoming->getTrace().size()) {
            EV_WARN << "No next hop found for NotifyVset with trace received at me = " << vid << " because no node in trace is a LINKED pnei, dropping packet: src = " << msgIncoming->getSrc() << ", dst = " << dstVid << ", trace = " << msgIncoming->getTrace() << endl;
        } else {    // nexthop found for SetupReply with trace
            VlrRingVID nextHopVid = msgIncoming->getTrace().at(nextHopIndex);
            EV_INFO << "Sending NotifyVset to dst = " << dstVid << ", src = " << msgIncoming->getSrc() << ", trace = " << msgIncoming->getTrace() << ", nexthop: " << nextHopVid << endl;
            sendCreatedNotifyVset(msgIncoming, /*outGateIndex=*/psetTable.getPneiGateIndex(nextHopVid), /*computeChunkLength=*/true);
            pktForwarded = true;
        }
    }
}

int Vlr::computeRepairLocalReqFloodByteLength(RepairLocalReqFloodInt* repairReq) const
{
    // unsigned int ttl;    // 2 byte
    // // // unsigned int floodSeqnum;    // 2 byte
    int chunkByteLength = 2;
    // std::map<unsigned int, std::set<VlrPathID>> dstToPathidsMap;
    const auto& dstToPathidsMap = repairReq->getDstToPathidsMap();
    for (auto it = dstToPathidsMap.begin(); it != dstToPathidsMap.end(); ++it)
        chunkByteLength += VLRRINGVID_BYTELEN + it->second.size() * VLRPATHID_BYTELEN;
    // std::set<VlrPathID> brokenPathids;
    chunkByteLength += repairReq->getBrokenPathids().size() * VLRPATHID_BYTELEN;
    // std::vector<unsigned int> linkTrace
    chunkByteLength += repairReq->getLinkTrace().size() * VLRRINGVID_BYTELEN;
    // // L3Address srcAddress;
    // chunkByteLength += addressByteLength;

    return chunkByteLength;
}

RepairLocalReqFloodInt* Vlr::createRepairLocalReqFlood()
{
    RepairLocalReqFloodInt *repairReq = new RepairLocalReqFloodInt(/*name=*/"RepairLocalReqFlood");
    repairReq->setTtl(repairLinkReqFloodTTL);
    repairReq->setFloodSeqnum(++floodSeqnum);

    repairReq->getLinkTraceForUpdate().push_back(vid);  // put myself in linkTrace
    // repairReq->setSrcAddress(getSelfAddress());

    return repairReq;
}

// if computeChunkLength = true, compute chunk length because chunk was just created with createSetupReq() or modified after dupShared() from another chunk
// else, no need to compute chunk length because chunk was dupShared() (not modified) from another chunk that has chunkLength set
void Vlr::sendCreatedRepairLocalReqFlood(RepairLocalReqFloodInt *repairReq, bool computeChunkLength/*=true*/, double delay/*=0*/)
{
    if (computeChunkLength)
        repairReq->setByteLength(computeRepairLocalReqFloodByteLength(repairReq));
    EV_DEBUG << "Sending repairLocalReqFlood: src = " << repairReq->getLinkTrace().at(0) << ", ttl = " << repairReq->getTtl() << endl;

    // repairReq->getVlrOptionForUpdate().setPrevHopVid(vid);    // set packet prevHopVid to myself
    
    // NOTE addTag should be executed after chunkLength has been set, and chunkLength shouldn't be changed before findTag/getTag

    // all multihop VLR packets (setupReq, setupReply, etc) L3 dst are set to a pnei, greedy routing at L3 in routeDatagram() isn't needed, but we do greedy routing and deal with VlrOption at L4 (in processSetupReq() for example) 
    // udpPacket->addTagIfAbsent<VlrIntOptionReq>()->setVlrOption(vlrOption);      // VlrOption to be set in IP header in datagramLocalOutHook()
    
    sendCreatedPacket(repairReq, /*unicast=*/false, /*outGateIndex=*/-1, /*delay=*/delay, /*checkFail=*/true);
}

void Vlr::processRepairLocalReqFlood(RepairLocalReqFloodInt *reqIncoming, bool& pktForwarded)
{
    EV_DEBUG << "Received RepairLocalReqFlood" << endl;
    
    const VlrIntVidToPathidSetMap& dstToPathidsMap = reqIncoming->getDstToPathidsMap();
    VlrRingVID srcVid = reqIncoming->getLinkTrace().at(0);
    unsigned int floodTtl = reqIncoming->getTtl();
    unsigned int floodSeqnum = reqIncoming->getFloodSeqnum();
    EV_INFO << "Processing RepairLocalReqFlood: src = " << srcVid << ", ttl = " << floodTtl << ", dst = {";
    for (auto it = dstToPathidsMap.begin(); it != dstToPathidsMap.end(); ++it)
        EV_INFO << it->first << ' ';
    EV_INFO << "}" << endl;

    if (recordStatsToFile && recordReceivedMsg) {   // record received message
        // RepairLinkReqFlood doesn't have <VlrCreationTimeTag> tag
        recordMessageRecord(/*action=*/2, /*src=*/srcVid, /*dst=*/vid, "RepairLocalReqFlood", /*msgId=*/floodSeqnum, /*hopcount=*/reqIncoming->getLinkTrace().size()+1, /*chunkByteLength=*/reqIncoming->getByteLength());  // unimportant params (msgId)
    }
    bool pktForMe = false;      // set to true if this msg is directed to me or I processed it as its dst
    bool pktRecorded = false;      // set to true if this msg is recorded with recordMessageRecord()

    if (representativeFixed && representative.heardfromvid == VLRRINGVID_NULL) {        // if I don't have a valid rep yet, I won't process or forward this message
        EV_WARN << "No valid rep heard: " << representative << ", cannot accept overlay message" << endl;
        
        if (recordStatsToFile && recordDroppedMsg && !pktRecorded) {   // record dropped message
            recordMessageRecord(/*action=*/4, /*src=*/srcVid, /*dst=*/vid, "RepairLocalReqFlood", /*msgId=*/floodSeqnum, /*hopcount=*/reqIncoming->getLinkTrace().size()+1, /*chunkByteLength=*/reqIncoming->getByteLength(), /*infoStr=*/"no valid rep");  // unimportant params (msgId)
            pktRecorded = true;
        }
        return;
    }
    if (srcVid == vid) {
        EV_INFO << "Received RepairLocalReqFlood sent by me with flood seqnum = " << floodSeqnum << ", ignoring this flood request" << endl;

        if (recordStatsToFile && recordDroppedMsg && !pktRecorded) {   // record dropped message
            recordMessageRecord(/*action=*/4, /*src=*/srcVid, /*dst=*/vid, "RepairLocalReqFlood", /*msgId=*/floodSeqnum, /*hopcount=*/reqIncoming->getLinkTrace().size()+1, /*chunkByteLength=*/reqIncoming->getByteLength(), /*infoStr=*/"flood sent from me");  // unimportant params (msgId)
            pktRecorded = true;
        }
        return;
    }
    auto dstItr = dstToPathidsMap.find(vid);
    bool dstHasMe = (dstItr != dstToPathidsMap.end());
    // Commented out the flood request from srcVid may traverse different linkTrace, we want to find a linkTrace that which doesn't contain any vroutes in brokenPathids
    // // bool reqProcessed = false;      // if this flood request has been processed
    // // check if floodSeqnum is larger than previous one from src, i.e. if I've processed this flood request
    // auto recentFloodItr = recentReqFloodFrom.find(srcVid);
    // if (recentFloodItr != recentReqFloodFrom.end()) {       // if I've recently received flood request from src
    //     if (floodSeqnum <= recentFloodItr->second.first) {
    //         // NOTE flood request whose dst is myself won't be flooded to pneis
    //         // if (dstHasMe && floodSeqnum == recentFloodItr->second.first) { // dst received flood request from src that has been processed, probably via another trace, see if this trace is shorter than the existing one
    //         //     reqProcessed = true;
    //         // } else {
    //         EV_INFO << "Recently received RepairLinkReqFlood with flood seqnum = " << floodSeqnum << " from src = " << srcVid << ", ignoring this flood request" << endl;
    //         return;
    //         // }
    //     }
    // } else {    // add src in recentReqFloodFrom
    //     auto itr_bool = recentReqFloodFrom.insert({srcVid, {}});  // std::pair<iterator, bool>: iterator (to the inserted element), bool (whether the element is inserted)
    //     recentFloodItr = itr_bool.first;
    // }
    // // update floodSeqnum of src in recentReqFloodFrom
    // recentFloodItr->second.first = floodSeqnum;
    // recentFloodItr->second.second = simTime() + recentReqFloodExpiration;
    if (std::find(reqIncoming->getLinkTrace().begin(), reqIncoming->getLinkTrace().end(), vid) != reqIncoming->getLinkTrace().end()) {   // my vid found in linkTrace
        EV_INFO << "Received RepairLocalReqFlood whose linkTrace = " << reqIncoming->getLinkTrace() << " already contains me=" << vid << ", ignoring this flood request" << endl;

        if (recordStatsToFile && recordDroppedMsg && !pktRecorded) {   // record dropped message
            recordMessageRecord(/*action=*/4, /*src=*/srcVid, /*dst=*/vid, "RepairLocalReqFlood", /*msgId=*/floodSeqnum, /*hopcount=*/reqIncoming->getLinkTrace().size()+1, /*chunkByteLength=*/reqIncoming->getByteLength(), /*infoStr=*/"flood already forwarded");
            pktRecorded = true;
        }
        return;
    }
    // check if I've received linkTrace for brokenPathids in this repairLocalReq
    std::set<VlrPathID> forwardBrokenPathids;
    auto recentLocalReqItr = recentRepairLocalReqFrom.find(srcVid);
    if (recentLocalReqItr != recentRepairLocalReqFrom.end()) {
        // get brokenPathids in this repairLocalReq that are not already received from srcVid, add to forwardBrokenPathids
        std::set_difference(reqIncoming->getBrokenPathids().begin(), reqIncoming->getBrokenPathids().end(), recentLocalReqItr->second.first.begin(), recentLocalReqItr->second.first.end(), std::inserter(forwardBrokenPathids, forwardBrokenPathids.end()));
    } else {    // haven't received repairLocalReq from srcVid
        forwardBrokenPathids = reqIncoming->getBrokenPathids();
        auto itr_bool = recentRepairLocalReqFrom.insert({srcVid, {{}, simTime() + delayedRepairLinkReqExpireTimeCoefficient * beaconInterval}});  // std::pair<iterator, bool>: iterator (to the inserted element), bool (whether the element is inserted)
        recentLocalReqItr = itr_bool.first;
    }
    // update recentRepairLocalReqFrom[src] with new brokenPathids to forward
    recentLocalReqItr->second.first.insert(forwardBrokenPathids.begin(), forwardBrokenPathids.end());

    if (dstHasMe) {
        pktForMe = true;
        // const L3Address& srcAddr = reqIncoming->getSrcAddress();
        // get pathids in dstItr->second that also exist in forwardBrokenPathids (newly received from srcVid)
        std::vector<VlrPathID> brokenPathidsForMe;
        std::set_intersection(dstItr->second.begin(), dstItr->second.end(), forwardBrokenPathids.begin(), forwardBrokenPathids.end(), std::inserter(brokenPathidsForMe, brokenPathidsForMe.begin()));

        EV_INFO << "Handling RepairLocalReqFlood to me: src = " << srcVid << ", trace = " << reqIncoming->getLinkTrace() << ", brokenPathids = " << brokenPathidsForMe << endl;
        // linkTrace (I haven't added myself) is [src], meaning src and I are pneis
            // if src is the route nexthop
                // if src is still a LINKED pnei at me, route nexthop won't be unavailable and repairLocalReply won't be sent; if I also think src is a lost pnei, no next hop is available in linkTrace, repairLocalReply can't be sent
            // if src is next of nexthop, repairLocalReply still needs to be sent to notify src to change prevhop to me

        if (!brokenPathidsForMe.empty()) {
            std::vector<VlrPathID> delayedBrokenPathids;  // pathids (from brokenPathids) found in vlrRoutingTable but no nexthop breakage detected yet

            processRepairLocalReqInfo(brokenPathidsForMe, reqIncoming->getLinkTrace(), delayedBrokenPathids);
            
            if (!delayedBrokenPathids.empty()) {
                // generate hash(srcVid, currTime) as map key for delayedRepairLocalReq bc we can record repairLocalReq from same srcVid twice with different linkTrace
                bool addedRecord = false;
                unsigned int newReqKey;
                do {    // loop in case generated newReqKey isn't unique in my delayedRepairLocalReq
                    newReqKey = genPathID(srcVid);
                    addedRecord = delayedRepairLocalReq.insert({newReqKey, {delayedBrokenPathids, reqIncoming->getLinkTrace(), simTime() + delayedRepairLinkReqExpireTimeCoefficient * beaconInterval}}).second;
                } while (!addedRecord);
            }
        }
    }
    if (floodTtl <= 1) {
        EV_INFO << "Received RepairLocalReqFlood with from flood ttl = " << floodTtl << " from src = " << srcVid << ", dropping this flood request" << endl;
        // return;
    }
    else {  // broadcast RepairLocalReqFlood
        VlrIntVidToPathidSetMap& dstToPathidsMapForUpdate = reqIncoming->getDstToPathidsMapForUpdate();
        bool removedMeFromMap = dstToPathidsMapForUpdate.erase(vid);    // remove myself from dstToPathidsMap since I've processed RepairLinkReq for myself, now just need to broadcast to other dst
        if (!dstToPathidsMapForUpdate.empty()) {    // there're more dst in this RepairLinkReqFlood
            // NOTE I don't broadcast brokenPathids that exist in my vlrRoutingTable
            for (auto forwardBrokenItr = forwardBrokenPathids.begin(); forwardBrokenItr != forwardBrokenPathids.end(); ) {
                const VlrPathID& brokenPathid = *forwardBrokenItr;
                if (vlrRoutingTable.vlrRoutesMap.find(brokenPathid) != vlrRoutingTable.vlrRoutesMap.end())  // brokenPathid already in vlrRoutingTable
                    forwardBrokenPathids.erase(forwardBrokenItr++);     // step1: it_tobe = it+1; step2: erase(it); step3: it = it_tobe;
                else if (removedMeFromMap) {      // dstToPathidsMap size is reduced, ensure every brokenPathid is still associated w/ some dst in dstToPathidsMap, if not, there's no need to forward
                    bool pathidNeeded = false;
                    for (const auto& mappair : dstToPathidsMapForUpdate) {
                        const std::set<VlrPathID>& brokenPathidsForDst = mappair.second;
                        if (brokenPathidsForDst.find(brokenPathid) != brokenPathidsForDst.end()) {   // pathid is needed for some dst in dstToPathidsMap
                            pathidNeeded = true;
                            break;
                        }
                    }
                    if (pathidNeeded)
                        forwardBrokenItr++;
                    else    // brokenPathid isn't associated w/ any dst in dstToPathidsMap
                        forwardBrokenPathids.erase(forwardBrokenItr++);     // step1: it_tobe = it+1; step2: erase(it); step3: it = it_tobe;
                }
                else
                    forwardBrokenItr++;
            }
            if (!forwardBrokenPathids.empty()) {
                // remove pathids in dstToPathidsMap that are no longer in forwardBrokenPathids
                for (auto dstMapItr = dstToPathidsMapForUpdate.begin(); dstMapItr != dstToPathidsMapForUpdate.end(); ) {
                    std::set<VlrPathID> brokenPathidsForDst;
                    // trim pathids in dstMapItr->second so that they also exist in forwardBrokenPathids
                    std::set_intersection(dstMapItr->second.begin(), dstMapItr->second.end(), forwardBrokenPathids.begin(), forwardBrokenPathids.end(), std::inserter(brokenPathidsForDst, brokenPathidsForDst.begin()));
                    if (brokenPathidsForDst.empty())
                        dstToPathidsMapForUpdate.erase(dstMapItr++);     // step1: it_tobe = it+1; step2: erase(it); step3: it = it_tobe;
                    else {
                        dstMapItr->second = brokenPathidsForDst;
                        dstMapItr++;
                    }
                }
                // NOTE if forwardBrokenPathids isn't empty, there must be some dst in dstToPathidsMap that needs pathid in forwardBrokenPathids, otherwise the pathid would've been removed from forwardBrokenPathids
                // ASSERT(!dstToPathidsMapForUpdate.empty());

                auto reqOutgoing = reqIncoming;
                reqOutgoing->getLinkTraceForUpdate().push_back(vid);  // put myself in linkTrace
                reqOutgoing->setTtl(floodTtl-1);    // decrement ttl
                reqOutgoing->setBrokenPathids(forwardBrokenPathids);    // brokenPathids that don't exist

                // Commented out bc multiple dst contained in this RepairLinkReqFlood
                // double maxFloodJitterToDst = (psetTable.pneiIsLinked(dstVid)) ? (maxFloodJitter/8) : maxFloodJitter;    // reduce forward delay if dst is linked pnei
                double maxFloodJitterToDst = maxFloodJitter;
                sendCreatedRepairLocalReqFlood(reqOutgoing, /*computeChunkLength=*/true, /*delay=*/uniform(0, maxFloodJitterToDst));
                pktForwarded = true;
            }
        }
    }

    // utilize msg trace to record path to nodes close to me
    if (checkOverHeardTraces && reqIncoming->getLinkTrace().size() >= overHeardTraceMinCheckLen) {
        // trace: [src, .., node before me]
        addCloseNodesFromTrace(/*numHalf=*/vsetAndPendingHalfCardinality, reqIncoming->getLinkTrace(), /*removeLoop=*/false, /*traceEndIndex=*/reqIncoming->getLinkTrace().size()-1);
    } else if (checkOverHeardTraces) {
        // see if src of msg belongs to pendingVset
        pendingVsetAdd(srcVid, /*heardFrom=*/VLRRINGVID_NULL, /*addToEmpty=*/false);
    }
    // Commented out bc aren't sure if nodes in linkTrace satisfies greedy property
    // if (checkOverHeardMPneis && reqIncoming->getLinkTrace().size() >= 2) {
    //     const VlrIntVidVec& trace = reqIncoming->getLinkTrace();
    //     const VlrRingVID& prevhopVid = trace.back();
    //     if (psetTable.pneiIsLinked(prevhopVid)) {
    //         // trace: [src, .., node before me], go over every node in trace except node just before me
    //         for (int i = 0; i < trace.size()-1; i++)
    //             overheardMPneis[trace[i]] = std::make_pair(prevhopVid, simTime() + neighborValidityInterval);   // initialize expireTime of overheard trace to node
    //     }
    // }

    if (recordStatsToFile && recordDroppedMsg && !pktRecorded) {   // record message
        if (pktForMe)
            recordMessageRecord(/*action=*/1, /*src=*/srcVid, /*dst=*/vid, "RepairLocalReqFlood", /*msgId=*/floodSeqnum, /*hopcount=*/reqIncoming->getLinkTrace().size()+1, /*chunkByteLength=*/reqIncoming->getByteLength());
        else if (!pktForwarded)
            recordMessageRecord(/*action=*/4, /*src=*/srcVid, /*dst=*/vid, "RepairLocalReqFlood", /*msgId=*/floodSeqnum, /*hopcount=*/reqIncoming->getLinkTrace().size()+1, /*chunkByteLength=*/reqIncoming->getByteLength());
    }
}

// process all RepairLocalReqInfo in delayedRepairLocalReq after new breakage detected
void Vlr::processDelayedRepairLocalReq()
{
    for (auto reqInfoItr = delayedRepairLocalReq.begin(); reqInfoItr != delayedRepairLocalReq.end(); ) {
        if (reqInfoItr->second.expireTime <= simTime())    // repairLocalReqInfo has expired
            delayedRepairLocalReq.erase(reqInfoItr++);     // step1: it_tobe = it+1; step2: erase(it); step3: it = it_tobe;
        else {  // process repairLocalReqInfo as if I just received the repairLocalReq from src
            std::vector<VlrPathID> delayedBrokenPathids;  // pathids (from brokenPathids) whose nexthop still not broken yet
            processRepairLocalReqInfo(reqInfoItr->second.brokenPathids, reqInfoItr->second.linkTrace, delayedBrokenPathids);
            
            if (!delayedBrokenPathids.empty()) {    // these pathids should remain in delayedRepairLocalReq[src] in case their nexthop breakage isn't detected yet
                reqInfoItr->second.brokenPathids = delayedBrokenPathids;
                reqInfoItr++;
            } else {    // all brokenPathids received in this repairLocalReq are processed, no need to keep this repairLocalReqInfo
                delayedRepairLocalReq.erase(reqInfoItr++);
                // continue;
            }
        }
    }
}

// delayedBrokenPathids: pathids (from brokenPathids) found in vlrRoutingTable but no nexthop breakage detected yet, will be added to delayedRepairLinkReq if this function is called from processRepairLinkReqFlood()
void Vlr::processRepairLocalReqInfo(const std::vector<VlrPathID>& brokenPathids, const VlrIntVidVec& linkTrace, std::vector<VlrPathID>& delayedBrokenPathids)
{
    const VlrRingVID& srcVid = linkTrace.at(0);
    // check if linkTrace is valid
    std::vector<VlrRingVID> trace = linkTrace;   // trace: [start node, .., previous node]
    unsigned int nextHopIndex = getNextHopIndexInTrace(trace, /*preferShort=*/false);
    // trace: [start node, .., previous node]
    if (nextHopIndex >= trace.size()) {
        EV_WARN << "No next hop found to send RepairLocalReply at me = " << vid << " to src = " << srcVid << " because no node in linkTrace " << trace << " is a LINKED pnei, dropping RepairLocalReq" << endl;
    } else {    // next hop in linkTrace valid, check if brokenPathids are valid
        VlrRingVID nextHopVid = trace[nextHopIndex];
        // key: brokenPathids that are indeed broken (nexthop unavailable), will be repaired by this RepairLocalReply sent to src
        // value: [fromVid, toVid (, prevhop, prev of prevhop, ..)]
        std::map<VlrPathID, std::vector<VlrRingVID>> pathidToPrevhopMap;

        EV_WARN << "processRepairLocalReqInfo to me=" << vid << ": src = " << srcVid << ", trace = " << linkTrace << ", brokenPathids = " << brokenPathids << endl;
        // check if brokenPathids are indeed broken (nexthop unavailable)
        for (const VlrPathID& brokenPathid : brokenPathids) {
            auto vrouteItr = vlrRoutingTable.vlrRoutesMap.find(brokenPathid);
            if (vrouteItr != vlrRoutingTable.vlrRoutesMap.end()) {  // pathid still in vlrRoutingTable
                if (vlrRoutingTable.getPrevNextIsUnavailable(vrouteItr->second.isUnavailable, /*getPrev=*/false) == 1 && vrouteItr->second.prevhopVid != nextHopVid) { // route nexthop is unavailable, and new nexthop != prevhop 
                    // replyBrokenPathids.push_back(brokenPathid);
                    // remove route from lostPneis[nexthopVid].brokenVroutes
                    VlrRingVID nexthopVid = vrouteItr->second.nexthopVid;    // map nexthopAddr L3Address (lost pnei) to VlrRingVID
                    auto lostPneiItr = lostPneis.find(nexthopVid);
                    if (lostPneiItr != lostPneis.end()) {
                        lostPneiItr->second.brokenVroutes.erase(brokenPathid);
                    }
                    // change route nexthop (unavailable) to next hop in linkTrace, which I've checked is LINKED pnei
                    vrouteItr->second.nexthopVid = nextHopVid;
                    vlrRoutingTable.setRouteItrPrevNextIsUnavailable(vrouteItr, /*setPrev=*/false, /*value=*/0);     // set route nexthop state to available 
                    vlrRoutingTable.addRouteEndInEndpointMap(brokenPathid, vrouteItr->second.toVid);     // add back route toVid in endpointToRoutesMap
                    // add route to RepairLocalReply to be sent
                    auto itr_bool = pathidToPrevhopMap.insert({brokenPathid, {vrouteItr->second.fromVid, vrouteItr->second.toVid}});
                    ASSERT(itr_bool.second);    // assert brokenPathid inserted to pathidToPrevhopMap
                    std::vector<VlrRingVID>& routeEndAndPrev = itr_bool.first->second;
                    routeEndAndPrev.insert(routeEndAndPrev.end(), vrouteItr->second.prevhopVids.begin(), vrouteItr->second.prevhopVids.end());  // [fromVid, toVid, prevhopVids]
                }
                else {    // maybe lost of nexthop hasn't been detected, record repairLocalReq received
                    delayedBrokenPathids.push_back(brokenPathid);
                }
            }
        }

        if (!pathidToPrevhopMap.empty()) {  // there exists broken vroutes (nexthop unavailable) that can be repaired by linkTrace to src
            EV_WARN << "Sending RepairLocalReply from me = " << vid << " to RepairLocalReqFlood.src = " << srcVid << ", repairing brokenPathids: {";
            for (const auto& elem : pathidToPrevhopMap)
                EV_WARN << elem.first << " ";
            EV_WARN << "}, linkTrace = " << linkTrace << ", nexthop: " << nextHopVid << endl;

            // send RepairLocalReply to src using linkTrace to setup temporary route
            auto replyOutgoing = createRepairLocalReply();
            replyOutgoing->setPathidToPrevhopMap(pathidToPrevhopMap);
            replyOutgoing->setLinkTrace(trace);
            sendCreatedRepairLocalReply(replyOutgoing, /*outGateIndex=*/psetTable.getPneiGateIndex(nextHopVid), /*computeChunkLength=*/true);

            if (recordStatsToFile) {   // record sent message
                recordMessageRecord(/*action=*/0, /*src=*/vid, /*dst=*/srcVid, "RepairLocalReply", /*msgId=*/0, /*hopcount=*/0, /*chunkByteLength=*/0);   // unused params (msgId, hopcount, chunkByteLength) are just assigned to 0
            }

            if (checkOverHeardTraces)
                // see if src of msg belongs to pendingVset
                pendingVsetAdd(srcVid, /*heardFrom=*/VLRRINGVID_NULL, /*addToEmpty=*/false);
            
        }
    }
}

int Vlr::computeRepairLocalReplyByteLength(RepairLocalReplyInt* msg) const
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
    // std::vector<unsigned int> prevhopVids
    chunkByteLength += routePrevhopVidsSize * VLRRINGVID_BYTELEN;
    // // L3Address srcAddress;
    // chunkByteLength += addressByteLength;

    return chunkByteLength;
}

// still need to set linkTrace and pathidToPrevhopMap
RepairLocalReplyInt* Vlr::createRepairLocalReply()
{
    RepairLocalReplyInt *msg = new RepairLocalReplyInt(/*name=*/"RepairLocalReply");
    msg->setSrc(vid);
    msg->getPrevhopVidsForUpdate().push_back(vid);
    msg->setOldestPrevhopIndex(0);

    initializeVlrOption(msg->getVlrOptionForUpdate());

    msg->setMessageId(++allSendMessageId);
    msg->setHopcount(0);

    return msg;
}

// if computeChunkLength = true, compute chunk length because chunk was just created with createSetupReq() or modified after dupShared() from another chunk
// else, no need to compute chunk length because chunk was dupShared() (not modified) from another chunk that has chunkLength set
void Vlr::sendCreatedRepairLocalReply(RepairLocalReplyInt *msg, const int& outGateIndex, bool computeChunkLength/*=true*/, double delay/*=0*/)
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

void Vlr::processRepairLocalReply(RepairLocalReplyInt *replyIncoming, bool& pktForwarded)
{
    EV_DEBUG << "Received RepairLocalReply" << endl;
    // // VlrIntOption *vlrOptionIn = nullptr;
    // VlrIntOption& vlrOptionIn = replyIncoming->getVlrOptionForUpdate();
    VlrRingVID msgPrevHopVid = replyIncoming->getVlrOption().getPrevHopVid();
    if (msgPrevHopVid == VLRRINGVID_NULL)
        throw cRuntimeError("Received RepairLocalReply with vlrOption.prevHopVid = null");
        
    VlrRingVID srcVid = replyIncoming->getSrc();
    VlrRingVID dstVid = replyIncoming->getLinkTrace().at(0);
    const VlrIntPathidToVidVecMap& pathidToPrevhopMap = replyIncoming->getPathidToPrevhopMap();

    EV_INFO << "Processing RepairLocalReply: src = " << srcVid << ", dstVid = " << dstVid << ", pathidToPrevhopMap size = " << pathidToPrevhopMap.size() << ", prevhop: " << msgPrevHopVid << endl;

    if (recordStatsToFile && recordReceivedMsg) {   // record received message
        recordMessageRecord(/*action=*/2, /*src=*/srcVid, /*dst=*/dstVid, "RepairLocalReply", /*msgId=*/replyIncoming->getMessageId(), /*hopcount=*/replyIncoming->getHopcount()+1, /*chunkByteLength=*/replyIncoming->getByteLength());    // unimportant params (msgId)
    }
    bool pktForMe = false;      // set to true if this msg is directed to me or I processed it as its dst
    bool pktRecorded = false;      // set to true if this msg is recorded with recordMessageRecord()

    std::map<VlrRingVID, std::pair<std::vector<VlrPathID>, char>> nextHopToPathidsMap;    // for Teardown to send, map next hop address to (pathids, next hop 2-bit isUnavailable) pair
    std::set<VlrPathID> brokenPathidsToAdd;   // pathids in pathidToPrevhopMap that can be added to my vlrRoutingTable, I'll forward repairLocalReply for them

    for (const auto& mappair : pathidToPrevhopMap) {
        const VlrPathID& newPathid = mappair.first;

        auto vrouteItr = vlrRoutingTable.vlrRoutesMap.find(newPathid);
        bool teardownPathid = false;
        if (vrouteItr != vlrRoutingTable.vlrRoutesMap.end()) {  // newPathid already in vlrRoutingTable
            bool teardownOnlyToMsgPrevHop = false;
            if (dstVid == vid) {    // newPathid should be in vlrRoutingTable bc I sent repairLocalReq to repair newPathid
                if (vlrRoutingTable.getPrevNextIsUnavailable(vrouteItr->second.isUnavailable, /*getPrev=*/true) != 1) { // route prevhop is available, inconsistent
                    EV_WARN << "Handling RepairLocalReply to me=" << vid << ", the broken pathid " << newPathid << " in my routing table but not unavailable (isUnavailable=" << (int)vrouteItr->second.isUnavailable << "), tearing down both paths with the same pathid" << endl;
                    teardownPathid = true;
                    // check if I've recently received a repairLocalReply for newPathid from a node closer to me than src of this repairLocalReply, if so, accept this repairLocalReply and tear down the previous one
                    if (vlrRoutingTable.getPrevNextIsUnavailable(vrouteItr->second.isUnavailable, /*getPrev=*/true) == 0) {
                        auto recentBrokenItr = recentRepairLocalBrokenVroutes.find(newPathid);
                        if (recentBrokenItr != recentRepairLocalBrokenVroutes.end()) {
                            std::vector<VlrRingVID>& routePrevhopVids = recentBrokenItr->second.first;  // (p, prev of p, .., from] where p is the src of previously accepted repairLocalReply
                            if (std::find(routePrevhopVids.begin(), routePrevhopVids.end(), srcVid) != routePrevhopVids.end()) {    // src of this repairLocalReply can override src of previously accepted repairLocalReply
                                EV_WARN << "Cancelling teardown of pathid " << newPathid << " not unavailable (isUnavailable=" << (int)vrouteItr->second.isUnavailable << ") because src = " << srcVid << " of RepairLocalReply exists in recentRepairLocalBrokenVroutes[pathid]" << endl;
                                teardownPathid = false;
                            } else {    // src of this repairLocalReply can NOT override src of previously accepted repairLocalReply; src sent this repairLocalReply to me bc I've sent repairLocalReq to it asking it to repair newPathid, therefore I can pick the repairLocalReply that's more suitable and reject the others w/o having to tear down the accepted repair 
                                EV_WARN << "Rejecting RepairLocalReply to me=" << vid << " but keeping pathid " << newPathid << " not unavailable (isUnavailable=" << (int)vrouteItr->second.isUnavailable << ") in my routing table, sending Teardown only to the previous hop of RepairLocalReply" << endl;
                                teardownOnlyToMsgPrevHop = true;
                            }
                        }
                    }
                }
            } else {    // newPathid shouldn't be in vlrRoutingTable
                teardownPathid = true;
            }
            if (teardownPathid) {   // tear down newPathid
                if (!teardownOnlyToMsgPrevHop) {
                    EV_WARN << "The broken pathid " << newPathid << " of repairLocalReply is already in my routing table, tearing down both paths with the same pathid" << endl;
                    // tear down path recorded in routing table (pathid duplicate, something is wrong, can be a loop)
                    std::vector<VlrRingVID> sendTeardownToAddrs = {vrouteItr->second.prevhopVid, vrouteItr->second.nexthopVid};
                    // get 2-bit status of prevhop and nexthop
                    std::vector<char> nextHopStates = {vlrRoutingTable.getPrevNextIsUnavailable(vrouteItr->second.isUnavailable, /*getPrev=*/true), vlrRoutingTable.getPrevNextIsUnavailable(vrouteItr->second.isUnavailable, /*getPrev=*/false)};
                    // send Teardown to prevhop and nexthop of newPathid recorded in routing table
                    for (size_t i = 0; i < sendTeardownToAddrs.size(); ++i) {
                        const VlrRingVID& nextHopVid = sendTeardownToAddrs[i];
                        if (nextHopVid != VLRRINGVID_NULL) {  // I'm not an endpoint of the vroute
                            const char& nextHopIsUnavailable = nextHopStates[i];
                            if (nextHopIsUnavailable != 1) {    // next hop isn't unavailable
                                auto nextHopItr = nextHopToPathidsMap.find(nextHopVid);
                                if (nextHopItr == nextHopToPathidsMap.end()) 
                                    nextHopToPathidsMap[nextHopVid] = {{newPathid}, nextHopIsUnavailable};
                                else
                                    nextHopItr->second.first.push_back(newPathid);
                            } else {    // next hop is unavailable, remove this vroute that have been torn down from lostPneis.brokenVroutes
                                removeRouteFromLostPneiBrokenVroutes(newPathid, /*lostPneiVid=*/nextHopVid);
                            }
                        }
                    }
                    // send Teardown to msgPrevHopVid
                    if (msgPrevHopVid != vrouteItr->second.prevhopVid && msgPrevHopVid != vrouteItr->second.nexthopVid) {
                        const VlrRingVID& nextHopVid = msgPrevHopVid;
                        auto nextHopItr = nextHopToPathidsMap.find(nextHopVid);
                        if (nextHopItr == nextHopToPathidsMap.end())
                            nextHopToPathidsMap[nextHopVid] = {{newPathid}, /*nextHopIsUnavailable=*/0};
                        else
                            nextHopItr->second.first.push_back(newPathid);
                    }
                    // check if I'm an endpoint of pathid, if so this may be a vset route, or maybe in nonEssRoutes
                    if (vrouteItr->second.fromVid == vid)
                        processTeardownAtEndpoint(newPathid, /*otherEnd=*/vrouteItr->second.toVid, /*msgIncoming=*/nullptr, /*rebuildTemp=*/true);
                    else if (vrouteItr->second.toVid == vid)
                        processTeardownAtEndpoint(newPathid, /*otherEnd=*/vrouteItr->second.fromVid, /*msgIncoming=*/nullptr, /*rebuildTemp=*/true);

                    vlrRoutingTable.removeRouteByPathID(newPathid);
                } else {    // send Teardown only to msgPrevHopVid w/o tearing down existing newPathid in my routing table
                    // tear down newPathid, i.e. send Teardown to msgPrevHopVid
                    auto nextHopItr = nextHopToPathidsMap.find(msgPrevHopVid);
                    if (nextHopItr == nextHopToPathidsMap.end())
                        nextHopToPathidsMap[msgPrevHopVid] = {{newPathid}, /*nextHopIsUnavailable=*/0};
                    else
                        nextHopItr->second.first.push_back(newPathid);
                }
            }
        } else {    // newPathid not found in vlrRoutingTable
            if (dstVid == vid) {    // newPathid should be in vlrRoutingTable bc I sent repairLocalReq to repair newPathid
                EV_WARN << "Handling RepairLocalReply to me=" << vid << ", but broken pathid " << newPathid << " is not found in my routing table, tearing down pathid" << endl;
                teardownPathid = true;

                // tear down newPathid, i.e. send Teardown to msgPrevHopVid
                auto nextHopItr = nextHopToPathidsMap.find(msgPrevHopVid);
                if (nextHopItr == nextHopToPathidsMap.end())
                    nextHopToPathidsMap[msgPrevHopVid] = {{newPathid}, /*nextHopIsUnavailable=*/0};
                else
                    nextHopItr->second.first.push_back(newPathid);
            }
        }
        
        if (!teardownPathid) {
            // checked newPathid not in vlrRoutingTable
            if (!psetTable.pneiIsLinked(msgPrevHopVid)) {        // if prevHopAddr isn't a LINKED pnei 
                EV_WARN << "Previous hop " << msgPrevHopVid << " of repairLocalReply is not a LINKED pnei, tearing down pathid " << newPathid << endl;
                // tear down newPathid, i.e. send Teardown to msgPrevHopVid
                auto nextHopItr = nextHopToPathidsMap.find(msgPrevHopVid);
                if (nextHopItr == nextHopToPathidsMap.end())
                    nextHopToPathidsMap[msgPrevHopVid] = {{newPathid}, /*nextHopIsUnavailable=*/0};
                else
                    nextHopItr->second.first.push_back(newPathid);
            }
            // checked this message received from a LINKED pnei
            else if (representativeFixed && representative.heardfromvid == VLRRINGVID_NULL) {        // if I don't have a valid rep yet, I won't process or forward this message
                EV_WARN << "No valid rep heard: " << representative << ", cannot accept overlay message, tearing down pathid " << newPathid << endl;
                // tear down newPathid, i.e. send Teardown to msgPrevHopVid
                auto nextHopItr = nextHopToPathidsMap.find(msgPrevHopVid);
                if (nextHopItr == nextHopToPathidsMap.end())
                    nextHopToPathidsMap[msgPrevHopVid] = {{newPathid}, /*nextHopIsUnavailable=*/0};
                else
                    nextHopItr->second.first.push_back(newPathid);
            }
            // checked I have a valid rep
            else {
                brokenPathidsToAdd.insert(newPathid);
            }
        }
    }


    if (dstVid == vid) {
        pktForMe = true;
        EV_WARN << "Handling RepairLocalReply to me=" << vid << ": src = " << srcVid << ", pathidToPrevhopMap = {";
        for (const auto& mapelem : pathidToPrevhopMap) {
            EV_WARN << mapelem.first << ": [";
            for (const auto& elem : mapelem.second)
                EV_WARN  << elem << " ";
            EV_WARN << "], ";
        }
        EV_WARN << "}" << endl;

        std::vector<VlrRingVID> replyPrevhopVids;   // [closest prevhop, .., farthest prevhop] traversed by this repairLocalReply
        setRoutePrevhopVids(replyPrevhopVids, replyIncoming->getPrevhopVids(), replyIncoming->getOldestPrevhopIndex());

        if (!brokenPathidsToAdd.empty()) {

            // process repairLocalReply, only keep brokenPathidsToAdd in pathidToPrevhopMap
            VlrIntPathidToVidVecMap& pathidToPrevhopMapForUpdate = replyIncoming->getPathidToPrevhopMapForUpdate();     // using ..ForUpdate() (bc setRoutePrevhopVidsFromPrevhopMap() doesn't work w/ const ref) but not modifying pathidToPrevhopMap in RepairLocalReply bc we aren't forwarding it
            std::map<VlrRingVID, std::pair<VlrIntPathidToVidVecMap, char>> nextHopToPathidPrevhopMap;    // for RepairLocalPrev to send, map next hop address to (pathidToPrevhopMap, next hop 2-bit isUnavailable) pair

            for (const auto& brokenPathid : brokenPathidsToAdd) {
                auto vrouteItr = vlrRoutingTable.vlrRoutesMap.find(brokenPathid);
                // checked brokenPathid still in vlrRoutingTable and route prevhop is unavailable
                ASSERT(vrouteItr != vlrRoutingTable.vlrRoutesMap.end());  // pathid still in vlrRoutingTable
                VlrRingVID prevhopVid = vrouteItr->second.prevhopVid;    // map prevhopAddr L3Address (lost pnei) to VlrRingVID
                if (vlrRoutingTable.getPrevNextIsUnavailable(vrouteItr->second.isUnavailable, /*getPrev=*/true) == 0) { // route prevhop is available
                    // send Teardown to prevhop
                    auto nextHopItr = nextHopToPathidsMap.find(prevhopVid);
                    if (nextHopItr == nextHopToPathidsMap.end()) 
                        nextHopToPathidsMap[prevhopVid] = {{brokenPathid}, /*isUnavailable=*/0};
                    else
                        nextHopItr->second.first.push_back(brokenPathid);
                } else {    // route prevhop is unavailable
                    ASSERT(vlrRoutingTable.getPrevNextIsUnavailable(vrouteItr->second.isUnavailable, /*getPrev=*/true) == 1); // route prevhop is unavailable
                    // remove route from lostPneis[prevhopVid].brokenVroutes
                    
                    auto lostPneiItr = lostPneis.find(prevhopVid);
                    if (lostPneiItr != lostPneis.end()) {
                        lostPneiItr->second.brokenVroutes.erase(brokenPathid);
                    }
                }
                
                // change route prevhop (unavailable) to prev hop in linkTrace, which I've checked is LINKED pnei
                vrouteItr->second.prevhopVid = msgPrevHopVid;
                vlrRoutingTable.setRouteItrPrevNextIsUnavailable(vrouteItr, /*setPrev=*/true, /*value=*/0);     // set route prevhop state to available
                vlrRoutingTable.addRouteEndInEndpointMap(brokenPathid, vrouteItr->second.fromVid);     // add back route fromVid in endpointToRoutesMap

                // record recentRepairLocalBrokenVroutes[brokenPathid] = (p, prev of p, .., from] where p is src of this repairLocalReply
                auto recentBrokenItr = recentRepairLocalBrokenVroutes.find(brokenPathid);
                if (recentBrokenItr == recentRepairLocalBrokenVroutes.end()) {
                    auto itr_bool = recentRepairLocalBrokenVroutes.insert({brokenPathid, {vrouteItr->second.prevhopVids, simTime() + repairLinkReqWaitTime}});
                    recentBrokenItr = itr_bool.first;

                    VlrRingVID fromVid = vrouteItr->second.fromVid;
                    std::vector<VlrRingVID>& routePrevhopVids = recentBrokenItr->second.first;
                    if (!routePrevhopVids.empty() && routePrevhopVids.back() != fromVid)  // fromVid isn't in routePrevhopVids
                        routePrevhopVids.push_back(fromVid);
                }
                // remove from prevhopVids any nodes before and including src
                int srcIndex = -1;
                std::vector<VlrRingVID>& recentBrokenPrevhopVids = recentBrokenItr->second.first;
                for (unsigned int i = 0; i < recentBrokenPrevhopVids.size(); i++)
                    if (recentBrokenPrevhopVids[i] == srcVid)
                        srcIndex = i;
                ASSERT(srcIndex >= 0);      // assert src is found in recentBrokenPrevhopVids[brokenPathid]
                recentBrokenPrevhopVids.erase(recentBrokenPrevhopVids.begin(), recentBrokenPrevhopVids.begin() + srcIndex+1);

                // change route prevhopVids
                auto pathidMapItr = pathidToPrevhopMapForUpdate.find(brokenPathid);
                ASSERT(pathidMapItr != pathidToPrevhopMapForUpdate.end());      // all pathids in brokenPathidsToAdd were added from pathidToPrevhopMap
                std::vector<VlrRingVID>& routeEndAndPrev = pathidMapItr->second;
                std::vector<VlrRingVID>& routePrevhopVids = vrouteItr->second.prevhopVids;
                // routePrevhopVids = replyPrevhopVids + routeEndAndPrev[2:], limit size to routePrevhopVidsSize
                routePrevhopVids = replyPrevhopVids;    // NOTE replyPrevhopVids.size() <= routePrevhopVidsSize
                unsigned int routeEndAndPrev_startindex = 2; // routeEndAndPrev[2:] is prevhopVids of newPathid at src, may be empty
                setRoutePrevhopVidsFromPrevhopMap(routePrevhopVids, routeEndAndPrev, routeEndAndPrev_startindex, /*routeEndAndPrev_trim=*/false);

                // send RepairLocalPrev to nexthop in brokenPathid
                const VlrRingVID& nextHopVid = vrouteItr->second.nexthopVid;
                char nextHopIsUnavailable = vlrRoutingTable.getPrevNextIsUnavailable(vrouteItr->second.isUnavailable, /*getPrev=*/false);
                if (nextHopVid != VLRRINGVID_NULL && nextHopIsUnavailable != 1) {   // there's nexthop in brokenPathid and it's connected
                    auto nextHopItr = nextHopToPathidPrevhopMap.find(nextHopVid);
                    if (nextHopItr == nextHopToPathidPrevhopMap.end()) {
                        auto itr_bool = nextHopToPathidPrevhopMap.insert({nextHopVid, {{}, nextHopIsUnavailable}});  // std::pair<iterator, bool>: iterator (to the inserted element), bool (whether the element is inserted)
                        nextHopItr = itr_bool.first;
                    }
                    auto itr_bool = nextHopItr->second.first.insert({brokenPathid, {}});  // std::pair<iterator, bool>: iterator (to the inserted element), bool (whether the element is inserted)
                    // NOTE brokenPathid shouldn't already exist in pathidToPrevhopMap to be sent to nextHopVid, bc brokenPathidsToAdd is a set, every brokenPathid is inserted only once
                    ASSERT(itr_bool.second);
                    std::vector<VlrRingVID>& routePrevInMap = itr_bool.first->second;

                    // routeEndAndPrev before (in RepairLocalReply): [fromVid, toVid, prevhopVids at src]
                    // routePrevInMap  after  (in RepairLocalPrev):  [me, prevhopVids at me (exclude farthest prevhop from me)]
                    routePrevInMap = {vid};
                    ASSERT(!routePrevhopVids.empty());  // I have at least a prevhop bc prevhop sent this repairLocalReply to me
                    setRoutePrevhopVidsInRepairLocalPrevToForward(routePrevInMap, routePrevhopVids);    // copy prevhopVids at me, but limit total size to routePrevhopVidsSize
                }
            }

            EV_WARN << "Sending RepairLocalPrev from me = " << vid << " for RepairLocalReply.src = " << srcVid << ", repairing brokenPathids: {";
            for (const auto& nextHopMappair : nextHopToPathidPrevhopMap) {
                EV_WARN << "(nextHop=" << nextHopMappair.first << ") ";
                for (const auto& pathidPrevhopMappair : nextHopMappair.second.first) {
                    EV_WARN << pathidPrevhopMappair.first << ": [";
                    for (const auto& prevhop : pathidPrevhopMappair.second)
                        EV_WARN << prevhop << " ";
                    EV_WARN << "], ";
                }
            }
            EV_WARN << "}" << endl;

            // send RepairLocalPrev
            for (const auto& mappair : nextHopToPathidPrevhopMap) {
                auto msgOut = createRepairLocalPrev();
                msgOut->setPathidToPrevhopMap(mappair.second.first);
                VlrRingVID resultNextHopVid = getVlrOptionToNextHop(/*msg=*/msgOut, /*nextHopVid=*/mappair.first, /*nextHopIsUnavailable=*/mappair.second.second);     // check if next hop is a linked or lost pnei
                // checked that nextHopIsUnavailable != 1
                if (resultNextHopVid != VLRRINGVID_NULL) {
                    sendCreatedRepairLocalPrev(msgOut, /*nextHopPnei=*/resultNextHopVid, /*computeChunkLength=*/true);

                    if (recordStatsToFile) {   // record sent message
                        recordMessageRecord(/*action=*/0, /*src=*/vid, /*dst=*/VLRRINGVID_NULL, "RepairLocalPrev", /*msgId=*/0, /*hopcount=*/0, /*chunkByteLength=*/0);   // unused params (msgId, hopcount, chunkByteLength) are just assigned to 0
                    }
                }
            }

            if (checkOverHeardTraces)
                // see if src of msg belongs to pendingVset
                pendingVsetAdd(srcVid, /*heardFrom=*/VLRRINGVID_NULL, /*addToEmpty=*/false);            
        }

    }
    else {  // this repairLocalReply isn't destined for me
        if (!brokenPathidsToAdd.empty()) {
            unsigned int nextHopIndex = getNextHopIndexInTrace(replyIncoming->getLinkTraceForUpdate(), /*preferShort=*/false);
            // replyIncoming->getTrace(): [start node, .., parent node]
            if (nextHopIndex >= replyIncoming->getLinkTrace().size()) {
                EV_WARN << "No next hop found for RepairLocalReply with trace received at me = " << vid << " because no node in trace is a LINKED pnei, tearing down vroutes " << brokenPathidsToAdd << ", src = " << srcVid << ", linkTrace = " << replyIncoming->getLinkTrace() << endl;
                // teardown every route in brokenPathidsToAdd
                for (const auto& newPathid : brokenPathidsToAdd) {
                    // tear down newPathid, i.e. send Teardown to msgPrevHopVid
                    auto nextHopItr = nextHopToPathidsMap.find(msgPrevHopVid);
                    if (nextHopItr == nextHopToPathidsMap.end())
                        nextHopToPathidsMap[msgPrevHopVid] = {{newPathid}, /*nextHopIsUnavailable=*/0};
                    else
                        nextHopItr->second.first.push_back(newPathid);
                }
            }
            else {  // found LINKED next hop in linkTrace
                VlrRingVID nextHopVid = replyIncoming->getLinkTrace().at(nextHopIndex);

                std::vector<VlrRingVID> replyPrevhopVids;   // [closest prevhop, .., farthest prevhop] traversed by this repairLocalReply
                unsigned int oldestPrevhopIndex = setRoutePrevhopVidsFromMessage(replyPrevhopVids, replyIncoming->getPrevhopVidsForUpdate(), replyIncoming->getOldestPrevhopIndex());
                replyIncoming->setOldestPrevhopIndex(oldestPrevhopIndex);

                // forward repairLocalReply, only keep brokenPathidsToAdd in pathidToPrevhopMap
                VlrIntPathidToVidVecMap& pathidToPrevhopMapForUpdate = replyIncoming->getPathidToPrevhopMapForUpdate();
                for (auto it = pathidToPrevhopMapForUpdate.begin(); it != pathidToPrevhopMapForUpdate.end(); ) {
                    if (brokenPathidsToAdd.find(it->first) == brokenPathidsToAdd.end())     // pathid shouldn't be added to vlrRoutingTable
                        pathidToPrevhopMapForUpdate.erase(it++);     // step1: it_tobe = it+1; step2: erase(it); step3: it = it_tobe;
                    else {
                        const VlrPathID& newPathid = it->first;
                        std::vector<VlrRingVID>& routeEndAndPrev = it->second;
                        auto itr_bool = vlrRoutingTable.addRoute(newPathid, /*fromVid=*/routeEndAndPrev[0], /*toVid=*/routeEndAndPrev[1], /*prevhopVid=*/msgPrevHopVid, /*nexthopVid=*/nextHopVid, /*isVsetRoute=*/false);
                        std::vector<VlrRingVID>& routePrevhopVids = itr_bool.first->second.prevhopVids;
                        // routePrevhopVids = replyPrevhopVids + routeEndAndPrev[2:], limit size to routePrevhopVidsSize
                        routePrevhopVids = replyPrevhopVids;    // NOTE replyPrevhopVids.size() <= routePrevhopVidsSize
                        unsigned int routeEndAndPrev_startindex = 2; // routeEndAndPrev[2:] is prevhopVids of newPathid at src, may be empty

                        setRoutePrevhopVidsFromPrevhopMap(routePrevhopVids, routeEndAndPrev, routeEndAndPrev_startindex, /*routeEndAndPrev_trim=*/true);

                        it++;
                    }
                }
                auto replyOutgoing = replyIncoming;
                EV_INFO << "Sending RepairLocalReply to dst = " << dstVid << ", src = " << srcVid << ", pathids = " << brokenPathidsToAdd << ", trace = " << replyOutgoing->getLinkTrace() << endl;
                sendCreatedRepairLocalReply(replyOutgoing, /*outGateIndex=*/psetTable.getPneiGateIndex(nextHopVid), /*computeChunkLength=*/true);
                pktForwarded = true;
                
            }
        }
    }
    // send Teardown for brokenPathids that I didn't repair
    for (const auto& mappair : nextHopToPathidsMap) {
        const auto& teardownOut = createTeardown(/*pathids=*/mappair.second.first, /*addSrcVset=*/false, /*rebuild=*/true, /*dismantled=*/false);   // don't keep dismantled bc possible loop in vroute
        // checked that nextHopIsUnavailable != 1
        sendCreatedTeardownToNextHop(teardownOut, /*nextHopAddr=*/mappair.first, /*nextHopIsUnavailable=*/mappair.second.second);

        if (recordStatsToFile) {   // record sent message
            recordMessageRecord(/*action=*/0, /*src=*/vid, /*dst=*/VLRRINGVID_NULL, "Teardown", /*msgId=*/0, /*hopcount=*/0, /*chunkByteLength=*/0, /*infoStr=*/"repairLocalReply forward error");   // unused params (msgId, hopcount, chunkByteLength) are just assigned to 0
        }
    }

    if (recordStatsToFile && recordDroppedMsg && !pktRecorded) {   // record message
        if (pktForMe)
            recordMessageRecord(/*action=*/1, /*src=*/srcVid, /*dst=*/dstVid, "RepairLocalReply", /*msgId=*/replyIncoming->getMessageId(), /*hopcount=*/replyIncoming->getHopcount()+1, /*chunkByteLength=*/replyIncoming->getByteLength());
        else if (!pktForwarded)
            recordMessageRecord(/*action=*/4, /*src=*/srcVid, /*dst=*/dstVid, "RepairLocalReply", /*msgId=*/replyIncoming->getMessageId(), /*hopcount=*/replyIncoming->getHopcount()+1, /*chunkByteLength=*/replyIncoming->getByteLength());
    }
}

int Vlr::computeRepairLocalPrevByteLength(RepairLocalPrevInt* msg) const
{
    int chunkByteLength = 0;
    // std::map<VlrPathID, std::vector<unsigned int>> pathidToPrevhopMap;
    const auto& pathidToPrevhopMap = msg->getPathidToPrevhopMap();
    for (auto it = pathidToPrevhopMap.begin(); it != pathidToPrevhopMap.end(); ++it)
        chunkByteLength += VLRPATHID_BYTELEN + it->second.size() * VLRRINGVID_BYTELEN;

    return chunkByteLength;
}

// still need to set pathidToPrevhopMap
RepairLocalPrevInt* Vlr::createRepairLocalPrev()
{
    RepairLocalPrevInt *msg = new RepairLocalPrevInt(/*name=*/"RepairLocalPrev");
    msg->setSrc(vid);                      // vidByteLength

    initializeVlrOption(msg->getVlrOptionForUpdate());

    msg->setMessageId(++allSendMessageId);
    msg->setHopcount(0);

    return msg;
}

// VlrIntOption isn't needed in RepairLocalPrev bc temporary routes aren't needed to accommodate loops
void Vlr::sendCreatedRepairLocalPrev(RepairLocalPrevInt *msg, VlrRingVID nextHopPnei, bool computeChunkLength/*=true*/, double delay/*=0*/)
{
    if (computeChunkLength)
        msg->setByteLength(computeRepairLocalPrevByteLength(msg));
    EV_DEBUG << "Sending RepairLocalPrev: src = " << msg->getSrc() << ", pathids = {" << endl;
    for (auto it = msg->getPathidToPrevhopMap().begin(); it != msg->getPathidToPrevhopMap().end(); ++it)
        EV_DEBUG << it->first << " ";
    EV_DEBUG << "}" << endl;

    msg->getVlrOptionForUpdate().setPrevHopVid(vid);    // set packet prevHopVid to myself
    msg->setHopcount(msg->getHopcount() +1);    // increment packet hopcount

    sendCreatedPacket(msg, /*unicast=*/true, /*outGateIndex=*/psetTable.getPneiGateIndex(nextHopPnei), /*delay=*/delay, /*checkFail=*/true);
}

void Vlr::processRepairLocalPrev(RepairLocalPrevInt *msgIncoming, bool& pktForwarded)
{
    EV_DEBUG << "Received RepairLocalPrev" << endl;
    // IP.SourceAddress: address of the node from which the packet was received
    VlrIntOption& vlrOptionIn = msgIncoming->getVlrOptionForUpdate();
    VlrRingVID msgPrevHopVid = vlrOptionIn.getPrevHopVid();
    if (msgPrevHopVid == VLRRINGVID_NULL)
        throw cRuntimeError("Received RepairLocalPrev with vlrOption.prevHopVid = null");

    const VlrIntPathidToVidVecMap& pathidToPrevhopMap = msgIncoming->getPathidToPrevhopMap();
    EV_INFO << "Processing RepairLocalPrev: pathidToPrevhopMap = {";
    for (const auto& mapelem : pathidToPrevhopMap) {
        EV_WARN << mapelem.first << ": [";
        for (const auto& elem : mapelem.second)
            EV_WARN  << elem << " ";
        EV_WARN << "], ";
    }
    EV_WARN << "}" << endl;

    if (recordStatsToFile && recordReceivedMsg) {   // record received message
        // RepairRoute doesn't have <VlrCreationTimeTag> tag
        recordMessageRecord(/*action=*/2, /*src=*/msgIncoming->getSrc(), /*dst=*/vid, "RepairLocalPrev", /*msgId=*/pathidToPrevhopMap.begin()->first, /*hopcount=*/msgIncoming->getHopcount()+1, /*chunkByteLength=*/msgIncoming->getByteLength());  // unimportant params (msgId, hopcount)
    }
    bool pktForMe = false;      // set to true if this msg is directed to me or I processed it as its dst
    bool pktRecorded = false;      // set to true if this msg is recorded with recordMessageRecord()

    // const auto& networkHeader = getNetworkProtocolHeader(packet);
    // VlrIntOption *vlrOptionIn = const_cast<VlrIntOption *>(findVlrOptionInNetworkDatagram(networkHeader));  // may be nullptr if no VlrOption is provided in IP header, i.e. not traversing a temporary route
    
    // if traversing the temporary route portion of the vroute to forward RepairLocalPrev
    VlrPathID tempPathid = VLRPATHID_INVALID;
    tempPathid = vlrOptionIn.getTempPathid();
    if (tempPathid != VLRPATHID_INVALID) {
        auto tempPathItr = vlrRoutingTable.vlrRoutesMap.find(tempPathid);
        if (tempPathItr != vlrRoutingTable.vlrRoutesMap.end()) {   // tempPathid is in vlrRoutingTable
            VlrRingVID tempTowardVid = vlrOptionIn.getTempTowardVid();
            if (tempTowardVid == vid) {     // reached the end of temporary route portion
                const VlrRingVID& otherEnd = (vid == tempPathItr->second.fromVid) ? tempPathItr->second.toVid : tempPathItr->second.fromVid;
                auto lostPneiItr = lostPneis.find(otherEnd);
                if (lostPneiItr != lostPneis.end()) {
                    msgPrevHopVid = lostPneiItr->first;  // as if this msg is received directly from lost pnei
                } else {    // if otherEnd not in lostPneis, this means vroutes broken by otherEnd have been torn down and no longer exist in vlrRoutingTable, we no longer need to follow vroute to fix prevhopVids
                    EV_WARN << "RepairLocalPrev received from previous hop " << msgPrevHopVid << " via temporary route pathid " << tempPathid << ", but otherEnd = " << otherEnd << " of temporary route not found in lostPneis" << endl;
                    msgPrevHopVid = otherEnd;
                }
            } else {    // send to next hop in temporary route
                VlrRingVID nextHopVid = vlrRoutingTable.getRouteItrNextHop(tempPathItr, tempTowardVid);
                // forward msg with vlrOption
                // VlrIntOption* vlrOptionOut = vlrOptionIn->dup();
                // auto msgOut = staticPtrCast<RepairLocalPrevInt>(msgIncoming->dupShared());
                EV_INFO << "Sending RepairLocalPrev along temporary route = " << tempPathid << ", nexthop: " << nextHopVid << endl;
                sendCreatedRepairLocalPrev(msgIncoming, /*nextHopPnei=*/nextHopVid, /*computeChunkLength=*/false);
                pktForwarded = true;
                return;
            }
        } else {
            EV_WARN << "The temporary route pathid " << tempPathid << " of RepairLocalPrev is not in my routing table, dropping the RepairLocalPrev message and sending Teardown to previous hop " << msgPrevHopVid << endl;
            
            // tear down tempPathid, i.e. send Teardown to prevHopAddr
            const auto& teardownOut = createTeardownOnePathid(tempPathid, /*addSrcVset=*/false, /*rebuild=*/true);
            sendCreatedTeardown(teardownOut, /*nextHopPnei=*/msgPrevHopVid);

            if (recordStatsToFile) {   // record sent message
                recordMessageRecord(/*action=*/0, /*src=*/vid, /*dst=*/VLRRINGVID_NULL, "Teardown", /*msgId=*/0, /*hopcount=*/0, /*chunkByteLength=*/0, /*infoStr=*/"processRepairLocalPrev: using temporary route but not found");   // unused params (msgId, hopcount, chunkByteLength) are just assigned to 0

                if (recordDroppedMsg && !pktRecorded) {     // record dropped message
                    recordMessageRecord(/*action=*/4, /*src=*/msgIncoming->getSrc(), /*dst=*/vid, "RepairLocalPrev", /*msgId=*/pathidToPrevhopMap.begin()->first, /*hopcount=*/msgIncoming->getHopcount()+1, /*chunkByteLength=*/msgIncoming->getByteLength());  // unimportant params (msgId, hopcount)
                    pktRecorded = true;
                }
            }
            return;
        }
    }

    std::map<VlrRingVID, std::pair<VlrIntPathidToVidVecMap, char>> nextHopToPathidPrevhopMap;    // for RepairLocalPrev to send, map next hop address to (pathidToPrevhopMap, next hop 2-bit isUnavailable) pair
    for (const auto& pathidMappair : pathidToPrevhopMap) {
        const VlrPathID& oldPathid = pathidMappair.first;
        const std::vector<VlrRingVID>& pathidPrevhopVids = pathidMappair.second;

        auto vrouteItr = vlrRoutingTable.vlrRoutesMap.find(oldPathid);
        if (vrouteItr == vlrRoutingTable.vlrRoutesMap.end()) {  // oldPathid not found in vlrRoutingTable
            EV_WARN << "The pathid " << oldPathid << " of RepairLocalPrev is not in my routing table, dropping message for this pathid and sending Teardown to previous hop " << msgPrevHopVid << endl;

            if (tempPathid != VLRPATHID_INVALID) {  // this message arrived through temporary route, I must be an endpoint of the temporary route, previous hop doesn't have oldPathid, need to notify other end of tempPathid
                auto tempPathItr = vlrRoutingTable.vlrRoutesMap.find(tempPathid);
                if (tempPathItr != vlrRoutingTable.vlrRoutesMap.end()) {
                    const VlrRingVID& otherEnd = (vid == tempPathItr->second.fromVid) ? tempPathItr->second.toVid : tempPathItr->second.fromVid;
                    auto lostPneiItr = lostPneis.find(otherEnd);
                    if (lostPneiItr != lostPneis.end()) {
                        const VlrRingVID& prevHopVid = lostPneiItr->first;  // send Teardown of oldPathid to other end of tempPathid
                        EV_WARN << "Sending Teardown for unfound pathid = " << oldPathid << " through tempPathid=" << tempPathid << " to previous hop vid=" << otherEnd << endl;
                        const auto& teardownOut = createTeardownOnePathid(oldPathid, /*addSrcVset=*/false, /*rebuild=*/true);
                        sendCreatedTeardownToNextHop(teardownOut, prevHopVid, /*nextHopIsUnavailable=*/2);

                        if (recordStatsToFile) {   // record sent message
                            recordMessageRecord(/*action=*/0, /*src=*/vid, /*dst=*/VLRRINGVID_NULL, "Teardown", /*msgId=*/0, /*hopcount=*/0, /*chunkByteLength=*/0, /*infoStr=*/"processRepairLocalPrev: pathid not found");   // unused params (msgId, hopcount, chunkByteLength) are just assigned to 0
                        }
                    }
                }
            } else {    // previous hop is directly connected
                // tear down oldPathid, i.e. send Teardown to prevHopAddr
                const auto& teardownOut = createTeardownOnePathid(oldPathid, /*addSrcVset=*/false, /*rebuild=*/true);
                sendCreatedTeardown(teardownOut, /*nextHopPnei=*/msgPrevHopVid);

                if (recordStatsToFile) {   // record sent message
                    recordMessageRecord(/*action=*/0, /*src=*/vid, /*dst=*/VLRRINGVID_NULL, "Teardown", /*msgId=*/0, /*hopcount=*/0, /*chunkByteLength=*/0, /*infoStr=*/"processRepairLocalPrev: pathid not found");   // unused params (msgId, hopcount, chunkByteLength) are just assigned to 0
                }
            }
        }
        else {  // checked oldPathid is in vlrRoutingTable
            if (msgPrevHopVid != vrouteItr->second.prevhopVid && msgPrevHopVid != vrouteItr->second.nexthopVid) {
                EV_WARN << "Sender " << msgPrevHopVid << " of RepairLocalPrev is neither prevhop nor nexthop of pathid " << oldPathid << " to be repaired, dropping message for this pathid" << endl;
            } else {    // sender of this message is either prevhopAddr or nexthopAddr of the vroute
                bool isToNexthop = (msgPrevHopVid == vrouteItr->second.prevhopVid);
                const VlrRingVID& nextHopVid = (isToNexthop) ? vrouteItr->second.nexthopVid : vrouteItr->second.prevhopVid;
                char nextHopIsUnavailable = vlrRoutingTable.getPrevNextIsUnavailable(vrouteItr->second.isUnavailable, /*getPrev=*/!isToNexthop);     // if isToNexthop, next hop is nexthop, getPrev=false
                EV_INFO << "The pathid " << oldPathid << " of RepairLocalPrev is found in routing table, nextHopVid: " << nextHopVid << " (specified=" << (nextHopVid!=VLRRINGVID_NULL) << ", isUnavailable=" << (int)nextHopIsUnavailable << ")"
                        << ", modified prevhopVids = " << pathidPrevhopVids << endl;

                ASSERT(msgPrevHopVid == vrouteItr->second.prevhopVid);  // RepairLocalPrev is received from prevhop and forwarded to nexthop for every route
                std::vector<VlrRingVID>& routePrevhopVids = vrouteItr->second.prevhopVids;

                unsigned int diffIndex = getRoutePrevhopVidsDiffIndex(routePrevhopVids, pathidPrevhopVids);     // index of first node that's different in original prevhopVids and new prevhopVids received
                routePrevhopVids = pathidPrevhopVids;  // modify route prevhopVids

                if (diffIndex < routePrevhopVidsSize-1) {    // not only the farthest prevhop is modified (if only the farthest prevhop is modified, farthest prevhop will be crowded out by my vid in prevhopVids to be forwarded, then assuming nexthop's prevhopVids is consistent w/ me, nexthop won't need to modify its prevhopVids)
                    if (nextHopVid != VLRRINGVID_NULL && nextHopIsUnavailable != 1) {   // there's nexthop in oldPathid and it's connected
                        auto nextHopItr = nextHopToPathidPrevhopMap.find(nextHopVid);
                        if (nextHopItr == nextHopToPathidPrevhopMap.end()) {
                            auto itr_bool = nextHopToPathidPrevhopMap.insert({nextHopVid, {{}, nextHopIsUnavailable}});  // std::pair<iterator, bool>: iterator (to the inserted element), bool (whether the element is inserted)
                            nextHopItr = itr_bool.first;
                        }
                        auto itr_bool = nextHopItr->second.first.insert({oldPathid, {}});  // std::pair<iterator, bool>: iterator (to the inserted element), bool (whether the element is inserted)
                        std::vector<VlrRingVID>& routePrevInMap = itr_bool.first->second;

                        // routePrevInMap (in RepairLocalPrev): [me, prevhopVids at me (exclude farthest prevhop from me)]
                        routePrevInMap = {vid};
                        ASSERT(!routePrevhopVids.empty());  // I have at least a prevhop bc prevhop forwarded this repairLocalPrev to me
                        setRoutePrevhopVidsInRepairLocalPrevToForward(routePrevInMap, routePrevhopVids);    // copy prevhopVids at me, but limit total size to routePrevhopVidsSize
                    }
                } else    // RepairLocalPrev doesn't need to be forwarded to next hop
                    pktForMe = true;
            }
        }
    }
    // forward RepairLocalPrev
    int numSent = 0;
    for (const auto& mappair : nextHopToPathidPrevhopMap) {
        if (numSent++ == 0)     // numSent == 0, haven't forwarded the Teardown I received
            pktForwarded = true;
        else    // numSent > 0, already sent the Teardown I received
            msgIncoming = msgIncoming->dup();
        
        msgIncoming->setPathidToPrevhopMap(mappair.second.first);   // pathidToPrevhopMap must be modified bc prevhopVids for each route has been modified
        
        VlrRingVID resultNextHopVid = getVlrOptionToNextHop(msgIncoming, /*nextHopVid=*/mappair.first, /*nextHopIsUnavailable=*/mappair.second.second);     // check if next hop is a linked or lost pnei
        // checked that nextHopIsUnavailable != 1
        if (resultNextHopVid != VLRRINGVID_NULL)
            sendCreatedRepairLocalPrev(msgIncoming, /*nextHopPnei=*/resultNextHopVid, /*computeChunkLength=*/true);
        
    }

    if (recordStatsToFile && recordDroppedMsg && !pktRecorded) {   // record message
        if (pktForMe)
            recordMessageRecord(/*action=*/1, /*src=*/msgIncoming->getSrc(), /*dst=*/vid, "RepairLocalPrev", /*msgId=*/pathidToPrevhopMap.begin()->first, /*hopcount=*/msgIncoming->getHopcount()+1, /*chunkByteLength=*/msgIncoming->getByteLength());  // unimportant params (msgId, hopcount)
        else if (!pktForwarded)
            recordMessageRecord(/*action=*/4, /*src=*/msgIncoming->getSrc(), /*dst=*/vid, "RepairLocalPrev", /*msgId=*/pathidToPrevhopMap.begin()->first, /*hopcount=*/msgIncoming->getHopcount()+1, /*chunkByteLength=*/msgIncoming->getByteLength());  // unimportant params (msgId, hopcount)
    }
}


// set routePrevhops: [closest prevhop, .., farthest prevhop]
void Vlr::setRoutePrevhopVids(VlrIntVidVec& routePrevhops, const VlrIntVidVec& msgPrevhops, const unsigned int& oldestPrevhopIndex)
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
unsigned int Vlr::setRoutePrevhopVidsFromMessage(VlrIntVidVec& routePrevhops, VlrIntVidVec& msgPrevhops, const unsigned int& oldestPrevhopIndex)
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

// routePrevhops: [prevhop, prev of prevhop, ..], msgPrevhops: [.., prev of prevhop, prevhop], oldestPrevhopIndex: index of oldest prevhopVid in msgPrevhops
// set msgPrevhops based on routePrevhops      NOTE oldestPrevhopIndex will always be at 0 in modified msgPrevhops
// void Vlr::setMessagePrevhopVidsFromRoute(const VlrIntVidVec& routePrevhops, VlrIntVidVec& msgPrevhops)
// {
//     msgPrevhops.clear();
//     // oldestPrevhopIndex should be 0 when routePrevhops.size() + 1 (myself) <= routePrevhopVidsSize
//     if (!routePrevhops.empty()) {
//         // copy vids in routePrevhops to msgPrevhops in reverse order
//         for (auto rit = routePrevhops.rbegin(); rit!= routePrevhops.rend(); ++rit)    // rit type: std::vector<VlrRingVID>::reverse_iterator
//             msgPrevhops.push_back(*rit);
//     }
// }

// if routePrevhopVids.size() < routePrevhopVidsSize, append nodes in routeEndAndPrev (but limit size to routePrevhopVidsSize), starting from routeEndAndPrev[routeEndAndPrev_startindex] if routeEndAndPrev.size() permits
// if routeEndAndPrev_trim=true, remove the last node appended to routePrevhopVids in routeEndAndPrev, and any nodes after it, if no node appended, all nodes from routeEndAndPrev_startindex will be removed
void Vlr::setRoutePrevhopVidsFromPrevhopMap(std::vector<VlrRingVID>& routePrevhopVids, std::vector<VlrRingVID>& routeEndAndPrev, unsigned int routeEndAndPrev_startindex, bool routeEndAndPrev_trim)
{
    // routeEndAndPrev[2:] is prevhopVids of newPathid at src, may be empty
    unsigned int routeEndAndPrev_index = routeEndAndPrev_startindex;
    while (routePrevhopVids.size() < routePrevhopVidsSize) {
        if (routeEndAndPrev_index < routeEndAndPrev.size())
            routePrevhopVids.push_back( routeEndAndPrev[ routeEndAndPrev_index++ ] );
        else   // reached end of routeEndAndPrev
            break;
    }
    if (routeEndAndPrev_trim) {
        // if added from routeEndAndPrev, last node (farthest prevhop) added to routePrevhopVids is routeEndAndPrev[routeEndAndPrev_index-1]
        // if didn't add from routeEndAndPrev, it's either because routePrevhopVids is full or routeEndAndPrev doesn't have enough nodes to fill routePrevhopVids (in this case we don't remove nodes from routeEndAndPrev)
        // if routePrevhopVids is full, we can remove the farthest prevhop added, and any prevhops even farther
        if (routePrevhopVids.size() >= routePrevhopVidsSize) {
            if (routeEndAndPrev_index > routeEndAndPrev_startindex)     // added at least one prevhop from routeEndAndPrev to routePrevhopVids, we'll remove the farthest prevhop added, and any prevhops even farther
                routeEndAndPrev.erase(routeEndAndPrev.begin() + routeEndAndPrev_index-1, routeEndAndPrev.end());
            else if (routeEndAndPrev.size() > routeEndAndPrev_startindex)   // didn't add any prevhop from routeEndAndPrev, only keep routeEndAndPrev[0~1] which are route fromVid and toVid
                routeEndAndPrev.erase(routeEndAndPrev.begin() + routeEndAndPrev_startindex, routeEndAndPrev.end());
        }
    }
}

// append nodes in routePrevhopVids to routePrevToForward, but limit size to routePrevhopVidsSize
void Vlr::setRoutePrevhopVidsInRepairLocalPrevToForward(std::vector<VlrRingVID>& routePrevToForward, const std::vector<VlrRingVID>& routePrevhopVids)
{
    unsigned int numToInsert = routePrevhopVids.size();
    if (numToInsert + routePrevToForward.size() > routePrevhopVidsSize)
        numToInsert = routePrevhopVidsSize - routePrevToForward.size();
    routePrevToForward.insert(routePrevToForward.end(), routePrevhopVids.begin(), routePrevhopVids.begin() + numToInsert);
    // if (routePrevToForward.size() > routePrevhopVidsSize)
    //     routePrevToForward.erase(routePrevToForward.begin() + routePrevhopVidsSize, routePrevToForward.end());
}

// get index of first node that's different in vec1 and vec2, e.g. vec1=[0,4], vec2=[0,4,6], return diffIndex=2
unsigned int Vlr::getRoutePrevhopVidsDiffIndex(const std::vector<VlrRingVID>& vec1, const std::vector<VlrRingVID>& vec2) const
{
    unsigned int diffIndex = 0;
    unsigned int i = 0;
    while (i < vec1.size() && i < vec2.size()) {
        if (vec1[i] == vec2[i])
            diffIndex++;
        else
            break;
        i++;
    }
    return diffIndex;
}

// add nodes in packet's srcVset to my pendingVset if relevant
void Vlr::processOtherVset(const VlrIntSetupPacket *setupPacket, const VlrRingVID& srcVid, bool addToEmpty/*=true*/)
{
    int oVsetSize = setupPacket->getSrcVsetArraySize();
    
    for (int k = 0; k < oVsetSize; ++k) {
        VlrRingVID node = setupPacket->getSrcVset(k);
        pendingVsetAdd(node, /*heardFrom=*/srcVid, addToEmpty);
    }
}

// if addToEmpty = false && not inNetwork, node may only be added to pendingVset if there are at least <addToMinSize> existing nodes in it
// if nodeAlive = true, node should be alive and I can keep sending setupReq to it regardless of retryCount
bool Vlr::pendingVsetAdd(VlrRingVID node, VlrRingVID heardFrom/*=VLRRINGVID_NULL*/, bool addToEmpty/*=true*/, bool nodeAlive/*=true*/)
{
    int addToMinSize = 2 * vsetHalfCardinality;     // minimum pendingVset size to qualify node add if addToEmpty=false

    bool addedNode = false;
    int pendingVsetMaxSize = 2 * pendingVsetHalfCardinality;
    if (addToEmpty || selfInNetwork || pendingVset.size() >= addToMinSize) {   // if need condition vset.size() > 0, we should only add otherVset sent by my vnei to pendingVset, not otherVset in SetupFail, SetupReq from non-vnei, etc
        if (node != vid && vset.find(node) == vset.end() && node != VLRRINGVID_NULL) {  // node not myself and not a vnei in vset
            auto pendingItr = pendingVset.find(node);
            if (pendingItr == pendingVset.end()) {  // node not in pendingVset, see if it should be added
                if (pendingVset.size() < pendingVsetMaxSize) {   // pendingVset not yet full
                    pendingVset[node] = {nullptr, simTime(), heardFrom};
                    addedNode = true;
                    // pendingVsetFarthest needs to be recalculated bc we didn't compare node with current pendingVsetFarthest, thus it may be farther than them
                    pendingVsetFarthest.first = VLRRINGVID_NULL;

                    EV_DETAIL << "Adding node " << node << " to pendingVset {";
                    for (const auto& elem : pendingVset)
                        EV_DETAIL << elem.first << " ";
                    EV_DETAIL << "}" << endl;
                }
                else {      // pendingVset full
                    VlrRingVID& farthestCCW = pendingVsetFarthest.first;
                    VlrRingVID& farthestCW = pendingVsetFarthest.second;
                    if (farthestCCW == VLRRINGVID_NULL) {   // pendingVsetFarthest not yet set bc pendingVset wasn't full or bc previous farthest was removed, get farthest ccw and cw node in pendingVset
                        auto itrup = pendingVset.upper_bound(vid);      // itrup points to closest cw node in pendingVset
                        // increment itrup to get the node with largest cw distance to me
                        advanceIteratorWrapAround(itrup, pendingVsetHalfCardinality-1, pendingVset.begin(), pendingVset.end());
                        auto itrlow = itrup;
                        // increment itrlow to get the node with largest ccw distance to me
                        advanceIteratorWrapAround(itrlow, 1, pendingVset.begin(), pendingVset.end());

                        farthestCCW = itrlow->first;
                        farthestCW = itrup->first;
                    }
                    if (getVid_CCW_Distance(vid, node) < getVid_CCW_Distance(vid, farthestCCW)) {   // node is closer than the farthest ccw nei
                        pendingVset[node] = {nullptr, simTime(), heardFrom};
                        addedNode = true;
                        auto itrlow = pendingVset.find(farthestCCW);
                        auto itrlowNew = itrlow;
                        advanceIteratorWrapAround(itrlowNew, 1, pendingVset.begin(), pendingVset.end());
                        farthestCCW = itrlowNew->first;
                        EV_DETAIL << "Adding ccw node " << node << " to pendingVset, removing node " << itrlow->first << endl;
                        // delete nei originally farthest to me in pendingVset but now crowded out by node
                        // if (vsetHalfCardinality > 1 && itrlow->first == pendingVsetClosest.first)   // ensure pendingVsetClosest ccw/cw isn't crowded out, though not likely since they were closest to me when added to pendingVset
                        //     pendingVsetClosest.first = VLRRINGVID_NULL;
                        cancelAndDelete(itrlow->second.setupReqTimer);   // nullptr also accepted, in which case the method does nothing
                        pendingVset.erase(itrlow);

                    }
                    else if (getVid_CW_Distance(vid, node) < getVid_CW_Distance(vid, farthestCW)) {  // node is closer than the farthest cw nei
                        pendingVset[node] = {nullptr, simTime(), heardFrom};
                        addedNode = true;
                        auto itrup = pendingVset.find(farthestCW);
                        auto itrupNew = itrup;
                        advanceIteratorWrapAround(itrupNew, -1, pendingVset.begin(), pendingVset.end());
                        farthestCW = itrupNew->first;
                        EV_DETAIL << "Adding cw node " << node << " to pendingVset, removing node " << itrup->first << endl;
                        // delete nei originally farthest to me in pendingVset but now crowded out by node
                        // if (vsetHalfCardinality > 1 && itrup->first == pendingVsetClosest.second)   // ensure pendingVsetClosest ccw/cw isn't crowded out, though not likely since they were closest to me when added to pendingVset
                        //     pendingVsetClosest.second = VLRRINGVID_NULL;
                        cancelAndDelete(itrup->second.setupReqTimer);   // nullptr also accepted, in which case the method does nothing
                        pendingVset.erase(itrup);
                    }
                }
            } else {  // node already in pendingVset, update its lastHeard time
                pendingItr->second.lastHeard = simTime();
                if (heardFrom != VLRRINGVID_NULL)
                    pendingItr->second.heardFrom = heardFrom;
                if (nodeAlive && pendingItr->second.setupReqTimer != nullptr && pendingItr->second.setupReqTimer->isScheduled()) {
                    int retryCount = pendingItr->second.setupReqTimer->getRetryCount();
                    if (retryCount >= setupReqRetryLimit)
                        pendingItr->second.setupReqTimer->setRetryCount(retryCount-1);      // can reduce retryCount bc I'm sure node is still alive
                }
            }
        }
    }
    if (addedNode)
        pendingVsetChangedSinceLastCheckAndSchedule = true;

    return addedNode;
}

// remove node if exists in pendingVset, cancel corresponding WaitSetupReqIntTimer (allocated or nullptr, i.e. not deleted), and adjust pendingVsetFarthest if relevant
void Vlr::pendingVsetErase(VlrRingVID node)
{
    auto itr = pendingVset.find(node);
    if (itr != pendingVset.end()) { // if node exists in pendingVset
        cancelAndDelete(itr->second.setupReqTimer);
        pendingVset.erase(itr);
        
        // Commented out bc whenever a node is removed from pendingVset, the next node will be added to pendingVset bc pendingVset is unfull, not bc it's closer than farthest ccw or cw pendingVnei, thus pendingVsetFarthest needs to be recalculated whenever new pendingVnei is added bc pendingVset is unfull (i.e. didn't compare w/ current pendingVsetFarthest)
        // if (node == pendingVsetFarthest.first || node == pendingVsetFarthest.second) {  // if node was farthest ccw or cw nodes in pendingVset
        //     pendingVsetFarthest.first = VLRRINGVID_NULL;      // invalidate pendingVsetFarthest so that they'll be recalculated when needed in pendingVsetAdd(),  pendingVsetFarthest.first is farthestCCW
        // }
        pendingVsetChangedSinceLastCheckAndSchedule = true;
    }
}

void Vlr::vsetInsertRmPending(VlrRingVID node)
{
    // vset.insert(node);
    // vset.insert({node, 0});
    // vset.insert({node, {{}, nullptr}});  // vnei: {std::set<VlrPathID>, WaitSetupReqIntTimer*}
    vset.insert({node, {{}}});              // vnei: {std::set<VlrPathID>}
    pendingVsetErase(node);

    // firstVneiReceived = true;
    // if (vsetHalfCardinality > 1)   // record two closest vneis to me
    // if (vset.size() >= 2 * vsetHalfCardinality) {
    //     // vsetClosest = getClosestVneisInVset();
    //     // calcVneiDistInVset();
    // }    
}

bool Vlr::vsetEraseAddPending(VlrRingVID node)
{
    auto itr = vset.find(node);
    if (itr != vset.end()) { // if node exists in vset
        // cancelAndDelete(itr->second.setupReqTimer);
        vset.erase(itr);
        // node that was in vset shouldn't be in pendingVset
        pendingVsetAdd(node, /*heardFrom=*/VLRRINGVID_NULL, /*addToEmpty=*/true, /*nodeAlive=*/false);
        return true;
    }
    // if (vset.erase(node))

    return false;   // if node not in vset
}

bool Vlr::hasSetupReqPending() const
{
    for (const auto& pair : pendingVset) {
        if (pair.second.setupReqTimer != nullptr && pair.second.setupReqTimer->isScheduled())
            return true;
    }
    return false;
}

simtime_t_cref Vlr::getASetupReqPendingArrivalTime() const
{
    for (const auto& pair : pendingVset) {
        if (pair.second.setupReqTimer != nullptr && pair.second.setupReqTimer->isScheduled())
            return pair.second.setupReqTimer->getArrivalTime();
    }
    return SIMTIME_ZERO;
}

bool Vlr::IsLinkedPneiOrAvailableRouteEnd(VlrRingVID towardVid) const
{
    if (psetTable.pneiIsLinked(towardVid) || vlrRoutingTable.findAvailableRouteEndInEndpointMap(towardVid))
        return true;
    return false;
}

// return true if towardVid is a LINKED pnei or an endpoint of a wanted (not patched, temporary or dismantled) available vroute in vlrRoutingTable
// if routeBtwUs=true, return true if towardVid is a LINKED pnei or an endpoint of a wanted available vroute btw us (i.e. the other endpoint is me)
bool Vlr::IsLinkedPneiOrAvailableWantedRouteEnd(VlrRingVID towardVid, bool routeBtwUs/*=false*/) const
{
    if (psetTable.pneiIsLinked(towardVid))
        return true;
    
    auto epMapItr = vlrRoutingTable.endpointToRoutesMap.find(towardVid);
    if (epMapItr != vlrRoutingTable.endpointToRoutesMap.end()) {
        std::set<VlrPathID> vrouteSet;    // contains vroutes to towardVid in vlrRoutingTable that aren't in nonEssUnwantedRoutes
        // select vroutes in endpointToRoutesMap[towardVid] but not in nonEssUnwantedRoutes, put them in vrouteSet
        std::set_difference(epMapItr->second.begin(), epMapItr->second.end(), nonEssUnwantedRoutes.begin(), nonEssUnwantedRoutes.end(), std::inserter(vrouteSet, vrouteSet.end()));
        if (vrouteSet.empty())  // no existing non-patched available vroute to towardVid
            return false;
        else {           // I have existing non-patched available vroute to towardVid
            // check if there exists a vroute to towardVid that is NOT temporary or dismantled
            for (const auto& pathid : vrouteSet) {
                auto vrouteItr = vlrRoutingTable.vlrRoutesMap.find(pathid);
                ASSERT(vrouteItr != vlrRoutingTable.vlrRoutesMap.end());    // pathid in endpointToRoutesMap should exist in vlrRoutingTable
                bool isTemporaryRoute = vlrRoutingTable.getIsTemporaryRoute(vrouteItr->second.isUnavailable);
                bool isDismantledRoute = vlrRoutingTable.getIsDismantledRoute(vrouteItr->second.isUnavailable);
                if (!isTemporaryRoute && !isDismantledRoute) {
                    if (!routeBtwUs || (vrouteItr->second.fromVid == vid || vrouteItr->second.toVid == vid))
                        return true;
                }
            }
        }
    }
    return false;   // no existing available vroute to towardVid
}

// get a LINKED pnei as proxy for setupReq, return vid of proxy, or VLRRINGVID_NULL if no qualified proxy found
VlrRingVID Vlr::getProxyForSetupReq(bool checkInNetwork/*=true*/) const
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

void Vlr::processFillVsetTimer()
{
    EV_DEBUG << "Processing fillVsetTimer at node " << vid << endl;
    // if I haven't heard a valid rep seqNo, no need to try to establish vnetwork; vset, pendingVset, nonEssRoutes, etc. should be empty
    bool repChecked = (representativeFixed && representative.heardfromvid == VLRRINGVID_NULL) ? false : true;
    if (repChecked) {   // if representativeFixed but haven't heard from rep, don't do chores before purging records 
        //
        // send setupReq to potential vnei to join vnetwork and fill vset
        //
        // if I'm not inNetwork (and not in inNetwork warmup (unless I'm rep and just lost all vneis)), pick a pnei that's inNetwork and send setupReq to my own vid (potential vneis in pendingVset may be outdated)
        // if I have a setupReq timer scheduled in pendingVset, probably I've sent (or planning to send) a setupReq/setupReqTrace to a potential vnei, wait for timeout b4 sending a setupReq as a new join node
        // repSeqObserveTimer is useless if I allow temporary overlay partition (which is inevitable when recovering from failure), bc if I just rep-timeout, I don't have valid rep (repChecked=false) and shouldn't send setupReq at all; if I heard a new rep seqNo, meaning rep is still alive, then I don't have to let old overlay die out (this can lead to overlay partition if rep starts a new overlay while I'm joining the old one)
            // if repSeqObserveTimer is scheduled, i.e. I'm NOT rep and !inNetwork and just cleared vset and pendingVset bc rep-timeout, I don't send setupReq to pnei inNetwork bc it may rep-timeout soon
        if (!selfInNetwork && vset.empty() && !hasSetupReqPending() /*&& !repSeqObserveTimer->isScheduled()*/) {
            VlrRingVID proxy = getProxyForSetupReq();
            if (proxy != VLRRINGVID_NULL) {
                // send setupReq to my own vid
                sendSetupReq(vid, proxy, /*reqDispatch=*/true, /*recordTrace=*/false, /*reqVnei=*/true, /*setupReqTimer=*/nullptr);

                // found inNetwork pnei as proxy, delay inNetworkWarmupTimer which would set my inNetwork to true with empty vset
                simtime_t nextTimerArrivalTime = simTime() + routeSetupReqWaitTime;
                if (inNetworkWarmupTimer->isScheduled() && inNetworkWarmupTimer->getArrivalTime() < nextTimerArrivalTime) {
                    // inNetworkWarmupTimer is scheduled bc either no fixed startingRoot, or I'm startingRoot
                    cancelEvent(inNetworkWarmupTimer);
                    scheduleAt(nextTimerArrivalTime, inNetworkWarmupTimer);     // delay inNetworkWarmupTimer until this setupReq doesn't receive a reply
                }
            } else if (!representativeFixed && !startingRootFixed && !inNetworkWarmupTimer->isScheduled()) {    // no LINKED && inNetwork pnei found
                scheduleAt(simTime() + inNetworkEmptyVsetWarmupTime, inNetworkWarmupTimer);
                EV_DETAIL << "I'm not inNetwork, no fixed startingRoot, vset empty, no scheduled setupReq to pendingVnei, inNetwork scheduled to become true at " << inNetworkWarmupTimer->getArrivalTime() << endl;
            }
        }
        // if I'm not inNetwork but vset isn't empty, inNetwork will be true after inNetwork warmup
        // Commented out bc I'm not inNetwork when repairing overlay -- only send setupReq to potential vnei in pendingVset when I'm inNetwork
        else if (/*selfInNetwork &&*/ !pendingVset.empty()) {
            // only try to setup nonEss vroutes to pendingVnei once every setupNonEssRoutesInterval
            bool tryNonEssRoute = (setupNonEssRoutes && nextSetupNonEssRoutesTime <= simTime());
            if (tryNonEssRoute)
                nextSetupNonEssRoutesTime = simTime() + uniform(0.8, 1.2) * setupNonEssRoutesInterval;

            // if a pendingVnei that belongs to vset has retryCount > 1 (at least one setupReq timer to it has expired), also schedule setupReq(reqVnei=true) to the next closest pendingVnei in pendingVset, i.e. its alterPendingVnei
            int vsetHalfCardup = vsetHalfCardinality;
            int vsetHalfCardlow = vsetHalfCardinality;

            // temporarily add vneis to pendingVset to determine if any nodes in pendingVset belongs to vset
            for (const auto& vnei : vset)
                // pendingVset[vnei] = {nullptr, SIMTIME_ZERO, /*heardFrom=*/VLRRINGVID_NULL};
                pendingVset[vnei.first] = {nullptr, SIMTIME_ZERO, /*heardFrom=*/VLRRINGVID_NULL};

            int closeCount = 0;             // how close a pendingVset node is to me
            std::vector<VlrRingVID> expiredPendingVneis;  // expired nodes in pendingVset (not in vset) which I'll removed after traversing pendingVset
            auto itrup = pendingVset.upper_bound(vid);    // itrup points to closest cw node in pendingVset
            advanceIteratorWrapAround(itrup, 0, pendingVset.begin(), pendingVset.end());
            auto itrlow = itrup;
            for (int i = 0; i < vsetAndPendingHalfCardinality; ++i) {
                if (itrup == itrlow && i > 0)   // when i == 0, itrup == itrlow but itrup hasn't been examined yet
                    break;
                else {
                    if (vset.find(itrup->first) == vset.end()) {     // itrup->first is a pendingVset node
                        if (i < vsetHalfCardup) {   // itrup->first should be in vset
                            if (itrup->second.setupReqTimer != nullptr && itrup->second.setupReqTimer->getRetryCount() > 1)
                                vsetHalfCardup++;
                            setRouteToPendingVsetItr(/*pendingItr=*/itrup, /*sendSetupReq=*/true, /*isCCW=*/false, closeCount, tryNonEssRoute, /*expiredPendingVneisPtr=*/&expiredPendingVneis);
                        } else {                           // itrup->first should be in pendingVset
                            tryNonEssRoute = (tryNonEssRoute && i < vsetAndBackupHalfCardinality);    // only try to build vroutes to the closest vsetAndBackupHalfCardinality nodes to me in cw/ccw direction, not every node in pendingVset  NOTE i can goto vsetAndPendingHalfCardinality-1 at max
                            setRouteToPendingVsetItr(/*pendingItr=*/itrup, /*sendSetupReq=*/false, /*isCCW=*/false, closeCount, tryNonEssRoute, /*expiredPendingVneisPtr=*/&expiredPendingVneis);
                        }
                    }
                    advanceIteratorWrapAround(itrlow, -1, pendingVset.begin(), pendingVset.end());

                    if (itrlow == itrup)
                        break;
                    else {
                        if (vset.find(itrlow->first) == vset.end()) {     // itrlow->first is a pendingVset node
                            if (i < vsetHalfCardlow) {    // itrlow->first should be in vset
                                if (itrlow->second.setupReqTimer != nullptr && itrlow->second.setupReqTimer->getRetryCount() > 1)
                                    vsetHalfCardlow++;
                                setRouteToPendingVsetItr(/*pendingItr=*/itrlow, /*sendSetupReq=*/true, /*isCCW=*/true, closeCount, tryNonEssRoute, /*expiredPendingVneisPtr=*/&expiredPendingVneis);
                            } else {                           // itrlow->first should be in pendingVset
                                tryNonEssRoute = (tryNonEssRoute && i < vsetAndBackupHalfCardinality);    // only try to build vroutes to the closest vsetAndBackupHalfCardinality nodes to me in cw/ccw direction, not every node in pendingVset  NOTE i can goto vsetAndPendingHalfCardinality-1 at max
                                setRouteToPendingVsetItr(/*pendingItr=*/itrlow, /*sendSetupReq=*/false, /*isCCW=*/true, closeCount, tryNonEssRoute, /*expiredPendingVneisPtr=*/&expiredPendingVneis);
                            }
                        }
                        advanceIteratorWrapAround(itrup, 1, pendingVset.begin(), pendingVset.end());
                    }
                }
            }
            for (const auto& vnei : vset)
                // pendingVset.erase(vnei);
                pendingVset.erase(vnei.first);
            
            // remove expired pendingVset nodes
            for (const auto& node : expiredPendingVneis)
                pendingVsetErase(node);    // this setupReqTimer will be cancelAndDelete()
        }
        // either I've scheduled setupReq to every qualified pendingVnei currently in pendingVset, or vset empty and I haven't scheduled setupReq, even if there're still pendingVneis I give up on them
        pendingVsetChangedSinceLastCheckAndSchedule = false;
        
        
        // // build secondary vset routes to vneis if possible
        
        // if (setupSecondVsetRoutes && !vset.empty() && nextSetupSecondVsetRoutesTime <= simTime()) {
        //     for (auto vneiItr = vset.begin(); vneiItr != vset.end(); vneiItr++) {
        //         setSecondRouteToVnei(vneiItr);
        //     }
        //     nextSetupSecondVsetRoutesTime = simTime() + setupSecondVsetRoutesInterval;
        // }
        
        //
        // notify my vnei and pendingVnei of my vset
        //
        if (sendPeriodicNotifyVset && !vset.empty() && nextNotifyVsetSendTime <= simTime()) {
            VlrRingVID vneiToNotify;
            bool sendNotifyVset = false;
            bool sendToVnei;    // true if sending NotifyVset to vnei, false if sending to pendingVnei
            if (!nextNotifyVsetToPendingVnei) {     // send to a node in vset
                sendNotifyVset = true;
                auto vneiItr = vset.begin();    // if I haven't sent any NotifyVset yet, send to first vnei in vset
                if (lastNotifiedVnei != VLRRINGVID_NULL) {
                    vneiItr = vset.upper_bound(lastNotifiedVnei);               // get vnei just larger than lastNotifiedVnei in vset
                    advanceIteratorWrapAround(vneiItr, 0, vset.begin(), vset.end());
                }
                // vneiToNotify = *vneiItr;
                vneiToNotify = vneiItr->first;
                lastNotifiedVnei = vneiToNotify;
                sendToVnei = true;
                if (++vneiItr == vset.end())    // if vneiToNotify is the last node in vset, send NotifyVset to pendingVnei next time
                    nextNotifyVsetToPendingVnei= true;
            }
            else if (!pendingVset.empty()) {     // send to a node in pendingVset
                sendNotifyVset = true;
                auto vneiItr = pendingVset.begin();    // if I haven't sent any NotifyVset to pendingVnei yet, get first node in pendingVset
                if (lastNotifiedPendingVnei != VLRRINGVID_NULL) {
                    vneiItr = pendingVset.upper_bound(lastNotifiedPendingVnei);       // get node just larger than lastNotifiedPendingVnei in pendingVset
                    advanceIteratorWrapAround(vneiItr, 0, pendingVset.begin(), pendingVset.end());
                }
                while (sendNotifyVset && !IsLinkedPneiOrAvailableRouteEnd(vneiItr->first)) { // pendingVnei (vneiItr->first) isn't linked pnei or existing vroute endpoint in vlrRoutingTable, get another one
                    if (++vneiItr == pendingVset.end()) {
                        sendNotifyVset = false;
                        nextNotifyVsetToPendingVnei = false;
                        lastNotifiedPendingVnei = (--vneiItr)->first;
                    }
                }
                if (sendNotifyVset) {   // pendingVnei (vneiItr->first) is a linked pnei or existing vroute endpoint in vlrRoutingTable, I'll send a NotifyVset to it
                    vneiToNotify = vneiItr->first;
                    lastNotifiedPendingVnei = vneiToNotify;
                    sendToVnei = false;
                    if (++vneiItr == pendingVset.end())
                        nextNotifyVsetToPendingVnei = false;
                }
            } else {    // nextNotifyVsetToPendingVnei == true && pendingVset.empty()
                nextNotifyVsetToPendingVnei = false;
            }
            
            // send NotifyVset to selected vnei
            if (sendNotifyVset) {
                VlrIntOption vlrOptionOut;
                initializeVlrOption(vlrOptionOut, /*dstVid=*/vneiToNotify);
                VlrRingVID nextHopVid = findNextHop(vlrOptionOut, /*prevHopVid=*/VLRRINGVID_NULL, /*excludeVid=*/VLRRINGVID_NULL, /*allowTempRoute=*/true);
                if (nextHopVid == VLRRINGVID_NULL) {
                    // delete vlrOptionOut;
                    EV_WARN << "No next hop found to send NotifyVset at me = " << vid << " to vnei = " << vneiToNotify << ", shouldn't happen because if vset-route exists" << endl;
                } else {
                    auto msgOutgoing = createNotifyVset(/*dstVid=*/vneiToNotify, /*toVnei=*/sendToVnei);
                    msgOutgoing->setVlrOption(vlrOptionOut);

                    if (recordStatsToFile) {   // record sent message
                        recordMessageRecord(/*action=*/0, /*src=*/vid, /*dst=*/vneiToNotify, "NotifyVset", /*msgId=*/0, /*hopcount=*/0, /*chunkByteLength=*/0);   // unused params (msgId, hopcount, chunkByteLength) are just assigned to 0
                    }
                    EV_INFO << "Sending NotifyVset to vnei = " << vneiToNotify << ", nexthop: " << nextHopVid << endl;
                    sendCreatedNotifyVset(msgOutgoing, /*outGateIndex=*/psetTable.getPneiGateIndex(nextHopVid));

                    nextNotifyVsetSendTime = simTime() + uniform(0.8, 1.2) * notifyVsetSendInterval;
                }
            }
        }
    }
    
    //
    // purge recentSetupReqFrom of expired records
    //
    simtime_t currTime = simTime();
    for (auto it = recentSetupReqFrom.begin(); it != recentSetupReqFrom.end(); )
        if (it->second <= currTime)
            recentSetupReqFrom.erase(it++);     // step1: it_tobe = it+1; step2: recentSetupReqFrom.erase(it); step3: it = it_tobe;
        else
            it++;
    //
    // purge recentReqFloodFrom of expired records
    //
    for (auto it = recentReqFloodFrom.begin(); it != recentReqFloodFrom.end(); )
        if (it->second.second <= currTime)
            recentReqFloodFrom.erase(it++);     // step1: it_tobe = it+1; step2: erase(it); step3: it = it_tobe;
        else
            it++;
    //
    // purge delayedRepairLinkReq of expired records
    //
    for (auto it = delayedRepairLinkReq.begin(); it != delayedRepairLinkReq.end(); )
        if (it->second.expireTime <= currTime)
            delayedRepairLinkReq.erase(it++);     // step1: it_tobe = it+1; step2: erase(it); step3: it = it_tobe;
        else
            it++;
    //
    // purge recentTempRouteSent of expired records
    //
    for (auto it = recentTempRouteSent.begin(); it != recentTempRouteSent.end(); )
        if (it->second.second <= currTime)
            recentTempRouteSent.erase(it++);     // step1: it_tobe = it+1; step2: erase(it); step3: it = it_tobe;
        else
            it++;
    //
    // purge recentRepairLocalReqFrom of expired records
    //
    for (auto it = recentRepairLocalReqFrom.begin(); it != recentRepairLocalReqFrom.end(); )
        if (it->second.second <= currTime)
            recentRepairLocalReqFrom.erase(it++);     // step1: it_tobe = it+1; step2: erase(it); step3: it = it_tobe;
        else
            it++;
    //
    // purge recentRepairLocalBrokenVroutes of expired records
    //
    for (auto it = recentRepairLocalBrokenVroutes.begin(); it != recentRepairLocalBrokenVroutes.end(); )
        if (it->second.second <= currTime)
            recentRepairLocalBrokenVroutes.erase(it++);     // step1: it_tobe = it+1; step2: erase(it); step3: it = it_tobe;
        else
            it++;
    //
    // purge delayedRepairLocalReq of expired records
    //
    for (auto it = delayedRepairLocalReq.begin(); it != delayedRepairLocalReq.end(); )
        if (it->second.expireTime <= currTime)
            delayedRepairLocalReq.erase(it++);     // step1: it_tobe = it+1; step2: erase(it); step3: it = it_tobe;
        else
            it++;
    //
    // purge overheardTraces of expired records
    //
    for (auto it = overheardTraces.begin(); it != overheardTraces.end(); )
        if (it->second.second <= currTime)
            overheardTraces.erase(it++);     // step1: it_tobe = it+1; step2: erase(it); step3: it = it_tobe;
        else
            it++;
    //
    // purge representativeMap of expired records
    //
    simtime_t obsoleteLastheard = currTime - repSeqPersistenceInterval;
    for (auto it = representativeMap.begin(); it != representativeMap.end(); )
        if (it->second.lastHeard <= obsoleteLastheard /*&& it->second.lastHeardOldseqnum <= obsoleteLastheard.dbl()*/) {    // Commented out bc selfRepSeqnum is reset to 0 when failed nodes restart
            expiredErasedReps[it->first] = it->second.sequencenumber;
            representativeMap.erase(it++);     // step1: it_tobe = it+1; step2: recentSetupReqFrom.erase(it); step3: it = it_tobe;
        } else
            it++;
    // // //
    // // // purge overheardMPneis of expired records
    // // //
    // // for (auto it = overheardMPneis.begin(); it != overheardMPneis.end(); )
    // //     if (it->second.second <= currTime)
    // //         overheardMPneis.erase(it++);     // step1: it_tobe = it+1; step2: erase(it); step3: it = it_tobe;
    // //     else
    // //         it++;
    //
    // purge recentUnavailableRouteEnd of expired records
    //
    for (auto it = recentUnavailableRouteEnd.begin(); it != recentUnavailableRouteEnd.end(); )
        if (it->second.second <= currTime) {
            const VlrRingVID& lostVend = it->first;
            if (!vlrRoutingTable.findAvailableRouteEndInEndpointMap(lostVend)) {    // lostVend is still unavailable
                const VlrPathID& brokenPathid = it->second.first;
                auto vrouteItr = vlrRoutingTable.vlrRoutesMap.find(brokenPathid);
                if (vrouteItr != vlrRoutingTable.vlrRoutesMap.end()) {  // brokenPathid hasn't been removed or repaired
                    bool isDismantledRoute = vlrRoutingTable.getIsDismantledRoute(vrouteItr->second.isUnavailable);
                    if (!isDismantledRoute && vlrRoutingTable.isRouteItrNextHopAvailable(vrouteItr, lostVend)==0) {
                        it->second.second = currTime + recentUnavailableRouteEndExpiration;      // check if lostVend is still an unavailable endpoint in vlrRoutingTable
                        it++;
                        continue;
                    }
                }
            }
            recentUnavailableRouteEnd.erase(it++);     // step1: it_tobe = it+1; step2: erase(it); step3: it = it_tobe;
        }
        else
            it++;
    //
    // purge nonEssRoutes of expired records, i.e. send Teardown for expired non-essential routes
    //
    std::ostringstream s;
    s << "nonEssRoutes & dismantledRoutes ";
    std::set<VlrRingVID> closePendingVneis = getCloseNodesInMapToSet(/*numHalf=*/vsetAndBackupHalfCardinality, /*vidMap=*/pendingVset);
    std::map<VlrRingVID, std::pair<std::vector<VlrPathID>, char>> nextHopToPathidsMap;    // for Teardown to send, map next hop vid to (pathids, next hop 2-bit isUnavailable) pair
    for (auto it = nonEssRoutes.begin(); it != nonEssRoutes.end(); ) {
        if (it->second <= currTime) {
            // tear down it->first
            const VlrPathID& oldPathid = it->first;
            auto vrouteItr = vlrRoutingTable.vlrRoutesMap.find(oldPathid);
            if (vrouteItr != vlrRoutingTable.vlrRoutesMap.end()) {  // path to tear down found in vlrRoutingTable
                bool isToNexthop = (vid == vrouteItr->second.fromVid);
                const VlrRingVID& otherEnd = (isToNexthop) ? vrouteItr->second.toVid : vrouteItr->second.fromVid;
                const VlrRingVID& nextHopVid = (isToNexthop) ?  vrouteItr->second.nexthopVid : vrouteItr->second.prevhopVid;
                char nextHopIsUnavailable = vlrRoutingTable.getPrevNextIsUnavailable(vrouteItr->second.isUnavailable, /*getPrev=*/!isToNexthop);     // if isToNexthop, next hop is nexthop, getPrev=false

                // if expireTime=1, always remove
                // if otherEnd in pendingVset (and not a pnei), don't remove    NOTE it's possible this vroute isn't my only vroute to otherEnd, but at otherEnd this is the only vroute to me
                auto nonEssUnwantedRouteItr = nonEssUnwantedRoutes.find(oldPathid);
                if (it->second != 1 && !psetTable.pneiIsLinked(otherEnd) /*&& vlrRoutingTable.isOnlyRouteEndInEndpointMap(oldPathid, otherEnd)*/ && nonEssUnwantedRouteItr == nonEssUnwantedRoutes.end()) {   // oldPathid not in nonEssUnwantedRoutes
                    // if (pendingVset.find(otherEnd) != pendingVset.end()) {      // otherEnd in pendingVset
                    if (closePendingVneis.find(otherEnd) != closePendingVneis.end()) {      // otherEnd is one of close ccw/cw nodes to me in pendingVset
                        it->second = currTime + nonEssRouteExpiration;      // check if otherEnd is still in pendingVset after some time
                        it++;
                        continue;
                    }
                    // else (vset.find(otherEnd) != vset.end()) {}        // otherEnd in vset, check if next hop to otherEnd is the same as next hop in vset-route to otherEnd
                }
                if (nonEssUnwantedRouteItr != nonEssUnwantedRoutes.end()) {
                    nonEssUnwantedRoutes.erase(nonEssUnwantedRouteItr);     // remove oldPathid in nonEssUnwantedRoutes since we're tearing it down
                    // s << 'u';
                    EV_DETAIL << "nonEssRoutes pathid = " << oldPathid << " in nonEssUnwantedRoutes" << endl;
                }
                if (it->second == 1) {
                    // s << '1';
                    EV_DETAIL << "nonEssRoutes pathid = " << oldPathid << " expire time = 1" << endl;
                }
                if (psetTable.pneiIsLinked(otherEnd) == 1) {
                    // s << 'p';
                    EV_DETAIL << "nonEssRoutes pathid = " << oldPathid << " bc otherEnd=" << otherEnd << " is linked pnei" << endl;
                }

                ASSERT(!vrouteItr->second.isVsetRoute);     // since vroute is in nonEssRoutes, otherEnd must have been removed from vset
                EV_INFO << "Start tearing down non-essential vroute to node " << otherEnd << ": pathid = " << vrouteItr->first << " " << vrouteItr->second << endl;
                s << 'n' << oldPathid << ' ';
                
                if (nextHopIsUnavailable != 1) {  // next hop isn't unavailable
                    EV_DETAIL << "Teardown (pathid = " << oldPathid << ") will be sent to " << nextHopVid << endl;
                    auto nextHopItr = nextHopToPathidsMap.find(nextHopVid);
                    if (nextHopItr == nextHopToPathidsMap.end()) 
                        nextHopToPathidsMap[nextHopVid] = {{oldPathid}, nextHopIsUnavailable};
                    else
                        nextHopItr->second.first.push_back(oldPathid);
                } else {  // next hop is unavailable, remove this vroute from lostPneis.brokenVroutes
                    removeRouteFromLostPneiBrokenVroutes(oldPathid, /*lostPneiVid=*/nextHopVid);
                }

                vlrRoutingTable.removeRouteByPathID(oldPathid);     // remove vroute in vlrRoutingTable
            }
            nonEssRoutes.erase(it++);
        }
        else
            it++;
    }
    //
    // purge dismantledRoutes of expired records, i.e. send Teardown for expired dismantled routes
    //
    for (auto it = dismantledRoutes.begin(); it != dismantledRoutes.end(); ) {
        if (it->second <= currTime) {
            // tear down it->first
            const VlrPathID& oldPathid = it->first;
            auto vrouteItr = vlrRoutingTable.vlrRoutesMap.find(oldPathid);
            if (vrouteItr != vlrRoutingTable.vlrRoutesMap.end()) {  // path found in vlrRoutingTable
                ASSERT(vlrRoutingTable.getIsDismantledRoute(vrouteItr->second.isUnavailable));     // since vroute is dismantled
                EV_INFO << "Start removing dismantled vroute: pathid = " << vrouteItr->first << " " << vrouteItr->second << endl;

                std::vector<VlrRingVID> sendTeardownToAddrs = {vrouteItr->second.prevhopVid, vrouteItr->second.nexthopVid};
                // get 2-bit status of prevhop and nexthop
                std::vector<char> nextHopStates = {vlrRoutingTable.getPrevNextIsUnavailable(vrouteItr->second.isUnavailable, /*getPrev=*/true), vlrRoutingTable.getPrevNextIsUnavailable(vrouteItr->second.isUnavailable, /*getPrev=*/false)};
    
                for (size_t i = 0; i < sendTeardownToAddrs.size(); ++i) {
                    const VlrRingVID& nextHopVid = sendTeardownToAddrs[i];
                    const char& nextHopIsUnavailable = nextHopStates[i];
                    // Commented out bc after I set a vroute dismantled and noticed its next hop is patched, I'll tear down the vroute immediately
                    // ASSERT(nextHopIsUnavailable != 2);  // dismantled vroute shouldn't involve temporary route as it shouldn't be in brokenVroutes  NOTE 2-bit status in dismantled vroute can be 0, 3 (next hop LINKED, can send Teardown) or 1 (end of dismantled vroute, don't send Teardown) or 2 (don't send Teardown, lostPnei will tear down on its side, I'll only tear down on my side)
                    if (nextHopIsUnavailable != 1 && nextHopIsUnavailable != 2) {    // next hop isn't unavailable or patched
                        // if next hop is the endpoint, no need to send Teardown bc endpoint doesn't keep this dismantled vroute
                        const VlrRingVID& towardEnd = (i == 0) ? vrouteItr->second.fromVid : vrouteItr->second.toVid;
                        if (towardEnd == nextHopVid) {
                            EV_DETAIL << "Teardown of dismantled vroute (pathid = " << oldPathid << ") won't be sent to next hop " << nextHopVid << " which is an endpoint node " << towardEnd << endl;
                        } else {
                            EV_DETAIL << "Teardown (pathid = " << oldPathid << ") will be sent to " << nextHopVid << endl;
                            auto nextHopItr = nextHopToPathidsMap.find(nextHopVid);
                            if (nextHopItr == nextHopToPathidsMap.end()) 
                                nextHopToPathidsMap[nextHopVid] = {{oldPathid}, /*nextHopIsUnavailable=*/0};   // next hop isn't patched
                            else
                                nextHopItr->second.first.push_back(oldPathid);
                        }
                    }
                }
                s << 'd' << oldPathid << ' ';
                vlrRoutingTable.removeRouteByPathID(oldPathid);     // remove vroute in vlrRoutingTable
            }
            dismantledRoutes.erase(it++);
        }
        else
            it++;
    }
    // send Teardown with multiple pathids if possible
    for (const auto& mappair : nextHopToPathidsMap) {
        TeardownInt *teardownOut = createTeardown(/*pathids=*/mappair.second.first, /*addSrcVset=*/true, /*rebuild=*/false, /*dismantled=*/false);
        // checked that nextHopIsUnavailable != 1
        sendCreatedTeardownToNextHop(teardownOut, /*nextHopVid=*/mappair.first, /*nextHopIsUnavailable=*/mappair.second.second);

        if (recordStatsToFile) {   // record sent message
            recordMessageRecord(/*action=*/0, /*src=*/vid, /*dst=*/VLRRINGVID_NULL, "Teardown", /*msgId=*/0, /*hopcount=*/0, /*chunkByteLength=*/0, /*infoStr=*/s.str().c_str());   // unused params (msgId, hopcount, chunkByteLength) are just assigned to 0
        }
    }
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

// if sendSetupReq=true, pendingItr->first should be in my vset so I'll send setupReq to it, otherwise it should stay in pendingVset and I'll send AddRoute to it if tryNonEssRoute=true
// if isCCW=false, pendingItr->first is my cw nei or the only nei in pendingVset, otherwise ccw nei; if not sure, set isCCW false
// closeCount: number of pendingVneis already processed by this function, if < 2, pendingItr->first is one of closest ccw/cw node to me in pendingVset, else, already processed 2 other nodes in pendingVset closer to me
// if std::vector<VlrRingVID> *expiredPendingVneisPtr given (not nullptr), add pendingItr->first if it has expired and qualified to be removed from pendingVset
// // // std::vector<VlrRingVID> *overheardTracePtr=nullptr: [dst, .., (me)], if provided (not nullptr), it'll be used instead of overheardTraces[pendingItr->first], but if given trace is invalid, I won't try greedy routing to find pendingItr->first
void Vlr::setRouteToPendingVsetItr(std::map<VlrRingVID, PendingVneiInfo>::iterator pendingItr, bool sendSetupReq, bool isCCW, int& closeCount, bool tryNonEssRoute, std::vector<VlrRingVID> *expiredPendingVneisPtr/*=nullptr*/)
{
    closeCount++;
    // bool findWithoutTrace = (overheardTracePtr == nullptr);     // only try finding pendingItr->first w/o trace when overheardTracePtr isn't provided

    const VlrRingVID& closestNode = pendingItr->first;

    // not connected to closestNode directly or via vroute and it hasn't been heard recently and no setupReq scheduled to find it
    bool expirePendingVnei = (pendingItr->second.lastHeard <= (simTime() - pendingVneiValidityInterval) && (pendingItr->second.setupReqTimer == nullptr || !pendingItr->second.setupReqTimer->isScheduled()) && !IsLinkedPneiOrAvailableRouteEnd(pendingItr->first));
    // don't remove closestNode yet if it's a lost pnei or unavailable vend that's being repaired
    expirePendingVnei = (expirePendingVnei && lostPneis.find(closestNode) == lostPneis.end() && recentUnavailableRouteEnd.find(closestNode) == recentUnavailableRouteEnd.end());
    if (expirePendingVnei) {
        EV_INFO << "Deleting expired node " << closestNode << " from pendingVset" << endl;
        // cannot erase pendingItr here bc pendingItr may still be used for traversing pendingVset after this function
        if (expiredPendingVneisPtr)    // expiredPendingVneisPtr given
            expiredPendingVneisPtr->push_back(closestNode);
        return;
    }

    // if (sendSetupReq) {     // pendingItr->first should be added to my vset
    // if isScheduled but reqVnei was false but now true, schedule another setupReq to it with reqVnei=true
    if (pendingItr->second.setupReqTimer == nullptr || !pendingItr->second.setupReqTimer->isScheduled() || (sendSetupReq && !pendingItr->second.setupReqTimer->getReqVnei())) {

        // // check if should allow setupReqTrace to closestNode
        // bool wasClosestVnei = false;
        // Commented out to avoid sending setupReqTrace at all
        // if (closeCount <= 2) {      // if closeCount > 2, closestNode isn't closest ccw or cw node to me in pendingVset
        //     wasClosestVnei = shouldAllowSetupReqTraceToPendingClosest(closestNode, isCCW);
        // }

        // if closestNode doesn't belong to vset, and setupNonEssRoutes=false or tryNonEssRoute=false, we don't try to build nonEss vroutes to pendingVnei
        // if closestNode doesn't belong to vset, but it is linked pnei or existing vroute endpoint in vlrRoutingTable, we don't try to build nonEss vroutes to pendingVnei
        bool skipSetupNonEssRoute = (!sendSetupReq && (!setupNonEssRoutes || !tryNonEssRoute || IsLinkedPneiOrAvailableWantedRouteEnd(closestNode)));
        // if closestNode is a lost pnei or vend that has become unavailable bc broken vroute, we'll try to repair it with repairLinkReq rather than setupReq
        bool skipSetupRoute = (skipSetupNonEssRoute || checkLostPneiUnavailable(closestNode) || checkRecentUnavailableRouteEnd(closestNode));
        if (skipSetupRoute) {
            EV_INFO << "Skip setup vroute to pendingVset node " << pendingItr->first << ", belongToVset=" << sendSetupReq << ", setupNonEssRoutes=" << setupNonEssRoutes << ", tryNonEssRoute=" << tryNonEssRoute
                    << ", skipSetupNonEssRoute=" << skipSetupNonEssRoute << ", skipSetupRoute=" << skipSetupRoute << endl;
        } else {    // skipSetupRoute = false, schedule setupReq to closestNode
            if (pendingItr->second.setupReqTimer == nullptr) {
                char timerName[40] = "WaitSetupReqIntTimer:";
                pendingItr->second.setupReqTimer = new WaitSetupReqIntTimer(strcat(timerName, std::to_string(pendingItr->first).c_str()));
                pendingItr->second.setupReqTimer->setDst(pendingItr->first);
                pendingItr->second.setupReqTimer->setTimerType(0);
                pendingItr->second.setupReqTimer->setRepairRoute(false);
                pendingItr->second.setupReqTimer->setAlterPendingVnei(VLRRINGVID_NULL);
                // int retryCount = (wasClosestVnei) ? (VLRSETUPREQ_THRESHOLD - 1) : 0;    // if wasClosestVnei, we'll send setupReqTrace, so we don't need as many setupReq trials
                int retryCount = 0;
                pendingItr->second.setupReqTimer->setRetryCount(retryCount);
            }  // else, pendingItr->second is allocated but isn't scheduled, probably canceled            
            // pendingItr->second.setupReqTimer->setAllowSetupReqTrace(wasClosestVnei);   // allow setupReqTrace if closestNode is closer than my closest vneis in vset
            // pendingItr->second.setupReqTimer->setReqVnei(sendSetupReq);   // setupReq requests pendingItr->first as a vnei if sendSetupReq=true

            if (pendingItr->second.setupReqTimer->isScheduled())   // setupReq has been scheduled but reqVnei=false, now that reqVnei=true, send again
                cancelEvent(pendingItr->second.setupReqTimer);
        
            double delay = (sendSetupReq) ? 0 : (uniform(0, 0.2) * routeSetupReqWaitTime);  // add random delay for setupReq(reqVnei=false) to avoid sending multiple setupReq at same time when tryNonEssRoute becomes true
            scheduleAt(simTime() + delay, pendingItr->second.setupReqTimer);   // schedule setupReq to closestNode now
        }       
    }
    // setupReqTimer is allocated, update setupReqTimer.reqVnei, setupReqTimer will be deleted if received SetupFail/AddRoute 
    if (pendingItr->second.setupReqTimer != nullptr) {   
        // pendingItr->second.setupReqTimer->setAllowSetupReqTrace(wasClosestVnei);   // allow setupReqTrace if closestNode is closer than my closest vneis in vset
        pendingItr->second.setupReqTimer->setReqVnei(sendSetupReq);   // setupReq requests pendingItr->first as a vnei if sendSetupReq=true
    }
}

// return true if lostPnei is in lostPneis and it's still unavailable (i.e. no tempVlinks)
inline bool Vlr::checkLostPneiUnavailable(VlrRingVID lostPnei) const
{
    auto lostPneiItr = lostPneis.find(lostPnei);
    if (lostPneiItr != lostPneis.end() && lostPneiItr->second.tempVlinks.empty())   // no temporary route to lostPnei
        return true;
    return false;
}

// return true if lostVend is in recentUnavailableRouteEnd and it's still unavailable, remove lostVend from recentUnavailableRouteEnd if it's no longer unavailable
bool Vlr::checkRecentUnavailableRouteEnd(VlrRingVID lostVend)
{
    auto it = recentUnavailableRouteEnd.find(lostVend);
    if (it != recentUnavailableRouteEnd.end()) {
        if (!vlrRoutingTable.findAvailableRouteEndInEndpointMap(lostVend)) {    // lostVend is still unavailable
            const VlrPathID& brokenPathid = it->second.first;
            auto vrouteItr = vlrRoutingTable.vlrRoutesMap.find(brokenPathid);
            if (vrouteItr != vlrRoutingTable.vlrRoutesMap.end()) {  // brokenPathid hasn't been removed or repaired
                bool isDismantledRoute = vlrRoutingTable.getIsDismantledRoute(vrouteItr->second.isUnavailable);
                if (!isDismantledRoute && vlrRoutingTable.isRouteItrNextHopAvailable(vrouteItr, lostVend)==0) {
                    return true;
                }
            }
        }
        recentUnavailableRouteEnd.erase(it);
    }
    return false;
}

// alterPendingVnei: the closest pendingVnei just farther than referenceVid, if found, will be returned as pendingItr->first, else, pendingItr = pendingVset.end()
// return char if found-- 1: referenceVid found at itrup->first, 2: referenceVid found at itrlow->first
char Vlr::findAlterPendingVneiOf(VlrRingVID referenceVid, std::map<VlrRingVID, PendingVneiInfo>::iterator& pendingItr)
{
    char referenceVidFound = 0;     // 1: referenceVid found at itrup->first, 2: referenceVid found at itrlow->first
    // pendingVset won't be empty since referenceVid is in pendingVset
    auto itrup = pendingVset.upper_bound(vid);    // itrup points to closest cw node in pendingVset
    advanceIteratorWrapAround(itrup, 0, pendingVset.begin(), pendingVset.end());
    auto itrlow = itrup;
    pendingItr = pendingVset.end();    // points to next closest potential vnei in pendingVset farther but in same direction as referenceVid
    for (int i = 0; i < pendingVsetHalfCardinality; ++i) {
        if (itrup == itrlow && i > 0)   // when i == 0, itrup == itrlow but itrup hasn't been examined yet
            break;
        else {
            if (referenceVidFound == 1) {
                pendingItr = itrup;
                break;
            } else if (referenceVidFound == 0 && itrup->first == referenceVid)
                referenceVidFound = 1;

            advanceIteratorWrapAround(itrlow, -1, pendingVset.begin(), pendingVset.end());

            if (itrlow == itrup)
                break;
            else {
                if (referenceVidFound == 2) {
                    pendingItr = itrlow;
                    break;
                } else if (referenceVidFound == 0 && itrlow->first == referenceVid)
                    referenceVidFound = 2;
                
                advanceIteratorWrapAround(itrup, 1, pendingVset.begin(), pendingVset.end());
            }
        }
    }
    return referenceVidFound;
}

void Vlr::recordCurrentNodeStats(const char *stage)
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
    VlrRingVID currRep = vid;
    if (representativeFixed)
        currRep = (representative.heardfromvid != VLRRINGVID_NULL) ? representative.vid : VLRRINGVID_NULL;
    else if (!representativeMap.empty()) {
        if (representativeMap.begin()->first < currRep)
            currRep = representativeMap.begin()->first;
    }

    s << "nodeStats" << ',' << simulatedPneis << ',' << selfInNetwork << ',' << selfNodeFailure << ',' 
            << currRep << ',' << printVsetToString() << ',' << vidRingRegVset << ',' << printRoutesToMeToString() << ','
            << simulatedPneis.size() << ',' << vset.size() << ',' << pendingVset.size() << ',' << vlrRoutingTable.vlrRoutesMap.size() << ',' << totalNumBeaconSent << ','
            << representativeMap.size() << ',' << representativeMapActualMaxSize << ',' << stage;  //  << ',' << pendingVset

    recordNodeStatsRecord(/*infoStr=*/s.str().c_str());
}

void Vlr::finish()
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

    writeFirstRepSeqTimeoutToFile(/*writeAtFinish=*/true);
    writeTotalNumBeacomSentToFile();

    // cSimpleModule::finish();     // reference to 'finish' may be ambiguous
}


// prevHopVid is only used to send Teardown when vlrOption->getCurrentPathid() is not found in my routing table, prevHopVid = VLRRINGVID_NULL when the packet to be sent is initiated by myself
// if allowTempRoute=true, can use a vroute whose next hop is connected via temporary route
VlrRingVID Vlr::findNextHop_internal(VlrIntOption& vlrOption, VlrRingVID prevHopVid, const char* file_name, uint32_t linenumber, VlrRingVID excludeVid/*=VLRRINGVID_NULL*/, bool allowTempRoute/*=false*/, uint32_t parentlinenum/*=0*/)
{
    VlrRingVID dstVid = vlrOption.getDstVid();
    // ASSERT(dstVid != vid);  // not supposed to find nexthop to myself, bc no other node is closer to me than myself
    if (dstVid == vid)
        throw cRuntimeError("findNextHop(dst=%d) at me=%d, dst==me, filename=%s, linenumber=%d, parentlinenum=%d", dstVid, vid, file_name, linenumber, parentlinenum);
    EV_INFO << "Finding nexthop for destination vid = " << dstVid << ", excludeVid = " << excludeVid << ", allowTempRoute = " << allowTempRoute << endl;

    // first check if dstVid in pset
    // NOTE we only route to inNetwork pnei (unless dst is my pnei) bc if a node hasn't joined overlay it can't do greedy routing; for vroute endpoint, even though we aren't sure if it's inNetwork, at least it has joined overlay
    auto nextVidResult = getClosestPneiTo(dstVid, excludeVid, /*checkInNetwork=*/true);
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

    // check if message was using a temporary route
    VlrPathID tempPathid = vlrOption.getTempPathid();
    if (tempPathid != VLRPATHID_INVALID) {
        VlrRingVID tempTowardVid = vlrOption.getTempTowardVid();
        auto tempPathItr = vlrRoutingTable.vlrRoutesMap.find(tempPathid);
        if (tempPathItr != vlrRoutingTable.vlrRoutesMap.end()) {   // tempPathid is in vlrRoutingTable
            if (tempTowardVid == vid) {     // reached the end of temporary route portion
                EV_INFO << "Reached tempTowardVid=" << tempTowardVid << " in temporary pathid=" << tempPathid << ", current pathid=" << vlrOption.getCurrentPathid() << endl;
            } else {    // send to next hop in temporary route
                VlrRingVID nextHopVid = vlrRoutingTable.getRouteItrNextHop(tempPathItr, tempTowardVid);
                EV_INFO << "Found nexthop in temporary pathid=" << tempPathid << ", tempTowardVid=" << tempTowardVid << ", current vroute towardVid=" << vlrOption.getTowardVid() << ", nextHopVid=" << nextHopVid << endl;
                return nextHopVid;
            }
        } else {    // tempPathid not found in vlrRoutingTable
            EV_WARN << "Current temporary pathid=" << tempPathid << " (tempTowardVid=" << tempTowardVid << ") selected at previous hop but not in VlrRingRoutingTable at current node = " << vid << ", selecting new vroute" << endl;
            if (prevHopVid != VLRRINGVID_NULL) {   // prevHopAddrPtr != nullptr
                // const L3Address& prevHopAddr = *prevHopAddrPtr;
                EV_INFO << "Sending Teardown to previous hop " << prevHopVid << " for pathid = " << tempPathid << endl;
                // tear down tempPathid, i.e. send Teardown to prevHopAddr
                auto teardownOut = createTeardownOnePathid(tempPathid, /*addSrcVset=*/false, /*rebuild=*/true);
                sendCreatedTeardown(teardownOut, /*nextHopVid=*/prevHopVid);

                if (recordStatsToFile) {   // record sent message
                    recordMessageRecord(/*action=*/0, /*src=*/vid, /*dst=*/VLRRINGVID_NULL, "Teardown", /*msgId=*/0, /*hopcount=*/0, /*chunkByteLength=*/0, /*infoStr=*/"findNextHop: tempPathid not found in vlrRoutingTable");   // unused params (msgId, hopcount, chunkByteLength) are just assigned to 0
                }
            }
            vlrOption.setCurrentPathid(VLRPATHID_INVALID);     // currPathid can no longer be followed bc temporary route is broken, select new vroute
            vlrOption.setTempPathid(VLRPATHID_INVALID);
        }
    }

    // check if I can use rep path to dstVid, assume no loop in rep path, rep should be inNetwork unless rep == dstVid
    bool usingRepPath = false;
    if (representativeFixed) {
        if (/*allowTempRoute &&*/ representative.heardfromvid != VLRRINGVID_NULL && repSeqExpirationTimer->isScheduled() && representative.vid != excludeVid && representative.heardfromvid != excludeVid && psetTable.pneiIsLinked(representative.heardfromvid)) {  // rep path is available, and rep != excludeVid    NOTE at rep itself, heardfromvid=rep won't be a LINKED pnei
            unsigned int distance;
            if (dstVid == representative.vid) {
                EV_INFO << "Destination vid = " << dstVid << " is the representative, setting nexthop to my rep parent = " << representative.heardfromvid << endl;
                vlrOption.setCurrentPathid(VLRPATHID_REPPATH);
                vlrOption.setTowardVid(representative.vid);
                vlrOption.setTempPathid(VLRPATHID_INVALID);
                return representative.heardfromvid;
            }
            else if (representative.inNetwork && (distance = computeVidDistance(representative.vid, dstVid)) < minDistance) {
                EV_INFO << "Representative vid = " << representative.vid << " in consideration as towardVid, representative.heardfromvid = " << representative.heardfromvid << endl;
                minDistance = distance;
                nextVid = representative.vid;
                nexthopVid = representative.heardfromvid;
                usingRepPath = true;
            }
        }
    } else if (!representativeMap.empty()) {    // get a valid rep closest to dstVid in representativeMap, if currPathid is rep-path, select current towardVid if it's in representativeMap
        auto repResult = getClosestRepresentativeTo(dstVid, excludeVid, vlrOption, /*checkInNetwork=*/true);
        // if (repResult.first == VLRRINGVID_NULL), then repResult.second = VLRRINGVID_MAX, if this node has any available pnei, minDistance < VLRRINGVID_MAX, won't be using rep-path
        if (std::get<0>(repResult) == dstVid) { // if excludeVid == dstVid, this can't happen
            nexthopVid = std::get<2>(repResult);
            // if (simTime() > 500 && dstVid == 933) {
            //     auto repMapItr = representativeMap.find(dstVid);
            //     EV_WARN << "Destination found in representativeMap, dstVid=" << dstVid << ", rep inNetwork=" << repMapItr->second.inNetwork << ", lastHeard=" << repMapItr->second.lastHeard << ", seqNum=" << repMapItr->second.sequencenumber << ", setting nexthop to rep parent = " << nexthopVid << endl;
            // }
            vlrOption.setCurrentPathid(VLRPATHID_REPPATH);
            vlrOption.setTowardVid(dstVid);
            vlrOption.setTempPathid(VLRPATHID_INVALID);
            return nexthopVid;
        } else if (std::get<1>(repResult) < minDistance) {     // if no qualified rep was found as nextVid, minDistance = VLRRINGVID_MAX 
            minDistance = std::get<1>(repResult);
            nextVid = std::get<0>(repResult);
            nexthopVid = std::get<2>(repResult);
            usingRepPath = true;

            // if (simTime() > 500 && dstVid == 933) {
            //     auto repMapItr = representativeMap.find(nextVid);
            //     EV_WARN << "Selected nextVid in representativeMap, dstVid=" << dstVid << ", rep (nextVid)=" << nextVid << ", rep inNetwork=" << repMapItr->second.inNetwork << ", lastHeard=" << repMapItr->second.lastHeard << ", seqNum=" << repMapItr->second.sequencenumber << ", setting nexthop to rep parent = " << nexthopVid << endl;
            // }
        }
    }
    
    // checked message isn't using a temporary route
    // check if dstVid in vlrRoutingTable
    bool usingVroute = false;    // if we're routing with a vroute, need to ensure we use the same vroute if nextVid == towardVid
    auto rtResult = getClosestVendTo(dstVid, excludeVid, vlrOption, allowTempRoute);
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

    // check if dstVid in overheardMPneis
    // bool usingMPnei = false;
    // Commented out bc not sure if overheardMPnei is inNetwork or not, if not inNetwork probably it can't do greedy routing    NOTE overheardMPnei is either inNetwork pnei of a pnei or added by RepairLinkReqFlood, 
    // if (checkOverHeardMPneis && allowTempRoute && nextVid != dstVid) {   // need allowTempRoute=true because mpnei routes will only expire, not torn down, hence may be broken and result in loop (should be temporary situation)
    //     auto rtResult = getClosestOverheardMPneiTo(dstVid, excludeVid);
    //     // if no qualified node in overheardMPneis, rtResult = (VLRRINGVID_NULL, VLRRINGVID_MAX)
    //     if (std::get<1>(rtResult) < minDistance) {
    //         minDistance = std::get<1>(rtResult);
    //         nextVid = std::get<0>(rtResult);
    //         usingVroute = false;
    //         usingMPnei = true;
    //     }
    // }

    
    if (nextVid == VLRRINGVID_NULL) {
        EV_WARN << "No LINKED && inNetwork pnei or vroute endpoint exists for dstVid=" << dstVid << " (excludeVid = " << excludeVid << ")" << endl;
        return VLRRINGVID_NULL;
    }
    ASSERT(nextVid != excludeVid);
    // finally compare minDistance with my own distance to dst
    if (minDistance < computeVidDistance(vid, dstVid)) {
        // if message was using a vroute (currPathid), ensure nextVid isn't farther from dst than towardVid -- only possible if currPathid isn't in my vlrRoutingTable, or next hop of currPathid is unavailable (i.e. a lost pnei)
        VlrPathID currPathid = vlrOption.getCurrentPathid();
        auto currPathItr = vlrRoutingTable.vlrRoutesMap.end();
        unsigned int oldDistance = VLRRINGVID_MAX;       // distance btw towardVid and dst, only valid when currPathid != VLRPATHID_INVALID
        if (currPathid != VLRPATHID_INVALID) {
            if (currPathid != VLRPATHID_REPPATH) {
                currPathItr = vlrRoutingTable.vlrRoutesMap.find(currPathid);
                // vroute selected at previous hop but not found at this hop, broken vroute, send teardown message to prevHopAddr about currPathid
                if (currPathItr == vlrRoutingTable.vlrRoutesMap.end()) {
                    EV_WARN << "Current pathid=" << currPathid << " (towardVid=" << vlrOption.getTowardVid() << ") selected at previous hop but not in VlrRingRoutingTable at current node = " << vid << ", selecting new vroute" << endl;
                    if (prevHopVid != VLRRINGVID_NULL) {   // prevHopAddrPtr != nullptr
                        if (tempPathid != VLRPATHID_INVALID) {  // this message arrived through temporary route, previous hop doesn't have currPathid, need to notify other end of tempPathid
                            auto tempPathItr = vlrRoutingTable.vlrRoutesMap.find(tempPathid);
                            if (tempPathItr != vlrRoutingTable.vlrRoutesMap.end()) {
                                VlrRingVID otherEnd = (vid == tempPathItr->second.fromVid) ? tempPathItr->second.toVid : tempPathItr->second.fromVid;
                                auto lostPneiItr = lostPneis.find(otherEnd);
                                if (lostPneiItr != lostPneis.end()) {
                                    // const L3Address& prevHopAddr = lostPneiItr->second.address;  // send Teardown of currPathid to other end of tempPathid
                                    EV_WARN << "Sending Teardown for unfound currPathid = " << currPathid << " through tempPathid=" << tempPathid << " to previous hop vid=" << otherEnd << endl;
                                    auto teardownOut = createTeardownOnePathid(currPathid, /*addSrcVset=*/false, /*rebuild=*/true);
                                    sendCreatedTeardownToNextHop(teardownOut, /*nextHopVid=*/otherEnd, /*nextHopIsUnavailable=*/2);

                                    if (recordStatsToFile) {   // record sent message
                                        recordMessageRecord(/*action=*/0, /*src=*/vid, /*dst=*/VLRRINGVID_NULL, "Teardown", /*msgId=*/0, /*hopcount=*/0, /*chunkByteLength=*/0, /*infoStr=*/"findNextHop: currPathid not found in vlrRoutingTable");   // unused params (msgId, hopcount, chunkByteLength) are just assigned to 0
                                    }
                                } else
                                    EV_WARN << "Received Teardown for unfound currPathid = " << currPathid << " through tempPathid=" << tempPathid << " but otherEnd=" << otherEnd << " of temporary path not found in lostPneis" << endl;
                            } 
                        } else if (vlrOption.getTowardVid() != vid) {    // previous hop is directly connected, if towardVid is me, this may be a dismantled route not an inconsistency, also it's probably not a big deal when I don't have a route for which I'm the endpoint bc I'm the dst anyway
                            // const L3Address& prevHopAddr = *prevHopAddrPtr;
                            EV_WARN << "Sending Teardown for unfound currPathid = " << currPathid << " to previous hop " << prevHopVid << endl;
                            // tear down currPathid, i.e. send Teardown to prevHopAddr
                            auto teardownOut = createTeardownOnePathid(currPathid, /*addSrcVset=*/false, /*rebuild=*/true);
                            sendCreatedTeardown(teardownOut, /*nextHopVid=*/prevHopVid);

                            if (recordStatsToFile) {   // record sent message
                                recordMessageRecord(/*action=*/0, /*src=*/vid, /*dst=*/VLRRINGVID_NULL, "Teardown", /*msgId=*/0, /*hopcount=*/0, /*chunkByteLength=*/0, /*infoStr=*/"findNextHop: currPathid not found in vlrRoutingTable");   // unused params (msgId, hopcount, chunkByteLength) are just assigned to 0
                            }
                        }
                    }
                }
            }
            // ensure nextVid isn't farther from dst than towardVid selected at previous hop, otherwise forwarding isn't greedy, will likely result in loop
                // if currPathid is found in my vlrRoutingTable, but next hop to towardVid is unavailable, towardVid may not be in endpointToRoutesMap, so nextVid returned by getClosestVendTo() may be farther from dst than towardVid
            if (vlrOption.getTowardVid() == nextVid)
                oldDistance = minDistance;
            else {
                oldDistance = computeVidDistance(vlrOption.getTowardVid(), dstVid);
                // NOTE if both towardVid and another node A equally close to towardVid are my pneis, getClosestPneiTo() may not prefer towardVid, but sending to a pnei equally close to dst as towardVid won't result in loop
                if (minDistance > oldDistance /*|| (minDistance == oldDistance && usingVroute)*/) {       // only allow nextVid to be towardVid selected at previous hop or closer to dst than towardVid
                    EV_WARN << "No nexthop is closer to dst=" << dstVid << " than towardVid=" << vlrOption.getTowardVid() << " selected at previous hop ";
                    if (prevHopVid != VLRRINGVID_NULL)  // prevHopAddrPtr != nullptr
                        EV_WARN << prevHopVid;
                    EV_WARN << " (currentPathid=" << currPathid;
                    if (currPathItr != vlrRoutingTable.vlrRoutesMap.end())
                        EV_WARN << " " << currPathItr->second;
                    EV_WARN << "), closest one found is nextVid=" << nextVid << " (usingVroute=" << usingVroute << ")" << endl;

                    if (currPathid == VLRPATHID_REPPATH) {
                        auto repMapItr = representativeMap.find(vlrOption.getTowardVid());
                        if (repMapItr == representativeMap.end())
                            EV_WARN << "towardVid=" << vlrOption.getTowardVid() << " selected at previous hop isn't found in my representativeMap = " << printRepresentativeMapToString() << endl;
                        else
                            EV_WARN << "towardVid=" << vlrOption.getTowardVid() << " selected at previous hop is found in my representativeMap, rep inNetwork=" << repMapItr->second.inNetwork << ", lastHeard=" << repMapItr->second.lastHeard << ", seqNum=" << repMapItr->second.sequencenumber << ", rep parent = " << repMapItr->second.heardfromvid << endl;
                    }
                    return VLRRINGVID_NULL;
                }
            }
        }
        if (!usingVroute) {     // using physical link
            // if (usingMPnei) {   // nextVid in overheardMPneis
            //     nexthopVid = overheardMPneis[nextVid].first;
            //     EV_INFO << "Found nexthop to dstVid=" << dstVid << " in overheardMPneis, nextVid=" << nextVid << ", nexthopVid=" << nexthopVid << endl;
            //     vlrOption.setCurrentPathid(VLRPATHID_INVALID);
            // }
            if (usingRepPath) {  // nextVid is representative.vid
                if (currPathid == VLRPATHID_REPPATH && minDistance == oldDistance && nextVid != vlrOption.getTowardVid()) {
                    EV_WARN << "No rep is closer to dst=" << dstVid << " than rep-path towardVid=" << vlrOption.getTowardVid() << " selected at previous hop ";
                    if (prevHopVid != VLRRINGVID_NULL)  // prevHopAddrPtr != nullptr
                        EV_WARN << prevHopVid;
                    EV_WARN << " (currentPathid=VLRPATHID_REPPATH) but towardVid isn't found in representativeMap, best rep found is nextVid=" << nextVid << endl;
                    return VLRRINGVID_NULL;
                }
                EV_INFO << "Using representative as nexthop to dstVid=" << dstVid << ", nextVid=" << nextVid << ", nexthopVid=" << nexthopVid << endl;
                vlrOption.setCurrentPathid(VLRPATHID_REPPATH);
                vlrOption.setTowardVid(nextVid);
            } else {    // nextVid in PsetTable
                EV_INFO << "Found nexthop to dstVid=" << dstVid << " on a physical link, nextVid=" << nextVid << ", nexthopVid=" << nexthopVid << endl;
                vlrOption.setCurrentPathid(VLRPATHID_INVALID);
            }
            vlrOption.setTempPathid(VLRPATHID_INVALID);
            return nexthopVid;
        } else {        // using vroute in VlrRingRoutingTable
            VlrPathID& nextPathid = std::get<2>(rtResult);
            // L3Address nextHopAddr;
            if (nextPathid == VLRPATHID_INVALID) {     // excludeVid can't be next hop AND allowTempRoute=true, vroute hasn't been selected yet
                bool usingCurrPathid = false;
                if (currPathid != VLRPATHID_INVALID && currPathid != VLRPATHID_REPPATH) {    // message was using a vroute (currPathid)
                    // if nextVid == towardVid (towardVid is still one of closest endpoints to dst), use currPathid if it is available
                    if (vlrOption.getTowardVid() == nextVid) {
                        if (vlrRoutingTable.findRouteEndInEndpointMap(currPathid, nextVid)) {    // next hop in currPathid to towardVid is available
                            nextPathid = currPathid;
                            nexthopVid = vlrRoutingTable.getRouteItrNextHop(currPathItr, nextVid);
                            EV_INFO << "Found nexthop to dstVid=" << dstVid << " using current pathid=" << currPathid << currPathItr->second << ", towardVid=" << nextVid << ", nextHopVid=" << nexthopVid << endl;
                            usingCurrPathid = true;
                        }
                    }
                }
                if (!usingCurrPathid) {    // use a vroute that goes toward endpoint nextVid
                    // use a random vrouteID in endpointToRoutesMap[nextVid] set
                    int endpointRouteIndex = intuniform(0, vlrRoutingTable.getNumRoutesToEndInEndpointMap(nextVid) - 1);   // must be in range [0, set_size)
                    std::pair<VlrPathID, VlrRingVID> vrouteNhResult = vlrRoutingTable.getRouteToEndpoint(nextVid, /*endpointRouteIndex=*/endpointRouteIndex);
                    EV_INFO << "Found nexthop to dstVid=" << dstVid << " using new vroute pathid=" << vrouteNhResult.first << vlrRoutingTable.vlrRoutesMap.find(vrouteNhResult.first)->second << ", towardVid=" << nextVid << ", nextHopVid=" << vrouteNhResult.second << endl;
                    nextPathid = vrouteNhResult.first;
                    nexthopVid = vrouteNhResult.second;
                }
            } else {    // excludeVid may be next hop, so vroute has been selected to avoid using excludeVid as next hop
                nexthopVid = vlrRoutingTable.getRouteNextHop(nextPathid, nextVid);
                EV_INFO << "Found nexthop to dstVid=" << dstVid << " using selected pathid=" << nextPathid << vlrRoutingTable.vlrRoutesMap.find(nextPathid)->second << ", towardVid=" << nextVid << ", nextHopVid=" << nexthopVid << endl;
                
            }
            // Commented out to use another vroute to towardVid and hope this doesn't result in a loop :(
                // if nextVid isn't closer to dst than towardVid and we're still using vroute but we aren't using currPathid, this means currPathid isn't in my vlrRoutingTable, or next hop of currPathid is unavailable (i.e. a lost pnei)
            if (currPathid != VLRPATHID_INVALID && minDistance == oldDistance && nextPathid != currPathid) {  // message was using a vroute (currPathid) to towardVid, now I'm still routing to towardVid but using a different vroute (nextPathid), this can result in a loop
                // if (nextVid == dstVid && vlrOption.getTowardVid() != dstVid) { /* Commented out bc no nextVid will be equally close to dstVid as dstVid itself -- only allow switching vroute if previous towardVid wasn't dstVid but current towardVid is */ }
                    // if (currPathItr != vlrRoutingTable.vlrRoutesMap.end() && !vlrRoutingTable.getIsDismantledRoute(currPathItr->second.isUnavailable))     // only disallow changing currPathid if it is still in vlrRoutingTable but next hop is unavailable, bc it's likely to result in loop since I should have currPathid available
                EV_WARN << "No nexthop is closer to dst=" << dstVid << " than towardVid=" << vlrOption.getTowardVid() << " selected at previous hop ";
                if (prevHopVid != VLRRINGVID_NULL)  // prevHopAddrPtr != nullptr
                    EV_WARN << prevHopVid;
                EV_WARN << " (currentPathid=" << currPathid << ") but currentPathid isn't found or next hop in currentPathid is unavailable, best pathid found is nextPathid=" << nextPathid << "(nextVid=" << nextVid << ")" << endl;
                
                if (currPathid == VLRPATHID_REPPATH) {
                    auto repMapItr = representativeMap.find(vlrOption.getTowardVid());
                    if (repMapItr == representativeMap.end())
                        EV_WARN << "towardVid=" << vlrOption.getTowardVid() << " selected at previous hop isn't found in my representativeMap = " << printRepresentativeMapToString() << endl;
                    else
                        EV_WARN << "towardVid=" << vlrOption.getTowardVid() << " selected at previous hop is found in my representativeMap, rep inNetwork=" << repMapItr->second.inNetwork << ", lastHeard=" << repMapItr->second.lastHeard << ", seqNum=" << repMapItr->second.sequencenumber << ", rep parent = " << repMapItr->second.heardfromvid << endl;
                }
                return VLRRINGVID_NULL;
            }
            
            // can double check nextHopAddr is not excludeVid here
            // check if nextHopAddr is a LINKED pnei or connected via temporary route
            vlrOption.setCurrentPathid(nextPathid);
            vlrOption.setTowardVid(nextVid);
            if (allowTempRoute && vlrRoutingTable.isRouteItrNextHopAvailable(vlrRoutingTable.vlrRoutesMap.find(nextPathid), nextVid) == 2) {    // vroute uses temporary route; only if allowTempRoute=true can the selected vroute be using a temporary route
                // VlrRingVID lostPneiVid = getVidFromAddressInRegistry(nextHopAddr);    // map nextHopAddr L3Address (lost pnei) to VlrRingVID
                VlrRingVID lostPneiVid = nexthopVid;
                auto lostPneiItr = lostPneis.find(lostPneiVid);
                if (lostPneiItr != lostPneis.end()) {
                    tempPathid = *(lostPneiItr->second.tempVlinks.begin());
                    nexthopVid = vlrRoutingTable.getRouteNextHop(tempPathid, lostPneiVid);
                    vlrOption.setTempPathid(tempPathid);
                    vlrOption.setTempTowardVid(lostPneiVid);
                    EV_INFO << "Selected nexthop to dstVid=" << dstVid << " using pathid=" << nextPathid << ", towardVid=" << nextVid << ", nextHopVid=" << lostPneiVid << " connected via temporary route tempPathid=" << tempPathid << "new nexthopVid=" << nexthopVid << endl;
                } else {
                    auto nextPathItr = vlrRoutingTable.vlrRoutesMap.find(nextPathid);
                    if (nextPathItr == vlrRoutingTable.vlrRoutesMap.end())
                        EV_WARN << "Selected nextPathid doesn't exist in vlrRoutingTable!" << endl;
                    else
                        EV_WARN << "Selected nextPathid=" << nextPathid << ": isVsetRoute=" << nextPathItr->second.isVsetRoute << ", isUnavailable=" << (int)nextPathItr->second.isUnavailable << ", prevhopRepairRouteSent=" << nextPathItr->second.prevhopRepairRouteSent << ", prevhopVids=" << nextPathItr->second.prevhopVids << endl;
                    EV_WARN << "Selected nexthop to dstVid=" << dstVid << " using pathid=" << nextPathid << ", towardVid=" << nextVid << ", nextHopVid=" << lostPneiVid << " connected via temporary route, but not found in lostPneis" << endl;
                    return VLRRINGVID_NULL;
                }
            } else  // nextHopAddr is a LINKED pnei
                vlrOption.setTempPathid(VLRPATHID_INVALID);
            return nexthopVid;
        }
    } else {
        EV_WARN << "No nexthop is closer than me = " << vid << " to dstVid=" << dstVid << ", closest one found is nextVid=" << nextVid << ", usingVroute=" << usingVroute << endl;
        return VLRRINGVID_NULL;
    }
}

// for routing SetupReply/SetupFail, first check if newnode is a LINKED pnei, if so send SetupReply to it directly
VlrRingVID Vlr::findNextHopForSetupReply(VlrIntOption& vlrOption, VlrRingVID prevHopVid, VlrRingVID newnode, bool allowTempRoute/*=false*/)
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
    return findNextHop(vlrOption, prevHopVid, /*excludeVid=*/VLRRINGVID_NULL, allowTempRoute);
}

// for routing SetupReq with transferNode, if transferNode is null or vlrOption->dstVid == dst, route to dst, otherwise if dst is a LINKED pnei or available vend, route to dst, otherwise send to transferNode, once transferNode is reached, transferNode is set to null, SetupReq is sent to dst
VlrRingVID Vlr::findNextHopForSetupReq_internal(VlrIntOption& vlrOption, VlrRingVID prevHopVid, const char* file_name, uint32_t linenumber, VlrRingVID dstVid, VlrRingVID newnode, bool allowTempRoute/*=true*/)
{
    EV_DEBUG << "Finding nexthop for SetupReq: dst = " << dstVid << ", vlrOption->dst = " << vlrOption.getDstVid() << endl;
    
    bool routeToDst = false;    // change vlrOption->dstVid from transferNode to dstVid
    if (vlrOption.getDstVid() == dstVid || IsLinkedPneiOrAvailableRouteEnd(dstVid)) {    // no transferNode or has transferNode but dst is a LINKED pnei or available vend, route to dst
        routeToDst = true;
    } else {    //  vlrOption->dstVid != dst or dst isn't a LINKED pnei or available vend, continue to find nexthop to transferNode
        VlrRingVID nextHopVid = findNextHop(vlrOption, prevHopVid, /*excludeVid=*/newnode, allowTempRoute, linenumber);
        if (nextHopVid == VLRRINGVID_NULL) {
            EV_WARN << "No next hop found for findNextHopForSetupReq(vlrOption->dst=" << vlrOption.getDstVid() << ", dstVid = " << dstVid << ", newnode = " << newnode << ", allowTempRoute=" << allowTempRoute << ") at me = " << vid << ", changing vlrOption->dst to dstVid and try again" << endl;
            routeToDst = true;
        } else         // nexthop found to vlrOption->dstVid
            return nextHopVid;
    }

    // if (routeToDst) // routeToDst must be true here
    if (vlrOption.getDstVid() != dstVid) {     // vlrOption->dstVid was probably transferNode
        vlrOption.setDstVid(dstVid);
        vlrOption.setCurrentPathid(VLRPATHID_INVALID);
        vlrOption.setTempPathid(VLRPATHID_INVALID);
    }
    // find nexthop to dst
    return findNextHop(vlrOption, prevHopVid, /*excludeVid=*/newnode, allowTempRoute, linenumber);
    
    // Commented out because when transferNode is reached, vlrOption->dstVid should have been set to dst
    // if (vlrOption->getDstVid() == vid) {
    //     EV_WARN << "No nexthop because I'm the transferNode but dst = " << dstVid << " is not a LINKED pnei or available endpoint" << endl;
    //     return L3Address();
    // }
    // // continue to find nexthop to transferNode
    // return findNextHop(vlrOption, prevHopVid, /*excludeVid=*/newnode, allowTempRoute);
}

// for routing SetupReply/SetupFail, check if any node along the trace before myself is a LINKED pnei, if so return its index in trace
// if preferShort=false, use the last node who's a LINKED pnei before myself in trace
// nodes after and including myself will be removed from trace; if no LINKED pnei found before myself, return originalTrace.size()
// trace contains [dst, .., nextHop, (me, skipped nodes..)]
unsigned int Vlr::getNextHopIndexInTrace(VlrIntVidVec& trace, bool preferShort/*=true*/) const
{
    EV_DEBUG << "Finding nexthop for SetupReply/SetupFail with trace: " << trace << endl;
    std::set<VlrRingVID> linkedPneis = psetTable.getPneisLinkedSet();
    unsigned int nextHopIndex = trace.size();
    bool nextHopFound = false;
    unsigned int i;
    for (i = 0; i < trace.size(); i++) {   // check every node in trace except the last one
        if (trace[i] == vid)        // trace[i] is myself
            break;
        else if ((!nextHopFound || !preferShort) && linkedPneis.find(trace[i]) != linkedPneis.end()) {    // trace[i] is a LINKED pnei
            nextHopIndex = i;
            nextHopFound = true;
        }
    }
    trace.erase(trace.begin()+i, trace.end());      // erase nodes after and including myself from trace
    return nextHopIndex;
}

//
// get a LINKED (&& inNetwork if checkInNetwork=true) pnei closest to targetVid (or targetVid itself if it's a LINKED pnei) with wrap-around searching in map, also return its distance to targetVid from computeVidDistance()
// excludeVid won't be considered as the closest pnei, if no node excluded, excludeVid = VLRRINGVID_NULL
// return (VLRRINGVID_NULL, 0) if no pnei in pset -- no need to check vroutes
// return (VLRRINGVID_NULL, VLRRINGVID_MAX) if no LINKED && inNetwork pnei in pset -- can still check vroutes, pnei may be in inNetwork warmup, if pnei is no longer available I should've received Teardown for its vroutes
//
std::pair<VlrRingVID, unsigned int> Vlr::getClosestPneiTo(VlrRingVID targetVid, VlrRingVID excludeVid, bool checkInNetwork/*=false*/) const
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
        if (itr->first == targetVid && itr->second.state == PNEI_LINKED && itr->first != excludeVid) {     // targetVid is a LINKED pnei
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
        return (pneiItr->second.state == PNEI_LINKED && (!checkInNetwork || pneiItr->second.inNetwork) && pneiItr->first != excludeVid);
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
// get a vroute endpoint closest to targetVid (or targetVid itself if it's an endpoint, shouldn't return my own vid) with wrap-around searching in map, return VLRRINGVID_NULL if no qualified endpoints in vlrRoutingTable
// excludeVid won't be considered as next endpoint, but may be returned as the next physical hop (subject to condition), if no node excluded, excludeVid = VLRRINGVID_NULL
// if allowTempRoute=false, vroute endpoint whose next hop is temporary route won't be considered as next endpoint
// if currPathid != VLRPATHID_INVALID and towardVid is still one of the closest endpoints to targetVid, return towardVid (instead of other endpoints with the same distance to targetVid as towardVid)
// result<0>: closest endpoint, result<1>: distance btw result<0> and targetVid, result<2>: vroute pathid to result<0>, VLRPATHID_INVALID if any vroute to result<0> is ok
// NOTE may not return my own vid if I'm the closest to targetVid (my own vid exists as endpoint in routing table)
//
std::tuple<VlrRingVID, unsigned int, VlrPathID> Vlr::getClosestVendTo(VlrRingVID targetVid, VlrRingVID excludeVid, const VlrIntOption& vlrOption, bool allowTempRoute) const
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
    
    bool isExcludePnei = (excludeVid != VLRRINGVID_NULL);   // true if excludeVid could be a prevhop/nexthop in a vroute at this node, if true, exclude excludeVid as next hop
    bool isExcludeLinked = psetTable.pneiIsLinked(excludeVid);  // true if excludeVid is currently a LINKED pnei
    // prevhop/nexthop in a vroute may be connected by a temporary route (in which case it's a lost pnei) and not my LINKED pnei
    isExcludePnei = isExcludePnei && (isExcludeLinked || lostPneis.find(excludeVid) != lostPneis.end());
    // isExcludePnei = isExcludePnei && (targetVid == excludeVid);    // only exclude excludeVid as next hop if it's targetVid bc it won't have any nextVid closer to itself
    // L3Address excludeAddr;      // L3Address of excludeVid
    // if (isExcludePnei) {
    //     if (isExcludeLinked)
    //         excludeAddr = psetTable.getPneiL3Address(excludeVid);
    //     else    // excludeVid is in lostPneis
    //         excludeAddr = lostPneis.at(excludeVid).address;
    // }
        
    const VlrPathID& currPathid = vlrOption.getCurrentPathid();
    VlrRingVID towardVid = vlrOption.getTowardVid();
    bool usingVroute = (currPathid != VLRPATHID_INVALID);   // if previous hop routed the message on a vroute or rep-path
    

    // define a lambda function to determine if an endpoint != excludeVid and next hop != excludeVid
    auto vendItrQualified = [this, excludeVid, isExcludePnei, usingVroute, currPathid, towardVid, allowTempRoute, &nextPathid, targetVid](std::map<VlrRingVID, std::set<VlrPathID>>::const_iterator vendItr) -> bool
    {
        if (vendItr->first == excludeVid)
            return false;
        if (vendItr->first == vid)  // next hop to myself isUnspecified()
            return false;
        if (!isExcludePnei && allowTempRoute && dismantledRoutes.empty()) // if return true here, nextPathid should be VLRPATHID_INVALID, and every vend at this node should return true here, so no need to worry one vend set nextPathid, then later vend return here but nextPathid isn't set back to VLRPATHID_INVALID
            return true;
        // only exclude excludeVid as next hop if its distance to targetVid <= vend's distance to targetVid, bc it won't have any nextVid closer to targetVid than itself
        bool excludeAsNext = isExcludePnei && (targetVid == excludeVid || computeVidDistance(excludeVid, targetVid) <= computeVidDistance(vendItr->first, targetVid));
        // check if excludeVid is the next hop toward vend
        if (usingVroute && vendItr->first == towardVid) {
            // NOTE if next hop of currPathid is unavailable, currPathid won't be in endpointToRoutesMap[towardVid]
            if ((vendItr->second).find(currPathid) != (vendItr->second).end()) {    // next hop in currPathid isn't unavailable
                const auto vrouteItr = vlrRoutingTable.vlrRoutesMap.find(currPathid);
                // if currPathid in endpointToRoutesMap and its next hop toward vend isn't excludeVid AND
                // if allowTempRoute=false, next hop in currPathid doesn't use temporary route, if allowTempRoute=true AND next hop in currPathid uses temporary route, currPathid can't be dismantled (dismantled route shouldn't use temporary route but currPathid might have expired but not removed yet)
                bool nextHopLinked = (vlrRoutingTable.isRouteItrNextHopAvailable(vrouteItr, vendItr->first) == 1);
                if ((!excludeAsNext || vlrRoutingTable.getRouteItrNextHop(vrouteItr, vendItr->first) != excludeVid) && ((allowTempRoute && (nextHopLinked || !vlrRoutingTable.getIsDismantledRoute(vrouteItr->second.isUnavailable))) || nextHopLinked)) {
                    nextPathid = currPathid;
                    return true;
                }
            }
            // if don't allow changing currPathid when selected endpoint == towardVid (to eliminate possibility of loop), return false here
            // return false;
        }
        int randomRouteIndex = intuniform(0, vendItr->second.size()-1);   // to avoid selecting the same vroute to vendItr->first each time, select last qualified vroute before (and including) vendItr->second[randomRouteIndex]
        int routeIndex = 0;   // to avoid selecting the same vroute to vendItr->first each time, select last qualified vroute before vendItr->second[randomRouteIndex]
        bool foundQualifiedVend = false;
        for (const auto& pathid : vendItr->second) {
            // as long as there's one vroute to vend whose next hop isn't excludeVid and satisfies allowTempRoute requirement, vend is qualified
            const auto vrouteItr = vlrRoutingTable.vlrRoutesMap.find(pathid);
            bool nextHopLinked = (vlrRoutingTable.isRouteItrNextHopAvailable(vrouteItr, vendItr->first) == 1);
            if ((!excludeAsNext || vlrRoutingTable.getRouteItrNextHop(vrouteItr, vendItr->first) != excludeVid) && ((allowTempRoute && (nextHopLinked || !vlrRoutingTable.getIsDismantledRoute(vrouteItr->second.isUnavailable))) || nextHopLinked)) {
                nextPathid = pathid;
                foundQualifiedVend = true;
                // prioritize vroute that doesn't use temporary route toward vend
                // if (allowTempRoute && vlrRoutingTable.isRouteItrNextHopAvailable(vrouteItr, vendItr->first)==2) // next hop uses temporary route, keep checking if other vroutes doesn't use temporary route toward vend
                //     ;
                // else    // next hop doesn't use temporary route, this vroute is no need to check other vroutes toward vend
                //     return true;

                // break loop as long as a qualified is found
                // return true;
            }
            // select last qualified vroute before (and including) vendItr->second[randomRouteIndex]
            if (foundQualifiedVend && routeIndex >= randomRouteIndex)   // routeIndex reached randomRouteIndex and I've found a qualified vroute
                return true;
            
            routeIndex++;
        }
        if (foundQualifiedVend)
            return true;

        return false;
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
        } else if (distance == minDistance && usingVroute && itr->first == towardVid) {
            // if towardVid has the same distance to targetVid as closestVend found so far, use towardVid
            closestVend = itr->first;
            closestPathid = nextPathid;
            EV_DEBUG << "Found current vroute endpoint " << closestVend << " (vlrOption->towardVid) in cw direction of target vid=" << targetVid << " in VlrRingRoutingTable" << endl;
        }
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
            } else if (distance == minDistance && usingVroute && itr->first == towardVid) {
                // if towardVid has the same distance to targetVid as closestVend found so far, use towardVid
                closestVend = itr->first;
                closestPathid = nextPathid;
                EV_DEBUG << "Found current vroute endpoint " << closestVend << " (vlrOption->towardVid) in cw direction of target vid=" << targetVid << " in VlrRingRoutingTable" << endl;
            }
        }
    }
   
    if (closestVend != VLRRINGVID_NULL) {
        EV_DEBUG << "Found vroute endpoint " << closestVend << " in VlrRingRoutingTable that is closest to vid = " << targetVid << ", distance = " << minDistance << ", pathid = " << closestPathid << endl;
    }  
    else
        EV_DEBUG << "No qualified vroute endpoint in VlrRingRoutingTable that is closest to vid = " << targetVid << endl;
    
    return result;
}

//
// get a rep closest to targetVid with wrap-around searching in map, also return its distance to targetVid from computeVidDistance(), and next hop in rep-path
// excludeVid won't be considered, if no node excluded, excludeVid = VLRRINGVID_NULL
// if currPathid == VLRPATHID_REPPATH and towardVid is still one of the closest rep to targetVid in representativeMap (no matter if entry to towardVid had expired), return towardVid (instead of other reps with the same distance to targetVid as towardVid)
// if checkInNetwork=true, returned rep should be inNetwork unless rep == targetVid
//
std::tuple<VlrRingVID, unsigned int, VlrRingVID> Vlr::getClosestRepresentativeTo(VlrRingVID targetVid, VlrRingVID excludeVid, const VlrIntOption& vlrOption, bool checkInNetwork) const
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

    const VlrPathID& currPathid = vlrOption.getCurrentPathid();
    VlrRingVID towardVid = vlrOption.getTowardVid();
    bool usingRepPath = (currPathid == VLRPATHID_REPPATH);   // if previous hop routed the message on rep-path (don't care if previous hop was using vroute bc if previously selected vroute endpoint towardVid is equally close to dst as some rep I have, rep-path will be preferred than vroute anyway)

    // if true, exclude excludeVid as next hop
    // bool isExcludePnei = (excludeVid != VLRRINGVID_NULL && targetVid == excludeVid);   // only exclude excludeVid as next hop if it's targetVid bc it won't have any nextVid closer to itself
    bool isExcludePnei = (excludeVid != VLRRINGVID_NULL && psetTable.pneiIsLinked(excludeVid));

    // define a lambda function to determine if a rep != excludeVid and hasn't expired and next hop in rep-path is LINKED
    simtime_t expiredLastheard = simTime() - repSeqValidityInterval;
    simtime_t expiredLastheard2 = simTime() - 20*repSeqValidityInterval;
    auto repMapItrQualified = [targetVid, excludeVid, isExcludePnei, checkInNetwork, expiredLastheard, usingRepPath, towardVid, expiredLastheard2](std::map<VlrRingVID, Representative>::const_iterator repMapItr) 
    { 
        bool excludeAsNext = isExcludePnei && (targetVid == excludeVid || computeVidDistance(excludeVid, targetVid) <= computeVidDistance(repMapItr->first, targetVid));
        bool selectedByPrevHop = (usingRepPath && towardVid == repMapItr->first && repMapItr->second.lastHeard > expiredLastheard2);   // repMapItr->first is the towardVid selected at previous hop, since I still have it in representativeMap, I allow it to be traversed even though it has expired
        return (repMapItr->first != excludeVid && repMapItr->second.heardfromvid != VLRRINGVID_NULL && (!excludeAsNext || repMapItr->second.heardfromvid != excludeVid) && (!checkInNetwork || repMapItr->first == targetVid || repMapItr->second.inNetwork) && (selectedByPrevHop || repMapItr->second.lastHeard > expiredLastheard));
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
        } else if (distance == minDistance && usingRepPath && itr->first == towardVid) {
            // if towardVid has the same distance to targetVid as closest rep found so far, use towardVid
            closestPnei = itr->first;
            // minDistance = distance;      // distance == minDistance now
            closestNextHop = itr->second.heardfromvid;
            EV_DEBUG << "Found current rep-path endpoint " << closestPnei << " (vlrOption->towardVid) in cw direction of target vid=" << targetVid << " in representativeMap" << endl;
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
            } else if (distance == minDistance && usingRepPath && itr->first == towardVid) {
                // if towardVid has the same distance to targetVid as closest rep found so far, use towardVid
                closestPnei = itr->first;
                // minDistance = distance;      // distance == minDistance now
                closestNextHop = itr->second.heardfromvid;
                EV_DEBUG << "Found current rep-path endpoint " << closestPnei << " (vlrOption->towardVid) in cw direction of target vid=" << targetVid << " in representativeMap" << endl;
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


void Vlr::handleStartOperation()
{
    RoutingBase::handleStartOperation();

    scheduleBeaconTimer(/*firstTimer=*/true);
    scheduleFillVsetTimer(/*firstTimer=*/true);
    if (sendTestPacket)
        scheduleTestPacketTimer(/*firstTimer=*/true);

    if (representativeFixed) {
        representative.heardfromvid = VLRRINGVID_NULL;
        representative.sequencenumber = 0;
        // representative.hopcount = 0;
        representative.lastBeaconSeqnum = 0;
        representative.lastBeaconSeqnumUnchanged = false;
    } else {    // we use representativeMap, which should've been cleared when node failed
        selfRepSeqnum = 0;
    }
    
    if ((representativeFixed || startingRootFixed) && representative.vid == vid) {    // I'm the starting root of the network
        selfInNetwork = true;           // initially only rep.selfInNetwork = true
        representative.heardfromvid = vid;
    }

    selfNodeFailure = false;    // stop simulating node failure at me
}

void Vlr::handleStopOperation()
{
    EV_WARN << "Handling stop operation at node " << vid << endl;
    if (recordStatsToFile) { // write node status update
        recordNodeStatsRecord(/*infoStr=*/"beforeNodeFailure");
    }
    clearState(/*clearPsetAndRep=*/true);
    selfNodeFailure = true;

    nodesVsetCorrect.erase(vid);
}

// if failedGateIndex > -1, failedPneis should only contain one pnei, and failedGateIndex is its gate index
void Vlr::handleFailureLinkSimulation(const std::set<unsigned int>& failedPneis, int failedGateIndex/*=-1*/)
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

void Vlr::handleFailureLinkRestart(const std::set<unsigned int>& restartPneis)
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

void Vlr::handleFailureNodeRestart()
{
    EV_INFO << "Handling failure node Restart at node " << vid << ", broadcasting NotifyLinkFailure(linkUp) to pneis" << endl;
    // send NotifyLinkFailure(linkUp) to affected pneis to remove my vid in their failureSimulationPneiVidMap
    NotifyLinkFailureInt *msg = createNotifyLinkFailure(/*simLinkUp=*/true);
    sendCreatedPacket(msg, /*unicast=*/false, /*outGateIndex=*/-1, /*delay=*/0, /*checkFail=*/false);

    if (recordStatsToFile) { // write node status update
        recordNodeStatsRecord(/*infoStr=*/"beforeNodeRestart");
    }
}

VlrRingVID Vlr::getMessagePrevhopVid(cMessage *message) const
{
    VlrRingVID msgPrevHopVid = VLRRINGVID_NULL;
    if (auto unicastPacket = dynamic_cast<VlrIntUniPacket *>(message))
        msgPrevHopVid = unicastPacket->getVlrOption().getPrevHopVid();
    else if (auto beacon = dynamic_cast<VlrIntBeacon *>(message))
        msgPrevHopVid = beacon->getVid();
    else if (auto repairLinkReq = dynamic_cast<RepairLinkReqFloodInt *>(message))
        msgPrevHopVid = repairLinkReq->getLinkTrace().back();
    else if (auto repairLocalReq = dynamic_cast<RepairLocalReqFloodInt *>(message))
        msgPrevHopVid = repairLocalReq->getLinkTrace().back();
    
    return msgPrevHopVid;
}

// return false if any of tempPathid in tempVlinks hasn't expired in recentTempRouteSent, i.e. just built by me
bool Vlr::checkLostPneiTempVlinksNotRecent(const std::set<VlrPathID>& tempVlinks) const
{
    for (const auto& vlinkid : tempVlinks) {
        auto recentTempItr = recentTempRouteSent.find(vlinkid);
        if (recentTempItr != recentTempRouteSent.end() && recentTempItr->second.second > simTime()) // recentTempRouteSent record hasn't expired
            return false;
    }
    return true;
}

void Vlr::cancelPendingVsetTimers()
{
    for (auto& elem : pendingVset)
        cancelAndDelete(elem.second.setupReqTimer);
}

void Vlr::cancelRepairLinkTimers()
{
    for (auto& elem : lostPneis)
        cancelAndDelete(elem.second.timer);
}

void Vlr::clearState(bool clearPsetAndRep)
{
    // cancel all setupReq timers in pendingVset
    cancelPendingVsetTimers();
    // clear vneis recorded in vset and pendingVset
    pendingVset.clear();
    pendingVsetFarthest = std::make_pair(VLRRINGVID_NULL, VLRRINGVID_NULL);
    vset.clear();
    // clear routing table
    vlrRoutingTable.clear();
    // clear records of non-essential vroutes and recently accepted setupReq
    nonEssRoutes.clear();
    nonEssUnwantedRoutes.clear();
    recentSetupReqFrom.clear();
    // cancel all repairLink timers of lost pneis
    cancelRepairLinkTimers();
    lostPneis.clear();
    recentReqFloodFrom.clear();
    
    selfInNetwork = false;
    cancelEvent(inNetworkWarmupTimer);  // leaveOverlay won't be called on rep, rep.selfInNetwork will become true after asserting itself as rep after enough wait time
    cancelEvent(delayedRepairReqTimer);  // delayedRepairLinkReq and delayedRepairLocalReq will be cleared, no need to check
    cancelEvent(recentReplacedVneiTimer);  // recentReplacedVneis will be cleared, no need to check
    
    if (clearPsetAndRep) {
        // clear pset
        psetTable.clear();
        // reset representative
        representative.heardfromvid = VLRRINGVID_NULL;
        representative.sequencenumber = 0;
        selfRepSeqnum = 0;
        // reset representativeMap
        representativeMap.clear();
        // testDstList.clear();         // don't clear testDstList bc it's only read and assigned in initialize()
        cancelEvent(beaconTimer);
        cancelEvent(purgeNeighborsTimer);
        cancelEvent(fillVsetTimer);
        cancelEvent(repSeqExpirationTimer);
        cancelEvent(repSeqObserveTimer);
        cancelEvent(testPacketTimer);
        // cancelEvent(writeRoutingTableTimer);     // don't cancel writeRoutingTableTimer bc it's only scheduled in initialize()
    }
    // clear temporary data
    delayedRepairLinkReq.clear();
    recentTempRouteSent.clear();
    recentRepairLocalReqFrom.clear();
    recentRepairLocalBrokenVroutes.clear();
    delayedRepairLocalReq.clear();
    recentUnavailableRouteEnd.clear();
    overheardTraces.clear();
    recentReplacedVneis.clear();
    // overheardMPneis.clear();
    // reset notifyVset send state
    nextNotifyVsetToPendingVnei = false;
}

std::string Vlr::printVlrOptionToString(const VlrIntOption& vlrOption) const
{
    std::ostringstream s;
    s << '{';
    s << "dst=" << vlrOption.getDstVid() << " toward=" << vlrOption.getTowardVid() << " currpath=" << vlrOption.getCurrentPathid();
    s << '}';
    return s.str();
}

std::set<VlrRingVID> Vlr::convertVsetToSet() const
{
    std::set<VlrRingVID> vsetSet;
    for (auto& vnei : vset) 
        vsetSet.insert(vnei.first);
    return vsetSet;
}

std::string Vlr::printVsetToString(bool printVpath/*=false*/) const
{
    std::string str ("{");      // must use double quotes as string constructed from c-string (null-terminated character sequence): string (const char* s)
    for (auto& vnei : vset) {
        if (!printVpath)
            // str += std::to_string(vnei) + ' ';
            str += std::to_string(vnei.first) + ' ';
        else {
            str += std::to_string(vnei.first) + ':';
            for (const VlrPathID& vsetpath : vnei.second.vsetRoutes)
                str += std::to_string(vsetpath) + ' ';
        }
    }
    str += '}';
    return str;
}

std::string Vlr::printPendingVsetToString() const
{
    std::ostringstream s;
    s << '{';
    for (auto& pair : pendingVset) {
        s << pair.first;
        if (pair.second.setupReqTimer != nullptr && pair.second.setupReqTimer->isScheduled())
            s << ':' << pair.second.setupReqTimer->getRetryCount();
        s << ' ';
    }
    s << '}';
    return s.str();
}

std::string Vlr::printRepresentativeMapToString() const
{
    std::ostringstream s;
    s << '{';
    for (auto& rep : representativeMap) {
        s << rep.first << ":s" << rep.second.sequencenumber << ":h" << rep.second.lastHeard << ' ';
    }
    s << '}';
    return s.str();
}

// print pathids in vlrRoutingTable whose toVid is me
std::string Vlr::printRoutesToMeToString() const
{
    std::string str ("{");      // must use double quotes as string constructed from c-string (null-terminated character sequence): string (const char* s)
    const auto epMapItr = vlrRoutingTable.endpointToRoutesMap.find(vid);
    if (epMapItr != vlrRoutingTable.endpointToRoutesMap.end()) {
        for (const auto& pathid : epMapItr->second) {
            auto vrouteItr = vlrRoutingTable.vlrRoutesMap.find(pathid);
            ASSERT(vrouteItr != vlrRoutingTable.vlrRoutesMap.end());    // pathid in endpointToRoutesMap should exist in vlrRoutingTable
            bool isTemporaryRoute = vlrRoutingTable.getIsTemporaryRoute(vrouteItr->second.isUnavailable);
            bool isDismantledRoute = vlrRoutingTable.getIsDismantledRoute(vrouteItr->second.isUnavailable);
            if (vrouteItr->second.toVid == vid && !isTemporaryRoute && !isDismantledRoute && nonEssUnwantedRoutes.find(pathid) == nonEssUnwantedRoutes.end()) {       // vroute.toVid == me, wanted (i.e. not patched)
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

void Vlr::writeRoutingTableToFile()
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
            bool isTemporaryRoute = vlrRoutingTable.getIsTemporaryRoute(vrouteItr->second.isUnavailable);
            bool isDismantledRoute = vlrRoutingTable.getIsDismantledRoute(vrouteItr->second.isUnavailable);
            if (!isTemporaryRoute && !isDismantledRoute) {
                // record for each vroute whose toVid == me "[pathid],fromVid,toVid,prevhopVid,nexthopVid\n"
                // resultFile << '[' << vrouteItr->first << "],";
                resultFile << vrouteItr->second.fromVid << ',' << vrouteItr->second.toVid << ',';
                resultFile << vrouteItr->second.prevhopVid << ',' << vrouteItr->second.nexthopVid << endl;
            }
        }
        resultFile.close();
    }
}

// if writeAtFinish=true, write firstRepSeqTimeoutTime and totalNumRepSeqTimeout to file (only one node writes them), else if writeAtFinish=false, record firstRepSeqTimeoutTime
void Vlr::writeFirstRepSeqTimeoutToFile(bool writeAtFinish) {
    const char *fname = par("firstRepSeqTimeoutCSVFile");
    if (strlen(fname) > 0) {    // repSeqTimeout file provided
        if (writeAtFinish) {
            std::ofstream resultFile;
            if (!firstRepSeqTimeoutCSVFileWritten)     // I'm the first node to write to this file
                // resultFile.open(fname, std::ofstream::out);   // open in write mode
                resultFile.open(fname, std::ofstream::app);   // open in append mode
            else 
                return;
                
            if (!resultFile.is_open()) {
                std::string filename(fname);
                // char errmsg[80] = "Unable to open routingTable csv file ";
                // throw cRuntimeError(strcat(errmsg, fname));
                EV_WARN << "Unable to open repSeqTimeout csv file " << fname << endl;
            } else {    // write to file
                if (!firstRepSeqTimeoutCSVFileWritten) {    // initialize file with header line
                    // resultFile << "vid,time" << endl;
                    firstRepSeqTimeoutCSVFileWritten = true;
                }
                int simulationSeed = par("simulationSeed");
                // if (firstRepSeqTimeoutTime == 0) {   // if rep seqNo never timeout, record firstRepSeqTimeoutTime as finish time  NOTE if firstRepSeqTimeoutTime==0, finalRepSeqTimeoutTime must also be 0
                //     firstRepSeqTimeoutTime = simTime().dbl();
                // }

                resultFile << beaconInterval << ',' << maxJitter << ',' << repSeqValidityInterval << ',' << simulateBeaconLossRate << ',' << simulationSeed << ',' << simTime() << ',' << totalNumRepSeqTimeout << ',' << firstRepSeqTimeoutTime << ',' << finalRepSeqTimeoutTime << ',' << finalRepSeqTimeoutNode << ',' << nodesRepSeqValid.size() << endl;

                resultFile.close();
            } 

        } else {
            if (firstRepSeqTimeoutTime == 0)
                firstRepSeqTimeoutTime = simTime().dbl();
            finalRepSeqTimeoutTime = simTime().dbl();
            finalRepSeqTimeoutNode = vid;
        }
    }
}

void Vlr::writeTotalNumBeacomSentToFile() {
    const char *fname = par("totalNumBeacomSentCSVFile");
    if (strlen(fname) > 0) {    // totalNumBeacomSent file provided
        std::ofstream resultFile;
        // if (!totalNumBeacomSentCSVFileCreated)     // I'm the first node to write to this file
        //     resultFile.open(fname, std::ofstream::out);   // open in write mode
        // else 
        resultFile.open(fname, std::ofstream::app);   // open in append mode
            
        if (!resultFile.is_open()) {
            std::string filename(fname);
            // char errmsg[80] = "Unable to open routingTable csv file ";
            // throw cRuntimeError(strcat(errmsg, fname));
            EV_WARN << "Unable to open totalNumBeacomSent csv file " << fname << endl;
        } else {    // write to file
            // if (!totalNumBeacomSentCSVFileCreated) {    // initialize file with header line
            //     // resultFile << "vid,time" << endl;
            //     totalNumBeacomSentCSVFileCreated = true;
            // }
            int simulationSeed = par("simulationSeed");
            resultFile << beaconInterval << ',' << maxJitter << ',' << repSeqValidityInterval << ',' << simulateBeaconLossRate << ',' << simulationSeed << ',' << simTime() << ',' << vid << ',' << totalNumBeaconSent << endl;

            resultFile.close();
        }
    }
}


}; // namespace omnetvlr
