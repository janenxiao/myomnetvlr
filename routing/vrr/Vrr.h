//
// This file is part of an OMNeT++/OMNEST simulation example.
//
// Copyright (C) 1992-2015 Andras Varga
//
// This file is distributed WITHOUT ANY WARRANTY. See the file
// `license' for details on this and other legal matters.
//

#ifndef __OMNETVLR_ROUTING_VRR_H
#define __OMNETVLR_ROUTING_VRR_H

#include <map>
#include <string>
#include <omnetpp.h>

#include "../RoutingBase.h"
// #include "Vlr_m.h"   // included in RoutingBase.h
#include "../vlr/PsetTable.h"
#include "../vlr/VlrRingRoutingTable.h"

using namespace omnetpp;

namespace omnetvlr {

class Vrr : public RoutingBase
{
  public:
    struct LostPneiInfo {
      std::set<VlrPathID> brokenVroutes;
      WaitRepairLinkIntTimer *timer;
      // std::set<VlrPathID> tempVlinks;   // either only one temporary route is in this set, or none
      // unsigned int tempVlinkHopcount;   // hopcount of the recently built temporary route in tempVlinks, only valid when recently processed repairLinkReq from lost pnei and sent out repairLinkReply
    };

  protected:
    // VRR parameters
    double beaconInterval;
    double maxJitter;
    double neighborValidityInterval;
    double repSeqValidityInterval;   // max time that node can hold an unchanged rep sequence number, after that the rep is considered expired
    double inNetworkEmptyVsetWarmupTime;      // wait time before changing inNetwork to true (with empty vset) if a node cannot find a inNetwork neighbor to use as a proxy to join the network

    static const int routeSetupReqWaitTime = 20;    // timeout for reply to a setupReq sent
    static const int fillVsetInterval = 10;         // time interval to check on pendingVset, purge nonEssRoutes, and do other periodic chores
    static const int representativeMapMaxSize = 6;         // max number of reps (excluding myself) in representativeMap, NOTE can include expired records

    int setupReqRetryLimit;   // only used for lost vnei
    static const int routePrevhopVidsSize = 2;     // max number of prevhopVids recorded for a route in routing table, including prevhop itself, related to repairLinkReqFloodTTL depending on physical topo
    static const bool checkUniPacketHopcountLimit = false;
    static const int uniPacketHopcountLimit = 10000;     // max number of hops a unicast packet can traverse, if msgHopcount (including myself) >= uniPacketHopcountLimit, I won't forward the packet

    bool sendRepairLocalNoTemp;           // whether to send out repairLocalReply instead of tearing down broken vroutes directly
    double repairLinkReqWaitTime;         // wait time before tearing down broken vroutes
    static const int repairLinkReqRetryLimit = 1;
    bool removeBrokenVrouteEndpointInRepair;

    // statistics measurement
    bool sendTestPacket;
    static const int testSendInterval = 10;         // time interval to send a test message for statistics measurement
    static const bool recordReceivedMsg = false;          // record received message (may not be directed to me) with recordMessageRecord(/*action=*/2
    static const bool recordDroppedMsg = true;          // record message arrived and destined for me, or dropped at me with recordMessageRecord(/*action=*/1 or 4
    unsigned int numTestPacketReceived = 0;       // number of test messages received (handled or forwarded)

    // time to start sending TestPacket
    static const double sendTestPacketOverlayWaitTime;  // wait time after last node joined overlay before changing sendTestPacketStart to true
    static double lastTimeNodeJoinedInNetwork;  // last time that a node joined overlay
    static bool sendTestPacketStart;    // if we've started sending TestPacket (start sending once all nodes have joined the overlay)

    // context
    cModule *host = nullptr;

    // node internal self-messages
    cMessage *beaconTimer = nullptr;            // timer to send beacon
    cMessage *purgeNeighborsTimer = nullptr;    // timer to purge the neighbour table of expired neighbours
    cMessage *fillVsetTimer = nullptr;          // timer to fill up / check on vset, send setupReq if needed
    cMessage *inNetworkWarmupTimer = nullptr;   // timer to change selfInNetwork to true
    // cMessage *repSeqExpirationTimer = nullptr;   // timer to purge an expired rep sequence number
    cMessage *nonEssRouteTeardownTimer = nullptr;   // timer to clear nonEssRoutes
    
    cMessage *testPacketTimer = nullptr;          // timer to send testPacket for statistics measurement

    // node internal state
    bool selfInNetwork = false;

    PsetTable<VlrRingVID> psetTable;
    // Representative representative; // later in initialization will set representative.vid (either fixed or myself) as needed
    unsigned int selfRepSeqnum;       // rep seqNo when I'm rep
    std::map<VlrRingVID, Representative> representativeMap;   // doesn't include myself
    unsigned int totalNumBeaconSent = 0;

    int vsetHalfCardinality;

    VlrRingRoutingTable vlrRoutingTable;
    std::map<VlrRingVID, std::set<VlrPathID>> vset;

    std::map<VlrRingVID, WaitSetupReqIntTimer *> pendingVset;

