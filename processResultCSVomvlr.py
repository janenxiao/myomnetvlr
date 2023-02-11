import networkx as nx
import matplotlib.pyplot as plt
# from sortedcontainers import SortedDict, SortedList, SortedSet
from random import sample, randrange, choice
import numpy as np
import pandas

from networkx.readwrite import json_graph
import pickle
import csv
import json
from os.path import exists
import shutil
import re
from math import sqrt


# get <numNeis> closest item indices before and after <index> in a list of size <vecSize>, e.g. vecSize=3 (i.e. indices=[0, 1, 2]), index=1, numNeis=1, return [0, 2]
def getNeiIndicesInVectorWrapAround(index, numNeis, vecSize):
    neiIndices = []
    indexlow = index
    indexup = index
    while numNeis > 0:
        if indexlow == 0:
            indexlow = vecSize
        if indexup == vecSize - 1:
            indexup = -1
        indexlow -= 1
        indexup += 1
        neiIndices.append(indexlow)
        neiIndices.append(indexup)
        
        numNeis -= 1
    return neiIndices

def processResultNodeFile(nodefileToRead, edgelistFile, gpickleFile, numvroutesFile, vroutepathsFile, constructtimeFile, constructlogFile):
    PG = nx.Graph()     # only 1 edge is allowed btw a pair of nodes
    # nodePositions = dict()  # {vid: xy coordinates}
    vsetHalfCardinality = None

    startrow = 0
    endrow = 0
    i = 0

    with open(numvroutesFile, 'w', newline='') as csvfile1:
        csvwriter_numvroutes = csv.writer(csvfile1)
        if startrow <= 0:
            csvwriter_numvroutes.writerow(["node", "numRoutes"])

        with open(vroutepathsFile, 'w', newline='') as csvfile2:
            csvwriter_vroutepaths = csv.writer(csvfile2)
            if startrow <= 0:
                csvwriter_vroutepaths.writerow(["src", "dst", "physical path length"])

            with open(constructtimeFile, 'w', newline='') as csvfile3:
                csvwriter_constructtime = csv.writer(csvfile3)
                if startrow <= 0:
                    csvwriter_constructtime.writerow(["time", "node", "status", "numNodesVsetFull"])
        
                # constructtime = ''  # time when last "vsetFull" is recorded
                nodesVsetFull = set()  # nodes that have recorded "vsetFull"
                # row format: node vid, time, "nodeStats", xy coordinates, L3Address, linkedPneis, inNetwork, hasRoot, vset, routes to me in vlrRoutingTable, linkedPneis size, vset size, pendingVset size, number of vroutes in vlrRoutingTable
                with open(nodefileToRead, newline='') as csvfileToRead:
                    spamreader = csv.reader(csvfileToRead)
                    # next(spamreader)    # skip header row
                    for row in spamreader:
                        if i < startrow:    # line x (x starts at 1) in nodefileToRead corresponds to i=x-1
                            i += 1
                            continue
                        if endrow > 0 and i >= endrow:
                            break
                        else:
                            i += 1
                            # print(row)

                            node = int(row[0])
                            timeStr = row[1]
                            infoStr = row[2]
                            if infoStr == "nodeStats":
                                stageStr = row[14]
                                if stageStr == "finish":
                                    simPosition = eval(row[3])
                                    linkedPneiStr = row[5][1:-1].strip()    # strip() removes the leading and trailing whitespaces
                                    vsetStr = row[8][1:-1].strip()
                                    hasRoot = bool(int(row[7]))
                                    vroutesToMeStr = row[9][1:-1].strip()
                                    numRoutes = int(row[13])
                                    PG.add_node(node)
                                    PG.nodes[node]['simPosition'] = simPosition
                                    # nodePositions[node] = simPosition

                                    if hasRoot:     # if node doesn't have root, it shouldn't have any vroutes in vlrRoutingTable

                                        PG.nodes[node]['vset'] = {int(s) for s in re.findall(r'\d+', vsetStr)}  # parse numbers in vsetStr and store as attribute "vset"
                                        vsetHalfCard = int(len(PG.nodes[node]['vset']) / 2)
                                        if vsetHalfCardinality is None or vsetHalfCardinality < vsetHalfCard:   # vsetHalfCardinality = max vset size / 2
                                            vsetHalfCardinality = vsetHalfCard
                                        
                                        if linkedPneiStr:
                                            for pneiStr in linkedPneiStr.split(' '):
                                                pnei = int(pneiStr)
                                                PG.add_edge(node, pnei)
                                                # print("add_edge: ", node, pnei)
                                        if vroutesToMeStr:
                                            for vrouteStr in vroutesToMeStr.split(' '):
                                                vrouteItems = vrouteStr.split(':')
                                                csvwriter_vroutepaths.writerow(vrouteItems)
                                        
                                        csvwriter_numvroutes.writerow([node, numRoutes])
                            
                            elif infoStr.startswith('vsetFull'):
                                nodesVsetFull.add(node)
                                csvwriter_constructtime.writerow([timeStr, node, "vsetFull", len(nodesVsetFull)])
                            
                            elif infoStr.startswith('vsetUnfull') or infoStr.startswith('rootLost') or infoStr.find('NodeFailure') != -1:
                                nodesVsetFull.discard(node)     # remove(node) throws KeyError exception if node isn't found in set, while discard(node) doesn't
                                if infoStr.startswith('vsetUnfull'):
                                    vsetStatus = 'vsetUnfull'
                                elif infoStr.startswith('rootLost'):
                                    vsetStatus = 'rootLost'
                                else:
                                    vsetStatus = 'NodeFailure'
                                csvwriter_constructtime.writerow([timeStr, node, vsetStatus, len(nodesVsetFull)])

    with open(constructlogFile, 'w') as filewrite_constructlog:
            
        print("vsetHalfCardinality:", vsetHalfCardinality)
        filewrite_constructlog.write(f"vsetHalfCardinality: {vsetHalfCardinality}\n")

        largest_connected_component = max(nx.connected_components(PG), key=len) # set of nodes in the largest connected component in PG
        nodesVsetNotFull = largest_connected_component.difference(nodesVsetFull)    # nodesVsetNotFull: nodes in largest_connected_component (should be in overlay) but not in nodesVsetFull
        if not nodesVsetNotFull:    # nodesVsetNotFull is empty
            print(f"All nodes in largest_connected_component vsetFull")
            print(f"largest_connected_component size: {len(largest_connected_component)}")
            filewrite_constructlog.write("All nodes in largest_connected_component vsetFull\n")
            filewrite_constructlog.write(f"largest_connected_component size: {len(largest_connected_component)}\n")
        else:
            print(f"Error! nodes not vsetFull: {nodesVsetNotFull}")
            print(f"nodesVsetFull: {nodesVsetFull}")
            filewrite_constructlog.write(f"Error! nodes not vsetFull: {nodesVsetNotFull}\n")
            filewrite_constructlog.write(f"nodesVsetFull: {nodesVsetFull}\n")

        # verify each node has correct vset
        numConnectedCorrect = 0     # number of nodes in largest_connected_component with correct vset
        connected_node_vec = sorted(largest_connected_component)
        nodeToIndex = dict()    # map node id to its index in connected_node_vec
        for i in range(len(connected_node_vec)):
            nodeToIndex[connected_node_vec[i]] = i
        for node in connected_node_vec:
            vsetCorrect = {connected_node_vec[neiIndex] for neiIndex in getNeiIndicesInVectorWrapAround(nodeToIndex[node], vsetHalfCardinality, len(connected_node_vec))}
            if PG.nodes[node]['vset'] != vsetCorrect:
                print(f"Error! node {node} correct vset: {vsetCorrect}, actual vset: {PG.nodes[node]['vset']}")
                filewrite_constructlog.write(f"Error! node {node} correct vset: {vsetCorrect}, actual vset: {PG.nodes[node]['vset']}\n")
            else:
                numConnectedCorrect += 1
            
        print(f"Number of nodes with correct vset: {numConnectedCorrect}, expected number: {len(largest_connected_component)}")
        filewrite_constructlog.write(f"Number of nodes with correct vset: {numConnectedCorrect}, expected number: {len(largest_connected_component)}\n")
               

    # # print(list(PG.edges))
    # print(PG.degree[0])
    # print(PG.nodes(data=True))
    # # nx.draw(PG, with_labels=True)
    # nx.draw(PG, with_labels=True, pos=nodePositions)
    # print(nx.info(PG))
    # plt.show()

    if edgelistFile is not None and type(edgelistFile) == str:
        # check if this file is overwriting an existing file
        if exists(edgelistFile) and input(f"Are you sure to overwrite the existing file {edgelistFile}? Enter Y to continue") != 'Y':
            print(f"Didn't write edgelist to file {edgelistFile}")
        else:
            with open(edgelistFile, 'wb') as fh:    # need to have the directory created
                nx.write_edgelist(PG, fh, delimiter=',', data=False)
            # write NetworkX graphs as Python pickle
            if gpickleFile is not None and type(gpickleFile) == str:
                nx.write_gpickle(PG, gpickleFile)
        
        # graph data in node-link format that is suitable for JSON serialization
        # with open(edgelistFile, 'w') as fh:
        #     node_link_data = json_graph.node_link_data(PG)
        #     print(node_link_data)
        #     json.dump(node_link_data, fh, indent=4) # position tuple will be converted to list
        # with open(edgelistFile, 'r') as fh:
        #     data = json.load(fh)
        #     print(data)
        #     H = json_graph.node_link_graph(data)
        #     print(H.nodes(data=True))
        #     nx.draw(H, with_labels=True)
        #     plt.show()

    return PG

