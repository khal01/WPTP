/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
/////    Author: Khurshid Alam <khurshid.alam@web.de>       /////
/////    Date: 27.06.2018                                   /////
/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////

/***************************************************************/
/* -*- ___________________________________________________ -*- */
/////~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~/////
///___________________________________________________________///


#include "ns3/log.h"
#include "ns3/config.h"
#include "ns3/command-line.h"
#include "ns3/uinteger.h"
#include "ns3/boolean.h"
#include "ns3/string.h"
#include "ns3/yans-wifi-helper.h"
#include "ns3/mobility-helper.h"
#include "ns3/internet-stack-helper.h"

#include "ns3/ipv4-address-helper.h"
#include "ns3/address.h"

#include "ns3/packet-sink-helper.h"
#include "ns3/packet-sink.h"

#include "ns3/ssid.h"
#include "ns3/wifi-mac-header.h"


#include "ns3/application.h"
#include "ns3/event-id.h"
#include "ns3/ptr.h"
#include "ns3/data-rate.h"

#include "ns3/traced-callback.h"

#include <stdint.h>
#include "ns3/object-factory.h"
#include "ns3/attribute.h"
#include "ns3/net-device.h"

#include "ns3/on-off-helper.h"
#include "wptp-application.h"

#include "ns3/inet-socket-address.h"
#include "ns3/packet-socket-address.h"

#include "ns3/names.h"
#include "ns3/random-variable-stream.h"
#include "ns3/inet6-socket-address.h"
#include "ns3/node.h"
#include "ns3/nstime.h"
#include "ns3/socket.h"
#include "ns3/simulator.h"
#include "ns3/socket-factory.h"
#include "ns3/packet.h"
#include "ns3/trace-source-accessor.h"
#include "ns3/udp-socket-factory.h"
#include "ns3/pointer.h"


namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("WptpApplication");

NS_OBJECT_ENSURE_REGISTERED (WptpApplication);


TypeId
WptpApplication::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::WptpApplication")
    .SetParent<Application> ()
    .SetGroupName("Applications")
    .AddConstructor<WptpApplication> ()
    .AddAttribute ("DataRate", "The data rate in on state.",
                   DataRateValue (DataRate ("500kb/s")),
                   MakeDataRateAccessor (&WptpApplication::m_cbrRate),
                   MakeDataRateChecker ())
    .AddAttribute ("PacketSize", "The size of packets sent in on state",
                   UintegerValue (512),
                   MakeUintegerAccessor (&WptpApplication::m_pktSize),
                   MakeUintegerChecker<uint32_t> (1))
    .AddAttribute ("Remote", "The address of the destination",
                   AddressValue (),
                   MakeAddressAccessor (&WptpApplication::m_peer),
                   MakeAddressChecker ())
    .AddAttribute ("OnTime", "A RandomVariableStream used to pick the duration of the 'On' state.",
                   StringValue ("ns3::ConstantRandomVariable[Constant=1.0]"),
                   MakePointerAccessor (&WptpApplication::m_onTime),
                   MakePointerChecker <RandomVariableStream>())
    .AddAttribute ("OffTime", "A RandomVariableStream used to pick the duration of the 'Off' state.",
                   StringValue ("ns3::ConstantRandomVariable[Constant=1.0]"),
                   MakePointerAccessor (&WptpApplication::m_offTime),
                   MakePointerChecker <RandomVariableStream>())
    .AddAttribute ("MaxBytes", 
                   "The total number of bytes to send. Once these bytes are sent, "
                   "no packet is sent again, even in on state. The value zero means "
                   "that there is no limit.",
                   UintegerValue (0),
                   MakeUintegerAccessor (&WptpApplication::m_maxBytes),
                   MakeUintegerChecker<uint64_t> ())
    .AddAttribute ("Protocol", "The type of protocol to use. This should be "
                   "a subclass of ns3::SocketFactory",
                   TypeIdValue (UdpSocketFactory::GetTypeId ()),
                   MakeTypeIdAccessor (&WptpApplication::m_tid),
                   // This should check for SocketFactory as a parent
                   MakeTypeIdChecker ())
    .AddTraceSource ("Tx", "A new packet is created and is sent",
                     MakeTraceSourceAccessor (&WptpApplication::m_txTrace),
                     "ns3::Packet::TracedCallback")
    .AddTraceSource ("TxWithAddresses", "A new packet is created and is sent",
                     MakeTraceSourceAccessor (&WptpApplication::m_txTraceWithAddresses),
                     "ns3::Packet::TwoAddressTracedCallback")
  ;
  


  //////////////////////////////////////////////////////////////////////////////////////////
  NS_LOG_UNCOND("Initiate the WptpApplication and settings\n");
  //////////////////////////////////////////////////////////////////////////////////////////


  return tid;

}


