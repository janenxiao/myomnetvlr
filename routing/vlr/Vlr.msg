//
// This file is part of an OMNeT++/OMNEST simulation example.
//
// Copyright (C) 1992-2015 Andras Varga
//
// This file is distributed WITHOUT ANY WARRANTY. See the file
// `license' for details on this and other legal matters.
//

cplusplus {{
#include "VlrDefs.h"
}}

namespace omnetvlr;     // must come after #include bc VlrDefs.h has its own namespace specification, if put after line, those defs would have namespace omnetvlr::omnetvlr

cplusplus {{
typedef std::set<unsigned int> VlrIntVidSet;
typedef std::vector<unsigned int> VlrIntVidVec;
//typedef std::vector<std::pair<unsigned int, unsigned int>> VlrIntVidPairVec;		// used in SetupReqTrace
//typedef std::vector<VlrPathID> VlrIntPathidVec;
typedef std::map<unsigned int, std::set<VlrPathID>> VlrIntVidToPathidSetMap;		// used in RepairLinkReqFlood
typedef std::map<VlrPathID, std::vector<unsigned int>> VlrIntPathidToVidVecMap;		// used in RepairLocalReply
}}

//class VlrRingVID
//{
//    @existingClass;
//    @opaque;
//    @fromString(string2ulong($));
//    @toString(ulong2string($));
//}

class VlrPathID
{
    @existingClass;
    @opaque;
    @fromString(string2ulong($));
    @toString(ulong2string($));
}

class VlrIntVidSet { @existingClass; }
class VlrIntVidVec { @existingClass; }
//class VlrIntVidPairVec { @existingClass; }
//class VlrIntPathidVec { @existingClass; }
class VlrIntVidToPathidSetMap { @existingClass; }
class VlrIntPathidToVidVecMap { @existingClass; }


struct VlrIntRepState
{
    unsigned int vid;
    unsigned int sequencenumber;
    // unsigned int hopcount;
    bool inNetwork;
}

//
// Represents an internal timer for a reply to SetupReq
//
message WaitSetupReqIntTimer
{
    unsigned int dst;
    int retryCount;
    bool repairRoute;   // if setupReq should be sent with repairRoute=true
    VlrPathID patchedRoute;   // valid only if repairRoute=true, the patched vroute to follow to send setupReq to dst
    // bool allowSetupReqTrace;   // if setupReqTrace should be sent after setupReqRetryLimit has been reached
    bool reqVnei;   // if setupReq should be sent with reqVnei=true
    unsigned int alterPendingVnei;   // valid only if reqVnei=true, subsequent node in pendingVset that's farther but in same direction to me as dst, if set and alterPendingVnei has been added vset, I'll not try another pendingVnei
    char timerType;   // location of this WaitSetupReqIntTimer, 0: pendingVset[dst], 1: vset[dst], 2: repairRouteSetupReq[patchedRoute]
}

//
// Represents an internal timer to simulate delay in detecting that a packet can't be sent bc next hop isn't linked
//
message FailedPacketDelayTimer
{
    int failedGateIndex;
    unsigned int failedPnei;
    cPacket *failedPacket;
}

//
// Represents an internal timer for a reply to RepairLinkReq
//
message WaitRepairLinkIntTimer
{
    unsigned int dst;
    int retryCount;
}

//
// The beacon packet is sent periodically by all VLR routers to notify
// the neighbors about the router's vid.
//
packet VlrIntBeacon
{
    unsigned int vid;
    bool inNetwork;
    VlrIntRepState repstate;
    VlrIntRepState repstate2;
    unsigned int psetNeighbour[];
    bool psetNeighbourIsLinked[];      // psetNeighbourIsLinked[i] indicates whether psetNeighbour[i] is a linked pnei of sender
    bool psetNeighbourIsInNetwork[];   // psetNeighbourIsInNetwork[i] indicates whether psetNeighbour[i] is inNetwork
}

//
// notify my pnei to simulate a link failure between us, uses unsigned int for vid
//
packet NotifyLinkFailureInt {
    unsigned int src;
    bool simLinkUp;      // if true, link btw src and me should restart 
}

//
// The TlvOption is used to add extra routing information for network datagrams.
//
class VlrIntOption {
    unsigned int dstVid;
    unsigned int towardVid;
    VlrPathID currentPathid;
    unsigned int tempTowardVid;
    VlrPathID tempPathid;

    unsigned int prevHopVid;
}

//
// VLR unicast packet (uses VLR protocol) that uses unsigned int for vid
//
packet VlrIntUniPacket {
    unsigned int messageId;  // message id at the src   // for statistics
    VlrIntOption vlrOption;
    unsigned int hopcount;      // for statistics
}

//
// VLR overlay setup packet that uses unsigned int for vid
//
class VlrIntSetupPacket extends VlrIntUniPacket {
    unsigned int srcVset[];
}

//
// setupReq that uses unsigned int for vid
//
class SetupReqInt extends VlrIntSetupPacket
{
    unsigned int dst;
    unsigned int newnode;
    unsigned int proxy;
    // unsigned int removedNei;
    unsigned int transferNode;
    VlrIntVidSet knownSet;
    bool reqDispatch;       // if newnode needs setupReq to be dispatched to potential vneis except dst
    bool repairRoute;       // if newnode is repairing an existing vset-route to dst, if true, traceVec is provided
    VlrPathID patchedRoute; // if repairRoute=true, indicates the patched route that this setupReq is repairing
    bool recordTrace;       // if nodes traversed by this setupReq should be recorded in traceVec
    bool reqVnei;           // if newnode is requesting dst to be its vnei
    VlrIntVidVec traceVec;
    unsigned int indexInTrace;    // index of myself in traceVec, only used when traceVec.size() > 0 and recordTrace == false, meaning path of this setupReq is specified
}