def writeMsgIdMapToDeliveryFile(msgIdSent, testdeliveryFile, maxDeliveryTime, currTime):
    # msgIdSent - {(src, msgId): [sendTime, dst, (deliveredTime, hopcount)]}
    # if currTime is None, write every item in msgIdSent to testdeliveryFile
    with open(testdeliveryFile, 'a', newline='') as csvfile2:
        csvwriter_testdelivery = csv.writer(csvfile2)
        
        for srcIdTuple in list(msgIdSent.keys()):
            msgInfoList = msgIdSent[srcIdTuple]
            if len(msgInfoList) == 4:    # msg sent and arrived, msgInfoList: [sendTime, dst, deliveredTime, hopcount]
                msgInfoList[1:1] = [srcIdTuple[0]]    # insert src after sendTime   NOTE right hand side must be an iterable of size at least 1
                csvwriter_testdelivery.writerow(msgInfoList)
                del msgIdSent[srcIdTuple]
            elif currTime is None or float(msgInfoList[0]) +1 < currTime - maxDeliveryTime:   # msg sent but didn't arrive in maxDeliveryTime, msgInfoList: [sendTime, dst] 
                msgInfoList[1:1] = [srcIdTuple[0]]    # insert src after sendTime   NOTE right hand side must be an iterable of size at least 1
                msgInfoList.extend([None, None])    # insert deliveredTime=None, hopcount=None after dst
                csvwriter_testdelivery.writerow(msgInfoList)
                del msgIdSent[srcIdTuple]
            else:   # msg sent but didn't arrive, but maxDeliveryTime hasn't passed after sendTime, keep waiting before deleting this row and rows afterward from msgInfoList
                break


