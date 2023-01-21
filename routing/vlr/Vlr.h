//
// This file is part of an OMNeT++/OMNEST simulation example.
//
// Copyright (C) 1992-2015 Andras Varga
//
// This file is distributed WITHOUT ANY WARRANTY. See the file
// `license' for details on this and other legal matters.
//

#ifndef __OMNETVLR_ROUTING_VLR_H
#define __OMNETVLR_ROUTING_VLR_H

#include <map>
#include <string>
#include <omnetpp.h>

#include "../RoutingBase.h"
// #include "Vlr_m.h"   // included in RoutingBase.h
#include "PsetTable.h"
#include "VlrRingRoutingTable.h"

using namespace omnetpp;

namespace omnetvlr {

class Vlr : public RoutingBase
{
  public:
    struct VsetVneiInfo {
      std::set<VlrPathID> vsetRoutes;
      // WaitSetupReqIntTimer *setupReqTimer;
    };

    struct PendingVneiInfo {
      WaitSetupReqIntTimer *setupReqTimer;
      simtime_t lastHeard;
      VlrRingVID heardFrom;   // omit if VLRRINGVID_NULL
    };

    struct LostPneiInfo {
      std::set<VlrPathID> brokenVroutes;
      WaitRepairLinkIntTimer *timer;
      std::set<VlrPathID> tempVlinks;   // either only one temporary route is in this set, or none
      // unsigned int tempVlinkHopcount;   // hopcount of the recently built temporary route in tempVlinks, only valid when recently processed repairLinkReq from lost pnei and sent out repairLinkReply
    };

    struct RepairLinkReqInfo {
      std::set<VlrPathID> brokenPathids;
      VlrIntVidVec linkTrace;
      simtime_t expireTime;
    };

    struct RepairLocalReqInfo {
      std::vector<VlrPathID> brokenPathids;
      VlrIntVidVec linkTrace;
      simtime_t expireTime;
    };

  protected:
    // VlrBase
    // VLR parameters
    double beaconInterval;
    double maxJitter;
    double neighborValidityInterval;
    // double pneiDiscoveryTime;        // minimum warmup time to discover pneis and fill up pset with beacons before sending the first setupReq
    double repSeqValidityInterval;   // max time that node can hold an unchanged rep sequence number, after that the rep is considered expired
    
    int setupReqRetryLimit;
    int repairLinkReqFloodTTL = 8;     // max number of hops a repairLinkReqFlood will propagate, i.e. the number of nodes that will send/forward a repairLinkReqFlood
    int routePrevhopVidsSize = 4;     // max number of prevhopVids recorded for a route in routing table, including prevhop itself, related to repairLinkReqFloodTTL depending on physical topo

    // static VLR parameters
    static const int setupReqTraceRetryLimit = 1;   // for static, only static constant integral types can be initialized inside the class
    static const int repairLinkReqRetryLimit = 1;

