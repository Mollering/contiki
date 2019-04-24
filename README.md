The Contiki Operating System
============================

[![Build Status](https://travis-ci.org/contiki-os/contiki.svg?branch=master)](https://travis-ci.org/contiki-os/contiki/branches)

Contiki is an open source operating system that runs on tiny low-power
microcontrollers and makes it possible to develop applications that
make efficient use of the hardware while providing standardized
low-power wireless communication for a range of hardware platforms.

Contiki is used in numerous commercial and non-commercial systems,
such as city sound monitoring, street lights, networked electrical
power meters, industrial monitoring, radiation monitoring,
construction site monitoring, alarm systems, remote house monitoring,
and so on.

For more information, see the Contiki website:

[http://contiki-os.org](http://contiki-os.org)



# Getting Started

 - Install Instant Contiki from **[http://www.contiki-os.org/start.html](http://www.contiki-os.org/start.html)**
 - Check some tutorials at **[http://anrg.usc.edu/contiki/index.php/Contiki_tutorials](http://anrg.usc.edu/contiki/index.php/Contiki_tutorials)** and examples at **[http://senstools.gforge.inria.fr/doku.php?id=contiki%3Aexamples](http://senstools.gforge.inria.fr/doku.php?id=contiki%3Aexamples)**
 - About timers: **[https://github.com/contiki-os/contiki/wiki/Timers](https://github.com/contiki-os/contiki/wiki/Timers)**
 - Very good manual: https://www.researchgate.net/file.PostFileLoader.html?id=5912cfa3f7b67ef5d630d3bc&assetKey=AS%3A492384527097856%401494405027722 
 - Other interesting resources: https://team.inria.fr/fun/files/2014/04/exercise_partII.pdf and http://www.ce.unipr.it/~davoli/attachment/teaching/iot/2015/tutorial.pdf and 

# How-To
### Network Setup for IPv6

 1. Install everything as presented in the install link above
 2. Build Cooja:
	`cd contiki/tools/cooja`
	`ant run`

3. If there is some git error:
    `git submodule update --init`
    `ant run`

4. *File > New Simulation*
	4.1. Choose a Simulation Name
	4.2. Radio Medium: Unit Disk Graph Medium (UDGM): Distance Loss
	4.3. Mote startup delay (ms): 1000
	4.4. Random Seed: 123456
	4.5. New random seed on reload: No

5. *Motes > Add motes > Create new mote type > Z1 mote*
	This will be the Sink Node
	5.1. Browse and Open: *Firmware > contiki/examples/ipv6/rpl-border-router/border-router.c*
	5.2. Compile
	5.3. Create:
	5.3.1. Number of new motes: 1 | Other options as default
	5.3.2. Add motes

6. *Motes > Add motes > Create new mote type > Wismote mote*
	These will act as sensor nodes
	6.1. Browse and Open: *Firmware > contiki/examples/er-rest-example/er-example-server.c*
	5.2. Compile
	5.3. Create:
	5.3.1. Number of new motes: As desired | Other options as desired
	5.3.2. Add motes

7. *Network window > View > Mote IDs*
	*Network window > View > Mote type*
	*Network window > View > Radio traffic*

8. How to configure motes communication ranges:
	8.1. Right button click on mote: *Change transmission ranges*
> From the sender's point of view: the TX range is the range in which the transmitted packet can be received correctly by any node within this range. The interference range is the range in which the transmission can be heard but the transmitted packet cannot be received correctly. Outside of these two ranges, the packet can not be heard.
> TX and RX ratios are just random variables that are added to the sending or reception of a packet to allow the simulation of random errors in tx or rx respectively.
> A collision will be sensed if a node tries to transmit while it lies in the tx or int range of a node that is sending at the same time.
If you are using csma, the packet will be retransmitted after a random back-off time. if not, then the packet will be dropped.

### Running a Simulation 

1. Click on the Sink with the right mouse button
	1.1. *Mote tools for Z1 > Serial Socket (SERVER)*
	1.2. Click on Start (listen port remains as default)

2. Compile and Build Tunslip6: https://www.iot-lab.info/tutorials/build-tunslip6/ to interface the simulation sink node with "our" computer, so we can read messages sent to the sink node (it is a network bridge)
	2.1. Open a new terminal
	 `cd contiki/tools`
	 `make tunslip6`
	 `sudo ./tunslip6 -a localhost aaaa::1/64`

3. In the window "Simulation Control", press "Start" or "Reload"
4. Ping a node: (new terminal)
	`ping6 aaaa::<internal ipv6 node>` (example `ping6 aaaa::200:0:0:2`)

# Using Rime protocol

- Find Rime source-code at ``contiki/core/net/rime``, some underlying modules at ``contiki/core/net`` (**packetbuf.h** and **queuebuf.h**).
- Find examples using Rime at ``contiki/examples/rime`` -- this is where the *rime-mobiwise* was based on, specifically in the examples **example-mesh.c** and **example-multihop.c**.
- As we are currently using the Sky Motes to evaluate the software, there is a very interesting example at ``contiki/examples/sky/sky-collect.c`` -- based on the Rime collect module, that needs further study, because there are some very interesting features in this module that can be used in the MobiWise work.
- It is hard to find deep dopcumentation about the modules, but here is what I found interesting until now:
-- Contiki 3.x DOxygen documentation: http://www.eistec.se/docs/contiki/files.html
-- More detailed infromation about the packetbuffer module (attention that Contiki 3.x has some differences from the link, but the basics are the same): https://anrg.usc.edu/contiki/index.php/Packetbuffer_Basics#void_packetbuf_clear.28void.29
-- A small explanation about packets and queues in Rime: https://contiki-developers.narkive.com/yqz9pnSy/difference-between-rime-packetbuf-and-rime-queuebuf

## Working with Rime-MobiWise

Rime-MobiWise is just a fork from the main Contiki Repository that can be found at https://github.com/Mollering/contiki. The goal for this repository is to create a functional example that replicates the findings in previous MobiWise work (Modeling and Analysis of IoT Energy Harvesting Networks). For that, not just the source code for the Sink Node and the Sensor Nodes is being constructed, but sometimes there is necessary to perform some changes to the underlying network modules so they behave as we intend.


