SUBDIRS = ui pixmaps libkmsn
INCLUDES = -I$(top_srcdir)/kopete/protocols/msn/ui -I$(top_srcdir)/kopete/library -I$(top_srcdir)/kopete/library/ui -I$(top_srcdir)/kopete/protocols/msn/libkmsn $(all_includes)
lib_LTLIBRARIES = libkopetemsn.la

libkopetemsn_la_SOURCES     = msnprotocol.cpp msnpreferences.cpp protocol.cpp msnaddcontactpage.cpp
libkopetemsn_la_METASOURCES = AUTO
libkopetemsn_la_LDFLAGS     = 
libkopetemsn_la_LIBADD      = $(top_srcdir)/kopete/library/libkopete.la $(top_srcdir)/kopete/protocols/msn/ui/libkopetemsnui.la $(top_srcdir)/kopete/protocols/msn/libkmsn/libkmsn.la 

data_DATA = msn.plugin
datadir = $(kde_datadir)/kopete