    // static VLR expiration times (so no need to read from NED or INI files) -- definition in VlrBase.cc file // NOTE The normal simtime_t type (i.e. SimTime) cannot be used for static variables, because the scale exponent is only available after the configuration has been read by the simulation program.
    static const double routeSetupReqWaitTime;    // timeout for reply to a setupReq sent
    static const double routeSetupReqTraceWaitTime; // timeout for reply to a setupReqTrace sent
    double repairLinkReqWaitTime;         // timeout for reply to a repairLinkReq sent
    static double repairLinkReqSmallEndSendDelay;         // (0 for repairLocalReq) after link breakage detected, smaller link end will send repairLinkReq after this delay (larger link end will send immediately) to avoid two ends sending repairLinkReq and receiving sending repairLinkReply at the same time
    // static const double repairLinkReqfailureSimulateLinkMaxDelay;   // max delay after link breakage detected before sending repairLinkReq if failureSimulateLink=true, bc all link failures will be detected at the same time at both ends, add random delay to avoid many nodes flooding at the same time
    static const double nonEssRouteExpiration;    // wait time after which teardown of a non-essential vroute can be executed, since it's added to nonEssRoutes 
    static const double patchedRouteExpiration;    // wait time after which teardown of a patched vroute (repairRoute has been received) can be executed, since it's added to nonEssRoutes 
    static const double patchedRouteRepairSetupReqMaxDelay;    // max delay before sending a setupReq to remove loop in a patched vroute (repairRoute has been received), to avoid jamming the failure area with too many repair messages at the same time  
    static const double dismantledRouteExpiration;    // wait time after which teardown of a dismantled vroute (repairRoute has been received) can be executed, since it's added to dismantledRoutes 
    static const double overHeardTraceExpiration;    // wait time after which an overheard trace can be removed from overheardTraces
    static const double recentSetupReqExpiration;    // wait time after which a newly added vnei (bc received setupReq) is removed from recentSetupReqFrom, i.e. setupReq from it to me is no longer ignored
    static const double recentReqFloodExpiration;    // wait time after which a node from which I received a flood request is removed from recentReqFloodFrom to same memory
    static const double delayedRepairLinkReqExpireTimeCoefficient;    // expireTime of a delayed repairLinkReq = receivingTime + this coefficient * beaconInterval
    static const double inNetworkWarmupTime;      // wait time before changing inNetwork to true after receiving the first setupReply, once timeout I expected to have received setupReply from all my vneis (successfully joined overlay)
    double inNetworkEmptyVsetWarmupTime;        // used when !representativeFixed && !startingRootFixed, wait time before changing inNetwork to true (with empty vset) if a node cannot find a inNetwork neighbor to use as a proxy to join the network
    static const double fillVsetInterval;         // time interval to check on pendingVset, purge nonEssRoutes, and do other periodic chores
    static const double notifyVsetSendInterval;         // time interval to send NotifyVset to a vnei in vset
    static const double setupSecondVsetRoutesInterval;         // time interval to check on vneis in vset (if possible to setup secondary vset routes)
    static const double setupNonEssRoutesInterval;         // time interval to check on pendingVneis that doesn't belong to vset (if possible to setup nonEss routes)
    static double recentUnavailableRouteEndExpiration;         // wait time after which a routeEnd set unavailable bc broken vroute will be removed from recentUnavailableRouteEnd if broken vroute has been repaired or removed from vlrRoutingTable
    
    bool setupTempRoute;   // in handleLostPneis(), whether to send repairLinkReq to build temporary route to lost pnei, or send Teardown for vroutes broken by lost pnei
    bool keepDismantledRoute;   // whether to keep dismantled route (Teardown has been sent/received) in vlrRoutingTable and dismantledRoutes
    bool checkOverHeardTraces;   // whether to check recorded trace in forwarded messages for nodes close to me and record path to them
    static const int overHeardTraceMinCheckLen = 3;   // the minimum length of recorded trace in forwarded messages for me to check for nodes close to me and record path to them
    // static const bool checkOverHeardMPneis = false;   // whether to check recorded trace in repairLinkReqFlood to record nodes close to me in physical neighbourhood and record nexthop to them
    static const bool setupNonEssRoutes = true;   // whether to build nonEss vroutes to pendingVnei
    static const bool recordTraceForDirectedSetupReq = false;    // whether to record trace when sending setupReq to specified dst (upon WaitSetupReqIntTimer) if not using overheard trace
    // static const bool setupSecondVsetRoutes = false;   // whether to build secondary vset routes to current vneis
    // static const int vneiVsetRoutesSize = 2;      // max number of vset routes in vset[vnei].vsetRoutes to setup for a vnei, if setupSecondVsetRoutes=false, only setup 1 vset route
    static const double maxFloodJitter;         // max delay before forwarding a flood request after receiving it
    bool sendPeriodicNotifyVset;
    bool sendRepairLocalNoTemp;

    // statistics measurement
    bool sendTestPacket;
    static const double testSendInterval;         // time interval to send a test message for statistics measurement
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
    cMessage *repSeqExpirationTimer = nullptr;   // timer to purge an expired rep sequence number  NOTE only used when representativeFixed=true and I'm not the predefined rep
    cMessage *repSeqObserveTimer = nullptr;     // timer to start looking for overlay after rep-timeout, when scheduled, I just left overlay bc rep-timeout, don't try to join a new overlay right away, wait for beaconInterval so my pneis also rep-timeout
    cMessage *delayedRepairReqTimer = nullptr;     // timer to check delayedRepairLinkReq or delayedRepairLocalReq after new link breakage detected
    
    cMessage *testPacketTimer = nullptr;          // timer to send testPacket for statistics measurement

    // node internal state
    bool selfInNetwork = false;
    
