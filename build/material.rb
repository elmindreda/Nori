#!/usr/bin/env ruby

require 'getoptlong'
require 'rexml/document'
require 'rexml/formatters/pretty'

def usage()
end

def version()
  $stdout.puts "#{__FILE__} 0.1"
end

options = [[ '--help', '-h', GetoptLong::NO_ARGUMENT ],
           [ '--version', '-v', GetoptLong::NO_ARGUMENT ],
           [ '--verbose', '-V', GetoptLong::NO_ARGUMENT],
           [ '--dir', '-d', GetoptLong::REQUIRED_ARGUMENT],
           [ '--program', '-p', GetoptLong::REQUIRED_ARGUMENT]]

material_dir = '.'
program_name = 'FAIL'

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
      material_dir = arg

    when '--program'
      program_name = arg
  end
end

ARGV.each do |path|
  next unless path =~ /\.obj$/

  names = []

  File.open(path, 'rb') do |file|
    usemtl = /^usemtl\s(\w+)/
    file.each_line do |line|
      match = usemtl.match(line)
      next unless match
      names << match[1]
    end
  end

  names.sort.uniq.each do |name|
    material_path = material_dir + '/' + name + '.material'
    next if File.exists?(material_path)

    begin
      File.open(material_path, 'wb') do |file|
        document = REXML::Document.new
        document << REXML::XMLDecl.new

        material = document.add_element('material', { 'version' => 4 })
        technique = material.add_element('technique', { 'quality' => 1 })
        pass = technique.add_element('pass')
        pass.add_element('program', { 'name' => program_name })

        formatter = REXML::Formatters::Pretty.new
        formatter.write(document, file)
      end
      $stdout.puts "Creating #{material_path}" if verbose
    rescue
      $stderr.puts "Failed to create #{material_path}"
      exit 1
    end
  end
end