WptpApplication::WptpApplication ()
  : m_socket (0),
    m_connected (false),
    m_residualBits (0),
    m_lastStartTime (Seconds (0)),
    m_totBytes (0)
{
  NS_LOG_FUNCTION (this);
}


WptpApplication::~WptpApplication()
{
  NS_LOG_FUNCTION (this);
}


void 
WptpApplication::SetMaxBytes (uint64_t maxBytes)
{
  NS_LOG_FUNCTION (this << maxBytes);
  m_maxBytes = maxBytes;
}


Ptr<Socket>
WptpApplication::GetSocket (void) const
{
  NS_LOG_FUNCTION (this);
  return m_socket;
}


int64_t 
WptpApplication::AssignStreams (int64_t stream)
{
  NS_LOG_FUNCTION (this << stream);
  m_onTime->SetStream (stream);
  m_offTime->SetStream (stream + 1);
  return 2;
}


void
WptpApplication::DoDispose (void)
{
  NS_LOG_FUNCTION (this);

  m_socket = 0;
  // chain up
  Application::DoDispose ();
}


// Application Methods
void WptpApplication::StartApplication () // Called at time specified by Start
{
  NS_LOG_FUNCTION (this);

  // Create the socket if not already
  if (!m_socket)
    {
      m_socket = Socket::CreateSocket (GetNode (), m_tid);
      if (Inet6SocketAddress::IsMatchingType (m_peer))
        {
          if (m_socket->Bind6 () == -1)
            {
              NS_FATAL_ERROR ("Failed to bind socket");
            }
        }
      else if (InetSocketAddress::IsMatchingType (m_peer) ||
               PacketSocketAddress::IsMatchingType (m_peer))
        {
          if (m_socket->Bind () == -1)
            {
              NS_FATAL_ERROR ("Failed to bind socket");
            }
        }
      m_socket->Connect (m_peer);
      m_socket->SetAllowBroadcast (true);
      m_socket->ShutdownRecv ();

      m_socket->SetConnectCallback (
        MakeCallback (&WptpApplication::ConnectionSucceeded, this),
        MakeCallback (&WptpApplication::ConnectionFailed, this));
    }
  m_cbrRateFailSafe = m_cbrRate;

  // Insure no pending event
  CancelEvents ();
  // If we are not yet connected, there is nothing to do here
  // The ConnectionComplete upcall will start timers at that time
  //if (!m_connected) return;
  ScheduleStartEvent ();



  //////////////////////////////////////////////////////////////////////////////////////////////
  NS_LOG_UNCOND("The Application is being started!\n");
  //////////////////////////////////////////////////////////////////////////////////////////////


}


void WptpApplication::StopApplication () // Called at time specified by Stop
{
  NS_LOG_FUNCTION (this);

  CancelEvents ();
  if(m_socket != 0)
    {
      m_socket->Close ();
    }
  else
    {
      NS_LOG_WARN ("WptpApplication found null socket to close in StopApplication");
    }
}


void WptpApplication::CancelEvents ()
{
  NS_LOG_FUNCTION (this);

  if (m_sendEvent.IsRunning () && m_cbrRateFailSafe == m_cbrRate )
    { // Cancel the pending send packet event
      // Calculate residual bits since last packet sent
      Time delta (Simulator::Now () - m_lastStartTime);
      int64x64_t bits = delta.To (Time::S) * m_cbrRate.GetBitRate ();
      m_residualBits += bits.GetHigh ();
    }
  m_cbrRateFailSafe = m_cbrRate;
  Simulator::Cancel (m_sendEvent);
  Simulator::Cancel (m_startStopEvent);
}


