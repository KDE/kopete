#!/usr/bin/ruby
#
#    Copyright (c) 2005      by Duncan Mac-Vicar       <duncan@kde.org>
#
#    Kopete    (c) 2002-2005 by the Kopete developers  <kopete-devel@kde.org>
#
#    *************************************************************************
#    *                                                                       *
#    * This program is free software; you can redistribute it and/or modify  *
#    * it under the terms of the GNU General Public License as published by  *
#    * the Free Software Foundation; either version 2 of the License, or     *
#    * (at your option) any later version.                                   *
#    *                                                                       *
#    *************************************************************************

className = ARGV[0]

if className.nil?
	puts "Need a class name"
	exit
end

puts "Creating test for class #{className}"

hBase = "template_test.h"
cppBase = "template_test.cpp"

fileH = File.new(hBase).read
fileCpp = File.new(cppBase).read

fileH.gsub!(/TEMPLATE/, className.upcase.gsub(/::/,""))
fileH.gsub!(/Template/, className.gsub(/::/,""))
fileH.gsub!(/some requirement/, className + " class.")

fileCpp.gsub!(/TEMPLATE/, className.upcase.gsub(/::/,""))
fileCpp.gsub!(/template/, className.downcase.gsub(/::/,""))
fileCpp.gsub!(/Template/, className.gsub(/::/,""))
fileCpp.gsub!(/some requirement/, className + " class.")

makefileAm = "kunittest_template_test_la_SOURCES = template_test.cpp\nkunittest_template_test_la_LIBADD = -lkunittest ../mock/libkopete_mock.la\nkunittest_template_test_la_LDFLAGS = -module $(KDE_CHECK_PLUGIN) $(all_libraries)\n"
makefileAm.gsub!(/template/, className.downcase.gsub(/::/,""))

hNew = hBase.gsub(/template/, className.downcase.gsub(/::/,""))
cppNew = cppBase.gsub(/template/, className.downcase.gsub(/::/,""))

hOut = File.new(hNew, "w")
cppOut = File.new(cppNew, "w")

hOut.write(fileH)
cppOut.write(fileCpp)

puts "#{hNew} and #{cppNew} writen."

puts "Please add the following to Makefile.am:"
puts makefileAm

