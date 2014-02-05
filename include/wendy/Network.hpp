///////////////////////////////////////////////////////////////////////
// Wendy - a simple game engine
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
#ifndef WENDY_NETWORK_HPP
#define WENDY_NETWORK_HPP
///////////////////////////////////////////////////////////////////////

#include <list>

#include <wendy/Core.hpp>
#include <wendy/Time.hpp>
#include <wendy/ID.hpp>

//////////////////////////////////////////////////////////////////////

namespace wendy
{

///////////////////////////////////////////////////////////////////////

class NetworkObject;

///////////////////////////////////////////////////////////////////////

/*! Network channel ID.
 *  @ingroup net
 */
typedef uint8 ChannelID;

/*! Network target ID.
 *  @ingroup net
 */
typedef uint8 TargetID;

/*! Network event ID.
 *  @ingroup net
 */
typedef uint8 EventID;

/*! Network object ID.
 *  @ingroup net
 */
typedef uint16 NetworkObjectID;


///////////////////////////////////////////////////////////////////////

enum
{
  /*! Sends to the local machine.
   */
  LOCAL,
  /*! If on the server, sends to the local machine.
   *  If on a client, sends to the server.
   */
  SERVER,
  /*! If on the server, sends to all clients.
   *  If on a client, discards the packet.
   */
  BROADCAST,
  /*! If on the server, sends to the first client.
   *  If on a client, generates an error.
   */
  FIRST_CLIENT,
};

///////////////////////////////////////////////////////////////////////

/*! Packet transport type enumeration.
 *  @ingroup net
 */
enum PacketType
{
  /*! Packets are sent unreliably and without any guaranteed order.
   */
  UNSEQUENCED,
  /*! Packets are sent unreliably but are guaranteed to arrive in order.
   */
  SEQUENCED,
  /*! Packets are sent reliably and are guaranteed to arrive in order.
   */
  RELIABLE,
};

///////////////////////////////////////////////////////////////////////

enum
{
  EVENT_CUSTOM_BASE,
};

///////////////////////////////////////////////////////////////////////

enum
{
  OBJECT_ID_INVALID,
  OBJECT_ID_GAME,
  OBJECT_ID_LEVEL,
  OBJECT_ID_POOL_BASE,
};

///////////////////////////////////////////////////////////////////////

/*! Packet data descriptor.
 *  @ingroup net
 */
class PacketData
{
public:
  PacketData();
  PacketData(void* data, size_t capacity, size_t size = 0);
  uint8 read8();
  uint16 read16();
  uint32 read32();
  half read16f();
  float read32f();
  template <typename T>
  void read(T& value);
  void write8(uint8 value);
  void write16(uint16 value);
  void write32(uint32 value);
  void write16f(half value);
  void write32f(float value);
  template <typename T>
  void write(const T& value);
  bool isEmpty();
  size_t size() const;
  size_t capacity() const;
  const void* data() const;
private:
  uint8* m_data;
  size_t m_capacity;
  size_t m_size;
  size_t m_offset;
};

///////////////////////////////////////////////////////////////////////

/*! Network peer.
 *  @ingroup net
 */
class Peer
{
  friend class Host;
public:
  bool sendPacket(ChannelID channel, PacketType type, const PacketData& data);
  void disconnect(uint32 reason);
  bool isClient() const { return m_id != SERVER; }
  bool isServer() const { return m_id == SERVER; }
  TargetID id() const { return m_id; }
  const String& name() const { return m_name; }
  uint32 address() const;
  Time roundTripTime() const;
private:
  Peer(void* peer, TargetID targetID, const char* name);
  void* m_peer;
  TargetID m_id;
  String m_name;
  bool m_disconnecting;
  uint32 m_reason;
};

///////////////////////////////////////////////////////////////////////

/*! Network host event listener.
 *  @ingroup net
 */
class HostObserver
{
  friend class Host;
public:
  virtual ~HostObserver();
protected:
  virtual void onPeerConnected(Peer& client) = 0;
  virtual void onPeerDisconnected(Peer& client, uint32 reason) = 0;
  virtual void onPacketReceived(TargetID targetID, PacketData& data) = 0;
};

///////////////////////////////////////////////////////////////////////

/*! @brief Network host.
 *  @ingroup net
 */
class Host
{
  friend class NetworkObject;
public:
  ~Host();
  bool sendPacketTo(TargetID targetID,
                    ChannelID channel,
                    PacketType type,
                    const PacketData& data);
  bool update(Time timeout);
  void* allocatePacketData(size_t size);
  Peer* findPeer(TargetID targetID);
  NetworkObject* findObject(NetworkObjectID objectID);
  PacketData createEvent(EventID eventID, NetworkObjectID recipientID);
  bool dispatchEvent(TargetID sourceID, PacketData& data);
  bool isClient() const;
  bool isServer() const;
  uint totalIncomingBytes() const;
  uint totalOutgoingBytes() const;
  uint incomingBytesPerSecond() const;
  uint outgoingBytesPerSecond() const;
  void setObserver(HostObserver* newObserver);
  static Host* create(uint16 port, size_t maxClientCount, uint8 maxChannelCount = 0);
  static Host* connect(const String& name, uint16 port, uint8 maxChannelCount = 0);
private:
  Host();
  Host(const Host&) = delete;
  bool init(uint16 port, size_t maxClientCount, uint8 maxChannelCount);
  bool init(const String& name, uint16 port, uint8 maxChannelCount);
  bool broadcast(ChannelID channel, PacketType type, const PacketData& data);
  Host& operator = (const Host&) = delete;
  void* m_object;
  std::list<Peer> m_peers;
  HostObserver* m_observer;
  IDPool<TargetID> m_clientIDs;
  IDPool<NetworkObjectID> m_objectIDs;
  std::vector<NetworkObject*> m_objects;
  size_t m_allocated;
  uint8 m_buffer[65536];
  bool m_server;
  static uint m_count;
};

///////////////////////////////////////////////////////////////////////

/*! @brief Network object.
 */
class NetworkObject
{
  friend class Host;
public:
  NetworkObject(Host& host, NetworkObjectID objectID = OBJECT_ID_INVALID);
  virtual ~NetworkObject();
  virtual void synchronize();
  bool isOnServer() const { return m_host.isServer(); }
  bool isOnClient() const { return m_host.isClient(); }
  NetworkObjectID id() const { return m_id; }
  Host& host() const { return m_host; }
protected:
  PacketData createEvent(EventID eventID, NetworkObjectID recipientID) const;
  bool broadcastEvent(ChannelID channelID,
                      PacketType type,
                      PacketData& data) const;
  bool broadcastEvent(ChannelID channelID,
                      PacketType type,
                      NetworkObjectID recipientID,
                      EventID eventID) const;
  bool sendEvent(TargetID targetID,
                 ChannelID channelID,
                 PacketType type,
                 PacketData& data) const;
  bool sendEvent(TargetID targetID,
                 ChannelID channelID,
                 PacketType type,
                 NetworkObjectID recipientID,
                 EventID eventID) const;
  virtual void receiveEvent(TargetID senderID,
                            PacketData& data,
                            EventID eventID);
private:
  NetworkObjectID m_id;
  Host& m_host;
};

///////////////////////////////////////////////////////////////////////

} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
#endif /*WENDY_NETWORK_HPP*/
///////////////////////////////////////////////////////////////////////