    // VlrRing
    PsetTable<VlrRingVID> psetTable;
    Representative representative; // used when representativeFixed=true (representative.vid used to store root when startingRootFixed=true), later in initialization will set representative.vid to predefined repVid
    bool representativeFixed;
    bool startingRootFixed;
    // unsigned int lastBeaconRepSeqnum;     // repState.sequencenumber sent in the last beacon, used to check if rep seqNo is incremented in each beacon
    // unsigned int lastBeaconRepVid;     // repState.vid sent in the last beacon, used to check if rep seqNo is incremented in each beacon
    // bool lastBeaconRepSeqnumUnchanged = false;    // repState.sequencenumber sent in the last two beacons are the same, need to notify pneis asap if new rep seqNo received
    unsigned int selfRepSeqnum;       // rep seqNo when I'm rep
    std::map<VlrRingVID, Representative> representativeMap;   // used when representativeFixed=false, doesn't include myself
    static const int representativeMapMaxSize = 6;         // max number of reps (excluding myself) in representativeMap, NOTE can include expired records

    unsigned int totalNumBeaconSent = 0;

    int vsetHalfCardinality;
    int vsetAndBackupHalfCardinality; // vsetHalfCardinality + backupVsetHalfCardinality
    int pendingVsetHalfCardinality;    // at least vsetHalfCardinality + backupVsetHalfCardinality, preferably at least 2*vsetHalfCardinality
    int vsetAndPendingHalfCardinality; // vsetHalfCardinality + pendingVsetHalfCardinality

    VlrRingRoutingTable vlrRoutingTable;
    std::map<VlrRingVID, VsetVneiInfo> vset;

    simtime_t nextSetupNonEssRoutesTime;     // will check pendingVneis that doesn't belong to vset (whether to setup nonEss route) if currTime > nextSendTime

    VlrRingVID lastNotifiedVnei = VLRRINGVID_NULL;  // for sending notifyVset to every node in vset in order
    simtime_t nextNotifyVsetSendTime;         // will send NotifyVset if currTime > nextSendTime
    
    std::map<VlrRingVID, PendingVneiInfo> pendingVset;
    std::pair<VlrRingVID, VlrRingVID> pendingVsetFarthest = {VLRRINGVID_NULL, VLRRINGVID_NULL};   // farthest ccw and cw nodes in pendingVset; if farthestCCW = VLRRINGVID_NULL, values invalid

    bool nextNotifyVsetToPendingVnei = false;   // if true, next notifyVset will be sent to a node in pendingVset instead of vset
    VlrRingVID lastNotifiedPendingVnei = VLRRINGVID_NULL;  // for sending notifyVset to every node in pendingVset in order
    
    static double pendingVneiValidityInterval;            // validity interval of node in pendingVset, lastHeard + validityInterval = expireTime, expected wait time before which I should heard about the pendingVnei again, possibly via NotifyVset (should be set based on VlrBase::notifyVsetSendInterval)
    bool pendingVsetChangedSinceLastCheckAndSchedule = false;   // set to false every time I schedule setupReq timer to potential vneis in pendingVset in processFillVsetTimer(), set to true every time I add new node to pendingVset; this is needed to ensure I have scheduled setupReq timer for every potential vnei before calling hasSetupReqPending() to determine if I have potential vnei

    std::map<VlrPathID, simtime_t> nonEssRoutes;  // map nonEss vroute to its expiration time (after which it can be torn down), expireTime=1: I suspect this vroute is broken and will tear down right away, otherwise I may keep it if nonEss vroute is to a pendingVnei
    std::set<VlrPathID> nonEssUnwantedRoutes;     // pathids (of patched vroutes or temporary routes) in nonEssRoutes that should be torn down after expiry even though it may lead to a vnei or pendingVnei
  
    std::map<VlrRingVID, simtime_t> recentSetupReqFrom; // not needed now bc multiple vset-routes btw vneis allowed -- nodes I've recently received setupReq from and accepted as vnei, map it to its expiration time (after which it'll be removed from this map)

    std::map<VlrRingVID, LostPneiInfo> lostPneis;   // nodes to which I've sent repairLinkReq, they were once my pneis but are no longer connected, which broke some vset routes
    // std::map<VlrPathID, std::pair<VlrRingVID, VlrRingVID>> brokenRoutes;   // map broken vroute <pathid> to <lostPnei> (lost prevhop (I'll send repairLinkReq), lost nexthop (I'll receive repairLinkReq)) where lostPneis[lostPnei].brokenVroutes contains pathid
    std::map<VlrRingVID, RepairLinkReqInfo> delayedRepairLinkReq; // delay received repairLinkReq until I detect lost of pnei (I don't setup temporary route to repair vroutes unless I have vroutes broken by a lost pnei)
    
