
if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  add_definitions(-std=c++0x)
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  add_definitions(-std=c++11)
endif()

include_directories(${pugixml_SOURCE_DIR}
                    ${greg_SOURCE_DIR}
                    ${GLFW_SOURCE_DIR}
                    ${libogg_SOURCE_DIR}
                    ${libvorbis_SOURCE_DIR}
                    ${nanovg_SOURCE_DIR}
                    ${stb_SOURCE_DIR}
                    ${utf8cpp_SOURCE_DIR})

set(nori_SOURCES
    Nori.cpp

    Core.cpp Camera.cpp Frustum.cpp Image.cpp Mesh.cpp Path.cpp Pixel.cpp
    Primitive.cpp Profile.cpp Rect.cpp Resource.cpp Sample.cpp Signal.cpp
    Time.cpp Transform.cpp Vertex.cpp)

if (NORI_INCLUDE_NETWORK)
  include_directories(${enet_SOURCE_DIR})
  list(APPEND nori_SOURCES Network.cpp)
endif()

if (NORI_INCLUDE_AUDIO)
  include_directories(${OPENAL_INCLUDE_DIR})
  list(APPEND nori_SOURCES Audio.cpp)
endif()

if (NORI_INCLUDE_RENDERER)
  list(APPEND nori_SOURCES
       Material.cpp Model.cpp OpenGL.cpp Pass.cpp Program.cpp Query.cpp
       RenderBuffer.cpp RenderContext.cpp RenderQueue.cpp Renderer.cpp Scene.cpp
       Sprite.cpp Texture.cpp VectorContext.cpp Window.cpp)
endif()

if (NORI_INCLUDE_SQUIRREL)
  include_directories(${squirrel_SOURCE_DIR})
  list(APPEND nori_SOURCES Squirrel.cpp)
endif()

if (NORI_INCLUDE_BULLET)
  include_directories(${bullet_SOURCE_DIR})
  list(APPEND nori_SOURCES Bullet.cpp)
endif()

if (NORI_INCLUDE_UI_SYSTEM)
  list(APPEND nori_SOURCES
       Book.cpp Button.cpp Canvas.cpp Entry.cpp Item.cpp Label.cpp Layer.cpp
       Layout.cpp List.cpp Menu.cpp Popup.cpp Progress.cpp Scroller.cpp
       Slider.cpp Theme.cpp Widget.cpp WidgetReader.cpp)
endif()

if (NORI_INCLUDE_DEBUG_UI)
  list(APPEND nori_SOURCES DebugUI.cpp)
endif()

# We add headers to list of sources so that CMake's generator
# knows to add them to IDE project files for e.g. Qt Creator
file(GLOB_RECURSE nori_HEADERS ${CMAKE_SOURCE_DIR}/include/*.hpp)

add_library(nori STATIC ${nori_SOURCES} ${nori_HEADERS})
set_target_properties(nori PROPERTIES COMPILE_DEFINITIONS_DEBUG NORI_DEBUG)