//
// setup that uses unsigned int for vid
//
class SetupReplyInt extends VlrIntSetupPacket
{
    unsigned int proxy;
    unsigned int newnode;
    unsigned int src;
    VlrPathID pathid;
    VlrIntVidVec trace;         // if specified, not using greedy routing
    VlrIntVidVec prevhopVids;
    unsigned int oldestPrevhopIndex;
    // bool reqVnei;               // if src is requesting newnode to be its vnei, if false, this setupReply builds a second vset-route btw src and newnode
    // unsigned int hopcount;      // for statistics
}

//
// setupFail that uses unsigned int for vid
//
class SetupFailInt extends VlrIntSetupPacket
{
    unsigned int proxy;
    unsigned int newnode;
    unsigned int src;
    VlrIntVidVec trace;         // if specified, not using greedy routing
}

//
// setup a non-essential vroute, uses unsigned int for vid
//
class AddRouteInt extends VlrIntSetupPacket {
    unsigned int dst;
    unsigned int src;
    unsigned int proxy;         // for testing purpose only
    VlrPathID pathid;
    VlrIntVidVec trace;         // if specified, not using greedy routing
    VlrIntVidVec prevhopVids;
    unsigned int oldestPrevhopIndex;
    // unsigned int hopcount;      // for statistics
}

//
// teardown that uses unsigned int for vid
//
class TeardownInt extends VlrIntSetupPacket
{
    VlrPathID pathids[];
    unsigned int src;       // initiator of this Teardown
    bool rebuild;           // if src thinks vroute btw two endpoints should be rebuilt, default true, false if src no longer needs the vroute (value only considered by endpoint if src is the other endpoint)
    bool dismantled;        // if src thinks (at least some) vroute in pathids should be kept in vlrRoutingTable as dismantled route rather than being deleted
}

//
// teardown that uses unsigned int for vid
//
class DismantleInt extends VlrIntSetupPacket
{
    VlrPathID pathids[];
    unsigned int src;       // initiator of this Teardown
}

//
// test message to measure route length that uses unsigned int for vid
//
class VlrIntTestPacket extends VlrIntUniPacket {
    unsigned int dst;
    unsigned int src;
    // unsigned int hopcount;
}

//
// repairLinkReq with trace that uses unsigned int for vid
//
packet RepairLinkReqFloodInt {
    VlrIntVidToPathidSetMap dstToPathidsMap;
    unsigned int ttl;
    unsigned int floodSeqnum;
    VlrIntVidVec linkTrace;
    // L3Address srcAddress;
}

//
// repairLinkReply with trace that uses unsigned int for vid
//
class RepairLinkReplyInt extends VlrIntSetupPacket {
    unsigned int src;
    // L3Address srcAddress;
    VlrIntVidVec brokenPathids;		// avoid redefinition of std::vector<unsigned int>
    // VlrIntPathidVec brokenPathids;  // dst sent repairLinkReq to src, src is prevhop (or one of prevhopVids) in brokenPathids at dst; these vroutes were previously broken but now patched by the new temporary route
    // VlrIntPathidVec brokenPathids2;  // src should send repairLinkReq to dst, src is nexthop in brokenPathids at dst
    VlrPathID tempPathid;
    VlrIntVidVec linkTrace;
    // unsigned int hopcount;      // for statistics
}

//
// repairRoute that uses unsigned int for vid
//
class RepairRouteInt extends VlrIntSetupPacket
{
    VlrPathID pathids[];
    unsigned int src;      // for statistics
}

//
// notify other of my vset, uses unsigned int for vid
//
class NotifyVsetInt extends VlrIntSetupPacket {
    unsigned int dst;
    unsigned int src;
    bool toVnei;
}

//
// repairLocalReq with trace that uses unsigned int for vid
//
packet RepairLocalReqFloodInt {
    VlrIntVidToPathidSetMap dstToPathidsMap;
    VlrIntVidSet brokenPathids;     // avoid redefinition of std::set<unsigned int>
    // VlrIntPathidSet brokenPathids;  // dst is prevhop (or one of prevhopVids) in brokenPathids at src; nodes in linkTrace don't contain these brokenPathids
    unsigned int ttl;
    unsigned int floodSeqnum;    // for statistics
    VlrIntVidVec linkTrace;
    // L3Address srcAddress;
}

//
// repairLocalReply with trace that uses unsigned int for vid
//
class RepairLocalReplyInt extends VlrIntSetupPacket {
    unsigned int src;           // for statistics
    // L3Address srcAddress;
    // VlrIntVidVec brokenPathids;		// avoid redefinition of std::vector<unsigned int>
    // VlrIntPathidVec brokenPathids;  // dst sent repairLocalReq to src, src is prevhop (or one of prevhopVids) in brokenPathids at dst; these vroutes were previously broken but now ready to be repaired with linkTrace
    VlrIntPathidToVidVecMap pathidToPrevhopMap;     // map pathid in brokenPathids to [fromVid, toVid, prevhopVids] at src
    VlrIntVidVec linkTrace;
    VlrIntVidVec prevhopVids;
    unsigned int oldestPrevhopIndex;
    // unsigned int hopcount;      // for statistics
}

//
// repairLocalPrev that uses unsigned int for vid
//
class RepairLocalPrevInt extends VlrIntSetupPacket
{
    VlrIntPathidToVidVecMap pathidToPrevhopMap;     // map pathid in brokenPathids to [prevhop, prev of prevhop, ..]
    unsigned int src;      // for statistics
}
