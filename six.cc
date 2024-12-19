#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/csma-module.h"
#include "ns3/applications-module.h"
#include "ns3/netanim-module.h"
#include "ns3/trace-helper.h"
#include "ns3/packet.h"

// Default Network Topology with 4 nodes in LAN using CSMA
//
//       10.1.1.0
// n0 -------------- n1 -------------- n2 -------------- n3
//         CSMA              CSMA            CSMA
//

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("CSMAScriptExample");

int main (int argc, char *argv[])
{
  CommandLine cmd (__FILE__);
  cmd.Parse (argc, argv);
  
  Time::SetResolution (Time::NS);
  LogComponentEnable ("UdpEchoClientApplication", LOG_LEVEL_INFO);
  LogComponentEnable ("UdpEchoServerApplication", LOG_LEVEL_INFO);

  // Create four nodes
  NodeContainer nodes;
  nodes.Create (4);

  // Set up CSMA (Carrier Sense Multiple Access) Network
  CsmaHelper csma;
  csma.SetChannelAttribute ("DataRate", StringValue ("5Mbps"));
  csma.SetChannelAttribute ("Delay", StringValue ("2ms"));
  csma.SetDeviceAttribute ("Mtu", UintegerValue (1500)); // Standard MTU size for Ethernet

  NetDeviceContainer devices;
  devices = csma.Install (nodes);

  // Install Internet Stack (TCP/IP) to the nodes
  InternetStackHelper stack;
  stack.Install (nodes);

  // Assign IP addresses to the nodes
  Ipv4AddressHelper address;
  address.SetBase ("10.1.1.0", "255.255.255.0");
  Ipv4InterfaceContainer interfaces = address.Assign (devices);

  // Set up UDP Echo Server on node 2
  UdpEchoServerHelper echoServer (9); // Server port 9
  ApplicationContainer serverApps = echoServer.Install (nodes.Get (2));
  serverApps.Start (Seconds (1.0)); // Start time
  serverApps.Stop (Seconds (10.0)); // End time

  // Set up UDP Echo Client on node 0 (sends packets to node 2)
  UdpEchoClientHelper echoClient (interfaces.GetAddress (2), 9); // Target server at node 2
  echoClient.SetAttribute ("MaxPackets", UintegerValue (1)); // Send 1 packet
  echoClient.SetAttribute ("Interval", TimeValue (Seconds (1.0))); // Interval between packets
  echoClient.SetAttribute ("PacketSize", UintegerValue (1024)); // Packet size

  ApplicationContainer clientApps = echoClient.Install (nodes.Get (0));
  clientApps.Start (Seconds (2.0)); // Start time
  clientApps.Stop (Seconds (10.0)); // End time

  // Set node positions for animation (NetAnim)
  Ptr<Node> n0 = nodes.Get (0);
  Ptr<Node> n1 = nodes.Get (1);
  Ptr<Node> n2 = nodes.Get (2);
  Ptr<Node> n3 = nodes.Get (3);

  // Set up NetAnim visualization
  AnimationInterface anim("Myvissss.xml");
  anim.SetConstantPosition(n0, 100, 400);
  anim.SetConstantPosition(n1, 300, 400);
  anim.SetConstantPosition(n2, 500, 400);
  anim.SetConstantPosition(n3, 700, 400);

  // Trace Packet Information (Optional for packet analysis in Wireshark)
  AsciiTraceHelper ascii;
  csma.EnableAsciiAll(ascii.CreateFileStream("csma-trace.tr"));
  csma.EnablePcapAll("csma-pcap");

  // Run the simulator
  Simulator::Run ();
  Simulator::Destroy ();
  return 0;
}

