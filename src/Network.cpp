///////////////////////////////////////////////////////////////////////
// Nori - a simple game engine
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

#include <nori/Core.hpp>
#include <nori/Network.hpp>

#include <enet/enet.h>

#include <cstring>

namespace nori
{

namespace
{

const size_t MAX_EVENT_SIZE = 1024;

} /*namespace*/

PacketData::PacketData():
  m_data(nullptr),
  m_capacity(0),
  m_size(0),
  m_offset(0)
{
}

PacketData::PacketData(void* data, size_t capacity, size_t size):
  m_data(static_cast<uint8*>(data)),
  m_capacity(capacity),
  m_size(size),
  m_offset(0)
{
}

uint8 PacketData::read8()
{
  if (m_offset + 1 > m_size)
    panic("Packet data buffer underflow");

  return m_data[m_offset++];
}

uint16 PacketData::read16()
{
  if (m_offset + 2 > m_size)
    panic("Packet data buffer underflow");

  const uint16 value = ntohs(*(uint16*) (m_data + m_offset));
  m_offset += 2;
  return value;
}

uint32 PacketData::read32()
{
  if (m_offset + 4 > m_size)
    panic("Packet data buffer underflow");

  const uint32 value = ntohl(*(uint32*) (m_data + m_offset));
  m_offset += 4;
  return value;
}

float PacketData::read32f()
{
  return uintBitsToFloat(read32());
}

template <>
std::string PacketData::read()
{
  if (!std::memchr(m_data + m_offset, '\0', m_size - m_offset))
    panic("Missing null character in packet data");

  std::string result((char*) (m_data + m_offset));
  m_offset += result.length() + 1;
  return result;
}

void PacketData::write8(uint8 value)
{
  if (m_size + 1 > m_capacity)
    panic("Packet data buffer overflow");

  m_data[m_size++] = value;
}

void PacketData::write16(uint16 value)
{
  if (m_size + 2 > m_capacity)
    panic("Packet data buffer overflow");

  *((uint16*) (m_data + m_size)) = htons(value);
  m_size += 2;
}

void PacketData::write32(uint32 value)
{
  if (m_size + 4 > m_capacity)
    panic("Packet data buffer overflow");

  *((uint32*) (m_data + m_size)) = htonl(value);
  m_size += 4;
}

void PacketData::write32f(float value)
{
  write32(floatBitsToUint(value));
}

template <>
void PacketData::write(const std::string& value)
{
  for (char c : value)
    write8(c);

  write8('\0');
}

bool PacketData::isEmpty()
{
  return m_offset == m_size;
}

size_t PacketData::size() const
{
  return m_size;
}

size_t PacketData::capacity() const
{
  return m_capacity;
}

const void* PacketData::data() const
{
  return m_data;
}

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

  ENetPacket* packet = enet_packet_create(data.data(), data.size(), flags);
  if (!packet)
  {
    logError("Failed to create ENet packet");
    return false;
  }

  if (enet_peer_send((ENetPeer*) m_peer, channel, packet) < 0)
  {
    logError("Failed to send ENet packet to peer %s", m_name.c_str());
    return false;
  }

  return true;
}

void Peer::disconnect(uint32 reason)
{
  m_disconnecting = true;
  m_reason = reason;
  enet_peer_disconnect((ENetPeer*) m_peer, reason);
}

uint32 Peer::address() const
{
  return ((ENetPeer*) m_peer)->address.host;
}

Time Peer::roundTripTime() const
{
  return (Time) ((ENetPeer*) m_peer)->roundTripTime / 1000.0;
}

Peer::Peer(void* peer, TargetID targetID, const char* name):
  m_peer(peer),
  m_id(targetID),
  m_name(name),
  m_disconnecting(false),
  m_reason(0)
{
}

HostObserver::~HostObserver()
{
}

