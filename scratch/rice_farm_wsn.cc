// rice_farm_wsn.cc

#include "ns3/aodv-module.h"
#include "ns3/applications-module.h"
#include "ns3/core-module.h"
#include "ns3/energy-module.h"
#include "ns3/flow-monitor-module.h"
#include "ns3/internet-module.h"
#include "ns3/mobility-module.h"
#include "ns3/network-module.h"
#include "ns3/wifi-module.h"
#include <cmath>
#include <vector>

using namespace ns3;
using namespace ns3::energy;

NS_LOG_COMPONENT_DEFINE("RiceFarmWSN");

// ─── Simulation Parameters ────────────────────────────────────────────────────
static const uint32_t N_TOTAL     = 25;
static const uint32_t N_ROUTERS   = 10;
static const uint32_t N_MCP       = 9;
static const uint32_t N_LEAF      = 4;
static const uint32_t N_SOLAR     = 1;
static const uint32_t COORDINATOR = 0;

static const double   TX_INTERVAL_MCP = 300.0;
static const double   TX_INTERVAL_LW  = 600.0;
static const double   TX_INTERVAL_SOL = 300.0;
static const double   TX_RANGE        = 30.0;

// Router node indices for sequential failures in experiment 3.
// Ordered to fail the most critical relay routers first:
// R1(5,8) and R2(19,8) are closest to coordinator and relay most traffic;
// R5(12,25) and R3(33,8) bridge mid-field; remainder isolate far sensors.
static const uint32_t FAIL_NODES[] = { 1, 2, 5, 3, 6, 4, 7, 8, 9, 10 };

// ─── Node coordinates (x, y) metres ──────────────────────────────────────────
static const double NODE_X[] = {
    1,
    5, 19, 33, 47, 12, 26, 40,  5, 26, 47,   // R1-R10
    8, 25, 42,  8, 25, 42,  8, 25, 42,        // M1-M9
    12, 38, 12, 38,                            // L1-L4
    25                                         // S1
};
static const double NODE_Y[] = {
    1,
    8,  8,  8,  8, 25, 25, 25, 42, 42, 42,
    8,  8,  8, 25, 25, 25, 42, 42, 42,
    12, 12, 38, 38,
    25
};


// ─── Global experiment parameters ────────────────────────────────────────────
static uint32_t g_experiment   = 0;
static uint32_t g_extraNodes   = 5;
static uint32_t g_interferers  = 1;
static uint32_t g_failNodes    = 1;
static uint32_t g_trafficScale = 1;
static uint32_t g_pktSize      = 128;
static double   g_simTime      = 900.0;

// ─── Global packet counters (written by callbacks) ───────────────────────────
static uint64_t g_txPackets = 0;
static uint64_t g_rxPackets = 0;

// ─── Spread N points evenly across the field ─────────────────────────────────
std::vector<Vector> GeneratePositions(uint32_t n,
                                      double fieldSize = 50.0,
                                      double margin    = 2.0)
{
    std::vector<Vector> pos;
    uint32_t cols  = (uint32_t)std::ceil(std::sqrt((double)n));
    uint32_t rows  = (uint32_t)std::ceil((double)n / cols);
    double   xStep = (fieldSize - 2.0 * margin) / std::max(cols - 1u, 1u);
    double   yStep = (fieldSize - 2.0 * margin) / std::max(rows - 1u, 1u);
    for (uint32_t r = 0; r < rows && (uint32_t)pos.size() < n; r++)
        for (uint32_t c = 0; c < cols && (uint32_t)pos.size() < n; c++)
            pos.emplace_back(margin + c * xStep, margin + r * yStep, 0.0);
    return pos;
}

// ─── Fail a single node ───────────────────────────────────────────────────────
// Completely silences a node by:
//   1. Stopping all applications
//   2. Bringing down all IPv4 interfaces (stops AODV routing)
//   3. Moving the node out of TX range of all other nodes (to position -1000,-1000)
//      This is the most reliable way to isolate a node in ns-3 since PHY-level
//      disable methods (SetSleepMode, SetOffMode) don't fully stop forwarding.
void FailNode(Ptr<Node> node)
{
    // Stop all applications
    for (uint32_t i = 0; i < node->GetNApplications(); i++)
        node->GetApplication(i)->SetStopTime(Simulator::Now());

    // Bring down all IPv4 interfaces so AODV stops routing through this node
    Ptr<Ipv4> ipv4 = node->GetObject<Ipv4>();
    if (ipv4)
        for (uint32_t i = 1; i < ipv4->GetNInterfaces(); i++)
            ipv4->SetDown(i);

    // Move node far out of range — guaranteed to break all radio links
    Ptr<MobilityModel> mob = node->GetObject<MobilityModel>();
    if (mob)
        mob->SetPosition(Vector(-10000.0, -10000.0, 0.0));
}