// Event handlers
void WptpApplication::StartSending ()
{
  NS_LOG_FUNCTION (this);
  m_lastStartTime = Simulator::Now ();
  ScheduleNextTx ();  // Schedule the send packet event
  ScheduleStopEvent ();
}


void WptpApplication::StopSending ()
{
  NS_LOG_FUNCTION (this);
  CancelEvents ();

  ScheduleStartEvent ();
}


// Private helpers
void WptpApplication::ScheduleNextTx ()
{
  NS_LOG_FUNCTION (this);

  if (m_maxBytes == 0 || m_totBytes < m_maxBytes)
    {
      uint32_t bits = m_pktSize * 8 - m_residualBits;
      NS_LOG_LOGIC ("bits = " << bits);
      Time nextTime (Seconds (bits /
                              static_cast<double>(m_cbrRate.GetBitRate ()))); // Time till next packet
      NS_LOG_LOGIC ("nextTime = " << nextTime);
      m_sendEvent = Simulator::Schedule (nextTime,
                                         &WptpApplication::SendPacket, this);
    }
  else
    { // All done, cancel any pending events
      StopApplication ();
    }
}


void WptpApplication::ScheduleStartEvent ()
{  // Schedules the event to start sending data (switch to the "On" state)
  NS_LOG_FUNCTION (this);

  Time offInterval = Seconds (m_offTime->GetValue ());
  NS_LOG_LOGIC ("start at " << offInterval);
  m_startStopEvent = Simulator::Schedule (offInterval, &WptpApplication::StartSending, this);
}


void WptpApplication::ScheduleStopEvent ()
{  // Schedules the event to stop sending data (switch to "Off" state)
  NS_LOG_FUNCTION (this);

  Time onInterval = Seconds (m_onTime->GetValue ());
  NS_LOG_LOGIC ("stop at " << onInterval);
  m_startStopEvent = Simulator::Schedule (onInterval, &WptpApplication::StopSending, this);
}


void WptpApplication::SendPacket ()
{
  NS_LOG_FUNCTION (this);

  NS_ASSERT (m_sendEvent.IsExpired ());
  Ptr<Packet> packet = Create<Packet> (m_pktSize);
  m_txTrace (packet);
  m_socket->Send (packet);
  m_totBytes += m_pktSize;
  Address localAddress;
  m_socket->GetSockName (localAddress);
  if (InetSocketAddress::IsMatchingType (m_peer))
    {
      NS_LOG_INFO ("At time " << Simulator::Now ().GetSeconds ()
                   << "s wptp application sent "
                   <<  packet->GetSize () << " bytes to "
                   << InetSocketAddress::ConvertFrom(m_peer).GetIpv4 ()
                   << " port " << InetSocketAddress::ConvertFrom (m_peer).GetPort ()
                   << " total Tx " << m_totBytes << " bytes");
      m_txTraceWithAddresses (packet, localAddress, InetSocketAddress::ConvertFrom (m_peer));




      ///////////////////////////////////////////////////////////////////
      NS_LOG_UNCOND ("IPv4 matched");
      NS_LOG_UNCOND (InetSocketAddress::ConvertFrom(m_peer).GetIpv4 ());
      ///////////////////////////////////////////////////////////////////



    }
  else if (Inet6SocketAddress::IsMatchingType (m_peer))
    {
      NS_LOG_INFO ("At time " << Simulator::Now ().GetSeconds ()
                   << "s on-off application sent "
                   <<  packet->GetSize () << " bytes to "
                   << Inet6SocketAddress::ConvertFrom(m_peer).GetIpv6 ()
                   << " port " << Inet6SocketAddress::ConvertFrom (m_peer).GetPort ()
                   << " total Tx " << m_totBytes << " bytes");
      m_txTraceWithAddresses (packet, localAddress, Inet6SocketAddress::ConvertFrom(m_peer));




      /////////////////////////////////////////////////////
      NS_LOG_UNCOND ("IPv4 matched");
      /////////////////////////////////////////////////////


    }
  m_lastStartTime = Simulator::Now ();
  m_residualBits = 0;
  ScheduleNextTx ();



  //////////////////////////////////////////////////////////////////////////////////////////////////
  NS_LOG_UNCOND ("this sendPacket is being called instead.....New...");
  NS_LOG_UNCOND (localAddress);
  //////////////////////////////////////////////////////////////////////////////////////////////////



}