def processResultTestFile(testfileToRead, testpathsFile, PG, testdeliveryFile, maxDeliveryTime, vlrpacketsFile, testpathsStartTime=0):
    # maxDeliveryTime - packet sent in msgIdSent that doesn't get a reply in sendTime + maxDeliveryTime will be considered failed
    # testpathsStartTime - only record TestPacket arrived after testpathsStartTime in testpathsFile, usually some recovery time after failure, or 0 if no failure
    sumStretch = 0
    numStretch = 0

    startrow = 0
    endrow = 0

    msgIdSent = dict()  # {(src, msgId): [sendTime, dst, deliveredTime, hopcount]} for recording delivery of TestPacket
    nextCheckDeliveryTime = 0

    # testSrcDstArrived = dict()  # {src: {set of dst to which TestPacket from src has arrived}}

    with open(testpathsFile, 'w', newline='') as csvfile:
        csvwriter_testpaths = csv.writer(csvfile)
        if startrow <= 0:
            csvwriter_testpaths.writerow(["src", "dst", "physical path length", "shortest path length", "stretch"])
        largest_connected_component = max(nx.connected_components(PG), key=len) # set of nodes in the largest connected component in PG

        # NOTE only write a header row and close the file to open file properly in writeMsgIdMapToDeliveryFile()
        with open(testdeliveryFile, 'w', newline='') as csvfile2:
            csvwriter_testdelivery = csv.writer(csvfile2)
            if startrow <= 0:
                csvwriter_testdelivery.writerow(["sendTime", "src", "dst", "deliveredTime", "physical path length"])

        with open(vlrpacketsFile, 'w', newline='') as csvfile4:
            csvwriter_vlrpackets = csv.writer(csvfile4)
            if startrow <= 0:
                # msgState: sent/arrived/received
                #   sent:     time, src, dst, msgType, msgState=sent, hopcount=None, chunkByteLength=None
                #   arrived:  time, src, dst, msgType, msgState=arrived, hopcount, chunkByteLength
                #   received: time, src, current hop, msgType, msgState=received, hopcount=int or None (for SetupReq Teardown NotifyVset etc), chunkByteLength
                csvwriter_vlrpackets.writerow(["time", "src", "dst", "msgType", "msgState", "hopcount", "chunkByteLength", "infoStr"])
    
            with open(testfileToRead, newline='') as csvfileToRead:
                spamreader = csv.reader(csvfileToRead)
                # next(spamreader)    # skip header row
                i = 0
                # row format: node, src, messageId at src, dst, time, msgType, "sent/arrived/received", hopcount, byte length
                for row in spamreader:
                    if i < startrow:    # line x (x starts at 1) in nodefileToRead corresponds to i=x-1
                        i += 1
                        continue
                    if endrow > 0 and i >= endrow:
                        break
                    else:
                        i += 1
                        if i % 10000 == 0:
                            csvfile.flush()
                            print(f"i: {i}")
                    
                    node = int(row[0])
                    src = int(row[1])
                    msgId = int(row[2])
                    dst = int(row[3])
                    timeStr = row[4]
                    msgType = row[5]
                    msgState = row[6]
                    if msgType == "TestPacket":
                        if msgState == "arrived":
                            physicalLength = int(row[7])

                            msgTime = float(timeStr)

                            if msgTime > testpathsStartTime:
                                if src in largest_connected_component and dst in largest_connected_component:
                                    shortestPath = nx.shortest_path(PG, src, dst)   # throws networkx.exception.NetworkXNoPath if no path btw src and dst, i.e. not in the same connected component
                                    shortestLength = len(shortestPath)
                                    
                                    stretch = physicalLength / shortestLength
                                    sumStretch += stretch
                                    numStretch += 1

                                    csvwriter_testpaths.writerow([src, dst, physicalLength, shortestLength, stretch])  # <src>,<dst>,<physical path length>,<shortest path length>,<stretch>
                                
                                else:   # src or dst not in largest_connected_component of PG
                                    print(f"Error: vroute endpoint src={src} connected={(src in largest_connected_component)}, dst={dst} connected={(dst in largest_connected_component)}")

                            # testSrcDstArrived.setdefault(src, set()).add(dst)   # add dst to testSrcDstArrived[src] which is a set

                            if (src, msgId) in msgIdSent:
                                msgIdSent[(src, msgId)].extend([timeStr, physicalLength])

                            if msgTime > nextCheckDeliveryTime:
                                writeMsgIdMapToDeliveryFile(msgIdSent, testdeliveryFile, maxDeliveryTime, msgTime)
                                nextCheckDeliveryTime = msgTime + maxDeliveryTime
                        
                        elif msgState == "sent":
                            msgIdSent[(src, msgId)] = [timeStr, dst]
                            # if msgTime > 1019:
                            #     print(f"msgIdSent[{(src, msgId)}] = {msgIdSent[(src, msgId)]}")

                    # msg is a control packet
                    else:
                        infoStr = row[9] if len(row) > 9 else ""
                        hopcount = None if msgState == "sent" else int(row[7])
                        if hopcount == 0:
                            hopcount = None
                        chunkByteLength = None if msgState == "sent" else int(row[8])
                        if chunkByteLength == 0:
                            chunkByteLength = None
                        dst = node if msgState == "received" else dst   # record receiving node as dst for "received" control packets
                        csvwriter_vlrpackets.writerow([timeStr, src, dst, msgType, msgState, hopcount, chunkByteLength, infoStr])
                        
    
    # write every msg in msgIdSent to testdeliveryFile
    if msgIdSent:
        writeMsgIdMapToDeliveryFile(msgIdSent, testdeliveryFile, maxDeliveryTime, currTime=None)

    # with open(testpathsbysrcFile, 'w', newline='') as csvfile3:
    #     csvwriter_testpathsbysrc = csv.writer(csvfile3)
    #     if startrow <= 0:
    #         csvwriter_testpathsbysrc.writerow(["src", "numDst"])
    #     for src in testSrcDstArrived:
    #         csvwriter_testpathsbysrc.writerow([src, len(testSrcDstArrived[src])])


    print(f"final i: {i}")
    avgStretch = sumStretch / numStretch
    print(f'{sumStretch} / {numStretch} = {avgStretch}')


def getDeliveryFileFromResultTestFile(testfileToRead, testdeliveryFile, maxDeliveryTime):
    msgIdSent = dict()  # {(src, msgId): [sendTime, dst, deliveredTime, hopcount]} for recording delivery of TestPacket
    nextCheckDeliveryTime = 0

    # NOTE only write a header row and close the file to open file properly in writeMsgIdMapToDeliveryFile()
    with open(testdeliveryFile, 'w', newline='') as csvfile2:
        csvwriter_testdelivery = csv.writer(csvfile2)
        csvwriter_testdelivery.writerow(["sendTime", "src", "dst", "deliveredTime", "physical path length"])

    with open(testfileToRead, newline='') as csvfileToRead:
        spamreader = csv.reader(csvfileToRead)
        # next(spamreader)    # skip header row
        i = 0
        # row format: src, messageId at src, dst, time, msgType, "sent/arrived/received", hopcount, byte length
        for row in spamreader:
            i += 1
            if i % 10000 == 0:
                print(f"i: {i}")
            
            node = int(row[0])
            src = int(row[1])
            msgId = int(row[2])
            dst = int(row[3])
            timeStr = row[4]
            msgType = row[5]
            msgState = row[6]
            if msgType == "TestPacket":
                if msgState == "arrived":
                    physicalLength = int(row[7])

                    if (src, msgId) in msgIdSent:
                        msgIdSent[(src, msgId)].extend([timeStr, physicalLength])

                    if float(timeStr) > nextCheckDeliveryTime:
                        writeMsgIdMapToDeliveryFile(msgIdSent, testdeliveryFile, maxDeliveryTime, float(timeStr))
                        nextCheckDeliveryTime = float(timeStr) + maxDeliveryTime
                
                elif msgState == "sent":
                    msgIdSent[(src, msgId)] = [timeStr, dst]
                    # if float(timeStr) > 1019:
                    #     print(f"msgIdSent[{(src, msgId)}] = {msgIdSent[(src, msgId)]}")

            # else: msg is a control packet
    
    # write every msg in msgIdSent to testdeliveryFile
    if msgIdSent:
        writeMsgIdMapToDeliveryFile(msgIdSent, testdeliveryFile, maxDeliveryTime, currTime=None)


