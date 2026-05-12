echo "Experiment 2"
./ns3 run --no-build "rice_farm_wsn --experiment=2 --interferers=0 --pktSize=128 --simTime=900" &
./ns3 run --no-build "rice_farm_wsn --experiment=2 --interferers=1 --pktSize=128 --simTime=900" &
./ns3 run --no-build "rice_farm_wsn --experiment=2 --interferers=2 --pktSize=128 --simTime=900" &
./ns3 run --no-build "rice_farm_wsn --experiment=2 --interferers=3 --pktSize=128 --simTime=900" &
./ns3 run --no-build "rice_farm_wsn --experiment=2 --interferers=4 --pktSize=128 --simTime=900" &
./ns3 run --no-build "rice_farm_wsn --experiment=2 --interferers=5 --pktSize=128 --simTime=900" &
wait
echo "Done"
