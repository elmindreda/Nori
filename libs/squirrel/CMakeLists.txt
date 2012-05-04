
project(squirrel CXX)

include_directories(${squirrel_SOURCE_DIR})

add_definitions(-DNO_GARBAGE_COLLECTOR)

if (MSVC)
  add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif()

set(squirrel_SOURCES sqapi.cpp sqbaselib.cpp sqclass.cpp sqcompiler.cpp
                     sqdebug.cpp sqfuncstate.cpp sqlexer.cpp sqmem.cpp
                     sqobject.cpp sqstate.cpp sqstdmath.cpp sqstdrex.cpp
                     sqstdstring.cpp sqtable.cpp sqvm.cpp)

add_library(squirrel STATIC ${squirrel_SOURCES})

