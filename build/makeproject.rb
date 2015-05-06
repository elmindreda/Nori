#!/usr/bin/env ruby

require 'fileutils'

class Project

  attr_reader :type, :name, :path, :paths

  def initialize(type, name, path)
    @type, @name, @path = type, name, path
    @paths = []
  end

  def build()
    build_directory_tree()
    build_cmake()
    build_header()
    build_source()
  end

private

  def build_directory_tree()
    paths = @paths.sort.uniq.collect do |path|
      @path + '/' + path
    end

    paths.each do |path|
      if File.exists?(path)
        if File.directory?(path)
          next
        else
          raise "#{path} blocked"
        end
      end

      FileUtils.mkdir_p(path)
    end

    FileUtils.ln_s("../../nori/media/nori", @path + '/data/')
  end

  def build_cmake()
    File.open(@path + '/CMakeLists.txt', 'wb') do |file|
      file.print <<EOF

cmake_minimum_required(VERSION 2.8)

project(#{@name} C CXX)
set(VERSION 0.1)

add_subdirectory(${#{@name}_SOURCE_DIR}/../nori ${#{@name}_BINARY_DIR}/nori)

include_directories(${NORI_INCLUDE_DIRS})
list(APPEND #{@name}_LIBRARIES ${NORI_LIBRARIES})

add_subdirectory(src)

EOF
    end

    File.open(@path + '/src/CMakeLists.txt', 'wb') do |file|
      file.print <<EOF

if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  add_definitions(-std=c++0x)
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  add_definitions(-std=c++11)
endif()

set(#{@name}_SOURCES #{@type}.cpp #{@type}.hpp)

add_executable(#{@name} WIN32 MACOSX_BUNDLE ${#{@name}_SOURCES})
target_link_libraries(#{@name} nori ${#{@name}_LIBRARIES})

set_target_properties(#{@name} PROPERTIES
  MACOSX_BUNDLE_BUNDLE_NAME #{@name.capitalize}
  MACOSX_BUNDLE_GUI_IDENTIFIER org.elmindreda.#{@type.downcase}s.#{@name}
  DEBUG_POSTFIX "_debug")

if (MSVC)
  set_target_properties(#{@name} PROPERTIES LINK_FLAGS "/ENTRY:mainCRTStartup")
endif()

EOF
    end
  end

  def build_header()
    File.open(@path + "/src/#{@type}.hpp", 'wb') do |file|
      file.print <<EOF

#include <memory>

namespace #{@name}
{

using namespace nori;

class #{@type} : public EventHook
{
public:
  #{@type}();
  ~#{@type}();
  bool init();
  void run();
private:
  ResourceCache m_cache;
  std::unique_ptr<RenderContext> m_renderContext;
  std::unique_ptr<AudioContext> m_audioContext;
};

} /*namespace #{@name}*/

EOF
    end
  end

  def build_source()

    File.open(@path + "/src/#{@type}.cpp", 'wb') do |file|
      file.print <<EOF

#include <nori/Nori.hpp>

#include <cstdlib>

#include "#{@type}.hpp"

namespace #{@name}
{

using namespace nori;

#{@type}::#{@type}()
{
}

#{@type}::~#{@type}()
{
  m_renderContext = nullptr;
  m_audioContext = nullptr;
}

bool #{@type}::init()
{
  if (!m_cache.addSearchPath(Path("data")))
  {
    logError("Failed to locate data directory");
    return false;
  }

  m_audioContext = AudioContext::create(m_cache);
  if (!m_audioContext)
  {
    logError("Failed to create audio context");
    return false;
  }

  WindowConfig wc("#{@name.capitalize}");
  RenderConfig rc;

  m_renderContext = RenderContext::create(m_cache, wc, rc);
  if (!m_renderContext)
  {
    logError("Failed to create render context");
    return false;
  }

  return true;
}

void #{@type}::run()
{
  Window& window = m_renderContext->window();

  do
  {
    m_renderContext->clearBuffers();
  }
  while (window.update());
}

} /*namespace #{@name}*/

int main()
{
  std::unique_ptr<#{@name}::#{@type}> #{@type.downcase}(new #{@name}::#{@type}());
  if (!#{@type.downcase}->init())
    std::exit(EXIT_FAILURE);

  #{@type.downcase}->run();
  #{@type.downcase} = nullptr;

  std::exit(EXIT_SUCCESS);
}

EOF
    end
  end

end # class Project

def usage()
  puts "Usage: #{File.basename __FILE__} {Demo|Game|Test} <name> [<path>]"
end

unless (2..3).cover? ARGV.size
  usage()
  exit 1
end

type = ARGV[0].to_s.capitalize

unless ['Demo', 'Game', 'Test'].include?(type)
  $stderr.puts "#{type} is not a valid project type"
  usage()
  exit 1
end

name = ARGV[1].to_s.downcase

unless name =~ /^[[:alpha:]]\w*$/
  $stderr.puts "#{name} is not a valid project name"
  usage()
  exit 1
end

if ARGV.size == 3
  path = ARGV[2].to_s
else
  path = ARGV[1].to_s.downcase
end

project = Project.new(type, name, path)

project.paths << 'src'
project.paths << 'data/fonts'
project.paths << 'data/sounds'
project.paths << 'data/shaders'
project.paths << 'data/models'
project.paths << 'data/textures'

project.build()