void WptpApplication::ConnectionSucceeded (Ptr<Socket> socket)
{
  NS_LOG_FUNCTION (this << socket);
  m_connected = true;

}



void WptpApplication::ConnectionFailed (Ptr<Socket> socket)
{
  NS_LOG_FUNCTION (this << socket);

}


} // Namespace ns3

using namespace ns3;
NS_LOG_COMPONENT_DEFINE ("wptp-pcf");


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////**** WptpHelper class ***** ///////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class WptpHelper 
{
public:

  WptpHelper (std::string protocol, Address address);
  void SetAttribute (std::string name, const AttributeValue &value);
  //int64_t AssignStreams (NodeContainer c, int64_t stream);
  ApplicationContainer Install (Ptr<Node> node) const;
  
private:

  Ptr<WptpApplication> InstallPriv (Ptr<Node> node) const;
  ObjectFactory m_factory; //!< Object factory.
};

WptpHelper::WptpHelper (std::string protocol, Address address)
{
  
  m_factory.SetTypeId ("ns3::WptpApplication");
  m_factory.Set ("Protocol", StringValue (protocol));
  m_factory.Set ("Remote", AddressValue (address));
}

void 
WptpHelper::SetAttribute (std::string name, const AttributeValue &value)
{
  m_factory.Set (name, value);
}

ApplicationContainer
WptpHelper::Install (Ptr<Node> node) const
{
  
  return ApplicationContainer (InstallPriv (node));
}

Ptr<WptpApplication>
WptpHelper::InstallPriv (Ptr<Node> node) const
{
  Ptr<WptpApplication> app = m_factory.Create<WptpApplication> ();
  node->AddApplication (app);


  ////////////////////////////////////////////////////////////////
  NS_LOG_UNCOND("WptpApplication Created!");
  ////////////////////////////////////////////////////////////////

  return app;

}


/*
int64_t
wptpHelper::AssignStreams (NodeContainer c, int64_t stream)
{
  int64_t currentStream = stream;
  Ptr<Node> node;
  for (NodeContainer::Iterator i = c.Begin (); i != c.End (); ++i)
    {
      node = (*i);
      for (uint32_t j = 0; j < node->GetNApplications (); j++)
        {
          Ptr<WptpApplication> wptp = DynamicCast<WptpApplication> (node->GetApplication (j));
          if (wptp)
            {
              currentStream += wptp->AssignStreams (currentStream);
            }
        }
    }
  return (currentStream - stream);
}
*/


uint64_t m_countBeacon;
uint64_t m_countCfPoll;
uint64_t m_countCfPollAck;
uint64_t m_countCfPollData;
uint64_t m_countCfPollDataAck;
uint64_t m_countCfEnd;
uint64_t m_countCfEndAck;
uint64_t m_countDataNull;
uint64_t m_countData;

uint32_t nWifi = 3;

// Time vector to hold the different timestamps
//vector < Time > t1, t2, t3, t4;
//vector < vector <Time> > t1(nWifi), t11(nWifi), t2(nWifi), t3(nWifi), t4(nWifi);

void TxCallback (std::string context, Ptr<const Packet> p)
{
  WifiMacHeader hdr;
  p->PeekHeader (hdr);

  NS_LOG_UNCOND (hdr);
  
  if (hdr.IsBeacon ())
    {
      m_countBeacon++;
    }
  else if (hdr.IsCfPoll ())
    {
      if (hdr.IsCfAck () && hdr.HasData  ())
        {
          m_countCfPollDataAck++;
        }
      else if (!hdr.IsCfAck () && hdr.HasData ())
        {
          m_countCfPollData++;
        }
      else if (hdr.IsCfAck () && !hdr.HasData ())
        {
          m_countCfPollAck++;
        }
      else
        {
          m_countCfPoll++;
        }

     // t1.push_back(NanoSeconds(Simulator::Now()));

    }
  else if (hdr.IsCfEnd ())
    {
      if (hdr.IsCfAck ())
        {
          m_countCfEndAck++;
        }
      else
        {
          m_countCfEnd++;
        }
    }
  else if (hdr.IsData ())
    {
      if (!hdr.HasData ())
        {
          m_countDataNull++;
        }
      else
        {
          m_countData++;
        }

     // t3.push_back(NanoSeconds(Simulator::Now()));
    }





    //////////////////////////////////////////
    NS_LOG_UNCOND ("In TxCallback()");
    //////////////////////////////////////////
}

