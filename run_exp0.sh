echo "Experiment 0"
./ns3 run --no-build "rice_farm_wsn --experiment=0 --pktSize=128 --simTime=900" &
wait
echo "Done"