    unsigned int floodSeqnum = 0;
    std::map<VlrRingVID, std::pair<unsigned int, simtime_t>> recentReqFloodFrom;  // nodes I've recently received flood request from, map it to its most recent floodSeqnum

    // after receiving RepairLinkReq from srcVid, I may change nexthopVid of some broken vroutes srcBrokenPathids to srcVid, but srcVid doesn't know this until it receives my RepairLinkReply which builds tempPathid, if RepairLinkReply can't be forwarded and tempPathid is torn down, and if I have another temporary route to srcVid I won't consider srcBrokenPathids as broken, but they're still broken at srcVid bc it didn't receive my RepairLinkReply, inconsistent
    // thus I record tempPathid just sent and its srcBrokenPathids (brokenVroutes whose nexthopVid changed with new temporary route), if I receive Teardown soon after sending RepairLinkReply, meaning otherEnd hasn't received RepairLinkReply or updated prevhopVid to me for srcBrokenPathids, I'll try to ensure nexthopVid of srcBrokenPathids is unavailable so that they'll be torn down if can't be repaired 
    std::map<VlrPathID, std::pair<std::vector<std::pair<VlrPathID, VlrRingVID>>, simtime_t>> recentTempRouteSent;  // tempPathid of RepairLinkReply that I recently sent out, map it to ([(srcBrokenPathid, old nexthopVid) ..], expireTime)

    // local repair
    std::map<VlrRingVID, std::pair<std::set<VlrPathID>, simtime_t>> recentRepairLocalReqFrom;   // nodes I've recently received repairLocalReq from, map it to pathids in RepairLocalReqFlood.brokenPathids
    // std::map<VlrPathID, simtime_t>  recentRepairLocalBrokenVroutes;    // pathids for which I've recently sent repairLocalReply    Commented out bc pathids that I've recently repaired won't be broken (nexthop unavailable) thus won't be repaired again
    std::map<unsigned int, RepairLocalReqInfo> delayedRepairLocalReq;     // delay received repairLocalReq until I detect lost of pnei, map key is hash of RepairLocalReqInfo

    std::map<VlrPathID, simtime_t> dismantledRoutes;  // map one-direction vroute (has received Teardown) to its expiration time

    std::map<VlrRingVID, std::pair<std::vector<VlrRingVID>, simtime_t>> overheardTraces;   // traces I learnt from received messages with trace, {dst, (trace to dst: [me, .., dst], expireTime)}
    // std::map<VlrRingVID, std::pair<VlrRingVID, simtime_t>> overheardMPneis;   // multihop pneis (in my physical neighbourhood) I learnt from repairLinkReqFlood, {dst, (nexthop to dst, expireTime)}
    
    std::map<VlrRingVID, std::pair<VlrPathID, simtime_t>> recentUnavailableRouteEnd;   // vroute endpoints that have recently been set unavailable bc lost pnei in handleLostPneis(), {routeEnd, (vroute to routeEnd that's being repaired, expireTime)}, used in setRouteToPendingVsetItr() so I'll not send setupReq to unavailable endpoints to leave time for repair


  public:
    virtual ~Vlr();

  protected:
    virtual void initialize() override;
    virtual void handleMessage(cMessage *msg) override;
    virtual void finish() override;

    virtual void handleStartOperation() override;
    virtual void handleStopOperation() override;
    virtual void handleFailureLinkSimulation(const std::set<unsigned int>& failedPneis) override;
    virtual void handleFailureLinkRestart(const std::set<unsigned int>& restartPneis) override;
    virtual void handleFailureNodeRestart() override;
    virtual void processFailedPacket(cPacket *packet, unsigned int pneiVid) override;
    virtual void writeRoutingTableToFile() override;

    // handling messages
    void processSelfMessage(cMessage *message);
    void processMessage(cMessage *message);
    // virtual void processVlrPacket(Packet *packet);
    // virtual void processFailedPacket(Packet *packet);

    // handling beacon timers
    void scheduleBeaconTimer(bool firstTimer=false, double maxDelay=0);
    void processBeaconTimer();  

    // handling beacons
    virtual void sendBeacon();
    VlrIntBeacon* createBeacon();
    virtual void processBeacon(VlrIntBeacon *beacon, bool& pktForwarded);
    // void handleNewPnei(const VlrRingVID& pneiVid);
    void handleLinkedPnei(const VlrRingVID& pneiVid);
    void updateRepState(VlrRingVID pnei, const VlrIntRepState& pneiRepState, bool pneiIsLinked);

