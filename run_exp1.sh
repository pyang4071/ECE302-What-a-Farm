echo "Experiment 1"
./ns3 run --no-build "rice_farm_wsn --experiment=1 --extraNodes=1  --pktSize=128 --simTime=900" &
./ns3 run --no-build "rice_farm_wsn --experiment=1 --extraNodes=2  --pktSize=128 --simTime=900" & 
./ns3 run --no-build "rice_farm_wsn --experiment=1 --extraNodes=5  --pktSize=128 --simTime=900" &
./ns3 run --no-build "rice_farm_wsn --experiment=1 --extraNodes=10 --pktSize=128 --simTime=900" &
./ns3 run --no-build "rice_farm_wsn --experiment=1 --extraNodes=15 --pktSize=128 --simTime=900" &
./ns3 run --no-build "rice_farm_wsn --experiment=1 --extraNodes=20 --pktSize=128 --simTime=900" &
wait
echo "Done"