def calcVrouteStretchWithPG(vroutefileToRead, vroutepathsFile, PG):
    sumStretch = 0
    numStretch = 0

    startrow = 0
    endrow = 0

    largest_connected_component = max(nx.connected_components(PG), key=len) # set of nodes in the largest connected component in PG

    with open(vroutepathsFile, 'w', newline='') as csvfile:
        csvwriter = csv.writer(csvfile)
        if startrow <= 0:
            csvwriter.writerow(["src", "dst", "physical path length", "shortest path length", "stretch"])
        
        with open(vroutefileToRead, newline='') as csvfileToRead:
            spamreader = csv.reader(csvfileToRead)
            next(spamreader)    # skip header row
            i = 0
            # row format: src, messageId at src, dst, time, msgType, "sent/arrived/received", hopcount, byte length
            for row in spamreader:
                if i < startrow:    # line x (x starts at 1) in nodefileToRead corresponds to i=x-1
                    i += 1
                    continue
                if endrow > 0 and i >= endrow:
                    break
                else:
                    i += 1
                    if i % 10000 == 0:
                        csvfile.flush()
                        # print(f"i: {i}")
                
                src = int(row[0])
                dst = int(row[1])
                physicalLength = int(row[2])

                if src in largest_connected_component and dst in largest_connected_component:

                    shortestPath = nx.shortest_path(PG, src, dst)
                    shortestLength = len(shortestPath)
                    
                    stretch = physicalLength / shortestLength
                    sumStretch += stretch
                    numStretch += 1

                    csvwriter.writerow([src, dst, physicalLength, shortestLength, stretch])  # <src>,<dst>,<physical path length>,<shortest path length>,<stretch>
                
                else:   # src or dst not in largest_connected_component of PG
                    print(f"Error: vroute endpoint src={src} connected={(src in largest_connected_component)}, dst={dst} connected={(dst in largest_connected_component)}")


    print(f"final i: {i}")
    avgStretch = sumStretch / numStretch
    print(f'{sumStretch} / {numStretch} = {avgStretch}')