    // handling purge neighbors timers
    void schedulePurgeNeighborsTimer();
    void processPurgeNeighborsTimer();
    virtual void purgeNeighbors();
    /** get the expiration time of the oldest neighbour */ 
    virtual simtime_t getNextNeighborExpiration() const;
    void handleLostPneis(const std::vector<VlrRingVID>& pneiVids, const std::vector<VlrPathID> *rmVroutesPtr=nullptr);

    // handling delayed repairlinkReq or repairlocalReq timer timeout
    void processDelayedRepairReqTimer();

    NotifyLinkFailureInt* createNotifyLinkFailure(bool simLinkUp);
    void sendNotifyLinkFailure(const int& outGateIndex, bool simLinkUp);
    void processNotifyLinkFailure(NotifyLinkFailureInt *msgIncoming, bool& pktForwarded);

    // handling fill vset timers
    void scheduleFillVsetTimer(bool firstTimer=false, double maxDelay=0);
    virtual void processFillVsetTimer();
    void setRouteToPendingVsetItr(std::map<VlrRingVID, PendingVneiInfo>::iterator pendingItr, bool sendSetupReq, bool isCCW, int& closeCount, bool tryNonEssRoute, std::vector<VlrRingVID> *expiredPendingVneisPtr=nullptr);
    bool checkLostPneiUnavailable(VlrRingVID lostPnei) const;
    bool checkRecentUnavailableRouteEnd(VlrRingVID lostVend);

    // handling rep seqNo expiration timeout
    virtual void processRepSeqExpirationTimer();
    void leaveOverlay();
    // handling rep seqNo observe period timeout
    virtual void processRepSeqObserveTimer();
    // handling inNetwork wait time
    virtual void processInNetworkWarmupTimer();

    // handling SetupReq
    /** SetupReq timeout, resend setupReq if needed */
    virtual void processWaitSetupReqTimer(WaitSetupReqIntTimer *setupReqTimer);
    char findAlterPendingVneiOf(VlrRingVID referenceVid, std::map<VlrRingVID, PendingVneiInfo>::iterator& pendingItr);
    int computeSetupReqByteLength(SetupReqInt* setupReq) const;
    SetupReqInt* createSetupReq(const VlrRingVID& dst, const VlrRingVID& proxy, bool reqDispatch);
    void sendCreatedSetupReq(SetupReqInt *setupReq, const int& outGateIndex, bool computeChunkLength=true, double delay=0);
    void sendSetupReq(const VlrRingVID& dst, const VlrRingVID& proxy, bool reqDispatch, bool recordTrace, bool reqVnei, WaitSetupReqIntTimer *setupReqTimer);
    void processSetupReq(SetupReqInt* reqIncoming, bool& pktForwarded);
    void forwardSetupReq(SetupReqInt* reqOutgoing, bool& pktForwarded, bool withTrace, bool recordTrace);

    // handling SetupReply
    int computeSetupReplyByteLength(SetupReplyInt* msg) const;
    SetupReplyInt* createSetupReply(const VlrRingVID& newnode, const VlrRingVID& proxy, const VlrPathID& pathid);
    void sendCreatedSetupReply(SetupReplyInt *setupReply, const int& outGateIndex, bool computeChunkLength=true, double delay=0);
    void processSetupReply(SetupReplyInt *replyIncoming, bool& pktForwarded);
    void forwardSetupReply(SetupReplyInt* replyOutgoing, bool& pktForwarded, const VlrPathID& newPathid, VlrRingVID msgPrevHopVid, bool withTrace);
    void sendGreedySetupReplyAddVnei(const VlrRingVID& newnode, const VlrRingVID& proxy, const VlrIntOption& vlrOptionOut, const VlrRingVID& nextHopVid, const std::vector<VlrRingVID>& removedNeis);

    /** set routePrevhops based on msgPrevhops, add me in msgPrevhops and return updated oldestPrevhopIndex */
    void setRoutePrevhopVids(VlrIntVidVec& routePrevhops, const VlrIntVidVec& msgPrevhops, const unsigned int& oldestPrevhopIndex);
    unsigned int setRoutePrevhopVidsFromMessage(VlrIntVidVec& routePrevhops, VlrIntVidVec& msgPrevhops, const unsigned int& oldestPrevhopIndex);
    // void setMessagePrevhopVidsFromRoute(const VlrIntVidVec& routePrevhops, VlrIntVidVec& msgPrevhops);