Host::~Host()
{
  for (Peer& p : m_peers)
    enet_peer_disconnect_now((ENetPeer*) p.m_peer, 0);

  m_peers.clear();

  if (m_object)
  {
    enet_host_flush((ENetHost*) m_object);
    enet_host_destroy((ENetHost*) m_object);
    m_object = nullptr;
  }

  m_count--;
  if (!m_count)
    enet_deinitialize();
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
      if (m_observer)
      {
        PacketData copy = data;
        m_observer->onPacketReceived(targetID, copy);
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

  while (enet_host_service((ENetHost*) m_object, &event, ms) > 0)
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
          peerID = m_clientIDs.allocateID();

        m_peers.push_back(Peer(event.peer, peerID, name));
        event.peer->data = &(m_peers.back());

        if (m_observer)
          m_observer->onPeerConnected(m_peers.back());

        break;
      }

      case ENET_EVENT_TYPE_DISCONNECT:
      {
        const Peer* peer = static_cast<Peer*>(event.peer->data);

        for (auto p = m_peers.begin();  p != m_peers.end();  p++)
        {
          if (&(*p) == peer)
          {
            uint32 reason;

            if (peer->m_disconnecting)
              reason = peer->m_reason;
            else
              reason = event.data;

            if (m_observer)
              m_observer->onPeerDisconnected(*p, reason);

            m_clientIDs.releaseID(p->id());

            m_peers.erase(p);
            break;
          }
        }

        if (isClient())
          status = false;

        event.peer->data = nullptr;
        break;
      }

      case ENET_EVENT_TYPE_RECEIVE:
      {
        if (m_observer)
        {
          if (Peer* peer = static_cast<Peer*>(event.peer->data))
          {
            PacketData data(event.packet->data,
                            event.packet->dataLength,
                            event.packet->dataLength);

            m_observer->onPacketReceived(peer->id(), data);
          }
        }

        enet_packet_destroy(event.packet);
        break;
      }

      case ENET_EVENT_TYPE_NONE:
      {
        // This removes a useless warning by Clang
        break;
      }
    }
  }

  enet_host_flush((ENetHost*) m_object);

  m_allocated = 0;

  return status;
}

void* Host::allocatePacketData(size_t size)
{
  if (!size)
    return nullptr;

  if (size + m_allocated > sizeof(m_buffer))
    panic("Out of packet data memory");

  uint8* data = &m_buffer[m_allocated];
  m_allocated += size;

  return data;
}

Peer* Host::findPeer(TargetID targetID)
{
  for (Peer& p : m_peers)
  {
    if (p.id() == targetID)
      return &p;
  }

  return nullptr;
}

NetworkObject* Host::findObject(NetworkObjectID objectID)
{
  if (objectID < m_objects.size())
    return m_objects[objectID];
  else
    return nullptr;
}

PacketData Host::createEvent(EventID eventID, NetworkObjectID recipientID)
{
  const size_t size = MAX_EVENT_SIZE + sizeof(EventID) + sizeof(NetworkObjectID);

  PacketData data(allocatePacketData(size), size);
  data.write16(recipientID);
  data.write8(eventID);

  return data;
}

bool Host::dispatchEvent(TargetID sourceID, PacketData& data)
{
  const NetworkObjectID recipientID = data.read16();
  const EventID eventID = data.read8();

  NetworkObject* object = findObject(recipientID);
  if (!object)
  {
    if (isClient() || m_objectIDs.bucketOf(recipientID) == ID_BUCKET_UNUSED)
      logError("Failed to find recipient object %u", recipientID);

    return false;
  }

  object->receiveEvent(sourceID, data, eventID);
  return true;
}

bool Host::isClient() const
{
  return !m_server;
}

bool Host::isServer() const
{
  return m_server;
}

uint Host::totalIncomingBytes() const
{
  return ((ENetHost*) m_object)->totalReceivedData;
}

uint Host::totalOutgoingBytes() const
{
  return ((ENetHost*) m_object)->totalSentData;
}

uint Host::incomingBytesPerSecond() const
{
  return ((ENetHost*) m_object)->incomingBandwidth;
}

uint Host::outgoingBytesPerSecond() const
{
  return ((ENetHost*) m_object)->outgoingBandwidth;
}

void Host::setObserver(HostObserver* newObserver)
{
  m_observer = newObserver;
}

std::unique_ptr<Host> Host::create(uint16 port,
                                   size_t maxClientCount,
                                   uint8 maxChannelCount)
{
  std::unique_ptr<Host> host(new Host());
  if (!host->init(port, maxClientCount, maxChannelCount))
    return nullptr;

  return host;
}

std::unique_ptr<Host> Host::connect(const std::string& name,
                                    uint16 port,
                                    uint8 maxChannelCount)
{
  std::unique_ptr<Host> host(new Host());
  if (!host->init(name, port, maxChannelCount))
    return nullptr;

  return host;
}

