# VLR simulation
This is an implementation of our VLR protocol, along with the original VRR (Virtual Ring Routing) protocol, in the discrete event simulator [OMNeT++](https://omnetpp.org), intended for the evaluation of our protocol. For details regarding the protocol's design, alongside the setup and outcomes of our evaluation, please refer to [this document](https://hdl.handle.net/1807/130445).

### Usage
After downloading OMNeT++, you may add this project into the *samples* folder.

Start with one of the `omnetpp*.ini` files, different files target different testing scenarios, such that multiple scenarios can be run at the same time.

Each simulation generates a `nodeStats_*.csv` and a `sendRecords_*.csv` files into `results/` folder.

Run `processResultCSVomvlr.py` to process the generated files. It generates several new files such as `PGedgelist_*.csv`, `PGpickle_*.gpickle`, `numvroutes_*.csv`, etc., to prepare data for plotting.

Run `results/plotting/pandas_graph_omvlr.py` then utilizes the newly generated files for plotting with *matplotlib*.

### Folder Structure
`builder/netbuilder.ned` builds a network with topology defined by *nodesFile* and *connectionsFile* (topology files) in `networks/`.

`networks/physicalTopo/` contains topology files and failure scenario files used for evaluation of VLR.

Each node in the network (defined by `node/Node.ned`) has a submodule **routing** of type moduleinterface `routing/IRouting.ned`. 

`routing/vlr/Vlr.ned` implements VLR protocol, while `routing/vrr/Vrr.ned` implements VRR protocol. They both extend `routing/RoutingBase.ned` which has type **IRouting**.

One single **routingConfigurator** module (defined in `routing/configurator/`) in the network can be accessed by all nodes (specifically, by their **routing** module). It schedules simulated node/link shutdowns in the network based on failure scenario files in `networks/`.