    // handling SetupFail
    int computeSetupFailByteLength(SetupFailInt* msg) const;
    SetupFailInt* createSetupFail(const VlrRingVID& newnode, const VlrRingVID& proxy);
    void sendCreatedSetupFail(SetupFailInt *msg, const int& outGateIndex, bool computeChunkLength=true, double delay=0);
    void processSetupFail(SetupFailInt *msgIncoming, bool& pktForwarded);
    void forwardSetupFail(SetupFailInt *replyOutgoing, bool& pktForwarded, VlrRingVID msgPrevHopVid, bool withTrace);

    // handling AddRoute
    int computeAddRouteByteLength(AddRouteInt* msg) const;
    AddRouteInt* createAddRoute(const VlrRingVID& dst, const VlrPathID& pathid);
    void sendCreatedAddRoute(AddRouteInt *msg, const int& outGateIndex, bool computeChunkLength=true, double delay=0);
    void processAddRoute(AddRouteInt *msgIncoming, bool& pktForwarded);
    void forwardAddRoute(AddRouteInt *msgOutgoing, bool& pktForwarded, const VlrPathID& newPathid, VlrRingVID msgPrevHopVid, bool withTrace);

    // handling Teardown
    TeardownInt* createTeardownOnePathid(const VlrPathID& pathid, bool addSrcVset, bool rebuild);
    TeardownInt* createTeardown(const std::vector<VlrPathID>& pathids, bool addSrcVset, bool rebuild, bool dismantled);
    void setTeardownPathids(TeardownInt *teardown, const std::vector<VlrPathID>& pathids) const;
    void sendCreatedTeardownToNextHop(TeardownInt *msg, VlrRingVID nextHopVid, char nextHopIsUnavailable);
    void sendCreatedTeardown(TeardownInt *msg, VlrRingVID nextHopPnei, double delay=0);
    void processTeardown(TeardownInt* msgIncoming, bool& pktForwarded);
    void processTeardownAtEndpoint(const VlrPathID& oldPathid, const VlrRingVID& otherEnd, const TeardownInt* msgIncoming, bool rebuildTemp);
    void removeEndpointOnTeardown(const VlrPathID& pathid, const VlrRingVID& towardVid, bool pathIsVsetRoute, char pathIsTemporary, bool reqRepairRoute=false);

    // handling TestPacket
    // handling testPacket send timer
    void scheduleTestPacketTimer(bool firstTimer=false);
    virtual void processTestPacketTimer();
    VlrIntTestPacket* createTestPacket(const VlrRingVID& dstVid) const;
    void sendCreatedTestPacket(VlrIntTestPacket *msg, const int& outGateIndex, double delay=0);
    void processTestPacket(VlrIntTestPacket *msgIncoming, bool& pktForwarded);

    // handling RepairLinkReqFlood
    /** handling repairLink timeout, send RepairLinkReq or teardown brokenVroutes */
    virtual void processWaitRepairLinkTimer(WaitRepairLinkIntTimer *message);
    int computeRepairLinkReqFloodByteLength(RepairLinkReqFloodInt* repairReq) const;
    RepairLinkReqFloodInt* createRepairLinkReqFlood();
    void sendCreatedRepairLinkReqFlood(RepairLinkReqFloodInt *repairReq, bool computeChunkLength=true, double delay=0);
    void processRepairLinkReqFlood(RepairLinkReqFloodInt *reqIncoming, bool& pktForwarded);
    void processDelayedRepairLinkReq();
    void processRepairLinkReqInfo(const VlrRingVID& srcVid, const std::set<VlrPathID>& brokenPathids, const VlrIntVidVec& linkTrace, std::set<VlrPathID>& delayedBrokenPathids);
    void setLostPneiBrokenVroutes(std::map<VlrRingVID, LostPneiInfo>::iterator lostPneiItr, bool setBroken, std::vector<VlrPathID> *sendRepairRouteVroutesPtr=nullptr);

    // handling RepairLinkReply
    int computeRepairLinkReplyByteLength(RepairLinkReplyInt* msg) const;
    void sendCreatedRepairLinkReply(RepairLinkReplyInt *msg, const int& outGateIndex, bool computeChunkLength=true, double delay=0);
    void processRepairLinkReply(RepairLinkReplyInt *replyIncoming, bool& pktForwarded);
    void forwardRepairLinkReply(RepairLinkReplyInt* replyOutgoing, bool& pktForwarded, const VlrPathID& newPathid, VlrRingVID msgPrevHopVid);