// ─── Tx callback ─────────────────────────────────────────────────────────────
// UdpClient "Tx" trace signature: void(Ptr<const Packet>)
void TxCallback(Ptr<const Packet> /*pkt*/)
{
    g_txPackets++;
}

// ─── Rx callback ─────────────────────────────────────────────────────────────
// UdpServer "RxWithAddresses" trace signature:
//   void(Ptr<const Packet>, const Address&, const Address&)
void RxCallback(Ptr<const Packet> /*pkt*/,
                const Address&    /*local*/,
                const Address&    /*remote*/)
{
    g_rxPackets++;
}

// ─── Install sensor application on a node ────────────────────────────────────
void InstallSensorApp(Ptr<Node>   node,
                      Ipv4Address dst,
                      uint16_t    port,
                      double      startTime,
                      double      stopTime,
                      double      interval)
{
    UdpClientHelper client(dst, port);
    client.SetAttribute("MaxPackets", UintegerValue(0xFFFFFFFF));
    client.SetAttribute("Interval",   TimeValue(Seconds(interval)));
    client.SetAttribute("PacketSize", UintegerValue(g_pktSize));

    ApplicationContainer app = client.Install(node);
    app.Start(Seconds(startTime));
    app.Stop(Seconds(stopTime));

    // Correct signature: void(Ptr<const Packet>)
    app.Get(0)->TraceConnectWithoutContext("Tx", MakeCallback(&TxCallback));
}

