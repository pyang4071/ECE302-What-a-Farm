echo "Experiment 4"
./ns3 run --no-build "rice_farm_wsn --experiment=4 --trafficScale=800 --pktSize=64   --simTime=120" &
./ns3 run --no-build "rice_farm_wsn --experiment=4 --trafficScale=800 --pktSize=128  --simTime=120" &
./ns3 run --no-build "rice_farm_wsn --experiment=4 --trafficScale=800 --pktSize=256  --simTime=120" & 
./ns3 run --no-build "rice_farm_wsn --experiment=4 --trafficScale=800 --pktSize=512  --simTime=120" &
./ns3 run --no-build "rice_farm_wsn --experiment=4 --trafficScale=800 --pktSize=768  --simTime=120" &
./ns3 run --no-build "rice_farm_wsn --experiment=4 --trafficScale=800 --pktSize=1024 --simTime=120" &
./ns3 run --no-build "rice_farm_wsn --experiment=4 --trafficScale=800 --pktSize=1500 --simTime=120" &
./ns3 run --no-build "rice_farm_wsn --experiment=4 --trafficScale=800 --pktSize=1800 --simTime=120" &
./ns3 run --no-build "rice_farm_wsn --experiment=4 --trafficScale=800 --pktSize=2000 --simTime=120" &
./ns3 run --no-build "rice_farm_wsn --experiment=4 --trafficScale=800 --pktSize=3000  --simTime=120" &
./ns3 run --no-build "rice_farm_wsn --experiment=4 --trafficScale=800 --pktSize=5000  --simTime=120" & 
./ns3 run --no-build "rice_farm_wsn --experiment=4 --trafficScale=800 --pktSize=8000  --simTime=120" &
./ns3 run --no-build "rice_farm_wsn --experiment=4 --trafficScale=800 --pktSize=10000  --simTime=120" &
./ns3 run --no-build "rice_farm_wsn --experiment=4 --trafficScale=800 --pktSize=12000 --simTime=120" &
./ns3 run --no-build "rice_farm_wsn --experiment=4 --trafficScale=800 --pktSize=15000 --simTime=120" &
./ns3 run --no-build "rice_farm_wsn --experiment=4 --trafficScale=800 --pktSize=18000 --simTime=120" &
./ns3 run --no-build "rice_farm_wsn --experiment=4 --trafficScale=800 --pktSize=20000 --simTime=120" &
wait
echo "Done"