    std::set<VlrPathID> nonEssRoutes;  // map nonEss vroute to its expiration time (after which it can be torn down), expireTime=1: I suspect this vroute is broken and will tear down right away, otherwise I may keep it if nonEss vroute is to a pendingVnei

    std::map<VlrRingVID, LostPneiInfo> lostPneis;   // nodes to which I've sent repairLinkReq, they were once my pneis but are no longer connected, which broke some vset routes

  public:
    virtual ~Vrr();

  protected:
    virtual void initialize() override;
    virtual void handleMessage(cMessage *message) override;
    virtual void finish() override;

    virtual void handleStartOperation() override;
    virtual void handleStopOperation() override;
    virtual void handleFailureLinkSimulation(const std::set<unsigned int>& failedPneis, int failedGateIndex=-1) override;
    virtual void handleFailureLinkRestart(const std::set<unsigned int>& restartPneis) override;
    virtual void handleFailureNodeRestart() override;
    virtual void processFailedPacket(cPacket *packet, unsigned int pneiVid) override;
    virtual void writeRoutingTableToFile() override;
    virtual void recordCurrentNodeStats(const char *stage) override;

    // handling messages
    void processSelfMessage(cMessage *message);
    void processMessage(cMessage *message);

    NotifyLinkFailureInt* createNotifyLinkFailure(bool simLinkUp);
    void sendNotifyLinkFailure(const int& outGateIndex, bool simLinkUp);
    void processNotifyLinkFailure(NotifyLinkFailureInt *msgIncoming, bool& pktForwarded);

    // handling purge neighbors timers
    void schedulePurgeNeighborsTimer();
    void processPurgeNeighborsTimer();
    virtual void purgeNeighbors();
    /** get the expiration time of the oldest neighbour */ 
    virtual simtime_t getNextNeighborExpiration() const;
    void handleLostPneis(const std::vector<VlrRingVID>& pneiVids);

    // handling fill vset timers
    void scheduleFillVsetTimer(bool firstTimer=false, double maxDelay=0);
    virtual void processFillVsetTimer();

    //  // handling rep seqNo expiration timeout
    // virtual void processRepSeqExpirationTimer();
    // handling inNetwork wait time
    virtual void processInNetworkWarmupTimer();
    // handling nonEssRoute reardown timeout
    virtual void processNonEssRouteTeardownTimer();

    // handling beacon timers
    void scheduleBeaconTimer(bool firstTimer=false);
    void processBeaconTimer();
    // handling beacons
    virtual void sendBeacon();
    VlrIntBeacon* createBeacon();
    virtual void processBeacon(VlrIntBeacon *beacon, bool& pktForwarded);
    // void handleNewPnei(const VlrRingVID& pneiVid);
    void handleLinkedPnei(const VlrRingVID& pneiVid);
    void updateRepState(VlrRingVID pnei, const VlrIntRepState& pneiRepState, bool pneiIsLinked);

    // handling SetupReq
    /** SetupReq timeout, resend setupReq if needed */
    virtual void processWaitSetupReqTimer(WaitSetupReqIntTimer *setupReqTimer);
    int computeSetupReqByteLength(SetupReqInt* setupReq) const;
    SetupReqInt* createSetupReq(const VlrRingVID& dst, const VlrRingVID& proxy);
    void sendCreatedSetupReq(SetupReqInt *setupReq, const int& outGateIndex, bool computeChunkLength=true, double delay=0);
    void sendSetupReq(const VlrRingVID& dst, const VlrRingVID& proxy, WaitSetupReqIntTimer *setupReqTimer);
    void processSetupReq(SetupReqInt* reqIncoming, bool& pktForwarded);

    // handling SetupReply
    int computeSetupReplyByteLength(SetupReplyInt* msg) const;
    SetupReplyInt* createSetupReply(const VlrRingVID& newnode, const VlrRingVID& proxy, const VlrPathID& pathid, const std::set<VlrRingVID> *srcVsetPtr=nullptr);
    void sendCreatedSetupReply(SetupReplyInt *setupReply, const int& outGateIndex, bool computeChunkLength=true, double delay=0);
    void processSetupReply(SetupReplyInt *replyIncoming, bool& pktForwarded);
    void setRoutePrevhopVids(VlrIntVidVec& routePrevhops, const VlrIntVidVec& msgPrevhops, const unsigned int& oldestPrevhopIndex);
    unsigned int setRoutePrevhopVidsFromMessage(VlrIntVidVec& routePrevhops, VlrIntVidVec& msgPrevhops, const unsigned int& oldestPrevhopIndex);

    // handling SetupFail
    int computeSetupFailByteLength(SetupFailInt* msg) const;
    SetupFailInt* createSetupFail(const VlrRingVID& newnode, const VlrRingVID& proxy);
    void sendCreatedSetupFail(SetupFailInt *msg, const int& outGateIndex, bool computeChunkLength=true, double delay=0);
    void processSetupFail(SetupFailInt *msgIncoming, bool& pktForwarded);

