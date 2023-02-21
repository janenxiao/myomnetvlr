/*
 * VlrDefs.h
 *
 *  Created on: Dec. 13, 2022
 *      Author: Jane Wen
 */

#ifndef __OMNETVLR_ROUTING_VLRDEFS_H_
#define __OMNETVLR_ROUTING_VLRDEFS_H_

namespace omnetvlr {

// change std::vector<VlrRingVID> and std::vector<VlrPathID> when changing types for VlrRingVID and VlrPathID
typedef unsigned int VlrPathID;
typedef unsigned int VlrRingVID;

#define VLRPATHID_BYTELEN  4
#define VLRRINGVID_BYTELEN  4

#define VLRPATHID_INVALID  0
#define VLRPATHID_REPPATH  3

#define VLRRINGVID_NULL    65535
#define VLRRINGVID_DUMMY    65533
#define VLRRINGVID_MAX    60000     // max allowable VlrRingVID, used in computing distance in vid space (ensure maxVID+1 wouldn't overflow an unsigned int)

};  // namespace omnetvlr

#endif /* __OMNETVLR_ROUTING_VLRDEFS_H_ */
