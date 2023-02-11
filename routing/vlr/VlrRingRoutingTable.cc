//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
// 

#include <string>
#include <algorithm>    // for std::set_difference(..)

#include "VlrRingRoutingTable.h"

namespace omnetvlr {

void VlrRingRoutingTable::clear()
{
    endpointToRoutesMap.clear();
    vlrRoutesMap.clear();
}

// isVsetRoute=true means I'm one endpoint of this vroute, the other end is my vnei in vset; once this vroute becomes non-essential (other end no longer my vnei), isVsetRoute = false
std::pair<std::map<VlrPathID, VlrRingRoutingTable::VlrRingRoute>::iterator, bool> VlrRingRoutingTable::addRoute(const VlrPathID& pathid, const VlrRingVID& fromVid, const VlrRingVID& toVid, const VlrRingVID& prevhopVid, const VlrRingVID& nexthopVid, bool isVsetRoute, char isUnavailable/*=0*/)
{
    // if (vlrRoutesMap.find(pathid) != vlrRoutesMap.end())    // if pathid already exists in routing table
    // insert into routing table only if pathid is unique
    auto itr_bool = vlrRoutesMap.insert({pathid, {fromVid, toVid, prevhopVid, nexthopVid, isVsetRoute, isUnavailable, /*prevhopRepairRouteSent=*/false, /*prevhopVids=*/{}}});  // std::pair<iterator, bool>: iterator (to the inserted element), bool (whether the element is inserted)
    if (!itr_bool.second)   // if pathid already exists in routing table
        return itr_bool;
    // VlrRingRoute* routePtr = &(itr_bool.first->second);
    
    if (fromVid == 215 && toVid == 215)
        EV_DEBUG << "ohno" << endl;

    // insert route in endpointToRoutesMap
    endpointToRoutesMap[fromVid].insert(pathid);
    endpointToRoutesMap[toVid].insert(pathid);
    return itr_bool;    // itr_bool.second must be true
}

bool VlrRingRoutingTable::removeRouteByPathID(const VlrPathID& pathid)
{
    auto itr = vlrRoutesMap.find(pathid);
    if (itr == vlrRoutesMap.end())      // if pathid not in routing table
        return false;
    // VlrRingRoute* routePtr = &(itr->second);

    // delete route in endpointToRoutesMap
    removeRouteEndsFromEndpointMap(pathid, itr->second);

    // delete route in routing table
    vlrRoutesMap.erase(itr);
    return true;
}

// if pathid in endpointToRoutesMap, remove pathid from endpointToRoutesMap[vroute.fromVid] and endpointToRoutesMap[vroute.toVid]
void VlrRingRoutingTable::removeRouteEndsFromEndpointMap(const VlrPathID& pathid, const VlrRingRoute& vroute)
{
    // delete route fromVid in endpointToRoutesMap
    removeRouteEndFromEndpointMap(pathid, vroute.fromVid);
    // delete route toVid in endpointToRoutesMap
    removeRouteEndFromEndpointMap(pathid, vroute.toVid);
}

// if pathid in endpointToRoutesMap, remove pathid from endpointToRoutesMap[towardVid]
void VlrRingRoutingTable::removeRouteEndFromEndpointMap(const VlrPathID& pathid, const VlrRingVID& towardVid)
{
    auto epMapItr = endpointToRoutesMap.find(towardVid);
    if (epMapItr != endpointToRoutesMap.end()) {
        epMapItr->second.erase(pathid);
        if (epMapItr->second.empty())        // if no more route to towardVid after deletion of this pathid, delete towardVid from endpointToRoutesMap
            endpointToRoutesMap.erase(epMapItr);
    }
}

// insert pathid in endpointToRoutesMap[towardVid]
void VlrRingRoutingTable::addRouteEndInEndpointMap(const VlrPathID& pathid, const VlrRingVID& towardVid)
{
    endpointToRoutesMap[towardVid].insert(pathid);
}

// return true if towardVid in endpointToRoutesMap and pathid in endpointToRoutesMap[towardVid]
bool VlrRingRoutingTable::findRouteEndInEndpointMap(const VlrPathID& pathid, const VlrRingVID& towardVid) const
{
    auto epMapItr = endpointToRoutesMap.find(towardVid);
    if (epMapItr != endpointToRoutesMap.end() && epMapItr->second.find(pathid) != epMapItr->second.end())
        return true;
    return false;
}

// return true if towardVid in endpointToRoutesMap and pathid is the only vroute in endpointToRoutesMap[towardVid]
bool VlrRingRoutingTable::isOnlyRouteEndInEndpointMap(const VlrPathID& pathid, const VlrRingVID& towardVid) const
{
    auto epMapItr = endpointToRoutesMap.find(towardVid);
    if (epMapItr != endpointToRoutesMap.end() && epMapItr->second.find(pathid) != epMapItr->second.end() && epMapItr->second.size() == 1)
        return true;
    return false;
}

// return number of vroutes in endpointToRoutesMap[towardVid]
size_t VlrRingRoutingTable::getNumRoutesToEndInEndpointMap(const VlrRingVID& towardVid) const
{
    auto epMapItr = endpointToRoutesMap.find(towardVid);
    if (epMapItr != endpointToRoutesMap.end())
        return epMapItr->second.size();
    return 0;
}

// return true if towardVid in endpointToRoutesMap and there exists a pathid in endpointToRoutesMap[towardVid] whose next hop is available, similar to VlrRing::getClosestVendTo()::vendItrQualified()
bool VlrRingRoutingTable::findAvailableRouteEndInEndpointMap(const VlrRingVID& towardVid) const
{
    auto epMapItr = endpointToRoutesMap.find(towardVid);
    if (epMapItr != endpointToRoutesMap.end()) {
        // return true only if there exists a pathid whose next hop is linked
        // for (const auto& pathid : epMapItr->second) {
        //     const auto vrouteItr = vlrRoutesMap.find(pathid);
        //     if (isRouteItrNextHopAvailable(vrouteItr, towardVid) == 1)    // next hop in pathid doesn't use temporary route
        //         return true;
        // }
        // return true if there exists a pathid whose next hop is available (whether using temporary route or not)
        return true;
    }
    return false;
}

VlrRingVID VlrRingRoutingTable::getRouteNextHop(const VlrPathID& pathid, const VlrRingVID& towardVid) const
{
    const auto itr = vlrRoutesMap.find(pathid);
    ASSERT(itr != vlrRoutesMap.end());        // assert pathid exists in vlrRoutesMap
    ASSERT(towardVid == itr->second.fromVid || towardVid == itr->second.toVid);
    VlrRingVID nextHopVid = (towardVid == itr->second.fromVid) ? itr->second.prevhopVid : itr->second.nexthopVid;
    return nextHopVid;
}

VlrRingVID VlrRingRoutingTable::getRouteItrNextHop(std::map<VlrPathID, VlrRingRoute>::const_iterator itr, const VlrRingVID& towardVid) const
{
    ASSERT(itr != vlrRoutesMap.end());        // assert pathid exists in vlrRoutesMap
    ASSERT(towardVid == itr->second.fromVid || towardVid == itr->second.toVid);
    VlrRingVID nextHopVid = (towardVid == itr->second.fromVid) ? itr->second.prevhopVid : itr->second.nexthopVid;
    return nextHopVid;
}

// endpointRouteIndex: index of vroute to get in endpointToRoutesMap[towardVid] set, must be in range [0, endpointToRoutesMap.at(towardVid).size()-1]
std::pair<VlrPathID, VlrRingVID> VlrRingRoutingTable::getRouteToEndpoint(const VlrRingVID& towardVid, int endpointRouteIndex/*=0*/) const
{
    // VlrPathID pathid = *(endpointToRoutesMap[towardVid].begin());   // note std::map's operator[] is not declared as const, thus can't be used in const function
    // use the first vrouteID in set
    // VlrPathID pathid = *(endpointToRoutesMap.at(towardVid).begin());
    // use a vroute that doesn't involve temporary route if possible
    // for (const auto& vrouteid : endpointToRoutesMap.at(towardVid)) {
    //     if (isRouteItrNextHopAvailable(vlrRoutesMap.find(vrouteid), towardVid) == 1) {    // next hop in vrouteid doesn't use temporary route
    //         pathid = vrouteid;
    //         break;
    //     }
    // }

    // use a random vrouteID in set
    // Commented out bc intuniform() defined in ccomponent.h, but VlrRingRoutingTable doesn't extend cSimpleModule : cModule : cComponent, thus can't get random number
    // int endpointRouteIndex = intuniform(0, endpointToRoutesMap.at(towardVid).size()-1);
    auto vrouteSetItr = endpointToRoutesMap.at(towardVid).begin();
    for (int i = 0; i < endpointRouteIndex; i++)
        vrouteSetItr++;
    VlrPathID pathid = *vrouteSetItr;

    return std::make_pair(pathid, getRouteNextHop(pathid, towardVid));
}

// get vset route btw vnei and me
VlrPathID VlrRingRoutingTable::getVsetRouteToVnei(const VlrRingVID& vnei, const VlrRingVID& me) const
{
    // NOTE even if vset route btw vnei and me is broken and removed from endpointToRoutesMap[vnei], it won't be removed from endpointToRoutesMap[me] bc me can never be disconnected
    const auto itr = endpointToRoutesMap.find(me);
    if (itr != endpointToRoutesMap.end()) {
        const std::set<VlrPathID>& pathidSet = itr->second;   // get all vroutes with one endpoint == me
        for (const auto& pathid : pathidSet) {
            const VlrRingRoute& vroute = vlrRoutesMap.at(pathid);       // vroute has one endpoint == me
            if (vroute.isVsetRoute && (vroute.fromVid == vnei || vroute.toVid == vnei))   // if vroute other endpoint == vnei, and it is a vset route
                return pathid;
        }
    }
    return VLRPATHID_INVALID;   // shouldn't get to this step if vroute btw endVid1 and endVid2 exists in vlrRoutesMap
}

// return 0 if next hop to towardVid in vrouteItr is unavailable or blocked (bc endpoint unavailable), 1 if next hop is available, 2 if next hop uses temporary route
char VlrRingRoutingTable::isRouteItrNextHopAvailable(std::map<VlrPathID, VlrRingRoute>::const_iterator vrouteItr, const VlrRingVID& towardVid) const
{
    if (vrouteItr != vlrRoutesMap.end()) {
        if (vrouteItr->second.isUnavailable == 0 || vrouteItr->second.isUnavailable == 16)
            return 1;
        if (towardVid == vrouteItr->second.fromVid) {
            char hopIsUnavailable = getPrevNextIsUnavailable(vrouteItr->second.isUnavailable, /*getPrev=*/true);
            if (hopIsUnavailable == 0 /*|| hopIsUnavailable == 3*/)
                return 1;
            else if (hopIsUnavailable == 2)
                return 2;
            else    // hopIsUnavailable == 1 or 3
                return 0;
        } else if (towardVid == vrouteItr->second.toVid) {
            char hopIsUnavailable = getPrevNextIsUnavailable(vrouteItr->second.isUnavailable, /*getPrev=*/false);
            if (hopIsUnavailable == 0 /*|| hopIsUnavailable == 3*/)
                return 1;
            else if (hopIsUnavailable == 2)
                return 2;
            else    // hopIsUnavailable == 1 or 3
                return 0;
        }
    }
    return 0;   // invalid vrouteItr
}


bool VlrRingRoutingTable::getIsTemporaryRoute(char isUnavailable)
{
    if (isUnavailable == 0)
        return false;
    if (isUnavailable == 16)
        return true;
    char mask = 16;      // 0b10000
    return isUnavailable & mask;
}

bool VlrRingRoutingTable::getIsDismantledRoute(char isUnavailable)
{
    // char mask2bit1 = 3;      // 0b11
    // return ((isUnavailable & mask2bit1) == 3) || (((isUnavailable >> 2) & mask2bit1) == 3);
    char mask = 96;      // 0b1100000
    return isUnavailable & mask;
}

char VlrRingRoutingTable::getIsDismantledRoute2bit(char isUnavailable)
{
    unsigned int offset = 5;
    char mask2bit1 = 3;      // 0b11
    return (isUnavailable >> offset) & mask2bit1;
}

// return 2-bit isUnavailable of prevhop if getPrev=true, or of nexthop if getPrev=false
char VlrRingRoutingTable::getPrevNextIsUnavailable(char isUnavailable, bool getPrev)
{
    if (isUnavailable == 0 || isUnavailable == 16)
        return 0;
    unsigned int offset = (getPrev) ? 2 : 0;
    char mask2bit1 = 3;      // 0b11
    return (isUnavailable >> offset) & mask2bit1;
}

// set 2-bit vrouteItr->second.isUnavailable of prevhop if setPrev=true, or of nexthop if setPrev=false
void VlrRingRoutingTable::setRouteItrPrevNextIsUnavailable(std::map<VlrPathID, VlrRingRoute>::iterator vrouteItr, bool setPrev, char value)
{
    unsigned int offset = (setPrev) ? 2 : 0;
    char mask2bit1 = 3;      // 0b11
    char isUnavailable_new = vrouteItr->second.isUnavailable & ~(mask2bit1 << offset);     // set corresponding 2-bit value to 00
    if (value == 0)
        vrouteItr->second.isUnavailable = isUnavailable_new;
    else {
        isUnavailable_new = isUnavailable_new | (value << offset);     // set corresponding 2-bit value to value
        vrouteItr->second.isUnavailable = isUnavailable_new;
    }
    // reset prevhopRepairRouteSent to false if prevhop 2-bit isUnavailable is set to any value other than 2
    if (setPrev && (value != 2))
        vrouteItr->second.prevhopRepairRouteSent = false;
}

void VlrRingRoutingTable::setRouteItrIsDismantled(std::map<VlrPathID, VlrRingRoute>::iterator vrouteItr, bool setDismantled, bool setPrev)
{
    unsigned int offset = (setPrev) ? 6 : 5;
    char maskbit1 = 1;      // 0b1
    if (setDismantled)
        vrouteItr->second.isUnavailable = vrouteItr->second.isUnavailable | (maskbit1 << offset);     // set corresponding bit to 1
    else
        vrouteItr->second.isUnavailable = vrouteItr->second.isUnavailable & ~(maskbit1 << offset);     // set corresponding bit to 0
}



} /* namespace omnetvlr */