    // handling Teardown
    /** remove newPathid from vlrRoutingTable, send Teardown to its prevhopVid, nexthopVid and msgPrevHopVid */
    void vrrTeardownPath(VlrPathID newPathid, VlrRingVID msgPrevHopVid, bool addSrcVset, bool checkMeEndpoint);
    TeardownInt* createTeardown(const VlrPathID& pathid, bool addSrcVset);
    void sendCreatedTeardown(TeardownInt *msg, VlrRingVID nextHopPnei, double delay=0);
    void processTeardown(TeardownInt* msgIncoming, bool& pktForwarded);
    void processTeardownAtEndpoint(const VlrPathID& oldPathid, const VlrRingVID& otherEnd, const TeardownInt* msgIncoming);
    bool removeEndpointOnTeardown(const VlrPathID& pathid, const VlrRingVID& towardVid, bool pathIsVsetRoute, bool addToPending);

    // handling TestPacket
    void scheduleTestPacketTimer(bool firstTimer=false);
    virtual void processTestPacketTimer();
    VlrIntTestPacket* createTestPacket(const VlrRingVID& dstVid) const;
    void sendCreatedTestPacket(VlrIntTestPacket *msg, const int& outGateIndex, double delay=0);
    void processTestPacket(VlrIntTestPacket *msgIncoming, bool& pktForwarded);

    // handling RepairLocalReply
    /** handling repairLink timeout, send RepairLinkReq or teardown brokenVroutes */
    virtual void processWaitRepairLinkTimer(WaitRepairLinkIntTimer *message);
    int computeRepairLocalReplyByteLength(RepairLocalReplyInt* msg) const;
    RepairLocalReplyInt* createRepairLocalReply();
    void sendCreatedRepairLocalReply(RepairLocalReplyInt *msg, const int& outGateIndex, bool computeChunkLength=true, double delay=0);
    void processRepairLocalReply(RepairLocalReplyInt *replyIncoming, bool& pktForwarded);

    // routing
    VlrRingVID findNextHop(VlrIntOption& vlrOption, VlrRingVID excludeVid=VLRRINGVID_NULL);
    VlrRingVID findNextHopForSetupReply(VlrIntOption& vlrOption, VlrRingVID newnode);
    unsigned int getNextHopIndexInTrace(VlrIntVidVec& trace, bool preferShort=true) const;
    std::pair<VlrRingVID, unsigned int> getClosestPneiTo(VlrRingVID targetVid, VlrRingVID excludeVid, bool checkInNetwork=false) const;
    std::tuple<VlrRingVID, unsigned int, VlrPathID> getClosestVendTo(VlrRingVID targetVid, VlrRingVID excludeVid) const;
    std::tuple<VlrRingVID, unsigned int, VlrRingVID> getClosestRepresentativeTo(VlrRingVID targetVid, VlrRingVID excludeVid) const;

    void initializeVlrOption(VlrIntOption& vlrOption, const VlrRingVID& dstVid=VLRRINGVID_NULL) const;
    int getVlrUniPacketByteLength() const;
    /** set setupPacke->srcVset array based on vset and pendingVset, return number of nodes added to srcVset */
    unsigned int setupPacketSrcVset(VlrIntSetupPacket *setupPacket, const std::set<VlrRingVID> *srcVsetPtr=nullptr) const;
    VlrRingVID getMessagePrevhopVid(cMessage *message) const;

    // node status helper
    void removeRouteFromLostPneiBrokenVroutes(const VlrPathID& pathid, const VlrRingVID& lostPneiVid);
    VlrRingVID getProxyForSetupReq(bool checkInNetwork=true) const;
    bool pendingVsetAdd(VlrRingVID node, int numTrials);
    void pendingVsetErase(VlrRingVID node);
    void vsetInsertRmPending(VlrRingVID node);
    bool vsetEraseAddPending(VlrRingVID node, bool addToPending);
    /** assuming vset full (size==vsetHalfCardinality*2), get the farthest ccw vnei and cw vnei (wrap-around search in vset) to me */
    std::pair<VlrRingVID, VlrRingVID> getFarthestVneisInFullVset() const;
    /** return shouldAdd, removedNeis */
    std::tuple<bool, std::vector<VlrRingVID>> shouldAddVnei(const VlrRingVID& newnode) const;
    /** return if srcVid should be added to vset, send setupReq to nodes in setupPacket.srcVset that should be added to vset */
    bool vrrAdd(const VlrIntSetupPacket *setupPacket, const VlrRingVID& srcVid);
    /** set selfInNetwork=true and recordNodeStatsRecord() after adding a new vnei */
    void recordNewVnei(const VlrRingVID& newVnei);

    /** return arrival time of a scheduled setupReq timer in pendingVset */
    simtime_t_cref getASetupReqPendingArrivalTime() const;

    void cancelPendingVsetTimers();
    void cancelRepairLinkTimers();
    void clearState();
    std::set<VlrRingVID> convertVsetToSet() const;
    /** print vset for EV_INFO purpose */
    std::string printVsetToString() const;
    std::string printRoutesToMeToString() const;
};

}; // namespace omnetvlr

#endif
