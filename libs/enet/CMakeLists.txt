
project(enet C)

include(CheckIncludeFile)
include(CheckFunctionExists)
include(CheckStructHasMember)

set(enet_SOURCES callbacks.c compress.c host.c list.c packet.c peer.c protocol.c)
set(enet_LIBRARIES)

if (WIN32)
  list(APPEND enet_SOURCES win32.c)
  list(APPEND enet_LIBRARIES wsock32 ws2_32 winmm)
else()
  list(APPEND enet_SOURCES unix.c)
endif()

check_function_exists(gethostbyaddr_r HAS_GETHOSTBYADDR_R)
if (HAS_GETHOSTBYADDR_R)
  add_definitions(-DHAS_GETHOSTBYADDR_R)
endif()

check_function_exists(gethostbyname_r HAS_GETHOSTBYNAME_R)
if (HAS_GETHOSTBYNAME_R)
  add_definitions(-DHAS_GETHOSTBYNAME_R)
endif()

check_function_exists(poll HAS_POLL)
if (HAS_POLL)
  add_definitions(-DHAS_POLL)
endif()

check_function_exists(fcntl HAS_FCNTL)
if (HAS_FCNTL)
  add_definitions(-DHAS_FCNTL)
endif()

check_function_exists(inet_pton HAS_INET_PTON)
if (HAS_INET_PTON)
  add_definitions(-DHAS_INET_PTON)
endif()

check_function_exists(inet_ntop HAS_INET_NTOP)
if (HAS_INET_NTOP)
  add_definitions(-DHAS_INET_NTOP)
endif()

check_struct_has_member("struct msghdr" msg_flags sys/socket.h HAS_MSGHDR_FLAGS)
if (HAS_MSGHDR_FLAGS)
  add_definitions(-DHAS_MSGHDR_FLAGS)
endif()

if (UNIX)
  add_definitions(-DHAS_SOCKLEN_T)
endif()

include_directories(${enet_SOURCE_DIR})

add_library(enet STATIC ${enet_SOURCES})

set(ENET_LIBRARIES ${enet_LIBRARIES} CACHE STRING "Enet libraries")