def genGraphEdgelistFile(graphType, filepath, numCols, numNodes):
    maxNodeId = 60000
    # option 2: require node 0 at a random position in nodeList
    labelList = sample(range(1, maxNodeId), numNodes)      # get a list of int from [1, maxNodeId) without replacement
    node0Index = randrange(0, numNodes)   # get a random int from [0, numNodes)
    # node0Index = 3146
    print("node0Index", node0Index)
    if input(f"Are you sure to continue with node0Index? Enter Y to continue") != 'Y':
        print(f"Abandoned task")
        return
    labelList[node0Index] = 0

    if graphType == "squareGrid":
        G = nx.grid_2d_graph(numCols, numCols)
        position_generator = ((j, i) for i in range(0, numCols) for j in range(0, numCols))   # this makes a generator, which is an iterator. When you ask an iterator for an iterator (by passing it to the built-in iter function), it'll return itself
        # print(type(position_generator))     # <class 'generator'>
        # zip function works by getting an iterator from each of the given iterables and repeatedly calling next on that iterator, i.e. iter1 = iter(position_generator)   item = next(iter1)
        # zip function will only iterate over the smallest list passed
        labelMapping = dict(zip(position_generator, labelList))
        # print(labelMapping)

        # print grid to file
        xl = 0
        xr = numCols - 1
        yl = 0
        yr = numCols - 1
        with open(f"{filepath}printGrid_squareGrid_{numNodes}.csv", 'w') as file:
            rowstr = f"{' ': <3}\t\t"   # variable:{fill character}{align left< or right> or center^}{width}    print 3 whitespaces b4 \t\t
            for x in range(xl, xr+1):
                rowstr += f"{x:5d}\t"
            rowstr += "\n"
            file.write(rowstr)

            for y in range(yl, yr+1):
                rowstr = f"{y:3d}\t\t"  # specify width to be 3, padded with spaces
                for x in range(xl, xr+1):
                    rowstr += f"{labelMapping[(x,y)]:5d}\t"  # specify width to be 5, padded with spaces
                rowstr += "\n"
                file.write(rowstr)

        # add node original label (coordinate in grid) as node attribute
        for node in G.nodes:
            G.nodes[node]["position"] = node

        assert len(labelMapping) >= G.number_of_nodes(), f"labelMapping size={len(labelMapping)} smaller than Graph G size={G.number_of_nodes()}"
    
        G = nx.relabel_nodes(G, labelMapping, copy=False)
        print("node0Position: ", G.nodes[0]["position"])

        # # # srcdstpairs = [(176, 76)]
        # # # for src, dst in srcdstpairs:
        # # #     shortestPath = nx.shortest_path(G, src, dst)
        # # #     print("shortestPath: ", shortestPath, " length: ", len(shortestPath))

        # find node ids at special positions
        repIdCaseList = []      # contains lists, each list e.g. ["nodeAtCorner", nodeId1, nodeId2, nodeId3, nodeId4]
        # node at a corner
        xposlist = [0, (numCols-1)]
        poslist = [(j, i) for i in xposlist for j in xposlist]
        print("nodeAtCorner:", poslist)
        nodeIdlist = [labelMapping[pos] for pos in poslist]
        nodeIdlist.insert(0, "nodeAtCorner")    # insert "nodeAtCorner" at index 0
        repIdCaseList.append(nodeIdlist)
        # node at centre of an edge
        edgeIndexChoice = int((numCols-1) / 2)
        edgeIndexChoices = [edgeIndexChoice] * 4
        if numCols % 2 == 0:    # if numCols is even, there are two nodes considered "centre" on an edge
            for i in range(0, len(edgeIndexChoices)):
                edgeIndexChoices[i] += choice(range(0, 2))
        print(f"edgeIndexChoices: {edgeIndexChoices}")
        poslist = [(0, edgeIndexChoices[0]), ((numCols-1), edgeIndexChoices[1]), (edgeIndexChoices[2], 0), (edgeIndexChoices[3], (numCols-1))]
        print("nodeAtEdge:", poslist)
        nodeIdlist = [labelMapping[pos] for pos in poslist]
        nodeIdlist.insert(0, "nodeAtEdgece")
        repIdCaseList.append(nodeIdlist)
        # node on diagonal,  diagonalpc=25: node equally close to the corner and the centre, diagonalpc=50: node roughly at centre
        diagonalPercentages = [25, 50]
        for diagonalpc in diagonalPercentages:
            poslist = []
            corn20IndexChoice = round(diagonalpc/100 * numCols) - 1     # NOTE round() rounds half (.5) to even integers, e.g. 1.5=>2, 2.5=>2, 3.5=>4, 4.5=>4
            if diagonalpc < 50:
                if corn20IndexChoice == 0:
                    corn20IndexChoice = 1
                elif corn20IndexChoice >= int((numCols-1) / 2):
                    corn20IndexChoice -= 1
                # assert corn20IndexChoice > 0 and corn20IndexChoice < int((numCols-1) / 2), f"Error: corn20IndexChoice={corn20IndexChoice}"
            print(f"diagonalpc: {diagonalpc}, diagIndexChoice: {corn20IndexChoice}")
            for rowChoice in range(0, 2):
                for colChoice in range(0, 2):
                    corn20RowIndexChoice = corn20IndexChoice if rowChoice == 0 else (numCols-1 - corn20IndexChoice)
                    corn20ColIndexChoice = corn20IndexChoice if colChoice == 0 else (numCols-1 - corn20IndexChoice)
                    poslist.append((corn20RowIndexChoice, corn20ColIndexChoice))
            print(f"diagonalpc: {diagonalpc}, nodeAtDiag:", poslist)
            nodeIdlist = [labelMapping[pos] for pos in poslist]
            repPosName = f"nodeAtCorn{diagonalpc}"
            if diagonalpc == 50:
                repPosName = "nodeAtCentre"
            nodeIdlist.insert(0, repPosName)
            repIdCaseList.append(nodeIdlist)
        # write repIdPosFile
        repIdPosFile = f"{filepath}repPosNodeId_squareGrid_{numNodes}.csv"
        with open(repIdPosFile, 'w', newline='') as csvfile:
            csvwriter_repIdPos = csv.writer(csvfile)
            for i in range(len(repIdCaseList)):
                csvwriter_repIdPos.writerow(repIdCaseList[i])

        # write node vid file
        vidAssignmentFile = f"{filepath}vidlist_squareGrid_{numNodes}_maxId60000.csv"
        with open(vidAssignmentFile, 'w') as file:
            for label in labelList:
                file.write(f"{label}\n")

        # write edgelistFile and gpickleFile
        edgelistFile = f"{filepath}PGedgelist_squareGrid_{numNodes}.csv"
        gpickleFile = f"{filepath}PGpickle_squareGrid_{numNodes}.gpickle"
        with open(edgelistFile, 'wb') as fh:    # need to have the directory created
            nx.write_edgelist(G, fh, delimiter=',', data=False)
        # write NetworkX graphs as Python pickle
        if gpickleFile is not None and type(gpickleFile) == str:
            nx.write_gpickle(G, gpickleFile)

        # # # print(nx.info(G))
        # # print(G.nodes(data=True))
        # nodePositions = nx.get_node_attributes(G, 'position')
        # nx.draw(G, with_labels=True, pos=nodePositions)
        # plt.show()

    elif graphType == "powerlaw2":
        # G = nx.powerlaw_cluster_graph(numNodes, 2, 0.5)
        # read graph from existing file
        edgelistFileToRead = f"{filepath}edgelist_{graphType}-{numNodes}-nonConsec.csv"
        G = nx.read_edgelist(edgelistFileToRead, delimiter=',', nodetype=int)  # physical graph

        labelMapping = dict(zip(G.nodes, labelList))
        G = nx.relabel_nodes(G, labelMapping, copy=True)   # copy=False

        # write node vid file
        vidAssignmentFile = f"{filepath}vidlist_{graphType}_{numNodes}_maxId60000.csv"
        with open(vidAssignmentFile, 'w') as file:
            for label in labelList:
                file.write(f"{label}\n")

        # write edgelistFile and gpickleFile
        edgelistFile = f"{filepath}PGedgelist_{graphType}_{numNodes}.csv"
        gpickleFile = f"{filepath}PGpickle_{graphType}_{numNodes}.gpickle"
        with open(edgelistFile, 'wb') as fh:    # need to have the directory created
            nx.write_edgelist(G, fh, delimiter=',', data=False)
        # write NetworkX graphs as Python pickle
        if gpickleFile is not None and type(gpickleFile) == str:
            nx.write_gpickle(G, gpickleFile)

        # # print(nx.info(G))
        # # print(G.nodes(data=True))
        # nx.draw(G, with_labels=True)
        # plt.show()


    elif graphType == "pathGraph":
        G = nx.path_graph(numNodes)     # return the Path graph of <numNodes> linearly connected nodes

        # # write node vid file
        vidAssignmentFile = f"{filepath}vidlist_{graphType}_{numNodes}_consec.csv"
        with open(vidAssignmentFile, 'w') as file:
            for node in G.nodes:
                file.write(f"{node}\n")

        # # write edgelistFile and gpickleFile
        edgelistFile = f"{filepath}PGedgelist_{graphType}_{numNodes}.csv"
        gpickleFile = f"{filepath}PGpickle_{graphType}_{numNodes}.gpickle"
        with open(edgelistFile, 'wb') as fh:    # need to have the directory created
            nx.write_edgelist(G, fh, delimiter=',', data=False)
        # # write NetworkX graphs as Python pickle
        if gpickleFile is not None and type(gpickleFile) == str:
            nx.write_gpickle(G, gpickleFile)

        # # print(nx.info(G))
        # print(G.nodes(data=True))
        # nx.draw(G, with_labels=True)
        # # nodePositions = nx.get_node_attributes(G, 'position')
        # # nx.draw(G, with_labels=True, pos=nodePositions)
        # plt.show()



def readGraphpickle(graphType, filepath, numCols, numNodes):
    if graphType == "squareGrid":
        # generate vidAssignment file from squareGrid graph pickle
        gpickleFile = f"{filepath}PGpickle_{graphType}_{numNodes}.gpickle"
        G = nx.read_gpickle(gpickleFile)
        position_generator = ((j, i) for i in range(0, numCols) for j in range(0, numCols))   # this makes a generator, which is an iterator. When you ask an iterator for an iterator (by passing it to the built-in iter function), it'll return itself
        # print(type(position_generator))     # <class 'generator'>
        # zip function works by getting an iterator from each of the given iterables and repeatedly calling next on that iterator, i.e. iter1 = iter(position_generator)   item = next(iter1)
        # zip function will only iterate over the smallest list passed
        posOrderMapping = dict(zip(position_generator, range(numNodes)))    # map node position to node order in file
        # print(posOrderMapping)

        labelPosTuples = [(n, G.nodes[n]["position"]) for n in G.nodes]
        labelPosTuples.sort(key=lambda tup: posOrderMapping[tup[1]])     # sort based on posOrderMapping[node position]

        vidAssignmentFile = f"{filepath}vidlist_squareGrid_{numNodes}_maxId60000.csv"
        with open(vidAssignmentFile, 'w') as file:
            for tup in labelPosTuples:
                file.write(f"{tup[0]}\n")

 

