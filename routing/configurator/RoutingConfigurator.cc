//
// This file is part of an OMNeT++/OMNEST simulation example.
//
// Copyright (C) 1992-2015 Andras Varga
//
// This file is distributed WITHOUT ANY WARRANTY. See the file
// `license' for details on this and other legal matters.
//

#include "RoutingConfigurator.h"

#include <fstream>      // for reading/writing to file
#include <sstream>      // for std::stringstream, std::ostringstream, std::istringstream
#include <algorithm>      // for std::copy(..), std::find(..)
#include <iomanip>      // for std::setprecision()


namespace omnetvlr {

Define_Module(RoutingConfigurator);

RoutingConfigurator::~RoutingConfigurator()
{
}

void RoutingConfigurator::initialize()
{
    if (vidRingRegistry.size() < 1)
        vidRingRegistry.push_back({0, {{}}});    // initialize vidRingRegistry[0]
    // initialize failureSimulationMap
    if (failureSimulationMap.empty()) {
        // failureSimulationMap = {
        //     {2, {{80, "stop", {4}}}},
        // };
    }
    initializeFailureSimulationMap();   // first node read failure simulation file to initialize failureSimulationMap if it's empty
}

void RoutingConfigurator::initializeFailureSimulationMap()
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
                                throw cRuntimeError("Error to parse token \"%s\" in failure partition assignment file (vidRingRegistryIndex=%d, componentIndex=%lu) into unsigned int: %s", token, vidRingRegistryIndex, vidRingRegistry[vidRingRegistryIndex].second.size()-1, e.what());
                            }
                        }                    

                        if (vidRingRegistry[vidRingRegistryIndex].second.back().empty())
                            throw cRuntimeError("Failure partition assignment file (vidRingRegistryIndex=%d, componentIndex=%lu) error: not vid connected", vidRingRegistryIndex, vidRingRegistry[vidRingRegistryIndex].second.size()-1);
                    }
                }
            } else  // if failure scheduled but failure link simulation file not provided, perhaps only link failures simulated and they don't cause partition 
                EV_WARN << "Failure simulated but failure partition assignment file not provided" << endl;
        }
    } 
}


}; // namespace omnetvlr
