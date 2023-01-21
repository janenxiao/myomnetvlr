//
// This file is part of an OMNeT++/OMNEST simulation example.
//
// Copyright (C) 1992-2015 Andras Varga
//
// This file is distributed WITHOUT ANY WARRANTY. See the file
// `license' for details on this and other legal matters.
//

#include "RoutingBase.h"

#include <fstream>      // for reading/writing to file
#include <sstream>      // for std::stringstream, std::ostringstream, std::istringstream
#include <algorithm>      // for std::copy(..), std::find(..)


namespace omnetvlr {

std::map<unsigned int, std::vector<std::tuple<double, std::string, std::set<unsigned int>>>> RoutingBase::failureSimulationMap;
std::vector<std::pair<double, std::vector<std::set<unsigned int>>>> RoutingBase::vidRingRegistry;
std::vector<std::string> RoutingBase::allSendRecords;
std::vector<std::string> RoutingBase::allNodeRecords;
bool RoutingBase::resultNodeCSVFileCreated = false;
bool RoutingBase::resultTestCSVFileCreated = false;
const unsigned int RoutingBase::allSendRecordsCapacity = 65536;

bool RoutingBase::routingTableVidCSVFileCreated = false;
std::vector<double> RoutingBase::writeRoutingTableToFileTimes;


RoutingBase::~RoutingBase()
{
    cancelAndDelete(startTimer);
    cancelAndDelete(failureSimulationTimer);
    cancelAndDelete(vidRingRegVsetTimer);
    cancelAndDelete(writeRoutingTableTimer);

    for (auto& elem : failureGateToPacketMap)
        cancelAndDelete(elem.second);
}

//
// static functions
//

unsigned int RoutingBase::computeVidDistance(VlrRingVID src, VlrRingVID dst)
{
    unsigned int distance = (src < dst) ? (dst - src) : (src - dst);    // distance = abs(src - dst)
    unsigned int ccwDistance = VLRRINGVID_MAX +1 - distance;            // distance in ccw ring direction
    return std::min(distance, ccwDistance);
}

// cw distance = distance from src to dst going clockwise
unsigned int RoutingBase::getVid_CW_Distance(VlrRingVID src, VlrRingVID dst)
{
    if (src < dst)
        return dst - src;
    else
        return VLRRINGVID_MAX +1 - (src - dst);
}

// ccw distance = distance from src to dst going counterclockwise
unsigned int RoutingBase::getVid_CCW_Distance(VlrRingVID src, VlrRingVID dst)
{
    if (src < dst)
        return VLRRINGVID_MAX +1 - (dst - src);
    else
        return src - dst;
}

// Commented out bc definition of function template is in .h file if the function is to be used in multiple .cc files
// // NOTE itr, begin, end are iterators of the same type, if itr is const_iterator, begin and end have to be const_iterator as well
// template<class Iterator>
// void RoutingBase::advanceIteratorWrapAround(Iterator& itr, int times, const Iterator& begin, const Iterator& end)
// {
//     while (times > 0) {
//         if (itr == end)
//             itr = begin;
//         itr++;
//         times--;
//     }
//     while (times < 0) {
//         if (itr == begin)
//             itr = end;
//         itr--;
//         times++;
//     }
//     if (itr == end)
//         itr = begin;
// }

// NOTE vecSize has to be int type for correct % operation
void RoutingBase::advanceVectorIndexWrapAround(unsigned int& index, int times, const int& vecSize)
{
    if (vecSize < 2)
        return;
    times += index;
    if (times >= vecSize)
        index = times % vecSize;
    else if (times < 0)
        index = times % vecSize + vecSize;
    else    // 0 <= times < vecSize
        index = times;
}

std::vector<unsigned int> RoutingBase::removeLoopInTrace(const std::vector<unsigned int>& trace)
{
    std::vector<unsigned int> newTrace;
    std::set<unsigned int> nodes;     // nodes in newTrace
    for (const auto& currnode : trace) {
        if (nodes.find(currnode) != nodes.end()) {  // if currnode already exists in newTrace
            // delete the nodes between the 2 duplicate currnode, excluding the previous currnode
            auto tailItr = --newTrace.end();  // iterator to last node in newTrace
            while (*tailItr != currnode) {
                nodes.erase(*tailItr);
                newTrace.erase(tailItr--);
            }
        } else {    // if currnode hasn't appeared in newTrace
            newTrace.push_back(currnode);
            nodes.insert(currnode);
        }
    }
    return newTrace;
}


// return numHalf nodes close to me in ccw/cw direction in traceSet
// returned nodes in order: [closest cw, closest ccw, 2nd closest cw, 2nd closest ccw, ..]
std::vector<VlrRingVID> RoutingBase::getCloseNodesInSet(int numHalf, const std::set<VlrRingVID>& traceSet) const
{
    std::vector<VlrRingVID> closeVec;
    if (!traceSet.empty()) {
        auto itrup = traceSet.lower_bound(vid);    // itrup points to closest cw node in traceSet
        advanceIteratorWrapAround(itrup, 0, traceSet.begin(), traceSet.end());
        auto itrlow = itrup;
        if (*itrup == vid)
            advanceIteratorWrapAround(itrup, 1, traceSet.begin(), traceSet.end());

        for (int i = 0; i < numHalf; ++i) {
            if (itrup == itrlow && i > 0 || *itrup == vid)   // when i == 0, itrup == itrlow but itrup hasn't been examined yet; if *itrup == vid, I am the only node in traceSet 
                break;
            else {
                closeVec.push_back(*itrup);
                advanceIteratorWrapAround(itrlow, -1, traceSet.begin(), traceSet.end());
                if (itrlow == itrup)
                    break;
                else {
                    closeVec.push_back(*itrlow);
                    advanceIteratorWrapAround(itrup, 1, traceSet.begin(), traceSet.end());
                }
            }
        }
    }
    return closeVec;
}

// Commented out bc definition of function template is in .h file if the function is to be used in multiple .cc files
// template<class VidMap>
// std::vector<VlrRingVID> RoutingBase::getCloseNodesInMap(int numHalf, const VidMap& vidMap) const
// {
//     std::vector<VlrRingVID> closeVec;
//     if (!vidMap.empty()) {
//         auto itrup = vidMap.lower_bound(vid);    // itrup points to closest cw node in vidMap
//         advanceIteratorWrapAround(itrup, 0, vidMap.begin(), vidMap.end());
//         auto itrlow = itrup;
//         if (itrup->first == vid)
//             advanceIteratorWrapAround(itrup, 1, vidMap.begin(), vidMap.end());

//         for (int i = 0; i < numHalf; ++i) {
//             if (itrup == itrlow && i > 0 || itrup->first == vid)   // when i == 0, itrup == itrlow but itrup hasn't been examined yet; if *itrup == vid, I am the only node in vidMap 
//                 break;
//             else {
//                 closeVec.push_back(itrup->first);
//                 advanceIteratorWrapAround(itrlow, -1, vidMap.begin(), vidMap.end());
//                 if (itrlow == itrup)
//                     break;
//                 else {
//                     closeVec.push_back(itrlow->first);
//                     advanceIteratorWrapAround(itrup, 1, vidMap.begin(), vidMap.end());
//                 }
//             }
//         }
//     }
//     return closeVec;
// }

/**
 * generate new pathid = somehasher(myVid, dstVid, currentTime, randomSeed)
 */
VlrPathID RoutingBase::genPathID(VlrRingVID dstVid) const
{
    size_t myhash = vid + 127;      // * 2654435761U       U: unsigned int
    size_t seed = intuniform(0, RAND_MAX);
    myhash = ((myhash + seed) * (myhash + seed + 1) >> 1) + myhash;
    size_t dsthash = std::hash<VlrRingVID>{}(dstVid);
    myhash ^= dsthash + 0x9e3779b9 + (myhash << 6) + (myhash >> 2);
    size_t timehash = std::hash<int64_t>{}(simTime().raw());    // hash of current time
    myhash = myhash ^ (timehash << 1);
    // convert 64-bit hash to 32-bit
    while (myhash >> 32)
        myhash = (myhash & 0xFFFFFFFF) + (myhash >> 32);        // add upper 32 bits to lower 32 bits

    std::set<size_t> reservedPathids {VLRPATHID_INVALID, VLRPATHID_REPPATH};
    while (reservedPathids.find(myhash) != reservedPathids.end())   // myhash is one of the reserved pathids
    // if (myhash == VLRPATHID_INVALID)
        myhash ^= (1 << intuniform(0, 31));             // flip a random bit in the 32-bit hash
    return (VlrPathID)myhash;
}


void RoutingBase::initialize()
{
    vid = getParentModule()->par("address");
    if (vidRingRegistry.size() < 1)
        vidRingRegistry.push_back({0, {{}}});    // initialize vidRingRegistry[0]
    vidRingRegistry[0].second[0].insert(vid);      // since entire network is connected at stage 0, vid is always inserted into vidRingRegistry[0][0]

    dropSignal = registerSignal("drop");

    startTimer = new cMessage("startTimer");
    scheduleAt(0, startTimer);
    failureSimulationTimer = new cMessage("failureSimulationTimer");
    vidRingRegVsetTimer = new cMessage("vidRingRegVsetTimer");
    writeRoutingTableTimer = new cMessage("writeRoutingTableTimer");

    // initialize failureSimulationMap
    if (failureSimulationMap.empty()) {
        // failureSimulationMap = {
        //     {2, {{80, "stop", {4}}}},
        // };
    }
    initializeFailureSimulationMap();   // first node read failure simulation file to initialize failureSimulationMap if it's empty
    auto failureSimulationItr = failureSimulationMap.find(vid);
    if (failureSimulationItr != failureSimulationMap.end()) {
        const auto& tuple = failureSimulationItr->second.front();
        EV_DEBUG << "Scheduling initial failure simulation timer at " << std::get<0>(tuple) << endl;
        scheduleAt(std::get<0>(tuple), failureSimulationTimer);
    }
    // schedule first vidRingRegVsetTimer to set vidRingRegVset based on physical topo
    scheduleAt(2, vidRingRegVsetTimer);     // schedule at t=2 to initialize vidRingRegVset after all nodes have registered itself in vidRingRegistry[0] in initialize(), if failure simulated at start, failure should be scheduled at t=1

    // schedule first writeRoutingTableTimer if file provided
    initializeWriteRoutingTableToFileTimes();
}

void RoutingBase::sendCreatedPacket(cPacket *packet, bool unicast, int outGateIndex, double delay/*=0*/, bool checkFail/*=true*/, const std::vector<int> *exclOutGateIndexes/*=nullptr*/)
{
    if (unicast) {
        if (checkFail && failureSimulationPneiGates.find(outGateIndex) != failureSimulationPneiGates.end()) { // if checkFail=true, we don't send unicast msg through failed links
            if (failureGateToPacketMap.find(outGateIndex) == failureGateToPacketMap.end()) {
                // get failed pnei from failed gateIndex
                std::vector<VlrRingVID> pneiVidResults;
                for (const auto& mappair : failureSimulationPneiVidMap)
                    if (mappair.second == outGateIndex)
                        pneiVidResults.push_back(mappair.first);
                ASSERT(pneiVidResults.size() == 1);    // assert one and only one pnei associated with targetGate, if outGateIndex in failureSimulationPneiGates, corresponding pneiVid should be in failureSimulationPneiVidMap since they are added/removed at the same time

                // schedule a timer to process the failed packet later
                // char timerName[40];
                // sprintf(timerName, "FailedPacketDelayTimer:%d-%d", vid, outGateIndex);   // then new FailedPacketDelayTimer(timerName)
                std::ostringstream s;
                s << "FailedPacketDelayTimer:" << vid << '-' << outGateIndex;
                FailedPacketDelayTimer *failedpktTimer = new FailedPacketDelayTimer(s.str().c_str());
                failedpktTimer->setFailedGateIndex(outGateIndex);
                failedpktTimer->setFailedPnei(pneiVidResults[0]);
                failedpktTimer->setFailedPacket(packet);
                scheduleAt(simTime(), failedpktTimer);  // delay failed packet processing for 1 sec
                failureGateToPacketMap[outGateIndex] = failedpktTimer;
            } else {    // there's already a failed packet sent to outGateIndex scheduled to be processed, I'll know that outGateIndex isn't reachable, no need to record another failed packet to outGateIndex if we don't plan to resend packet
                delete packet;
            }
        } else {
            if (delay == 0)
                send(packet, "out", outGateIndex);
            else
                sendDelayed(packet, delay, "out", outGateIndex);
        }
    } else {    // broadcast packet to every out gate
        int numSent = 0;
        for (int i = 0; i < gateSize("out"); i++) {
            if (checkFail && failureSimulationPneiGates.find(i) != failureSimulationPneiGates.end())    // if checkFail=true, we don't send msg through failed links
                continue;
            if (exclOutGateIndexes && std::find(exclOutGateIndexes->begin(), exclOutGateIndexes->end(), i) != exclOutGateIndexes->end())   // we don't send msg to gates in exclOutGateIndexes
                continue;
            if (numSent > 0)    // if packet has been sent, create another packet to send to another pnei
                packet = packet->dup();
            if (delay == 0)
                send(packet, "out", i);
            else
                sendDelayed(packet, delay, "out", i);
            numSent++;
        }
        if (numSent == 0)
            delete packet;
    }
}

void RoutingBase::processFailedPacketDelayTimer(FailedPacketDelayTimer *failedpktTimer)
{
    int failedGateIndex = failedpktTimer->getFailedGateIndex();
    VlrRingVID failedPnei = failedpktTimer->getFailedPnei();
    cPacket *failedPacket = failedpktTimer->getFailedPacketForUpdate();
    EV_INFO << "Processing failed packet delay timer at node " << vid << ", failedGateIndex=" << failedGateIndex << endl;

    auto failurePacketMapItr = failureGateToPacketMap.find(failedGateIndex);
    ASSERT(failurePacketMapItr != failureGateToPacketMap.end());    // failedpktTimer must be a value in map with failedGateIndex as key
    
    processFailedPacket(failedPacket, failedPnei);      // failed packet is deleted here
    
    cancelAndDelete(failedpktTimer);
    failureGateToPacketMap.erase(failurePacketMapItr);
}

void RoutingBase::processFailureSimulationTimer()
{
    EV_INFO << "Processing failure simulation timer at node " << vid << endl;
    auto failureSimulationItr = failureSimulationMap.find(vid);
    if (failureSimulationItr != failureSimulationMap.end()) {
        // find the timestamp that triggered this failure simulation timeout
        simtime_t currTime = simTime();
        int i = failureSimulationItr->second.size() -1;
        for ( ; i >= 0; i--) {
            if (currTime >= std::get<0>(failureSimulationItr->second.at(i)))
                break;
        }
        ASSERT(i >= 0);     // failure simulation timer has timed out bc a timestamp in failureSimulationMap[me] vector has been reached
        const auto& tuple = failureSimulationItr->second.at(i);
        EV_INFO << "Simulating failure operation=" << std::get<1>(tuple) << " at me=" << vid << " with nodes " << std::get<2>(tuple) << endl;
        const auto& operation = std::get<1>(tuple);
        if (operation == "stop") {
            if (std::get<2>(tuple).empty())    // node failure
                handleStopOperation();
            else
                handleFailureLinkSimulation(std::get<2>(tuple));
        } else if (operation == "start") {
            if (std::get<2>(tuple).empty()) {   // node restart
                handleFailureNodeRestart();
                handleStartOperation();
            } else
                handleFailureLinkRestart(std::get<2>(tuple));
        }
        if (i < failureSimulationItr->second.size() -1)     // more failure operation to schedule
            scheduleAt(std::get<0>(failureSimulationItr->second.at(i+1)), failureSimulationTimer);
    }
}

void RoutingBase::processVidRingRegVsetTimer(int numHalf)
{
    EV_INFO << "Processing vidRingRegVsetTimer at node " << vid << endl;
    // find the timestamp that triggered this timer
    simtime_t currTime = simTime();
    int i = vidRingRegistry.size() -1;
    for ( ; i >= 0; i--) {
        if (currTime >= vidRingRegistry.at(i).first)
            break;
    }
    ASSERT(i >= 0);     // timer has timed out bc a timestamp in vidRingRegistry vector has been reached
    // reset vidRingRegVset based on vidRingRegistry
    vidRingRegVset.clear();
    
    const auto& componentVec = vidRingRegistry.at(i).second;      // list of sets, each with connected nodes in a connected component after currTime
    for (auto componentItr = componentVec.begin(); componentItr != componentVec.end(); ++componentItr) {
        const auto& vidSet = *componentItr;
        auto vidItr = vidSet.find(vid);
        if (vidItr != vidSet.end()) {
            // find numHalf nodes close to me in ccw/cw direction in traceSet
            std::vector<VlrRingVID> closeVec = getCloseNodesInSet(numHalf, vidSet);    // numHalf ccw/cw nodes close to me in traceSet
            ASSERT(closeVec.size() == 2 * numHalf);
            // NOTE if pass an output iterator vidRingRegVset.begin() directly to std::copy, you must make sure it points to a range that is at least large enough to hold the input range, otherwise needs std::inserter
            std::copy(closeVec.begin(), closeVec.end(), std::inserter(vidRingRegVset, vidRingRegVset.begin()));     // copy closeVec to vidRingRegVset
            break;      // found my vid in a component
        }
    }
    // schedule next vidRingRegVsetTimer
    if (i < vidRingRegistry.size() -1)     // more physical topo change to schedule
        scheduleAt(vidRingRegistry.at(i+1).first, vidRingRegVsetTimer);
}

VlrRingVID RoutingBase::getRandomVidInRegistry()
{
    // find the latest timestamp <= current time
    simtime_t currTime = simTime();
    int i = vidRingRegistry.size() -1;
    for ( ; i >= 0; i--) {
        if (currTime >= vidRingRegistry.at(i).first)
            break;
    }
    ASSERT(i >= 0);     // i = 0 if no failure partition assignment file provided, i.e. vidRingRegistry.size() == 1 recorded in initialize()

    const auto& componentVec = vidRingRegistry.at(i).second;      // list of sets, each with connected nodes in a connected component after currTime
    for (auto componentItr = componentVec.begin(); componentItr != componentVec.end(); ++componentItr) {
        const auto& vidSet = *componentItr;
        auto vidItr = vidSet.find(vid);
        if (vidItr != vidSet.end()) {       // found the component that contains my vid
    
            // get random vid from vidSet
            int maxTrialLimit = 10;
            for (int trialIndex = 0; trialIndex < maxTrialLimit; trialIndex++) {
                auto itr = vidSet.begin();
                int vidSetIndex = intuniform(0, vidSet.size() - 1);
                for (int i = 0; i < vidSetIndex; i++)
                    itr++;
                if (*itr != vid)
                    return *itr;
            }
            break;
        }
    }
    return VLRRINGVID_NULL;
}

void RoutingBase::processWriteRoutingTableTimer()
{
    EV_DEBUG << "Processing writeRoutingTableTimer at node " << vid << endl;
    // find the timestamp that triggered this timer
    simtime_t currTime = simTime();
    int i = writeRoutingTableToFileTimes.size() -1;
    for ( ; i >= 0; i--) {
        if (currTime >= writeRoutingTableToFileTimes.at(i))
            break;
    }
    ASSERT(i >= 0);     // timer has timed out bc a timestamp in writeRoutingTableToFileTimes vector has been reached
    // write routing table to file
    writeRoutingTableToFile();
    // schedule next writeRoutingTableTimer
    if (i < writeRoutingTableToFileTimes.size() -1)     // more writeRoutingTable timer to schedule
        scheduleAt(writeRoutingTableToFileTimes.at(i+1), writeRoutingTableTimer);
}

void RoutingBase::handleStartOperation()
{
    
}

void RoutingBase::initializeFailureSimulationMap()
{
    if (failureSimulationMap.empty()) {     // if failure simulation file provided, only one node should initialize failureSimulationMap
        // get failure times
        std::vector<double> failureOpTimeList;  // must be set before reading failure into failureSimulationMap
        const char *failureTimesPar = par("failureSimulationOpTimes");
        if (strlen(failureTimesPar) > 0) {    // failure operation times string provided
            cStringTokenizer tokenizer(failureTimesPar, /*delimiters=*/", ");
            const char *token;
            while ((token = tokenizer.nextToken()) != nullptr)
                failureOpTimeList.push_back((double)atoi(token));
        }

        const char *startstr = "operation=";
        // process link failure simulation file
        const char *fname = par("failureLinkSimulationFile");
        if (strlen(fname) > 0) {    // failure link simulation file provided
            std::ifstream vidFile(fname);
            if (!vidFile.good())
                throw cRuntimeError("Unable to load failureLink assignment file");
            
            // if (failureOpTimeList.size() == 0)
            //     throw cRuntimeError("At least one failure operation time must be specified in the parameter!");

            int failureOpTimeIndex = 0;
            double failureLinkSimulationStartTime = 0;
            std::string operation;
            
            unsigned int nodeVid;
            std::string line;
            while (std::getline(vidFile, line)) {   // for each line in file
                if (line.rfind(startstr, 0) == 0) { // line begins with startstr
                    cStringTokenizer tokenizer(line.c_str() + strlen(startstr), /*delimiters=*/", ");    // start parsing after startstr
                    const char *token = tokenizer.nextToken();
                    ASSERT(token != nullptr);
                    // failureLinkSimulationStartTime = (double)atoi(token);
                    // token = tokenizer.nextToken();
                    // ASSERT(token != nullptr);
                    operation = token;
                    ASSERT(failureOpTimeIndex < failureOpTimeList.size());
                    failureLinkSimulationStartTime = failureOpTimeList[failureOpTimeIndex++];
                } else {
                    std::istringstream iss(line);
                    std::string token;
                    std::vector<unsigned int> vidlist;
                    while(std::getline(iss, token, ',')) {  // split line with delimiter character ','
                        try{
                            nodeVid = std::stoul(token, nullptr, 0); // base=0: base used is determined by format in vidstr
                            vidlist.push_back(nodeVid);
                        }
                        catch (std::exception& e) {
                            throw cRuntimeError("Error to parse token \"%s\" in failureLink assignment file into unsigned int: %s", token.c_str(), e.what());
                        }
                    }
                    ASSERT(vidlist.size() == 2);
                    ASSERT(failureLinkSimulationStartTime > 0);
                    // append link if either link end is in failureSimulationMap
                    auto failureSimulationItr = failureSimulationMap.find(vidlist[0]);
                    if (failureSimulationItr != failureSimulationMap.end()) {
                        auto& tuple = failureSimulationItr->second.back();
                        if (std::get<0>(tuple) < failureLinkSimulationStartTime)
                            failureSimulationItr->second.push_back({failureLinkSimulationStartTime, operation, {vidlist[1]}});
                        else
                            std::get<2>(tuple).insert(vidlist[1]);
                    }
                    else if ((failureSimulationItr = failureSimulationMap.find(vidlist[1])) != failureSimulationMap.end()) {
                        auto& tuple = failureSimulationItr->second.back();
                        if (std::get<0>(tuple) < failureLinkSimulationStartTime)
                            failureSimulationItr->second.push_back({failureLinkSimulationStartTime, operation, {vidlist[0]}});
                        else
                            std::get<2>(tuple).insert(vidlist[0]);
                    }
                    else    // if neither link end is in failureSimulationMap, insert failureSimulationMap[vidlist[0]]
                        failureSimulationMap.insert({vidlist[0], {{failureLinkSimulationStartTime, operation, {vidlist[1]}}}});
                }
            }
            EV_INFO << "Initialized failureSimulationMap with failureLink assignment file" << endl;
            // for (const auto& mappair : failureSimulationMap) {
            //     EV_INFO << mappair.first << " [";
            //     for (const auto& tuple : mappair.second)
            //         EV_INFO << std::get<0>(tuple) << " " << std::get<1>(tuple) << " " << std::get<2>(tuple) << " ";
            //     EV_INFO << "]";
            // }
            
        } else {
            // process node failure simulation file
            const char *fname = par("failureNodeSimulationFile");
            if (strlen(fname) > 0) {    // failure link simulation file provided
                std::ifstream vidFile(fname);
                if (!vidFile.good())
                    throw cRuntimeError("Unable to load failureNode assignment file");

                int failureOpTimeIndex = 0;
                double failureNodeSimulationStartTime = 0;
                std::string operation;
                
                unsigned int nodeVid;
                std::string line;
                while (std::getline(vidFile, line)) {   // for each line in file
                    if (line.rfind(startstr, 0) == 0) { // line begins with startstr
                        cStringTokenizer tokenizer(line.c_str() + strlen(startstr), /*delimiters=*/", ");    // start parsing after startstr
                        const char *token = tokenizer.nextToken();
                        ASSERT(token != nullptr);
                        operation = token;
                        ASSERT(failureOpTimeIndex < failureOpTimeList.size());
                        failureNodeSimulationStartTime = failureOpTimeList[failureOpTimeIndex++];
                    } else {
                        std::istringstream iss(line);
                        std::string token;
                        std::vector<unsigned int> vidlist;
                        while(std::getline(iss, token, ',')) {  // split line with delimiter character ','
                            try{
                                nodeVid = std::stoul(token, nullptr, 0); // base=0: base used is determined by format in vidstr
                                vidlist.push_back(nodeVid);
                            }
                            catch (std::exception& e) {
                                throw cRuntimeError("Error to parse token \"%s\" in failureLink assignment file into unsigned int: %s", token.c_str(), e.what());
                            }
                        }
                        ASSERT(vidlist.size() == 1);
                        ASSERT(failureNodeSimulationStartTime > 0);
                        // append link if either link end is in failureSimulationMap
                        auto failureSimulationItr = failureSimulationMap.find(vidlist[0]);
                        if (failureSimulationItr == failureSimulationMap.end())     // failureNode doesn't already exist in failureSimulationMap
                            failureSimulationMap.insert({vidlist[0], {{failureNodeSimulationStartTime, operation, {}}}});
                        else {
                            auto& tuple = failureSimulationItr->second.back();
                            ASSERT(std::get<0>(tuple) < failureNodeSimulationStartTime);    // node failure of vidlist[0] at failureNodeSimulationStartTime isn't already scheduled
                            failureSimulationItr->second.push_back({failureNodeSimulationStartTime, operation, {}});
                        }
                    }
                }
                EV_INFO << "Initialized failureSimulationMap with failureNode assignment file" << endl;
            }
        }
        if (!failureSimulationMap.empty()) {    // if failure scheduled, read failure partition assignment file into vidRingRegistry
            const char *fname = par("failureRingPartitionFile");
            // if (strlen(fname) == 0)
            //     throw cRuntimeError("Failure simulated but failure partition assignment file not provided");
            if (strlen(fname) > 0) {    // failure link simulation file provided
                std::ifstream vidFile(fname);
                if (!vidFile.good())
                    throw cRuntimeError("Unable to load failure partition assignment file");
                
                // NOTE vidRingRegistry[i+1].first = failureOpTimeList[i] bc vidRingRegistry[0].first = 0
                int failureOpTimeIndex = 0;
                double vidRingRegistryStartTime = 0;
                int vidRingRegistryIndex;
                
                std::string line;
                const char *token;
                while (std::getline(vidFile, line)) {   // for each line in file
                    if (line.rfind(startstr, 0) == 0) { // line begins with startstr
                        cStringTokenizer tokenizer(line.c_str() + strlen(startstr), /*delimiters=*/", ");    // start parsing after startstr
                        const char *token = tokenizer.nextToken();
                        ASSERT(token != nullptr);
                        // failureLinkSimulationStartTime = (double)atoi(token);
                        // token = tokenizer.nextToken();
                        // ASSERT(token != nullptr);
                        // operation = token;
                        ASSERT(failureOpTimeIndex < failureOpTimeList.size());
                        vidRingRegistryIndex = failureOpTimeIndex + 1;
                        vidRingRegistryStartTime = failureOpTimeList[failureOpTimeIndex++];
                        if (vidRingRegistryIndex >= vidRingRegistry.size())
                            vidRingRegistry.push_back({vidRingRegistryStartTime, {}});    // initialize vidRingRegistry at a failure operation time
                    } else {
                        vidRingRegistry[vidRingRegistryIndex].second.push_back({});           // initialize empty component in vidRingRegistry[failureOpTime]
                        
                        cStringTokenizer tokenizer(line.c_str(), /*delimiters=*/", ");    // start parsing after startstr
                        while ((token = tokenizer.nextToken()) != nullptr) {
                            try{
                                vidRingRegistry[vidRingRegistryIndex].second.back().insert( (unsigned int)std::stoul(token, nullptr, 0) ); // base=0: base used is determined by format in vidstr
                            }
                            catch (std::exception& e) {
                                throw cRuntimeError("Error to parse token \"%s\" in failure partition assignment file (vidRingRegistryIndex=%d, componentIndex=%d) into unsigned int: %s", token, vidRingRegistryIndex, vidRingRegistry[vidRingRegistryIndex].second.size()-1, e.what());
                            }
                        }                    

                        if (vidRingRegistry[vidRingRegistryIndex].second.back().empty())
                            throw cRuntimeError("Failure partition assignment file (vidRingRegistryIndex=%d, componentIndex=%d) error: not vid connected", vidRingRegistryIndex, vidRingRegistry[vidRingRegistryIndex].second.size()-1);
                    }
                }
            } else  // if failure scheduled but failure link simulation file not provided, perhaps only link failures simulated and they don't cause partition 
                EV_WARN << "Failure simulated but failure partition assignment file not provided" << endl;
        }
    } 
}

void RoutingBase::initializeSelfTestDstList()
{
    if (testDstList.empty()) {     // if testDst assignment file provided, every node should initialize its testDstList
        const char *fname = par("testDstAssignmentFile");
        if (strlen(fname) > 0) {    // testDst assignment file provided
            std::ifstream vidFile(fname);
            if (!vidFile.good())
                throw cRuntimeError("Unable to load testDst assignment file");
            
            unsigned int nodeVid;
            std::string line;
            const char *token;
            while (std::getline(vidFile, line)) {   // for each line in file
                cStringTokenizer tokenizer(line.c_str(), /*delimiters=*/", ");
                token = tokenizer.nextToken();
                ASSERT(token != nullptr);
                nodeVid = std::stoul(token, nullptr, 0);
                if (nodeVid == vid) {
                    while ((token = tokenizer.nextToken()) != nullptr)
                        testDstList.push_back( (unsigned int)std::stoul(token, nullptr, 0) ); // base=0: base used is determined by format in vidstr
                    break;
                }
            }
            EV_INFO << "Initialized testDstList at me=" << vid << " with testDst assignment file: " << testDstList << endl;
        }
    }
}

void RoutingBase::initializeWriteRoutingTableToFileTimes()
{
    // get write routingTable to file and times, see if it's provided
    const char *fname = par("routingTableVidCSVFile");
    const char *writeTimesPar = par("writeRoutingTableToFileTimes");
    if (strlen(fname) > 0 && strlen(writeTimesPar) > 0) {    // routingTable file provided, write times string also provided
        if (writeRoutingTableToFileTimes.empty()) {     // writeRoutingTableToFileTimes not initialized yet
            cStringTokenizer tokenizer(writeTimesPar, /*delimiters=*/", ");
            const char *token;
            while ((token = tokenizer.nextToken()) != nullptr)
                writeRoutingTableToFileTimes.push_back((double)atoi(token));

            EV_INFO << "Initialized writeRoutingTableToFileTimes = [";
            for (const auto& time : writeRoutingTableToFileTimes)
                EV_INFO << time << ' ';
            EV_INFO << "] with routingTable file name = " << fname << endl;
        }
        if (writeRoutingTableToFileTimes.empty())
            throw cRuntimeError("RoutingTable write file specified but writeRoutingTableToFileTimes is empty");
        scheduleAt(writeRoutingTableToFileTimes[0], writeRoutingTableTimer);
    }
}

// record message record in allSendRecords
// action can be 0: sending, 1: receiving, 2: processing
void RoutingBase::recordMessageRecord(char action, const VlrRingVID& src, const VlrRingVID& dst, const char *msgType, unsigned int msgId, unsigned int hopcount, unsigned int chunkByteLength, const char *infoStr/*=""*/)
{
    std::ostringstream s;
    // Declaration of new variables in case statements requires {} block
    // The problem is that variables declared in one case are still visible in the subsequent cases unless an explicit {} block is used, but they will not be initialized because the initialization code belongs to another case.
    switch (action) {
        case 0:     // record a message to send in allSendRecords
            // Commented out bc we increment allSendMessageId when create a message
            // allSendMessageId++;     // increment message id for each message sent
            s << vid << ',' << src << ',' << allSendMessageId << ',' << dst << ',' << simTime() << ',' << msgType << ',' << "sent" << ",,," << infoStr;
            allSendRecords.push_back(s.str());
            break;
        case 1:     // record a received message destined for me in allSendRecords
            s << vid << ',' << src << ',' << msgId << ',' << dst << ',' << simTime() << ',' << msgType << ',' << "arrived" << ',' << hopcount << ',' << chunkByteLength << ',' << infoStr;
            allSendRecords.push_back(s.str());
            break;
        case 2:     // record a message received in allSendRecords
            s << vid << ',' << src << ',' << msgId << ',' << dst << ',' << simTime() << ',' << msgType << ',' << "received" << ',' << hopcount << ',' << chunkByteLength << ',' << infoStr;
            allSendRecords.push_back(s.str());
            break;
        case 4:     // record a message dropped in allSendRecords
            s << vid << ',' << src << ',' << msgId << ',' << dst << ',' << simTime() << ',' << msgType << ',' << "dropped" << ',' << hopcount << ',' << chunkByteLength << ',' << infoStr;
            allSendRecords.push_back(s.str());
            break;
    }
}

// record node stats record in allNodeRecords
void RoutingBase::recordNodeStatsRecord(const char *infoStr)
{
    std::ostringstream s;

    s << vid << ',' << simTime() << ',' << infoStr;
    allNodeRecords.push_back(s.str());
}

void RoutingBase::writeToResultNodeFile()
{
    const char *fname = par("resultNodeCSVFile");
    std::ofstream resultNodeFile;
    if (!resultNodeCSVFileCreated)     // I'm the first node to write to result node file
        resultNodeFile.open(fname, std::ofstream::out);   // open in write mode
    else 
        resultNodeFile.open(fname, std::ofstream::app);   // open in append mode
        
    if (!resultNodeFile.is_open()) {
        std::string filename(fname);
        // char errmsg[80] = "Unable to open result node csv file ";
        // throw cRuntimeError(strcat(errmsg, fname));
        EV_WARN << "Unable to open result node csv file " << fname << endl;
    } else {    // write to file
        if (!resultNodeCSVFileCreated) {    // initialize file with header line
            // resultNodeFile << "vid,time,topic,position,L3Address,simulatedPneis,selfInNetwork,hasRoot,vset,simulatedPneis size,vset size,pendingVset size,numVroutes,stage" << endl;
            resultNodeCSVFileCreated = true;
        }
        for(auto it = allNodeRecords.begin(); it != allNodeRecords.end(); ++it)
            resultNodeFile << *it << endl;
        resultNodeFile.close();
        // clear records that has been written to file
        allNodeRecords.clear();
    }
}

void RoutingBase::writeToResultMessageFile()
{
    // if (vid == getFirstConnVidInRegistry()) {    // first node in vidRegistryTable should write to result messages file
    const char *fname = par("resultTestCSVFile");
    std::ofstream resultTestFile;
    if (!resultTestCSVFileCreated)     // first time writing to this file
        resultTestFile.open(fname, std::ofstream::out);   // open in write mode
    else 
        resultTestFile.open(fname, std::ofstream::app);   // open in append mode

    if (!resultTestFile.is_open()) {
        // char errmsg[80] = "Unable to open result messages csv file ";
        // throw cRuntimeError(strcat(errmsg, fname));
        EV_WARN << "Unable to open result messages csv file " << fname << endl;
    } else {
        if (!resultTestCSVFileCreated) {    // initialize file with header line
            // resultTestFile << "src,dst,msgHopCount" << endl;
            resultTestCSVFileCreated = true;
        }
        for(auto it = allSendRecords.begin(); it != allSendRecords.end(); ++it)
            resultTestFile << *it << endl;
        resultTestFile.close();
        // clear records that has been written to file
        allSendRecords.clear();
        allSendRecords.reserve(allSendRecordsCapacity);     // request the minimum vector capacity at the start to avoid reallocation
    }
}


std::ostream &operator<<(std::ostream &o, const std::vector<unsigned int> &vids)
{
    o << "[ ";
    for (auto& vid : vids) {
        o << vid << " ";
    }
    o << "]";
    return o;
}

std::ostream &operator<<(std::ostream &o, const std::set<unsigned int> &vids)
{
    o << "{ ";
    for (auto& vid : vids) {
        o << vid << " ";
    }
    o << "}";
    return o;
}

std::ostream &operator<<(std::ostream &o, const RoutingBase::Representative &rep)
{
    o << "{" << rep.vid;
    o << ", seq=" << rep.sequencenumber;
    // o << ", hopcount=" << rep.hopcount;
    o << ", heardfrom=" << rep.heardfromvid;
    o << "}";
    return o;
}

}; // namespace omnetvlr
