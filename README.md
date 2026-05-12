# ECE303-What-a-Farm

This project is a rice farm wireless sensor network simulation on a 50m by 50m field using the 802.11b 1 Mbps physical link layer. The effective maximum threshold is approximately 600 Kbps. The routing protocol used is the Ad-hoc On-demand Distance Vector (AODV) protocol. 

There are a bases of 25 nodes which may change based on the experiment. The nodes are 1 coordinator, 10 routers, 9 MCP sensors, 4 leaf-wetness sensors, and 1 solar irradiance sensor.

## Running the code 


As a prerequiste, the code must exist in the scratch directory of the ns3-3.46.1 directory. Then, one must build the code using the following

```bash
./ns3 build 
```

After building, one can run the simulation under different stresses either one by one or by using the predefined bash scripts. As an example, the base case is ran with the following code 

```bash
./ns3 run rice_farm_wsn
```

### Experiment 0

This is the base case scenerio which the other results are compared against. While the above code would run the base case fine, a user can also explicitly define some of the paramters used in the simulation such as with the following code:

```bash
./ns3 run --no-build "rice_farm_wsn --experiment=0 --pktSize=128 --simTime=900"
```

The latter flags corresponds to the experiment number, packet size in bytes, and simulation time in seconds. The default values is experiment 0 with packet size 128 for 900 seconds. The last two flags could be used in any experiment. There is no maximum value for these two flags.

The following code would also run the base case:

```bash
./run_exp0.sh
```

### Experiment 1 

In this experiment, the number of nodes in the simulation is increased using an extra flag ```--extraNodes```. By default if experiment 1 is ran, 5 extra nodes are placed. These nodes are placed randomly. While this value does not have an upper limit, it is suggested to keep the value between 0 and 50. An example of running this experiment is shown below:

```bash
./ns3 run --no-build "rice_farm_wsn --experiment=1 --extraNodes=10 --pktSize=128 --simTime=900"
```

To run all the tests cases for this experiment, the user cna run the command below.

```bash
./run_exp1.sh
```

### Experiment 2 

This experiment test the effect of adding interferers into the network. Each interferers adds a 50 Kbps noise source near the center of the field. To change the number of interferers, the ```--interferers``` flag is used. This value can span from 1 to 5 with a default of 1 if experiment 2 is ran. An example of running experiment 2 is shown:

```bash
./ns3 run --no-build "rice_farm_wsn --experiment=2 --interferers=2 --pktSize=128 --simTime=900"
```
To run all the tests cases for this experiment, the user cna run the command below.

```bash
./run_exp2.sh
```

### Experiment 3 

This focuses on the effect of the number of failure nodes on the simulation which can be adjusted using the ```--failNodes``` flag. While running experiment 2, this value can span from 1 to 10 with a default value of 1. The node failures are in sequential order of the routers with R1 first and R10 last. An example of running this experiment is shown below:

```bash
./ns3 run --no-build "rice_farm_wsn --experiment=3 --failNodes=4  --pktSize=128 --simTime=900"
```
To run all the tests cases for this experiment, the user cna run the command below.

```bash
./run_exp3.sh
```

### Experiment 4 

For the last experiment, we are testing the effect of traffic on the simulation. Since the sensor only send data once every long interval, to see some explicit changes, the traffic scale is increase by 800 times to produce meaningful results using the ```--trafficScale``` flag which normally defaults to 1. Then, to model traffic without increasing the simulation time linearly, the packet size is scaled up. An example of the code used for this experiment is shown below:

```bash
./ns3 run --no-build "rice_farm_wsn --experiment=4 --trafficScale=800 --pktSize=512  --simTime=120"
```
To run all the tests cases for this experiment, the user cna run the command below.

```bash
./run_exp4.sh
```

