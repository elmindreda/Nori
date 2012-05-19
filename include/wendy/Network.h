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
#ifndef WENDY_NETWORK_H
#define WENDY_NETWORK_H
///////////////////////////////////////////////////////////////////////

#include <list>

//////////////////////////////////////////////////////////////////////

namespace wendy
{
  namespace net
  {

///////////////////////////////////////////////////////////////////////

bool initialize();
void shutdown();

///////////////////////////////////////////////////////////////////////

typedef uint8 ChannelID;
typedef uint16 TargetID;

///////////////////////////////////////////////////////////////////////

enum
{
  LOCAL,
  SERVER,
  BROADCAST,
  FIRST_CLIENT,
};

///////////////////////////////////////////////////////////////////////

enum PacketType
{
  UNSEQUENCED,
  SEQUENCED,
  RELIABLE,
};

///////////////////////////////////////////////////////////////////////

class PacketData
{
public:
  PacketData(void* data, size_t capacity, size_t size = 0);
  uint8 read8();
  uint16 read16();
  uint32 read32();
  float read32f();
  template <typename T>
  void read(T& value);
  void write8(uint8 value);
  void write16(uint16 value);
  void write32(uint32 value);
  void write32f(float value);
  template <typename T>
  void write(const T& value);
  bool isEmpty();
  size_t getSize() const;
  const void* getData() const;
private:
  uint8* data;
  size_t capacity;
  size_t size;
  size_t offset;
};

///////////////////////////////////////////////////////////////////////

class Peer
{
  friend class Host;
public:
  Peer(void* peer, TargetID ID, const char* name);
  bool sendPacket(ChannelID channel, PacketType type, const PacketData& data);
  void disconnect(uint32 reason);
  bool isClient() const;
  bool isServer() const;
  TargetID getTargetID() const;
  const String& getName() const;
  uint32 getAddress() const;
  Time getRoundTripTime() const;
private:
  void* peer;
  TargetID ID;
  String name;
};

///////////////////////////////////////////////////////////////////////

class HostListener
{
  friend class Host;
public:
  virtual ~HostListener();
protected:
  virtual void onPeerConnected(Peer& client) = 0;
  virtual void onPeerDisconnected(Peer& client, uint32 reason) = 0;
  virtual void onPacketReceived(TargetID targetID, PacketData& data) = 0;
};

///////////////////////////////////////////////////////////////////////

class Host
{
public:
  ~Host();
  bool sendPacketTo(TargetID targetID,
                    ChannelID channel,
                    PacketType type,
                    const PacketData& data);
  bool update(Time timeout);
  void* allocatePacketData(size_t size);
  Peer* findPeer(TargetID targetID);
  bool isClient() const;
  bool isServer() const;
  uint32 getTotalIncomingBytes() const;
  uint32 getTotalOutgoingBytes() const;
  uint32 getIncomingBytesPerSecond() const;
  uint32 getOutgoingBytesPerSecond() const;
  void setListener(HostListener* newListener);
  static Host* create(uint16 port, size_t maxClientCount, uint8 maxChannelCount = 0);
  static Host* connect(const String& name, uint16 port, uint8 maxChannelCount = 0);
private:
  Host();
  bool init(uint16 port, size_t maxClientCount, uint8 maxChannelCount);
  bool init(const String& name, uint16 port, uint8 maxChannelCount);
  bool broadcast(ChannelID channel, PacketType type, const PacketData& data);
  void* host;
  std::list<Peer> peers;
  HostListener* listener;
  TargetID nextClientID;
  uint8 buffer[65536];
  size_t allocated;
  bool server;
};

///////////////////////////////////////////////////////////////////////

  } /*namespace net*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
#endif /*WENDY_NETWORK_H*/
///////////////////////////////////////////////////////////////////////