/*******************
void RxCallback (std::string context, Ptr<const Packet> p)
{
  





  	//////////////////////////////////////////
    NS_LOG_UNCOND ("In RxCallback()");
    //////////////////////////////////////////
}

********************/

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//
//	Create the simulation settings!
//
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



int
main (int argc, char *argv[])
{
  
  bool enablePcap = true;
  bool enablePcf = true;
  bool withData = true;
  std::string trafficDirection = "upstream";
  uint64_t cfpMaxDurationUs = 65536; //microseconds
  double simulationTime = 10; //seconds
  
  CommandLine cmd;
  
  cmd.AddValue ("nWifi", "Number of wifi STA devices", nWifi);
  cmd.AddValue ("enablePcf", "Enable/disable PCF mode", enablePcf);
  cmd.AddValue ("withData", "Enable/disable UDP data packets generation", withData);
  cmd.AddValue ("trafficDirection", "Data traffic direction (if withData is set to 1):\
                upstream (all STAs -> AP) or downstream (AP -> all STAs)", trafficDirection);

  cmd.AddValue ("cfpMaxDuration", "CFP max duration in microseconds", cfpMaxDurationUs);
  cmd.AddValue ("simulationTime", "Simulation time in seconds", simulationTime);
  cmd.AddValue ("enablePcap", "Enable/disable PCAP output", enablePcap);
  
  cmd.Parse (argc, argv);
  
  m_countBeacon = 0;
  m_countCfEnd = 0;
  m_countCfEndAck = 0;
  m_countCfPoll = 0;
  m_countCfPollAck = 0;
  m_countCfPollData = 0;
  m_countCfPollDataAck = 0;
  m_countDataNull = 0;
  m_countData = 0;
  m_countDataNull = 0;
  m_countData = 0;

  NodeContainer wifiStaNodes;
  wifiStaNodes.Create (nWifi);

  NodeContainer wifiApNode;
  wifiApNode.Create (1);

  YansWifiChannelHelper channel = YansWifiChannelHelper::Default ();
  YansWifiPhyHelper phy = YansWifiPhyHelper::Default ();
  phy.SetPcapDataLinkType (YansWifiPhyHelper::DLT_IEEE802_11_RADIO);
  phy.SetChannel (channel.Create ());

  WifiHelper wifi;
  //wifi.SetStandard(WIFI_PHY_STANDARD_80211ac);

  WifiMacHelper mac;
  
  Ssid ssid = Ssid ("wptp-pcf");
  
  wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager", "DataMode", StringValue ("OfdmRate54Mbps"), "ControlMode", StringValue ("OfdmRate24Mbps"));

  NetDeviceContainer staDevices, apDevice;
  
  mac.SetType ("ns3::StaWifiMac",
               "Ssid", SsidValue (ssid),
               "ActiveProbing", BooleanValue (false),
               "PcfSupported", BooleanValue (enablePcf));
  
  staDevices = wifi.Install (phy, mac, wifiStaNodes);

  mac.SetType ("ns3::ApWifiMac",
               "Ssid", SsidValue (ssid),
               "BeaconGeneration", BooleanValue (true),
               "PcfSupported", BooleanValue (enablePcf),
               "CfpMaxDuration", TimeValue (MicroSeconds (cfpMaxDurationUs)));
  
  apDevice = wifi.Install (phy, mac, wifiApNode);

  MobilityHelper mobility;
  Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator> ();
  for (uint32_t i = 0; i < nWifi; i++)
    {
      positionAlloc->Add (Vector (1.0, 0.0, 0.0));
    }

  positionAlloc->Add (Vector (0.0, 0.0, 0.0));
  mobility.SetPositionAllocator (positionAlloc);
  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.Install (wifiApNode);
  mobility.Install (wifiStaNodes);

  InternetStackHelper stack;
  stack.Install (wifiApNode);
  stack.Install (wifiStaNodes);

  Ipv4AddressHelper address;

  address.SetBase ("10.1.1.0", "255.255.255.0");
  Ipv4InterfaceContainer StaInterface;
  StaInterface = address.Assign (staDevices);
  Ipv4InterfaceContainer ApInterface;
  ApInterface = address.Assign (apDevice);
  ApplicationContainer sourceApplications, sinkApplications;
  
  if (withData)
    {
      uint32_t portNumber = 9;
  
      for (uint32_t index = 0; index < nWifi; ++index)
        {
  
          auto ipv4 = (trafficDirection == "upstream") ? wifiApNode.Get (0)->GetObject<Ipv4> () : wifiStaNodes.Get (index)->GetObject<Ipv4> ();
          const auto address = ipv4->GetAddress (1, 0).GetLocal ();
          InetSocketAddress sinkSocket (address, portNumber++);
  
          WptpHelper ptpHelper ("ns3::UdpSocketFactory", sinkSocket);
          ptpHelper.SetAttribute ("OnTime", StringValue ("ns3::ConstantRandomVariable[Constant=1]"));
          ptpHelper.SetAttribute ("OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=0]"));
          ptpHelper.SetAttribute ("DataRate", DataRateValue (50000000 / nWifi));
          ptpHelper.SetAttribute ("PacketSize", UintegerValue (1000)); //bytes (default 1472)
  
          PacketSinkHelper packetSinkHelper ("ns3::UdpSocketFactory", sinkSocket);
  
          if (trafficDirection == "upstream")
            {
  
              sourceApplications.Add (ptpHelper.Install (wifiStaNodes.Get (index)));
              sinkApplications.Add (packetSinkHelper.Install (wifiApNode.Get (0)));
  
            }
  
          else if (trafficDirection == "downstream")
            {
   
              sinkApplications.Add (packetSinkHelper.Install (wifiStaNodes.Get (index)));
              sourceApplications.Add (ptpHelper.Install (wifiApNode.Get (0)));
   
            }
   
          else
            {
   
              NS_ASSERT_MSG (false, "Invalid trafficDirection!");
            }
        }
   
      sinkApplications.Start (Seconds (0.0));
      sinkApplications.Stop (Seconds (simulationTime + 1));
      sourceApplications.Start (Seconds (1.0));
      sourceApplications.Stop (Seconds (simulationTime + 1));
   
    }

  Config::Connect ("/NodeList/*/DeviceList/*/$ns3::WifiNetDevice/Phy/$ns3::WifiPhy/PhyTxBegin", MakeCallback (&TxCallback));
  //Config::Connect ("/NodeList/*/DeviceList/*/$ns3::WifiNetDevice/Phy/$ns3::WifiPhy/PhyRxBegin", MakeCallback (&RxCallback));

  if (enablePcap)
    {
   
      phy.EnablePcap ("wptp-pcf", apDevice.Get (0));
    }

  Simulator::Stop (Seconds (simulationTime + 1));
  Simulator::Run ();




  double throughput = 0;
  for (uint32_t index = 0; index < sinkApplications.GetN (); ++index)
    {
      uint64_t totalPacketsThrough = DynamicCast<PacketSink> (sinkApplications.Get (index))->GetTotalRx ();
      throughput += ((totalPacketsThrough * 8) / (simulationTime * 1000000.0)); //Mbit/s
    }

  std::cout << "Throughput: " << throughput << " Mbit/s" << std::endl;

  std::cout << "# tx beacons: " << m_countBeacon << std::endl;
  std::cout << "# tx CF-END: " << m_countCfEnd << std::endl;
  std::cout << "# tx CF-END-ACK: " << m_countCfEndAck << std::endl;
  std::cout << "# tx CF-POLL: " << m_countCfPoll << std::endl;
  std::cout << "# tx CF-POLL-ACK: " << m_countCfPollAck << std::endl;
  std::cout << "# tx CF-POLL-DATA: " << m_countCfPollData << std::endl;
  std::cout << "# tx CF-POLL-DATA-ACK: " << m_countCfPollDataAck << std::endl;
  std::cout << "# tx DATA-NULL: " << m_countDataNull << std::endl;
  std::cout << "# tx DATA: " << m_countData << std::endl;

  Simulator::Destroy ();



  return 0;
}