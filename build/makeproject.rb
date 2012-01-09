#!/usr/bin/env ruby

require 'fileutils'

class Project

  attr_reader :type, :name, :path, :data_paths

  def initialize(type, name, path)
    @type, @name, @path = type, name, path
    @data_paths = []
  end

  def build()
    build_directory_tree()
    build_cmake()
    build_header()
    build_source()
  end

private

  def build_directory_tree()
    paths = @data_paths.sort.uniq.collect do |path|
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
  end

  def build_cmake()
    File.open(@path + '/CMakeLists.txt', 'wb') do |file|
      file.print <<EOF

cmake_minimum_required(VERSION 2.6)

project(#{@name} C CXX)
set(VERSION 0.1)

include(FindPkgConfig)

add_subdirectory(${#{@name}_SOURCE_DIR}/../../wendy ${#{@name}_BINARY_DIR}/wendy)

include_directories(${WENDY_INCLUDE_DIRS})
list(APPEND #{@name}_LIBRARIES ${WENDY_LIBRARIES})

set(#{@name}_SOURCES #{@type}.cpp)

add_executable(#{@name} WIN32 MACOSX_BUNDLE ${#{@name}_SOURCES})
target_link_libraries(#{@name} wendy ${#{@name}_LIBRARIES})

set_target_properties(#{@name} PROPERTIES
  MACOSX_BUNDLE_BUNDLE_NAME #{@name.capitalize}
  MACOSX_BUNDLE_GUI_IDENTIFIER org.elmindreda.#{@type.downcase}s.#{@name}
  DEBUG_POSTFIX "_debug")

if(MSVC)
  set_target_properties(#{@name} PROPERTIES
    LINK_FLAGS "/ENTRY:mainCRTStartup")
endif(MSVC)

EOF
    end
  end

  def build_header()
    File.open(@path + "/#{@type}.h", 'wb') do |file|
      file.print <<EOF

namespace #{@name}
{

class #{@type} : public Trackable
{
public:
  #{@type}(void);
  ~#{@type}(void);
  bool init(void);
  void run(void);
private:
  ResourceCache cache;
  Ref<render::GeometryPool> pool;
};

} /*namespace #{@name}*/

EOF
    end
  end

  def build_source()

    File.open(@path + "/#{@type}.cpp", 'wb') do |file|
      file.print <<EOF

#include <wendy/Wendy.h>

#include <cstdlib>

using namespace wendy;

#include "#{@type}.h"

namespace #{@name}
{

#{@type}::#{@type}(void)
{
}

#{@type}::~#{@type}(void)
{
  pool = NULL;

  input::Context::destroySingleton();
  GL::Context::destroySingleton();
}

bool #{@type}::init(void)
{
  cache.addSearchPath(Path("data"));

  GL::WindowConfig wc;
  GL::ContextConfig cc;

  if (!GL::Context::createSingleton(cache, wc, cc))
  {
    logError("Failed to create OpenGL context");
    return false;
  }

  GL::Context* context = GL::Context::getSingleton();
  context->setTitle("#{@name.capitalize}");

  if (!input::Context::createSingleton(*context))
  {
    logError("Failed to create input context");
    return false;
  }

  pool = render::GeometryPool::create(*context);
  if (!pool)
  {
    logError("Failed to create geometry pool");
    return false;
  }

  return true;
}

void #{@type}::run(void)
{
  GL::Context* context = GL::Context::getSingleton();

  do
  {
    context->clearBuffers();
  }
  while (context->update());
}

} /*namespace #{@name}*/

int main()
{
  #{@name}::#{@type} #{@type.downcase};
  if (!#{@type.downcase}.init())
    exit(EXIT_FAILURE);

  #{@type.downcase}.run();
  std::exit(EXIT_SUCCESS);
}

EOF
    end
  end

end # class Project

def usage()
  puts "usage: #{__FILE__} {Demo|Game|Test} <name> <path>"
end

unless ARGV.size == 3
  $stderr.puts 'Invalid user'
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

unless name =~ /^[A-Za-z]\w*$/
  $stderr.puts "#{name} is not a valid project name"
  usage()
  exit 1
end

path = ARGV[2].to_s

project = Project.new(type, name, path)

project.data_paths << 'data/fonts'
project.data_paths << 'data/shaders'
project.data_paths << 'data/models'
project.data_paths << 'data/textures'

project.build()

