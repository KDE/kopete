SUBDIRS = ui
INCLUDES = -I$(top_srcdir)/kopete/library $(all_includes)
lib_LTLIBRARIES = libkopete.la
# kopete_include_HEADERS = contactlist.h  imcontact.h  improtocol.h kopete.h pluginmanager.h

libkopete_la_SOURCES = contactlist.cpp kopete.cpp improtocol.cpp imcontact.cpp pluginmanager.cpp
libkopete_la_METASOURCES = AUTO
libkopete_la_LDFLAGS     =
libkopete_la_LIBADD      =  $(top_srcdir)/kopete/library/ui/libui.a