    // handling RepairRoute
    VlrRingVID getVlrOptionToNextHop(VlrIntUniPacket* msg, VlrRingVID nextHopVid, char nextHopIsUnavailable) const;
    void setRepairRoutePathids(RepairRouteInt *msg, const std::vector<VlrPathID>& pathids) const;
    RepairRouteInt* createRepairRoute(const std::vector<VlrPathID>& pathids);
    void sendCreatedRepairRoute(RepairRouteInt *msg, VlrRingVID nextHopPnei, double delay=0);
    void processRepairRoute(RepairRouteInt *msgIncoming, bool& pktForwarded);

    // handling NotifyVset
    NotifyVsetInt* createNotifyVset(const VlrRingVID& dstVid, bool toVnei);
    void sendCreatedNotifyVset(NotifyVsetInt *msg, const int& outGateIndex);
    void processNotifyVset(NotifyVsetInt *msgIncoming, bool& pktForwarded);
    void forwardNotifyVset(NotifyVsetInt* msgIncoming, bool& pktForwarded);

    // handling RepairLocalReqFlood
    int computeRepairLocalReqFloodByteLength(RepairLocalReqFloodInt* repairReq) const;
    RepairLocalReqFloodInt* createRepairLocalReqFlood();
    void sendCreatedRepairLocalReqFlood(RepairLocalReqFloodInt *repairReq, bool computeChunkLength=true, double delay=0);
    void processRepairLocalReqFlood(RepairLocalReqFloodInt *reqIncoming, bool& pktForwarded);
    void processDelayedRepairLocalReq();
    void processRepairLocalReqInfo(const std::vector<VlrPathID>& brokenPathids, const VlrIntVidVec& linkTrace, std::vector<VlrPathID>& delayedBrokenPathids);

    // handling RepairLocalReply
    int computeRepairLocalReplyByteLength(RepairLocalReplyInt* msg) const;
    RepairLocalReplyInt* createRepairLocalReply();
    void sendCreatedRepairLocalReply(RepairLocalReplyInt *msg, const int& outGateIndex, bool computeChunkLength=true, double delay=0);
    void processRepairLocalReply(RepairLocalReplyInt *replyIncoming, bool& pktForwarded);
    void setRoutePrevhopVidsFromPrevhopMap(std::vector<VlrRingVID>& routePrevhopVids, std::vector<VlrRingVID>& routeEndAndPrev, unsigned int routeEndAndPrev_startindex, bool routeEndAndPrev_trim);

    // handling RepairLocalPrev
    int computeRepairLocalPrevByteLength(RepairLocalPrevInt* msg) const;
    RepairLocalPrevInt* createRepairLocalPrev();
    void sendCreatedRepairLocalPrev(RepairLocalPrevInt *msg, VlrRingVID nextHopPnei, bool computeChunkLength=true, double delay=0);
    void processRepairLocalPrev(RepairLocalPrevInt *msgIncoming, bool& pktForwarded);
    void setRoutePrevhopVidsInRepairLocalPrevToForward(std::vector<VlrRingVID>& routePrevToForward, const std::vector<VlrRingVID>& routePrevhopVids);
    unsigned int getRoutePrevhopVidsDiffIndex(const std::vector<VlrRingVID>& vec1, const std::vector<VlrRingVID>& vec2) const;

    // routing
    VlrRingVID findNextHop(VlrIntOption& vlrOption, VlrRingVID prevHopVid, VlrRingVID excludeVid=VLRRINGVID_NULL, bool allowTempRoute=false);
    VlrRingVID findNextHopForSetupReply(VlrIntOption& vlrOption, VlrRingVID prevHopVid, VlrRingVID newnode, bool allowTempRoute=false);
    VlrRingVID findNextHopForSetupReq(VlrIntOption& vlrOption, VlrRingVID prevHopVid, VlrRingVID dstVid, VlrRingVID newnode, bool allowTempRoute=true);
    unsigned int getNextHopIndexInTraceForSetupReq(const VlrIntVidVec& trace, unsigned int myIndexInTrace) const;
    unsigned int getNextHopIndexInTrace(VlrIntVidVec& trace, bool preferShort=true) const;
    std::pair<VlrRingVID, unsigned int> getClosestPneiTo(VlrRingVID targetVid, VlrRingVID excludeVid, bool checkInNetwork=false) const;
    std::tuple<VlrRingVID, unsigned int, VlrPathID> getClosestVendTo(VlrRingVID targetVid, VlrRingVID excludeVid, const VlrIntOption& vlrOption, bool allowTempRoute) const;
    std::tuple<VlrRingVID, unsigned int, VlrRingVID> getClosestRepresentativeTo(VlrRingVID targetVid, VlrRingVID excludeVid, const VlrIntOption& vlrOption, bool checkInNetwork) const;

