#!/usr/bin/env ruby

require 'getoptlong'
require 'rexml/document'

options = [[ '--help', '-h', GetoptLong::NO_ARGUMENT ],
           [ '--version', '-v', GetoptLong::NO_ARGUMENT ],
           [ '--verbose', '-V', GetoptLong::NO_ARGUMENT],
           [ '--dir', '-d', GetoptLong::REQUIRED_ARGUMENT],
           [ '--filter', '-f', GetoptLong::REQUIRED_ARGUMENT],
           [ '--address', '-a', GetoptLong::REQUIRED_ARGUMENT],
           [ '--rectangular', '-r', GetoptLong::NO_ARGUMENT],
           [ '--mipmapped', '-m', GetoptLong::NO_ARGUMENT]]

def usage()
end

def version()
  $stdout.puts "#{__FILE__} 0.1"
end

texture_attrs = {}
texture_attrs['version'] = 1

texture_dir = '.'

verbose = false

getopt = GetoptLong.new(*options)
getopt.each_option do |opt,arg|
  case opt
    when '--help'
      usage()
      exit

    when '--version'
      version()
      exit

    when '--verbose'
      verbose = true

    when '--dir'
      unless File.directory?(arg)
        $stderr.puts "#{arg} is not a directory"
        exit 1
      end
      texture_dir = arg

    when '--filter'
      unless ['nearest', 'linear', 'trilinear'].include?(arg)
        $stderr.puts "Invalid filter mode '#{arg}'"
        exit 1
      end
      texture_attrs['filter'] = arg

    when '--address'
      unless ['wrap', 'clamp'].include?(arg)
        $stderr.puts "Invalid address mode '#{arg}'"
        exit 1
      end
      texture_attrs['address'] = arg

    when '--rectangular'
      texture_attrs['rectangular'] = 'true'

    when '--mipmapped'
      texture_attrs['mipmapped'] = 'true'
  end
end

pattern = /([^\/]+)\.png$/

ARGV.each do |path|
  match = pattern.match(path)
  next unless match

  name = match[1]

  texture_path = texture_dir + '/' + name + '.texture'
  texture_path.gsub!(/\/\//, '/')

  begin
    File.open(texture_path, 'wb') do |file|
      document = REXML::Document.new
      document << REXML::XMLDecl.new

      texture_attrs['image'] = name
      root = document.add_element('texture', texture_attrs)

      formatter = REXML::Formatters::Pretty.new(2, true)
      formatter.write(document, file)
    end
    $stderr.puts "Created #{texture_path}" if verbose
  rescue
    $stderr.puts "Failed to create #{texture_path}"
    exit 1
  end
end

