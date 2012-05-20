///////////////////////////////////////////////////////////////////////
// Wendy network library
// Copyright (c) 2012 Camilla Berglund <elmindreda@elmindreda.org>
//
// This software is provided 'as-is', without any express or implied
// warranty. In no event will the authors be held liable for any
// damages arising from the use of this software.
//
// Permission is granted to anyone to use this software for any
// purpose, including commercial applications, and to alter it and
// redistribute it freely, subject to the following restrictions:
//
//  1. The origin of this software must not be misrepresented; you
//     must not claim that you wrote the original software. If you use
//     this software in a product, an acknowledgment in the product
//     documentation would be appreciated but is not required.
//
//  2. Altered source versions must be plainly marked as such, and
//     must not be misrepresented as being the original software.
//
//  3. This notice may not be removed or altered from any source
//     distribution.
//
///////////////////////////////////////////////////////////////////////

#include <wendy/Core.h>
#include <wendy/Network.h>

#include <enet/enet.h>

#include <cstring>

//////////////////////////////////////////////////////////////////////

namespace wendy
{
  namespace net
  {

///////////////////////////////////////////////////////////////////////

bool initialize()
{
  return enet_initialize() == 0;
}

void shutdown()
{
  enet_deinitialize();
}

///////////////////////////////////////////////////////////////////////

PacketData::PacketData(void* data, size_t capacity, size_t size):
  data(static_cast<uint8*>(data)),
  capacity(capacity),
  size(size),
  offset(0)
{
}

uint8 PacketData::read8()
{
  if (offset + 1 > size)
    panic("Packet data buffer underflow");

  return data[offset++];
}

uint16 PacketData::read16()
{
  if (offset + 2 > size)
    panic("Packet data buffer underflow");

  uint16 value = ntohs(*(uint16*) (data + offset));
  offset += 2;
  return value;
}

uint32 PacketData::read32()
{
  if (offset + 4 > size)
    panic("Packet data buffer underflow");

  uint32 value = ntohl(*(uint32*) (data + offset));
  offset += 4;
  return value;
}

float PacketData::read32f()
{
  return uintBitsToFloat(read32());
}

template <>
void PacketData::read(String& value)
{
  if (!std::memchr(data + offset, '\0', size - offset))
    panic("Missing null character in packet data");

  value.assign((char*) (data + offset));
  offset += value.length() + 1;
}

void PacketData::write8(uint8 value)
{
  if (size + 1 > capacity)
    panic("Packet data buffer overflow");

  data[size++] = value;
}

void PacketData::write16(uint16 value)
{
  if (size + 2 > capacity)
    panic("Packet data buffer overflow");

  *((uint16*) (data + size)) = htons(value);
  size += 2;
}

void PacketData::write32(uint32 value)
{
  if (size + 4 > capacity)
    panic("Packet data buffer overflow");

  *((uint32*) (data + size)) = htonl(value);
  size += 4;
}

void PacketData::write32f(float value)
{
  write32(floatBitsToUint(value));
}

template <>
void PacketData::write(const String& value)
{
  for (auto c = value.begin();  c != value.end();  c++)
    write8(*c);

  write8('\0');
}

bool PacketData::isEmpty()
{
  return offset == size;
}

size_t PacketData::getSize() const
{
  return size;
}

const void* PacketData::getData() const
{
  return data;
}

///////////////////////////////////////////////////////////////////////

bool Peer::sendPacket(ChannelID channel,
                      PacketType type,
                      const PacketData& data)
{
  uint32 flags = 0;

  if (type == RELIABLE)
    flags |= ENET_PACKET_FLAG_RELIABLE;
  else
  {
    if (type == UNSEQUENCED)
      flags |= ENET_PACKET_FLAG_UNSEQUENCED;

    flags |= ENET_PACKET_FLAG_NO_ALLOCATE;
  }

  ENetPacket* packet = enet_packet_create(data.getData(), data.getSize(), flags);
  if (!packet)
  {
    logError("Failed to create ENet packet");
    return false;
  }

  if (enet_peer_send((ENetPeer*) peer, channel, packet) < 0)
  {
    logError("Failed to send ENet packet to peer %s", name.c_str());
    return false;
  }

  return true;
}

void Peer::disconnect(uint32 reason)
{
  enet_peer_disconnect((ENetPeer*) peer, reason);
}

bool Peer::isClient() const
{
  return ID != SERVER;
}

bool Peer::isServer() const
{
  return ID == SERVER;
}

TargetID Peer::getTargetID() const
{
  return ID;
}

const String& Peer::getName() const
{
  return name;
}

uint32 Peer::getAddress() const
{
  return ((ENetPeer*) peer)->address.host;
}

Time Peer::getRoundTripTime() const
{
  return (Time) ((ENetPeer*) peer)->roundTripTime / 1000.0;
}

Peer::Peer(void* peer, TargetID ID, const char* name):
  peer(peer),
  ID(ID),
  name(name)
{
}

///////////////////////////////////////////////////////////////////////

HostListener::~HostListener()
{
}

///////////////////////////////////////////////////////////////////////

Host::~Host()
{
  for (auto p = peers.begin();  p != peers.end();  p++)
    enet_peer_disconnect_now((ENetPeer*) p->peer, 0);

  peers.clear();

  if (host)
  {
    enet_host_flush((ENetHost*) host);
    enet_host_destroy((ENetHost*) host);
    host = NULL;
  }
}

bool Host::sendPacketTo(TargetID targetID,
                        ChannelID channel,
                        PacketType type,
                        const PacketData& data)
{
  switch (targetID)
  {
    case LOCAL:
    {
      if (listener)
      {
        PacketData copy = data;
        listener->onPacketReceived(targetID, copy);
      }

      return true;
    }

    case BROADCAST:
      return broadcast(channel, type, data);

    case SERVER:
    {
      if (isServer())
        return sendPacketTo(LOCAL, channel, type, data);

      /* FALLTHROUGH */
    }

    default:
    {
      Peer* peer = findPeer(targetID);
      if (!peer)
      {
        logError("Cannot send event to unknown peer %u", targetID);
        return false;
      }

      return peer->sendPacket(channel, type, data);
    }
  }
}

bool Host::update(Time timeout)
{
  ENetEvent event;
  bool status = true;
  enet_uint32 ms = (enet_uint32) (timeout * 1000.0);

  while (enet_host_service((ENetHost*) host, &event, ms) > 0)
  {
    switch (event.type)
    {
      case ENET_EVENT_TYPE_CONNECT:
      {
        char name[2048];

        enet_address_get_host(&(event.peer->address), name, sizeof(name) - 1);
        name[sizeof(name) - 1] = '\0';

        TargetID peerID;

        if (isClient())
          peerID = SERVER;
        else
          peerID = nextClientID++;

        peers.push_back(Peer(event.peer, peerID, name));
        event.peer->data = &(peers.back());

        if (listener)
          listener->onPeerConnected(peers.back());

        break;
      }

      case ENET_EVENT_TYPE_DISCONNECT:
      {
        Peer* peer = static_cast<Peer*>(event.peer->data);

        for (auto p = peers.begin();  p != peers.end();  p++)
        {
          if (&(*p) == peer)
          {
            if (listener)
              listener->onPeerDisconnected(*p, event.data);

            peers.erase(p);
            break;
          }
        }

        if (isClient())
          status = false;

        event.peer->data = NULL;
        break;
      }

      case ENET_EVENT_TYPE_RECEIVE:
      {
        if (listener)
        {
          if (Peer* peer = static_cast<Peer*>(event.peer->data))
          {
            PacketData data(event.packet->data,
                        event.packet->dataLength,
                        event.packet->dataLength);

            listener->onPacketReceived(peer->getTargetID(), data);
          }
        }

        enet_packet_destroy(event.packet);
        break;
      }
    }
  }

  enet_host_flush((ENetHost*) host);

  allocated = 0;

  return status;
}

void* Host::allocatePacketData(size_t size)
{
  if (!size)
    return NULL;

  if (size + allocated > sizeof(buffer))
    panic("Out of packet data memory");

  uint8* data = buffer + allocated;
  allocated += size;

  return data;
}

Peer* Host::findPeer(TargetID targetID)
{
  for (auto p = peers.begin();  p != peers.end();  p++)
  {
    if (p->getTargetID() == targetID)
      return &(*p);
  }

  return NULL;
}

bool Host::isClient() const
{
  return !server;
}

bool Host::isServer() const
{
  return server;
}

uint Host::getTotalIncomingBytes() const
{
  return ((ENetHost*) host)->totalReceivedData;
}

uint Host::getTotalOutgoingBytes() const
{
  return ((ENetHost*) host)->totalSentData;
}

uint Host::getIncomingBytesPerSecond() const
{
  return ((ENetHost*) host)->incomingBandwidth;
}

uint Host::getOutgoingBytesPerSecond() const
{
  return ((ENetHost*) host)->outgoingBandwidth;
}

void Host::setListener(HostListener* newListener)
{
  listener = newListener;
}

Host* Host::create(uint16 port, size_t maxClientCount, uint8 maxChannelCount)
{
  Ptr<Host> host(new Host());
  if (!host->init(port, maxClientCount, maxChannelCount))
    return NULL;

  return host.detachObject();
}

Host* Host::connect(const String& name, uint16 port, uint8 maxChannelCount)
{
  Ptr<Host> host(new Host());
  if (!host->init(name, port, maxChannelCount))
    return NULL;

  return host.detachObject();
}

Host::Host():
  host(NULL),
  listener(NULL),
  nextClientID(FIRST_CLIENT),
  allocated(0)
{
}

bool Host::init(uint16 port, size_t maxClientCount, uint8 maxChannelCount)
{
  server = true;

  ENetAddress address;
  address.host = ENET_HOST_ANY;
  address.port = port;

  host = enet_host_create(&address, maxClientCount, maxChannelCount, 0, 0);
  if (!host)
  {
    logError("Failed to create ENet server host");
    return false;
  }

  if (enet_host_compress_with_range_coder((ENetHost*) host) < 0)
  {
    logError("Failed to create ENet range compressor");
    return false;
  }

  return true;
}

bool Host::init(const String& name, uint16 port, uint8 maxChannelCount)
{
  server = false;

  host = enet_host_create(NULL, 1, maxChannelCount, 0, 0);
  if (!host)
  {
    logError("Failed to create ENet client host");
    return false;
  }

  if (enet_host_compress_with_range_coder((ENetHost*) host) < 0)
  {
    logError("Failed to create ENet range compressor");
    return false;
  }

  ENetAddress address;
  address.port = port;

  if (enet_address_set_host(&address, name.c_str()) < 0)
  {
    logError("Failed to resolve server name \'%s\'", name.c_str());
    return false;
  }

  ENetPeer* peer = enet_host_connect((ENetHost*) host, &address, ENET_PROTOCOL_MAXIMUM_CHANNEL_COUNT, 0);
  if (!peer)
  {
    logError("Failed to connect to server \'%s:%u\'", name.c_str(), port);
    return false;
  }

  return true;
}

bool Host::broadcast(ChannelID channel, PacketType type, const PacketData& data)
{
  if (!isServer())
  {
    logError("Only the server is allowed to broadcast");
    return false;
  }

  uint32 flags = 0;

  if (type == RELIABLE)
    flags |= ENET_PACKET_FLAG_RELIABLE;
  else
  {
    if (type == UNSEQUENCED)
      flags |= ENET_PACKET_FLAG_UNSEQUENCED;

    flags |= ENET_PACKET_FLAG_NO_ALLOCATE;
  }

  ENetPacket* packet = enet_packet_create(data.getData(), data.getSize(), flags);
  if (!packet)
  {
    logError("Failed to create ENet packet");
    return false;
  }

  enet_host_broadcast((ENetHost*) host, channel, packet);
  return true;
}

///////////////////////////////////////////////////////////////////////

  } /*namespace net*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
