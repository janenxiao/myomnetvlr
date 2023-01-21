/*
 * PsetTable.h
 *
 *  Created on: Dec. 14, 2022
 *      Author: Jane Wen
 */

#ifndef __OMNETVLR_ROUTING_VLR_VLRRINGROUTINGTABLE_H_
#define __OMNETVLR_ROUTING_VLR_VLRRINGROUTINGTABLE_H_

#include <omnetpp.h>

#include <map>
#include <set>


#include "VlrDefs.h"

using namespace omnetpp;

namespace omnetvlr {


class VlrRingRoutingTable
{
  public:
    struct VlrRingRoute {
        VlrRingVID fromVid;
        VlrRingVID toVid;
        VlrRingVID prevhopVid;
        VlrRingVID nexthopVid;
        bool isVsetRoute;
        /*  8 bits - bit6-7: is dismantled route (bit 7/6: fromVid/toVid available) | bit5: is temporary route | bit3-4: prevhop status | bit1-2: nexthop status
         *  e.g. 0: available; 4(01|00): prevhop unavailable; 1(00|01): nexthop unavailable; 5(01|01): both prevhop and nexthop unavailable
         *       8(10|00): prevhop using temp route; 2(00|10): nexthop using temp Vide; 10(10|10): both prevhop and nexthop using temp route
         *  16: temporary route  */
        char isUnavailable;
        bool prevhopRepairRouteSent;    // true if prevhop 2-bit isUnavailable == 2 (patched) and I have sent RepairRoute for this route
        // VlrRingVID oldNexthopVid;    // if nexthopVid is changed recently, the old nexthopVid before change
        /*  [prevhop, prev of prevhop, ..] only recorded for regular routes (not temporary/dismantled routes) */
        std::vector<VlrRingVID> prevhopVids;
        /* recorded at toVid when route was first completed, only for regular routes (not temporary/dismantled routes) */
        unsigned int hopcount;  // for statistics
    };
    std::map<VlrPathID, VlrRingRoute> vlrRoutesMap;
    std::map<VlrRingVID, std::set<VlrPathID>> endpointToRoutesMap;

  public:
    VlrRingRoutingTable() { }
    // virtual ~VlrRingRoutingTable();
    void clear();

    // static function
    /** return 2-bit isUnavailable 00 (available) or 01 (unavailable) or 10 (using temp route) of prevhop if getPrev=true, or of nexthop if getPrev=false   Commented out 11 (endpoint unavailable) **/
    static char getPrevNextIsUnavailable(char isUnavailable, bool getPrev);
    /** return if isUnavailable indicates a temporary route */
    static bool getIsTemporaryRoute(char isUnavailable);
    /** return if isUnavailable indicates a dismantled route, i.e. from/to is unavailable, prev/next is blocked */
    static bool getIsDismantledRoute(char isUnavailable);
    /** return 2-bit isDismantled, i.e. 01 (toVid is unavailable), 10 (fromVid is unavailable) */
    static char getIsDismantledRoute2bit(char isUnavailable);

    std::pair<std::map<VlrPathID, VlrRingRoute>::iterator, bool> addRoute(const VlrPathID& pathid, const VlrRingVID& fromVid, const VlrRingVID& toVid, const VlrRingVID& prevhopVid, const VlrRingVID& nexthopVid, bool isVsetRoute, char isUnavailable=0);
    bool removeRouteByPathID(const VlrPathID& pathid);

    /** remove pathid from endpointToRoutesMap[fromVid] and endpointToRoutesMap[toVid] */
    void removeRouteEndsFromEndpointMap(const VlrPathID& pathid, const VlrRingRoute& vroute);
    /** remove pathid from endpointToRoutesMap[towardVid] */
    void removeRouteEndFromEndpointMap(const VlrPathID& pathid, const VlrRingVID& towardVid);
    /** add pathid in endpointToRoutesMap[towardVid] */
    void addRouteEndInEndpointMap(const VlrPathID& pathid, const VlrRingVID& towardVid);
    /** return true if pathid is in endpointToRoutesMap[towardVid] */
    bool findRouteEndInEndpointMap(const VlrPathID& pathid, const VlrRingVID& towardVid) const;
    /** return true if pathid is the only vroute in endpointToRoutesMap[towardVid] */
    bool isOnlyRouteEndInEndpointMap(const VlrPathID& pathid, const VlrRingVID& towardVid) const;
    /** return number of vroutes in endpointToRoutesMap[towardVid] */
    size_t getNumRoutesToEndInEndpointMap(const VlrRingVID& towardVid) const;
    /** return true if there exists a vroute to towardVid whose next hop is available */
    bool findAvailableRouteEndInEndpointMap(const VlrRingVID& towardVid) const;

    /** get next hop to towardVid in pathid */
    VlrRingVID getRouteNextHop(const VlrPathID& pathid, const VlrRingVID& towardVid) const;
    /** get next hop to towardVid in vroute pointed by vlrRoutesMap iterator itr */
    VlrRingVID getRouteItrNextHop(std::map<VlrPathID, VlrRingRoute>::const_iterator itr, const VlrRingVID& towardVid) const;
    /** get a vroute to towardVid, return pathid and next hop to towardVid */
    std::pair<VlrPathID, VlrRingVID> getRouteToEndpoint(const VlrRingVID& towardVid, int endpointRouteIndex=0) const;
    /** get a vset route (isVsetRoute=true) btw vnei and me, return pathid */
    VlrPathID getVsetRouteToVnei(const VlrRingVID& vnei, const VlrRingVID& me) const;

    /** return 0 if next hop to towardVid in vrouteItr is unavailable, 1 if next hop is available, 2 if next hop uses temporary route */
    char isRouteItrNextHopAvailable(std::map<VlrPathID, VlrRingRoute>::const_iterator vrouteItr, const VlrRingVID& towardVid) const;
    /** set 2-bit vrouteItr->second.isUnavailable of prevhop if setPrev=true, or of nexthop if setPrev=false; value can be 00 (available) or 01 (uvavailable) or 10 (using temp route) or 11 (endpoint unavailable) */
    void setRouteItrPrevNextIsUnavailable(std::map<VlrPathID, VlrRingRoute>::iterator vrouteItr, bool setPrev, char value);
    /** set isDismantled bit in vrouteItr->second.isUnavailable */
    void setRouteItrIsDismantled(std::map<VlrPathID, VlrRingRoute>::iterator vrouteItr, bool setDismantled, bool setPrev);


    // overload << operator to print VlrRingRoute
    friend std::ostream& operator<< (std::ostream& o, const VlrRingRoute& r) {
        o << "{ ";
        o << "from:" << r.fromVid << ", to:" << r.toVid << ", prev:" << r.prevhopVid << ", next:" << r.nexthopVid << ", isVsetRoute:" << r.isVsetRoute << ", isUnavailable:" << (int)r.isUnavailable;
        o << " }";
        return o;
    }

    // overload << operator to print VlrRingRoutingTable
    friend std::ostream& operator<< (std::ostream& o, const VlrRingRoutingTable& t) {
        o << "{ ";
        for(auto elem : t.vlrRoutesMap) {
            o << elem.first << ": " << elem.second;
        }
        o << " }";
        return o;
    }

};



}; // namespace omnetvlr



#endif /* __OMNETVLR_ROUTING_VLR_PSETTABLE_H_ */