    void initializeVlrOption(VlrIntOption& vlrOption, const VlrRingVID& dstVid=VLRRINGVID_NULL) const;
    int getVlrUniPacketByteLength() const;
    /** set setupPacke->srcVset array based on vset and pendingVset, return number of nodes added to srcVset */
    unsigned int setupPacketSrcVset(VlrIntSetupPacket *setupPacket) const;

    // node status helper
    VlrRingVID getProxyForSetupReq(bool checkInNetwork=true) const;
    /** get the closest ccw vnei and cw vnei (wrap-around search in vset) to me */
    std::pair<VlrRingVID, VlrRingVID> getClosestVneisInVset() const;
    /** assuming vset full (size==vsetHalfCardinality*2), get the farthest ccw vnei and cw vnei (wrap-around search in vset) to me */
    std::pair<VlrRingVID, VlrRingVID> getFarthestVneisInFullVset() const;
    /** check if targetVid is closer to me than my closest ccw or cw vnei in vset */
    bool isClosestVneiTo(VlrRingVID targetVid) const;
    /** assuming vset full, get relevant vneis in vset to forward SetupReq of newnode */
    void getVneisForwardInFullVsetTo(const VlrRingVID& newnode, const VlrIntVidSet& knownSet, bool isNewnodeCCW, std::set<VlrRingVID>& neisToForward) const;
    /** return shouldAdd, removedNeis, neisToForward */
    std::tuple<bool, std::vector<VlrRingVID>, std::set<VlrRingVID>> shouldAddVnei(const VlrRingVID& newnode, const VlrIntVidSet& knownSet, bool findNeisToForward) const;
    /** after adding a new vnei (and removing old vnei), modify selfInNetwork or inNetworkWarmupTimer, recordNodeStatsRecord() to record if vset full and correct */
    void recordNewVnei(const VlrRingVID& newVnei);

    void processOtherVset(const VlrIntSetupPacket *setupPacket, const VlrRingVID& srcVid, bool addToEmpty=true);
    bool pendingVsetAdd(VlrRingVID node, VlrRingVID heardFrom=VLRRINGVID_NULL, bool addToEmpty=true);
    void pendingVsetErase(VlrRingVID node);
    void vsetInsertRmPending(VlrRingVID node);
    bool vsetEraseAddPending(VlrRingVID node);
    void delayRouteTeardown(const VlrRingVID& oldVnei, const std::set<VlrPathID>& oldVsetRoutes);
    void delayLostPneiTempVlinksTeardown(const std::set<VlrPathID>& tempVlinks, double delay);
    void removeRouteFromLostPneiBrokenVroutes(const VlrPathID& pathid, const VlrRingVID& lostPneiVid);
    
    /** return true if node has scheduled setupReq timer in pendingVset */
    bool hasSetupReqPending() const;
    simtime_t_cref getASetupReqPendingArrivalTime() const;
     /** return true if towardVid is a LINKED pnei or an endpoint in vlrRoutingTable whose next hop is linked  */
    bool IsLinkedPneiOrAvailableRouteEnd(VlrRingVID towardVid) const;
    /** return true if pendingVnei is a LINKED pnei or an endpoint of a wanted (not patched, temporary or dismantled) available vroute in vlrRoutingTable  */
    bool IsLinkedPneiOrAvailableWantedRouteEnd(VlrRingVID towardVid, bool routeBtwUs=false) const;
    /** add nodes close to me from trace, add to pendingVset and record trace to them */
    void addCloseNodesFromTrace(int numHalf, const VlrIntVidVec& trace, bool removeLoop, unsigned int traceEndIndex);

    void clearState(bool clearPsetAndRep);
    void cancelPendingVsetTimers();
    void cancelRepairLinkTimers();
    std::set<VlrRingVID> convertVsetToSet() const;
    VlrRingVID readRepresentativeVidFromFile();
    /** print vset for EV_INFO purpose */
    std::string printVsetToString() const;
    std::string printRoutesToMeToString() const;
};

}; // namespace omnetvlr

#endif