def genFailureScenarioFileFromVid(graphType, filepath, numNodes, failureType, repType, repCase, nofailNodes):
    # labelList = []
    # with open(vidAssignmentFile, newline='') as fileToRead:
    #     # row format: vid
    #     for row in fileToRead:
    #         labelList.append(int(row))

    gpickleFile = f"{filepath}PGpickle_{graphType}_{numNodes}.gpickle"
    G = nx.read_gpickle(gpickleFile)

    # if failureType.startswith("nodeFailure"):
    #     pattern = r"nodeFailure(\d+)pc(-\w+)?"  # matches "nodeFailure10pc" or "nodeFailure10pc-hinumvr"
    #     regexmatch = re.fullmatch(pattern, failureType)

    #     assert regexmatch, f"Error: unrecognized failureType: {failureType}"
    #     failurePercent = int(regexmatch.group(1)) / 100     # if failureType="nodeFailure10pc", failurePercent = 0.1
    #     print(f"node failure percentage: {failurePercent}")
    #     if regexmatch.group(2):     # if suffix "-hinumvr" exists
    #         failureSuffix = regexmatch.group(2)[1:]
    #         print(f"failure suffix: {failureSuffix}")

    #     GnodeCanfailSet = set(G.nodes).difference(nofailNodes)
    #     numfailNodes = int(round(G.number_of_nodes() * failurePercent))
    #     print("GnodeCanfailSet.size: ", len(GnodeCanfailSet))
    #     print("numfailNodes: ", numfailNodes)

    #     if failureSuffix == "hinumvr":
    #         numvroutesfilepath_ = f"results/nofailure-version0/vsetHalfCard2-1/{repType}/{repCase}/"
    #         numvroutesfilename = f"{numvroutesfilepath_}numvroutes_squareGrid_{len(labelMapping)}.csv"
    #         df_numRoutes = pandas.read_csv(numvroutesfilename, usecols=["node", "numRoutes"])
    #         df_numRoutes = df_numRoutes[df_numRoutes["node"].isin(GnodeCanfailSet)]
    #         df_numRoutes.sort_values(by=['numRoutes'], ignore_index=True, inplace=True)
    #         # fix number of different weights
    #         numWeights = 100  # number of different weights
    #         # groupSize = int(len(df_numRoutes.index) / numWeights)   # number of rows with the same weight
    #         # df_numRoutes["numRank"] = df_numRoutes.index / groupSize
    #         # df_numRoutes["numRank"] = df_numRoutes["numRank"].apply(np.floor).apply(lambda x: x+1 if x < numWeights else x )
    #         # df_numRoutes["numRank"] = df_numRoutes["numRank"]
    #         # weight increases by 1 for each unique "numRoutes" value
    #         df_numRoutes["numRank"] = df_numRoutes["numRoutes"].rank(method='dense')    # rank always increases by 1 between groups with increasing "numRoutes"
    #         print(df_numRoutes)
    #         #         node  numRoutes  numRank
    #         # 0         0          6      1.0
    #         # 1     19721          6      1.0
    #         # 2      9320          8      2.0
    #         print("df_numRoutes.quantile", df_numRoutes.quantile([0.1, 0.25, 0.5, 0.75, 0.9]))
    #         # df_selected = df_numRoutes.sample(n=numfailNodes, weights=df_numRoutes["numRank"])
    #         # print(df_selected)
    #         # print(df_selected.quantile([0.1, 0.25, 0.5, 0.75, 0.9]))

    #     GisConnected = False
    #     while not GisConnected:
    #         if failureSuffix == "hinumvr":
    #             df_selected = df_numRoutes.sample(n=numfailNodes, weights=df_numRoutes["numRank"])
    #             # print(df_selected)
    #             print("df_selected.quantile", df_selected.quantile([0.1, 0.25, 0.5, 0.75, 0.9]))
    #             failNodeList = df_selected["node"].tolist()
    #         else:
    #             failNodeList = sample(list(GnodeCanfailSet), numfailNodes)
            
    #         G_copy = G.copy()
    #         for failNode in failNodeList:
    #             G_copy.remove_node(failNode)
    #         if nx.is_connected(G_copy):
    #             GisConnected = True
    #         else:
    #             print(f"Graph G w/o failNodeList={failNodeList} is not connected, try again")

    #     degree_series = pandas.Series(data=[degree for node, degree in G_copy.degree()])
    #     print("G degree value_counts: \n", degree_series.value_counts(normalize=True))
        
    #     nodePositions = nx.get_node_attributes(G_copy, 'position')
    #     # nx.draw(G_copy, with_labels=True, pos=nodePositions)     # node_size: default=300
    #     nx.draw(G_copy, with_labels=False, node_size=30, pos=nodePositions)     # draw node as small dot
    #     # print(nx.info(G_copy))
    #     plt.show()

    #     filepath = f"{filepath}{failureType}/{repType}/{repCase}/"
    #     with open(f"{filepath}failureNode_squareGrid_{len(labelMapping)}.csv", 'w') as file:
    #         for failNode in failNodeList:
    #             file.write(f"{failNode},{labelToIndex[failNode]}\n")
    #     xl = 0
    #     xr = numCols
    #     yl = 0
    #     yr = numCols
    #     xl = 0 if (xl < 0) else (numCols - 1) if (xl > numCols - 1) else xl
    #     xr = 0 if (xr < 0) else (numCols - 1) if (xr > numCols - 1) else xr
    #     yl = 0 if (yl < 0) else (numCols - 1) if (yl > numCols - 1) else yl
    #     yr = 0 if (yr < 0) else (numCols - 1) if (yr > numCols - 1) else yr
    #     with open(f"{filepath}printGrid_{len(labelMapping)}_{xl}-{xr}--{yl}-{yr}.csv", 'w') as file:
    #         rowstr = f"{' ': <3}\t\t"   # variable:{fill character}{align left< or right> or center^}{width}    print 3 whitespaces b4 \t\t
    #         for x in range(xl, xr+1):
    #             rowstr += f"{x:5d}\t"
    #         rowstr += "\n"
    #         file.write(rowstr)

    #         for y in range(yl, yr+1):
    #             rowstr = f"{y:3d}\t\t"  # specify width to be 3, padded with spaces
    #             for x in range(xl, xr+1):
    #                 if labelMapping[(x,y)] not in failNodeList:
    #                     rowstr += f"{labelMapping[(x,y)]:5d}\t"  # specify width to be 5, padded with spaces
    #                 else:
    #                     rowstr += f"{' ': <5}\t"   # variable:{fill character}{align left< or right> or center^}{width}    print ' ' with total width = 5, fill w/ whitespaces, b4 \t
    #             rowstr += "\n"
    #             file.write(rowstr)


    # elif failureType.startswith("linkFailure"):
    #     pattern = r"linkFailure(\d+)pc"
    #     regexmatch = re.fullmatch(pattern, failureType)

    #     assert regexmatch, f"Error: unrecognized failureType: {failureType}"
    #     failurePercent = int(regexmatch.group(1)) / 100     # if failureType="nodeFailure10pc", failurePercent = 0.1
    #     print(f"link failure percentage: {failurePercent}")

    #     numfailLinks = int(round(G.number_of_edges() * failurePercent))
    #     print("numfailLinks: ", numfailLinks)
    #     GisConnected = False
    #     while not GisConnected:
    #         failLinkList = sample(list(G.edges), numfailLinks)
    #         G_copy = G.copy()
    #         for failLink in failLinkList:
    #             G_copy.remove_edge(failLink[0], failLink[1])
    #         if nx.is_connected(G_copy):
    #             GisConnected = True
    #         else:
    #             print(f"Graph G w/o failLinkList={failLinkList} is not connected, try again")
                
    #     degree_series = pandas.Series(data=[degree for node, degree in G_copy.degree()])
    #     print("G degree value_counts: \n", degree_series.value_counts(normalize=True))
        
    #     nodePositions = nx.get_node_attributes(G_copy, 'position')
    #     # nx.draw(G_copy, with_labels=True, pos=nodePositions)     # node_size: default=300
    #     nx.draw(G_copy, with_labels=False, node_size=30, pos=nodePositions)     # draw node as small dot
    #     # print(nx.info(G_copy))
    #     plt.show()

    #     filepath = f"{filepath}{failureType}/{repType}/{repCase}/"
    #     with open(f"{filepath}failureLink_squareGrid_{len(labelMapping)}.csv", 'w') as file:
    #         for failLink in failLinkList:
    #             file.write(f"{failLink[0]},{failLink[1]}\n")
        

