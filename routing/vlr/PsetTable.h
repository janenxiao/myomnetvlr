/*
 * PsetTable.h
 *
 *  Created on: Dec. 14, 2022
 *      Author: Jane Wen
 */

#ifndef __OMNETVLR_ROUTING_VLR_PSETTABLE_H_
#define __OMNETVLR_ROUTING_VLR_PSETTABLE_H_

#include <omnetpp.h>

#include <map>
#include <set>
#include <algorithm>    // for std::set_difference(..)

#include "VlrDefs.h"

using namespace omnetpp;

namespace omnetvlr {

enum PNeiState {
    PNEI_PENDING = 0,
    PNEI_LINKED = 1
};

std::ostream& operator<<(std::ostream& o, const PNeiState pneistate);


template<class T>
class PsetTable
{
  public:
    struct PsetTableValue {
        // L3Address address;
        int gateIndex;
        PNeiState state;
        bool inNetwork;
        simtime_t lastHeard;
        std::set<T> mpneis;     // mpneis associated this pnei in mpneiToPneiMap, only recorded if pnei is LINKED
        bool hadBeenInNetwork;   // whether inNetwork had been true for this pnei since last time it was LINKED, if so, it has been added to routing table and can be used 
    };
    std::map<T, PsetTableValue> vidToStateMap;
    std::map<T, std::set<T>> mpneiToPneiMap;    // map 2-hop pnei to the pnei in vidToStateMap from which I heard about it

    std::map<T, int> recvVidToGateIndexMap;     // record pneiVid-gateIndex mapping for messages received, so that I can send to neighbour node I've heard from but somehow isn't in my psetTable yet

  public:
    PsetTable() { }
    // virtual ~PsetTable();

    void clear();

    std::vector<T> getPsetNeighbours() const;   // cannot change member variables in this function
    bool containsKey(const T& vid) const;
    bool pneiIsLinked(const T& vid) const;
    bool pneiIsLinkedInNetwork(const T& vid) const;
    // const L3Address& getPneiL3Address(const T& vid) const;
    /** find gateIndex associated with vid in vidToStateMap, return -1 if vid not found in vidToStateMap */
    int findPneiGateIndex(const T& vid) const;
    /** get gateIndex associated with vid in vidToStateMap, throw exception if vid not found in vidToStateMap */
    int getPneiGateIndex(const T& vid) const;
    /** get pnei associated with targetGate */
    T getPneiFromGateIndex(const int& targetGate) const;

    // "typename" specifies that qualified name "std::map<T,PsetTableValue>::const_iterator" is a type, not some member variable of std::map<T,PsetTableValue> class
    // std::pair<typename std::map<T,PsetTableValue>::const_iterator, typename std::map<T,PsetTableValue>::const_iterator> getClosestPneisTo(const T& vid) const;

    /** get LINKED pneis */
    std::vector<T> getPneisLinked() const;
    /** get LINKED pneis as a set */
    std::set<T> getPneisLinkedSet() const;

    /**
     * get pneis that are LINKED and inNetwork
     */
    std::vector<T> getPneisInNetwork() const;

    /**
     * get all expired pneis (LINKED or PENDING) whose lastHeard is older (smaller) than timestamp
     */
    std::vector<T> getExpiredNeighbours(simtime_t timestamp) const;

    void setNeighbour(const T& vid, /*const L3Address& address,*/ const int& gateIndex, const PNeiState& state, const bool& inNetwork, const std::set<T>& mpneis);

    void removeMPneisOfPnei(const T& vid);

    int findRecvNeiGateIndex(const T& vid) const;
    void setRecvNeiGateIndex(const T& vid, const int& gateIndex);


