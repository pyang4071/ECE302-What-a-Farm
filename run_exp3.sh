echo "Experiment 3"
./ns3 run --no-build "rice_farm_wsn --experiment=3 --failNodes=1  --pktSize=128 --simTime=900" &
./ns3 run --no-build "rice_farm_wsn --experiment=3 --failNodes=2  --pktSize=128 --simTime=900" &
./ns3 run --no-build "rice_farm_wsn --experiment=3 --failNodes=3  --pktSize=128 --simTime=900" &
./ns3 run --no-build "rice_farm_wsn --experiment=3 --failNodes=4  --pktSize=128 --simTime=900" &
./ns3 run --no-build "rice_farm_wsn --experiment=3 --failNodes=5  --pktSize=128 --simTime=900" &
./ns3 run --no-build "rice_farm_wsn --experiment=3 --failNodes=6  --pktSize=128 --simTime=900" &
./ns3 run --no-build "rice_farm_wsn --experiment=3 --failNodes=7  --pktSize=128 --simTime=900" &
./ns3 run --no-build "rice_farm_wsn --experiment=3 --failNodes=8  --pktSize=128 --simTime=900" &
./ns3 run --no-build "rice_farm_wsn --experiment=3 --failNodes=9  --pktSize=128 --simTime=900" &
./ns3 run --no-build "rice_farm_wsn --experiment=3 --failNodes=10 --pktSize=128 --simTime=900" &
wait
echo "Done"