# excludeNodes: set of nodes that can't appear in testDstList
def genTestDstAssignmentFileFromVid(graphType, filepath, numNodes, excludeNodes):
    numTestDst = 200    # number of nodes in testDstList per node in labelList
    labelList = list()  # nodes in vidAssignmentFile
    availableList = list()  # nodes in vidAssignmentFile but not in excludeNodes

    vidAssignmentFile = f"{filepath}vidlist_{graphType}_{numNodes}_maxId60000.csv"
    testDstAssignmentFile = f"{filepath}testDst_{graphType}_{numNodes}.csv"


    with open(vidAssignmentFile, newline='') as fileToRead:
        # row format: vid
        for row in fileToRead:
            nodeid = int(row)
            labelList.append(nodeid)
            if nodeid not in excludeNodes:
                availableList.append(nodeid)
    
    with open(testDstAssignmentFile, 'w', newline='') as csvfile:
        csvwriter = csv.writer(csvfile)
        for nodeid in labelList:
            # generate a list of numTestDst nodes by sampling from availableList
            # row format: my vid, testDst nodes ..
            numTestDst_remaining = numTestDst
            numNodes = len(availableList)
            testDstList = [nodeid]      # start with my vid as the first item!
            while (numTestDst_remaining > 0):
                numTestDst_thisround = min(numTestDst_remaining, numNodes)
                testDstList += sample(availableList, numTestDst_thisround)      # get a list of int without replacement as node ids
                numTestDst_remaining -= numTestDst_thisround
                # print("numTestDst_remaining =", numTestDst_remaining)
                # print("testDstList:", testDstList)
            csvwriter.writerow(testDstList)


def checkupCmdlogFile(resultCmdlogFile):
    with open(resultCmdlogFile, newline='') as fileToRead:
        # searchStr = "Incremented station SRC: stationShortRetryCounter = "
        # searchStr = "Incremented station LRC: stationLongRetryCounter = "
        # searchStr = "Removing next hop of failed VLR packet: "
        # searchStr = "Processing failed VLR packet: "
        searchStrs = ["handleLostPneis: ", "failed VLR packet: ", "Purging pset of expired pneis: ", "NonQosRecoveryProcedure::isRetryLimitReached: ", "has expired, leaving vnetwork", "WARN"]     # "Ieee802154Mac::manageMissingAck: "
        # searchStr1 = "ohno NonQosRecoveryProcedure::isRetryLimitReached: SRC = "
        # searchStr2 = "ohno NonQosRecoveryProcedure::isRetryLimitReached: LRC = "
        i = 0
        rcList = []
        rcLimitList = []
        lastrow = ""
        lastrowprinted = False
        for row in fileToRead:
            # i += 1
            # startIndex = row.find(searchStr1)
            # startIndex = row.find(searchStr2) if startIndex == -1 else startIndex
            # if startIndex > -1:
            #     rowpartial = row[startIndex + len(searchStr):]
            #     # digitList = list(filter(str.isdigit, rowpartial))    # get all characters where str.isdigit() returns true, put in list, e.g. ['1', '2']
            #     # number = int(''.join(digitList))  # join all number digits in list and convert to int
            #     numberList = [int(s) for s in re.findall(r'\d+', rowpartial)]
            #     # print(f"i={i}  number={numberList}")
            #     rcList.append(numberList[0])
            #     rcLimitList.append(numberList[1])

            rowprinted = False
            for searchStr in searchStrs:
                startIndex = row.find(searchStr) 
                if startIndex > -1:
                    if not lastrowprinted:
                        print(lastrow)
                    print(row)
                    rowprinted = True
                    break
            
            lastrow = row
            lastrowprinted = rowprinted

            # if i > 2000000:
            #     break
    # print("len(rcList): ", len(rcList))
    # if rcList:
    #     series_rc = pandas.Series(rcList)
    #     series_rclimit = pandas.Series(rcLimitList)
    #     # print(series_rc)
    #     # print(series_rc.quantile([0, 0.1, 0.25, 0.5, 0.75, 0.9, 1]))
    #     print("series_rc.shape: ", series_rc.shape)
    #     print(series_rc.value_counts())      # normalize=True
    #     # print(series_rclimit)
    #     print(series_rclimit.value_counts())     # normalize=True

def checkupResultNodeFile(nodefileToRead):
    col_names = ["node", "time", "infoStr", "position"]
    dummyList = "1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20".split(' ')
    numcols = 15    # number of columns in nodefileToRead csv
    col_names.extend(dummyList[:numcols-len(col_names)])
    # print(col_names)
    df = pandas.read_csv(nodefileToRead, names=col_names)
    # df_vsetFull = df[df.iloc[:, 1].str.startswith("vsetFull")]
    df_vsetFull = df[df["infoStr"].str.startswith("vsetFull")]
    # # df_vsetFull.columns = ["node", "infoStr"]
    # print(df_vsetFull)
    print("df_vsetFull.shape: ", df_vsetFull.shape)
    print("df_vsetFull['node'].nunique: ", df_vsetFull["node"].nunique())
    # print("df['infoStr'].value_counts: \n", df["infoStr"].value_counts())