Host::Host():
  m_object(nullptr),
  m_observer(nullptr),
  m_clientIDs(FIRST_CLIENT),
  m_objectIDs(OBJECT_ID_POOL_BASE),
  m_allocated(0)
{
}

bool Host::init(uint16 port, size_t maxClientCount, uint8 maxChannelCount)
{
  if (!m_count)
  {
    if (enet_initialize())
      return false;
  }

  m_count++;
  m_server = true;

  ENetAddress address;
  address.host = ENET_HOST_ANY;
  address.port = port;

  m_object = enet_host_create(&address, maxClientCount, maxChannelCount, 0, 0);
  if (!m_object)
  {
    logError("Failed to create ENet server host");
    return false;
  }

  if (enet_host_compress_with_range_coder((ENetHost*) m_object) < 0)
  {
    logError("Failed to create ENet range compressor");
    return false;
  }

  return true;
}

bool Host::init(const std::string& name, uint16 port, uint8 maxChannelCount)
{
  if (!m_count)
  {
    if (enet_initialize())
      return false;
  }

  m_count++;
  m_server = false;

  m_object = enet_host_create(nullptr, 1, maxChannelCount, 0, 0);
  if (!m_object)
  {
    logError("Failed to create ENet client host");
    return false;
  }

  if (enet_host_compress_with_range_coder((ENetHost*) m_object) < 0)
  {
    logError("Failed to create ENet range compressor");
    return false;
  }

  ENetAddress address;
  address.port = port;

  if (enet_address_set_host(&address, name.c_str()) < 0)
  {
    logError("Failed to resolve server name %s", name.c_str());
    return false;
  }

  ENetPeer* peer = enet_host_connect((ENetHost*) m_object,
                                     &address,
                                     ENET_PROTOCOL_MAXIMUM_CHANNEL_COUNT,
                                     0);
  if (!peer)
  {
    logError("Failed to connect to server %s:%u", name.c_str(), port);
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

  ENetPacket* packet = enet_packet_create(data.data(), data.size(), flags);
  if (!packet)
  {
    logError("Failed to create ENet packet");
    return false;
  }

  enet_host_broadcast((ENetHost*) m_object, channel, packet);
  return true;
}

uint Host::m_count = 0;

NetworkObject::NetworkObject(Host& host, NetworkObjectID objectID):
  m_id(objectID),
  m_host(host)
{
  if (isOnServer())
  {
    if (m_id == OBJECT_ID_INVALID)
      m_id = m_host.m_objectIDs.allocateID();
  }
  else
  {
    if (m_id == OBJECT_ID_INVALID)
      panic("Object on client created with invalid ID");
  }

  auto& objects = m_host.m_objects;

  if (objects.size() <= m_id)
    objects.insert(objects.end(), m_id * 2 - objects.size(), nullptr);

  objects[m_id] = this;
}

NetworkObject::~NetworkObject()
{
  if (isOnServer() && m_id >= OBJECT_ID_POOL_BASE)
    m_host.m_objectIDs.releaseID(m_id);

  m_host.m_objects[m_id] = nullptr;
}

void NetworkObject::synchronize()
{
}

PacketData NetworkObject::createEvent(EventID eventID, NetworkObjectID recipientID) const
{
  return m_host.createEvent(eventID, recipientID);
}

bool NetworkObject::broadcastEvent(ChannelID channelID,
                                   PacketType type,
                                   PacketData& data) const
{
  return sendEvent(BROADCAST, channelID, type, data);
}

bool NetworkObject::broadcastEvent(ChannelID channelID,
                                   PacketType type,
                                   NetworkObjectID recipientID,
                                   EventID eventID) const
{
  return sendEvent(BROADCAST, channelID, type, recipientID, eventID);
}

bool NetworkObject::sendEvent(TargetID targetID,
                              ChannelID channelID,
                              PacketType type,
                              PacketData& data) const
{
  return m_host.sendPacketTo(targetID, channelID, type, data);
}

bool NetworkObject::sendEvent(TargetID targetID,
                              ChannelID channelID,
                              PacketType type,
                              NetworkObjectID recipientID,
                              EventID eventID) const
{
  PacketData event = createEvent(eventID, recipientID);
  return sendEvent(targetID, channelID, type, event);
}

void NetworkObject::receiveEvent(TargetID senderID,
                                 PacketData& data,
                                 EventID eventID)
{
}

} /*namespace nori*/

