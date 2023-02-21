//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __OMNETVLR_ROUTINGBASE_H
#define __OMNETVLR_ROUTINGBASE_H

#include <map>
#include <set>
#include <tuple>
#include <omnetpp.h>

#include "vlr/Vlr_m.h"

using namespace omnetpp;

namespace omnetvlr {

class RoutingBase : public cSimpleModule
{
  public:
    struct Representative {
      VlrRingVID vid;
      unsigned int sequencenumber;
      // unsigned int hopcount;
      VlrRingVID heardfromvid;
      simtime_t lastHeard;
      bool inNetwork;

      unsigned int lastBeaconSeqnum;   // rep seqNo forwarded in my last beacon, if rep wasn't included in my last beacon, lastBeaconSeqnum = 0 (I'll never broadcast my own rep seqNo as 0)
      bool lastBeaconSeqnumUnchanged;    // rep seqNo was sent in the last two beacons and was the same, need to notify pneis asap if new rep seqNo received

      // static const unsigned int HOPCOUNT_INFINITY = 65534;
    };

  protected:
    unsigned int vid;
    static std::vector<std::pair<double, std::vector<std::set<unsigned int>>>> vidRingRegistry;      // vidRingRegistry[0] (network partition stage 0): (time, [set contains vids in a connected component after time])
    std::set<unsigned int> vidRingRegVset;
    static std::set<unsigned int> nodesVsetCorrect;

    simsignal_t dropSignal;

    cMessage *startTimer = nullptr; // should be created and scheduled in initialize(), once invoked, call handleStartOperation()
    cMessage *failureSimulationTimer = nullptr; // time for me to simulate node/link failure
    cMessage *vidRingRegVsetTimer = nullptr; // time for me to change vidRingRegVset according to vidRingRegistry
    cMessage *writeRoutingTableTimer = nullptr;   // timer to write vlrRoutingTable to write vlrRoutingTable at nodes to file
    cMessage *writeNodeStatsTimer = nullptr;   // timer to record nodeStats to allNodeRecords

    // statistics collection
    std::vector<unsigned int> testDstList;  // predetermined list of node vids (read from testDstAssignmentFile) to send TestPacket
    static std::vector<std::string> allSendRecords;  // each message generated should be recorded as a string, all strings should be written to a single file by a single node
    unsigned int allSendMessageId = 0;    // each message I sent and recorded in allSendRecords is associated with a unique message id
    static std::vector<std::string> allNodeRecords;  // record node stats as a string, all strings should be written to a single file by a single node
    static bool resultNodeCSVFileCreated;         // [ definition in VlrBase.cc file ] default false, first node that writes to result node statistics file should change this to true
    static bool resultTestCSVFileCreated;         // [ definition in VlrBase.cc file ] default false, first node that writes to result messages file should change this to true
    static const unsigned int allSendRecordsCapacity;

    static std::vector<double> writeRoutingTableToFileTimes;  // time after start to write vlrRoutingTable to file
    static bool routingTableVidCSVFileCreated;         // [ definition in VlrBase.cc file ] default false, first node that writes to the file should change this to true

    static std::vector<double> writeNodeStatsTimes;  // time after start to record nodeStats to allNodeRecords

    // failure simulation
    static std::map<unsigned int, std::vector<std::tuple<double, std::string, std::set<unsigned int>>>> failureSimulationMap;  // {node1, [(100, "stop", {node2, node3})]} means node1 stops processing messages from node2 and node3 after 100s; {node1, [(100, "stop", {})]} means node1 stops processing messages (node failure) after 100s
    std::map<unsigned int, int> failureSimulationPneiVidMap;   // pnei vids from which I won't process messages in order to simulate link failures, map pnei to its gate index at me
    std::set<int> failureSimulationPneiGates;   // pnei gate indexes from which I won't process messages in order to simulate link failures
    std::set<unsigned int> failureSimulationUnaddedPneiVids;  // pnei vids that haven't been added to failureSimulationPneiVidMap bc I don't know their corresponding gete index yet, once I receive a msg from any of them, I'll add it to failureSimulationPneiVidMap
    bool selfNodeFailure = false;   // if true, I'm simulating node failure and I won't process any VLR messages
    std::map<int, FailedPacketDelayTimer *> failureGateToPacketMap;   // store failed packet to some gateIndex to process later, simulate time for sending a packet multiple times b4 signaling a link break 

    double simulateBeaconLostRate;    // value in range [0, 1), ratio of beacons lost because of collision, default 0, e.g., 0.1 means 10% probability that I'll ignore a received beacon to simulate collision

  public:
    virtual ~RoutingBase();

