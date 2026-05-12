# ECE302-What-a-Farm

This project focuses on simulating a farm network consisting of a base of 25 nodes.

## Running the code 

There are a total of 5 different experiments a user can run. 
1. Base Case
2. Adding nodes 
3. Adding Interference
4. Remove nodes 
5. Increasing traffic 

As a prerequiste, the code must exist in the scratch directory of the ns3-3.46.1 directory. Then, one must build the code using the following

```
./ns3 build 
```

After building, one can run the simulation under different stresses either one by one or by using the predefined bash scripts. As an example, the base case is ran with the following code 

```
./ns3 run --no-build "rice_farm_wsn --experiment=0 --pktSize=128 --simTime=900"
```