    // overload << operator to print PsetTable
    friend std::ostream& operator<< (std::ostream& o, const PsetTable& t) {
        o << "{ ";
        for(auto elem : t.vidToStateMap) {
            std::string inNetStr = (elem.second.inNetwork) ? "inNet" : "noNet";
            std::string hadInNetStr = (elem.second.hadBeenInNetwork) ? "hadinNet" : "hadnoNet";
            o << elem.first << ":(" << elem.second.state << ";" << inNetStr << ";" << hadInNetStr << elem.second.lastHeard << ") ";
        }
        o << "}";
        return o;
    }

};

template <class T>
void PsetTable<T>::clear()
{
    vidToStateMap.clear();
    mpneiToPneiMap.clear();
}

template <class T>
bool PsetTable<T>::containsKey(const T& vid) const
{
    bool keyExists = (vidToStateMap.find(vid) == vidToStateMap.end()) ? false : true;
    return keyExists;
}

template <class T>
bool PsetTable<T>::pneiIsLinked(const T& vid) const
{
    const auto itr = vidToStateMap.find(vid);  // itr type: std::map<T, PsetTableValue>::iterator
    if (itr == vidToStateMap.end())            // vid doesn't exist in vidToStateMap
        return false;
    return (itr->second.state == PNEI_LINKED);
}

template <class T>
bool PsetTable<T>::pneiIsLinkedInNetwork(const T& vid) const
{
    const auto itr = vidToStateMap.find(vid);  // itr type: std::map<T, PsetTableValue>::iterator
    if (itr == vidToStateMap.end())            // vid doesn't exist in vidToStateMap
        return false;
    return (itr->second.state == PNEI_LINKED && itr->second.inNetwork);
}

// template <class T>
// const L3Address& PsetTable<T>::getPneiL3Address(const T& vid) const
// {
//     const auto itr = vidToStateMap.find(vid);  // itr type: std::map<T, PsetTableValue>::iterator
//     ASSERT(itr != vidToStateMap.end());        // assert vid exists in vidToStateMap
//     return itr->second.address;
// }

template <class T>
int PsetTable<T>::findPneiGateIndex(const T& vid) const
{
    const auto itr = vidToStateMap.find(vid);  // itr type: std::map<T, PsetTableValue>::iterator
    if (itr == vidToStateMap.end())            // vid doesn't exist in vidToStateMap
        return -1;
    return itr->second.gateIndex;
}

template <class T>
int PsetTable<T>::getPneiGateIndex(const T& vid) const
{
    const auto itr = vidToStateMap.find(vid);  // itr type: std::map<T, PsetTableValue>::iterator
    ASSERT(itr != vidToStateMap.end());        // assert vid exists in vidToStateMap
    return itr->second.gateIndex;
}

template <class T>
T PsetTable<T>::getPneiFromGateIndex(const int& targetGate) const
{
    std::vector<T> results;
    for (const auto& elem : vidToStateMap)
        if (elem.second.gateIndex == targetGate)
            results.push_back(elem.first);
    ASSERT(results.size() <= 1);    // assert only one pnei associated with targetGate
    return (results.empty() ? VLRRINGVID_NULL : results[0]);
}


template <class T>
std::vector<T> PsetTable<T>::getPsetNeighbours() const
{
    std::vector<T> neighbours;
    for (const auto& elem : vidToStateMap)     // elem type: std::pair<T, PsetTableValue>
        neighbours.push_back(elem.first);
    return neighbours;
}

template <class T>
std::vector<T> PsetTable<T>::getPneisLinked() const
{
    std::vector<T> neighbours;
    for (const auto& elem : vidToStateMap)
        if (elem.second.state == PNEI_LINKED)
            neighbours.push_back(elem.first);
    return neighbours;
}

template <class T>
std::set<T> PsetTable<T>::getPneisLinkedSet() const
{
    std::set<T> neighbours;
    for (const auto& elem : vidToStateMap)
        if (elem.second.state == PNEI_LINKED)
            neighbours.insert(elem.first);
    return neighbours;
}

template <class T>
std::vector<T> PsetTable<T>::getPneisInNetwork() const
{
    std::vector<T> neighbours;
    for (const auto& elem : vidToStateMap)
        if (elem.second.state == PNEI_LINKED && elem.second.inNetwork)
            neighbours.push_back(elem.first);
    return neighbours;
}

template <class T>
std::vector<T> PsetTable<T>::getExpiredNeighbours(simtime_t timestamp) const
{
    // timestamp is the latest lastHeard that's considered expired, any neighbour with lastHeard <= timestamp should be removed (bc the way we schedule purgeNeighborsTimer)
    std::vector<T> expiredPneis;

    for (auto it = vidToStateMap.begin(); it != vidToStateMap.end(); ++it)
        if (it->second.lastHeard <= timestamp)
            expiredPneis.push_back(it->first);
        
    return expiredPneis;
}

template <class T>
void PsetTable<T>::setNeighbour(const T& vid, const int& gateIndex, const PNeiState& state, const bool& inNetwork, const std::set<T>& mpneisNew)
{
    bool hadBeenInNetwork = false;
    // check if mpneis associated with pnei has changed
    const auto itr = vidToStateMap.find(vid);  // itr type: std::map<T, PsetTableValue>::iterator
    if (itr != vidToStateMap.end()) {           // vid already exists in vidToStateMap
        hadBeenInNetwork = itr->second.hadBeenInNetwork;
        std::vector<T> rmMpneis;    // mpneis that were associated with pnei but pnei no longer has them
        // select nodes in pnei's old mpneis but not in mpneisNew, put them in rmMpneis
        std::set_difference(itr->second.mpneis.begin(), itr->second.mpneis.end(), mpneisNew.begin(), mpneisNew.end(), std::inserter(rmMpneis, rmMpneis.begin()));
        for (const auto& mpnei : rmMpneis) {
            auto mpneiMapItr = mpneiToPneiMap.find(mpnei);
            if (mpneiMapItr != mpneiToPneiMap.end()) {
                mpneiMapItr->second.erase(vid);
                if (mpneiMapItr->second.empty())        // if no more 2-hop route to mpnei after pnei lost it, delete mpnei from mpneiToPneiMap
                    mpneiToPneiMap.erase(mpneiMapItr);
            }
        }
    }
    if (!hadBeenInNetwork && state == PNEI_LINKED && inNetwork)
        hadBeenInNetwork = true;

    // struct PsetTableValue tv = {};
    vidToStateMap[vid] = {gateIndex, state, inNetwork, simTime(), mpneisNew, hadBeenInNetwork};    

    // record mpneis of pnei
    if (state == PNEI_LINKED) 
        for (const auto& mpnei : mpneisNew)
            mpneiToPneiMap[mpnei].insert(vid);
}

template <class T>
void PsetTable<T>::removeMPneisOfPnei(const T& vid)
{
    const auto itr = vidToStateMap.find(vid);  // itr type: std::map<T, PsetTableValue>::iterator
    if (itr != vidToStateMap.end()) {           // vid exists in vidToStateMap
        for (const auto& mpnei : itr->second.mpneis) {  // remove 2-hop pneis associated with vid from mpneiToPneiMap
            auto mpneiMapItr = mpneiToPneiMap.find(mpnei);
            if (mpneiMapItr != mpneiToPneiMap.end()) {
                mpneiMapItr->second.erase(vid);
                if (mpneiMapItr->second.empty())        // if no more 2-hop route to mpnei after pnei lost it, delete mpnei from mpneiToPneiMap
                    mpneiToPneiMap.erase(mpneiMapItr);
            }
        }
        itr->second.mpneis.clear();
    }
}

template <class T>
int PsetTable<T>::findRecvNeiGateIndex(const T& vid) const
{
    const auto itr = recvVidToGateIndexMap.find(vid);  // itr type: std::map<T, int>::iterator
    if (itr == recvVidToGateIndexMap.end())           // vid doesn't exist in vidToStateMap
        return -1;
    return itr->second;
}

template <class T>
void PsetTable<T>::setRecvNeiGateIndex(const T& vid, const int& gateIndex)
{
    recvVidToGateIndexMap[vid] = gateIndex;
}


}; // namespace omnetvlr



#endif /* __OMNETVLR_ROUTING_VLR_PSETTABLE_H_ */