    // static function    NOTE static methods don't have an entry in the vtable, thus can't be overridden by subclasses
    static unsigned int computeVidDistance(VlrRingVID src, VlrRingVID dst);
    static unsigned int getVid_CW_Distance(VlrRingVID src, VlrRingVID dst);
    static unsigned int getVid_CCW_Distance(VlrRingVID src, VlrRingVID dst);
    // NOTE provide inline keyword if the definition of the function is in a header file that is included across different files.
    template<class Iterator>
    static inline void advanceIteratorWrapAround(Iterator& itr, int times, const Iterator& begin, const Iterator& end) {
      while (times > 0) {
          if (itr == end)
              itr = begin;
          itr++;
          times--;
      }
      while (times < 0) {
          if (itr == begin)
              itr = end;
          itr--;
          times++;
      }
      if (itr == end)
          itr = begin;
    }

    static void advanceVectorIndexWrapAround(unsigned int& index, int times, const int& vecSize);

    static std::vector<unsigned int> removeLoopInTrace(const std::vector<unsigned int>& trace);

  protected:
    virtual void initialize() override;
    // virtual void handleMessage(cMessage *msg);

    void sendCreatedPacket(cPacket *packet, bool unicast, int outGateIndex, double delay=0, bool checkFail=true, const std::vector<int> *exclOutGateIndexes=nullptr);

    // handling failure simulation timer timeout
    void processFailureSimulationTimer();
    // handling timer timeout to set vidRingRegVset based on vidRingRegistry
    void processVidRingRegVsetTimer(int numHalf);
    // handling timer timeout to process failed packet
    void processFailedPacketDelayTimer(FailedPacketDelayTimer *failedpktTimer);
    // handling write vlrRoutingTable to file timer timeout
    void processWriteRoutingTableTimer();
    // handling write nodeStats timer timeout
    void processWriteNodeStatsTimer();

    virtual void handleStartOperation();
    virtual void handleStopOperation() = 0;
    virtual void handleFailureLinkSimulation(const std::set<unsigned int>& failedPneis, int failedGateIndex=-1) = 0;
    virtual void handleFailureLinkRestart(const std::set<unsigned int>& restartPneis) = 0;
    virtual void handleFailureNodeRestart() = 0;
    virtual void processFailedPacket(cPacket *packet, unsigned int pneiVid) = 0;
    virtual void writeRoutingTableToFile() = 0;
    virtual void recordCurrentNodeStats(const char *stage) = 0;

    // const functions
    /** return numHalf nodes close to me in ccw/cw direction in traceSet */
    std::vector<VlrRingVID> getCloseNodesInSet(int numHalf, const std::set<VlrRingVID>& traceSet) const;

    template<class VidMap>
    inline std::vector<VlrRingVID> getCloseNodesInMap(int numHalf, const VidMap& vidMap) const
    {
        std::vector<VlrRingVID> closeVec;
        if (!vidMap.empty()) {
            auto itrup = vidMap.lower_bound(vid);    // itrup points to closest cw node in vidMap
            advanceIteratorWrapAround(itrup, 0, vidMap.begin(), vidMap.end());
            auto itrlow = itrup;
            if (itrup->first == vid)
                advanceIteratorWrapAround(itrup, 1, vidMap.begin(), vidMap.end());

            for (int i = 0; i < numHalf; ++i) {
                if (itrup == itrlow && i > 0 || itrup->first == vid)   // when i == 0, itrup == itrlow but itrup hasn't been examined yet; if *itrup == vid, I am the only node in vidMap 
                    break;
                else {
                    closeVec.push_back(itrup->first);
                    advanceIteratorWrapAround(itrlow, -1, vidMap.begin(), vidMap.end());
                    if (itrlow == itrup)
                        break;
                    else {
                        closeVec.push_back(itrlow->first);
                        advanceIteratorWrapAround(itrup, 1, vidMap.begin(), vidMap.end());
                    }
                }
            }
        }
        return closeVec;
    }

    VlrPathID genPathID(VlrRingVID dstVid) const;

    void initializeFailureSimulationMap();
    void initializeSelfTestDstList();
    void initializeWriteRoutingTableToFileTimes();
    void initializeWriteNodeStatsTimes();
    VlrRingVID getRandomVidInRegistry();

    // statistics helper
    void recordMessageRecord(char action, const VlrRingVID& src, const VlrRingVID& dst, const char *msgType, unsigned int msgId, unsigned int hopcount, unsigned int chunkByteLength, const char *infoStr="");
    void recordNodeStatsRecord(const char *infoStr);
    void writeToResultNodeFile();
    void writeToResultMessageFile();

  public:
    // must overload operator<< for variables being WATCH()
    friend std::ostream &operator<<(std::ostream &o, const Representative &rep);
};

// overload operator<< to print vector<unsigned int> for EV_INFO purpose
std::ostream &operator<<(std::ostream &o, const std::vector<unsigned int> &vids);
// overload operator<< to print vector<unsigned int> for EV_INFO purpose
std::ostream &operator<<(std::ostream &o, const std::set<unsigned int> &vids);

}; // namespace omnetvlr

#endif