def checkupResultTestFile(testfileToRead):
    print(testfileToRead)
    df = pandas.read_csv(testfileToRead, names=["node", "src", "msgId", "dst", "time", "msgType", "msgState", "hopcount", "chunkByteLength", "infoStr"])
    # print(df)
    # print(df["src"].unique())
    print("df['time'].iloc[-1]: ", df["time"].iloc[-1])
    print("df['src'].nunique: ", df["src"].nunique())
    # print("df['msgType'].unique: ", df["msgType"].unique())
    print("df['msgType'].value_counts: \n", df.value_counts(subset=["msgType", "msgState"], dropna=False))


if __name__ == "__main__":
    graphType = "powerlaw2"    # squareGrid, powerlaw2, pathGraph
    graphCase = "g1"
    numNodes = 5000
    maxDeliveryTime = 50    # TestPacket that aren't recorded "arrived" within sendTime + maxDeliveryTime will be considered failed
    testpathsStartTime = 700

    failureType = "nodeFailure20pc-2"    # nofailure, nodeFailure10pc, linkFailure10pc, -notemp-nodism-nohear
    repType = "repAtCentre"     # repAtCorner, repAtCentre, repAtCorn20, repAtEdge
    repCase = "repCase0"
    vsetCardType = "vsetHalfCard2-1"    # vsetHalfCard1-2, vsetHalfCard2-1, vsetHalfCard3-0
    filepath = f"results/"
    # filepath = f"results/nofailure/vsetHalfCard1-2/repAtCorner/"
    filepath = f"results/{failureType}/{vsetCardType}/{repType}/{repCase}/"
    # filepath = f"../../../myvlr_results/nofailure/"
    # resultNodeFile = f"{filepath}nodeStats.csv"
    # resultTestFile = f"{filepath}sendRecords.csv"
    resultNodeFile = f"{filepath}nodeStats_{graphType}_{numNodes}.csv"
    resultTestFile = f"{filepath}sendRecords_{graphType}_{numNodes}.csv"
    resultNodeFile_renamed = f"{filepath}nodeStats_{graphType}_{numNodes}.csv"
    resultTestFile_renamed = f"{filepath}sendRecords_{graphType}_{numNodes}.csv"
    edgelistFile = f"{filepath}PGedgelist_{graphType}_{numNodes}.csv"
    gpickleFile = f"{filepath}PGpickle_{graphType}_{numNodes}.gpickle"
    numvroutesFile = f"{filepath}numvroutes_{graphType}_{numNodes}.csv"
    vroutepathsFile_partial = f"{filepath}vroutepaths_partial_{graphType}_{numNodes}.csv"
    vroutepathsFile = f"{filepath}vroutepaths_{graphType}_{numNodes}.csv"
    testpathsFile = f"{filepath}testpaths_{graphType}_{numNodes}.csv"
    # testpathsbysrcFile = f"{filepath}testpathsbysrc_{graphType}_{numNodes}.csv"
    testdeliveryFile = f"{filepath}testdelivery_{graphType}_{numNodes}.csv"
    constructtimeFile = f"{filepath}constructtime_{graphType}_{numNodes}.csv"
    constructlogFile = f"{filepath}constructlog_{graphType}_{numNodes}.txt"
    vlrpacketsFile = f"{filepath}vlrpackets_{graphType}_{numNodes}.csv"

    resultCmdlogFile = f"{filepath}cmdenvlog_{graphType}_{numNodes}.out"
    # resultCmdlogFile = f"{filepath}Vlr-#0.out"

    filepath = f"networks/physicalTopo/{graphType}/{graphCase}/"
    numCols = int(sqrt(numNodes))
    genGraphEdgelistFile(graphType, filepath, numCols, numNodes)
    # readGraphpickle(graphType, filepath, numCols, numNodes)
    # filepath = f"vidAssignmentFile/failureScenarioFile/"
    # failureDirectory = "nodeFailure10pc-hinumvr"
    # genFailureScenarioFileFromVid(vidAssignmentFile, numCols, filepath, failureType=failureDirectory, repType="repAtCentre", repCase="repCase0", nofailNodes={330})

    # ### generate testDst that includes all nodes in vidAssignmentFile
    # genTestDstAssignmentFileFromVid(graphType, filepath, numNodes, excludeNodes=[])
    # ### exclude failed nodes from testDst
    # filepath = f"vidAssignmentFile/failureScenarioFile/{failureDirectory}/repAtCentre/repCase0/"
    # failureNodeFile = f"{filepath}failureNode_squareGrid_{numNodes}.csv"
    # testDstAssignmentFile = f"{filepath}testDst_ring_{numNodes}_maxId25000.csv"
    # failNodeList = []
    # with open(failureNodeFile, newline='') as fileToRead:
    #     # row format: node id, node index
    #     for row in fileToRead:
    #         failNodeList.append(eval(row)[0])     # eval(row) return a tuple
    # print("failNodeList size:", len(failNodeList))
    # genTestDstAssignmentFileFromVid(vidAssignmentFile, testDstAssignmentFile, excludeNodes=failNodeList)

    # checkupCmdlogFile(resultCmdlogFile)     # cmdenvlog_squareGrid_1024-warn.out
    # checkupResultNodeFile(resultNodeFile)
    # checkupResultTestFile(resultTestFile)

    
    # PG = processResultNodeFile(resultNodeFile, edgelistFile, gpickleFile, numvroutesFile, vroutepathsFile_partial, constructtimeFile, constructlogFile)

    # # PG = nx.Graph()
    # # PG.add_node(6)
    # # PG.add_edge(5, 4)
    # # PG.add_edge(5, 3)
    # # PG.add_edge(5, 2)

    # PG = nx.read_edgelist(edgelistFile, delimiter=',', nodetype=int)  # physical graph
    # PG = nx.read_gpickle(gpickleFile)
    # # nx.draw(PG, with_labels=True)
    # nodePositions = nx.get_node_attributes(PG, 'simPosition')
    # nx.draw(PG, with_labels=True, pos=nodePositions)
    # print(nx.info(PG))
    # # print(PG.nodes(data=True))
    # plt.show()

    # calcVrouteStretchWithPG(vroutepathsFile_partial, vroutepathsFile, PG)
    # processResultTestFile(resultTestFile, testpathsFile, PG, testdeliveryFile, maxDeliveryTime, vlrpacketsFile, testpathsStartTime)

    # getDeliveryFileFromResultTestFile(resultTestFile, testdeliveryFile, maxDeliveryTime)

    