// ─── Main ─────────────────────────────────────────────────────────────────────
int main(int argc, char* argv[])
{
    CommandLine cmd;
    cmd.AddValue("experiment",
                 "0=baseline 1=addNodes 2=interference 3=nodeFailure 4=highTraffic",
                 g_experiment);
    cmd.AddValue("extraNodes",
                 "Exp 1: number of extra sensor nodes to add (default 5)",
                 g_extraNodes);
    cmd.AddValue("interferers",
                 "Exp 2: number of interfering nodes (default 1, max 5)",
                 g_interferers);
    cmd.AddValue("failNodes",
                 "Exp 3: number of routers to fail sequentially (1-5, default 1)",
                 g_failNodes);
    cmd.AddValue("trafficScale",
                 "Exp 4: TX interval divisor — higher = more traffic (try 1-500)",
                 g_trafficScale);
    cmd.AddValue("pktSize",
                 "Payload size in bytes for all sensor packets (default 128)",
                 g_pktSize);
    cmd.AddValue("simTime",
                 "Simulation duration in seconds (default 900, use 120 for exp4 high-load)",
                 g_simTime);
    cmd.Parse(argc, argv);

    // ── Parameter guards ──────────────────────────────────────────────────────
    if (g_trafficScale == 0) g_trafficScale = 1;
    if (g_interferers  > 5)  g_interferers  = 5;
    if (g_failNodes    < 1)  g_failNodes    = 1;
    if (g_failNodes    > 10) g_failNodes    = 10;
    if (g_pktSize      < 1)  g_pktSize      = 1;
    if (g_simTime      < 10) g_simTime      = 10;

    // ── Nodes ──────────────────────────────────────────────────────────────────
    NodeContainer allNodes;
    allNodes.Create(N_TOTAL);

    NodeContainer extraNodeContainer;
    if (g_experiment == 1 && g_extraNodes > 0)
        extraNodeContainer.Create(g_extraNodes);

    // ── Mobility ───────────────────────────────────────────────────────────────
    MobilityHelper mobility;
    mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");

    Ptr<ListPositionAllocator> posAlloc = CreateObject<ListPositionAllocator>();
    for (uint32_t i = 0; i < N_TOTAL; i++)
        posAlloc->Add(Vector(NODE_X[i], NODE_Y[i], 0.0));
    mobility.SetPositionAllocator(posAlloc);
    mobility.Install(allNodes);

    if (g_experiment == 1 && g_extraNodes > 0)
    {
        auto positions = GeneratePositions(g_extraNodes);
        Ptr<ListPositionAllocator> ep = CreateObject<ListPositionAllocator>();
        for (auto& v : positions) ep->Add(v);
        mobility.SetPositionAllocator(ep);
        mobility.Install(extraNodeContainer);
        for (uint32_t i = 0; i < extraNodeContainer.GetN(); i++)
            allNodes.Add(extraNodeContainer.Get(i));
    }

    // ── WiFi PHY ───────────────────────────────────────────────────────────────
    WifiHelper wifi;
    wifi.SetStandard(WIFI_STANDARD_80211b);
    wifi.SetRemoteStationManager("ns3::ConstantRateWifiManager",
                                 "DataMode",    StringValue("DsssRate1Mbps"),
                                 "ControlMode", StringValue("DsssRate1Mbps"));

    WifiMacHelper mac;
    mac.SetType("ns3::AdhocWifiMac");

    YansWifiPhyHelper phy;
    YansWifiChannelHelper channel = YansWifiChannelHelper::Default();
    channel.SetPropagationDelay("ns3::ConstantSpeedPropagationDelayModel");
    channel.AddPropagationLoss("ns3::RangePropagationLossModel",
                               "MaxRange", DoubleValue(TX_RANGE));
    Ptr<YansWifiChannel> wifiChannel = channel.Create();
    phy.SetChannel(wifiChannel);

    NetDeviceContainer devices = wifi.Install(phy, mac, allNodes);

    // ── Interference (experiment 2) ────────────────────────────────────────────
    // Interferers clustered near the centre of the field to maximise disruption
    static const double IPOS_X[] = { 15, 20, 10, 20, 15 };
    static const double IPOS_Y[] = { 15, 20, 20, 10, 10 };

    NodeContainer interfererNodes;
    if (g_experiment == 2 && g_interferers > 0)
    {
        interfererNodes.Create(g_interferers);

        Ptr<ListPositionAllocator> iPos = CreateObject<ListPositionAllocator>();
        for (uint32_t i = 0; i < g_interferers; i++)
            iPos->Add(Vector(IPOS_X[i], IPOS_Y[i], 0.0));
        mobility.SetPositionAllocator(iPos);
        mobility.Install(interfererNodes);

        YansWifiPhyHelper iPhy;
        iPhy.SetChannel(wifiChannel);   // same channel → causes interference
        WifiMacHelper iMac;
        iMac.SetType("ns3::AdhocWifiMac");
        NetDeviceContainer iDevices = wifi.Install(iPhy, iMac, interfererNodes);

        InternetStackHelper iInternet;
        iInternet.Install(interfererNodes);
        Ipv4AddressHelper iAddr;
        iAddr.SetBase("10.2.0.0", "255.255.0.0");
        iAddr.Assign(iDevices);

        OnOffHelper flood("ns3::UdpSocketFactory",
                          InetSocketAddress(Ipv4Address("10.2.255.255"), 9999));
        flood.SetConstantRate(DataRate("50Kbps"), 128);
        flood.SetAttribute("OnTime",
                           StringValue("ns3::ConstantRandomVariable[Constant=1]"));
        flood.SetAttribute("OffTime",
                           StringValue("ns3::ConstantRandomVariable[Constant=0]"));
        ApplicationContainer floodApps = flood.Install(interfererNodes);
        floodApps.Start(Seconds(1.0));
        floodApps.Stop(Seconds(g_simTime));
    }

    // ── Internet + AODV ───────────────────────────────────────────────────────
    AodvHelper aodv;
    InternetStackHelper internet;
    internet.SetRoutingHelper(aodv);
    internet.Install(allNodes);

    Ipv4AddressHelper address;
    address.SetBase("10.1.0.0", "255.255.0.0");
    Ipv4InterfaceContainer interfaces = address.Assign(devices);
    Ipv4Address coordAddr = interfaces.GetAddress(COORDINATOR);

    // ── Energy ─────────────────────────────────────────────────────────────────
    BasicEnergySourceHelper energySrcHelper;
    energySrcHelper.Set("BasicEnergySourceInitialEnergyJ", DoubleValue(100.0));
    EnergySourceContainer energySources = energySrcHelper.Install(allNodes);

    // Install radio energy models (return value intentionally discarded)
    WifiRadioEnergyModelHelper radioEnergyHelper;
    radioEnergyHelper.Install(devices, energySources);

    // ── Coordinator sink ───────────────────────────────────────────────────────
    uint16_t sinkPort = 9;
    UdpServerHelper server(sinkPort);
    ApplicationContainer serverApp = server.Install(allNodes.Get(COORDINATOR));
    serverApp.Start(Seconds(0.0));
    serverApp.Stop(Seconds(g_simTime));
    serverApp.Get(0)->TraceConnectWithoutContext(
        "RxWithAddresses", MakeCallback(&RxCallback));

    // ── Sensor applications ────────────────────────────────────────────────────
    double mcpInterval  = TX_INTERVAL_MCP / (double)g_trafficScale;
    double leafInterval = TX_INTERVAL_LW  / (double)g_trafficScale;
    double solInterval  = TX_INTERVAL_SOL / (double)g_trafficScale;

    uint32_t sBase = 1 + N_ROUTERS;   // index of first sensor node in allNodes

    // MCP sensors
    for (uint32_t i = 0; i < N_MCP; i++)
        InstallSensorApp(allNodes.Get(sBase + i),
                         coordAddr, sinkPort,
                         10.0 + i * 5.0, g_simTime, mcpInterval);

    // Leaf-wetness sensors
    for (uint32_t i = 0; i < N_LEAF; i++)
        InstallSensorApp(allNodes.Get(sBase + N_MCP + i),
                         coordAddr, sinkPort,
                         15.0 + i * 7.0, g_simTime, leafInterval);

    // Solar irradiance sensor
    InstallSensorApp(allNodes.Get(sBase + N_MCP + N_LEAF),
                     coordAddr, sinkPort,
                     20.0, g_simTime, solInterval);

    // Extra nodes (experiment 1)
    if (g_experiment == 1 && g_extraNodes > 0)
    {
        for (uint32_t i = N_TOTAL; i < allNodes.GetN(); i++)
            InstallSensorApp(allNodes.Get(i),
                             coordAddr, sinkPort,
                             25.0 + (i - N_TOTAL) * 3.0, g_simTime, mcpInterval);
    }

    // ── Node failure (experiment 3) ────────────────────────────────────────────
    if (g_experiment == 3)
    {
        double spacing = g_simTime / (double)(g_failNodes + 1);
        for (uint32_t f = 0; f < g_failNodes; f++)
        {
            double    failTime = spacing * (f + 1);
            Ptr<Node> fNode    = allNodes.Get(FAIL_NODES[f]);
            Simulator::Schedule(Seconds(failTime), &FailNode, fNode);
        }
    }

    // ── Flow monitor ───────────────────────────────────────────────────────────
    FlowMonitorHelper flowMonHelper;
    Ptr<FlowMonitor>  flowMonitor = flowMonHelper.InstallAll();

    // ── Run ────────────────────────────────────────────────────────────────────
    Simulator::Stop(Seconds(g_simTime));
    Simulator::Run();

    // ── Statistics ─────────────────────────────────────────────────────────────
    flowMonitor->CheckForLostPackets();
    Ptr<Ipv4FlowClassifier> classifier =
        DynamicCast<Ipv4FlowClassifier>(flowMonHelper.GetClassifier());
    FlowMonitor::FlowStatsContainer stats = flowMonitor->GetFlowStats();

    double   totalTx = 0, totalRx = 0, totalDelay = 0, totalTput = 0;
    uint32_t flowCount = 0;

    for (auto& [id, fs] : stats)
    {
        Ipv4FlowClassifier::FiveTuple t = classifier->FindFlow(id);
        if (t.destinationAddress != coordAddr) continue;
        totalTx    += fs.txPackets;
        totalRx    += fs.rxPackets;
        totalDelay += (fs.rxPackets > 0) ? fs.delaySum.GetSeconds() : 0.0;
        totalTput  += fs.rxBytes * 8.0 / g_simTime / 1e3;
        flowCount++;
    }

    double aggPDR   = (totalTx > 0) ? 100.0 * totalRx / totalTx : 0.0;
    double aggPLR   = 100.0 - aggPDR;
    double aggDelay = (totalRx > 0) ? totalDelay / totalRx * 1000.0 : 0.0;

    double totalInitial = 0, totalRemaining = 0;
    for (uint32_t i = 0; i < energySources.GetN(); i++)
    {
        Ptr<BasicEnergySource> src =
            DynamicCast<BasicEnergySource>(energySources.Get(i));
        totalInitial   += src->GetInitialEnergy();
        totalRemaining += src->GetRemainingEnergy();
    }
    double consumed = totalInitial - totalRemaining;

    // ── Human-readable summary ─────────────────────────────────────────────────
    std::cerr << "\n========== AGGREGATE RESULTS (experiment=" << g_experiment;
    if (g_experiment == 1) std::cerr << " extraNodes="   << g_extraNodes;
    if (g_experiment == 2) std::cerr << " interferers="  << g_interferers;
    if (g_experiment == 3) std::cerr << " failNodes="    << g_failNodes;
    if (g_experiment == 4) std::cerr << " trafficScale=" << g_trafficScale;
    std::cerr << ") ==========\n"
              << "  Sim Time              : " << g_simTime   << " s\n"
              << "  Flows                 : " << flowCount           << "\n"
              << "  Total TX packets      : " << (uint64_t)totalTx   << "\n"
              << "  Total RX packets      : " << (uint64_t)totalRx   << "\n"
              << "  Packet Delivery Ratio : " << aggPDR   << " %\n"
              << "  Packet Loss Rate      : " << aggPLR   << " %\n"
              << "  Avg End-to-End Delay  : " << aggDelay << " ms\n"
              << "  Total Throughput      : " << totalTput << " kbps\n"
              << "  Offered Load          : "
              << (g_trafficScale * 14.0 * g_pktSize * 8.0 / (TX_INTERVAL_MCP * 1000.0))
              << " kbps\n"
              << "  Packet Size           : " << g_pktSize << " bytes\n"
              << "  Energy Consumed       : " << consumed  << " J"
              << "  (" << consumed / energySources.GetN() << " J/node avg)\n"
              << "=============================================================\n";

    Simulator::Destroy();
    return 0;
}
