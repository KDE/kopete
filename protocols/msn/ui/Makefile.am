noinst_LTLIBRARIES = libkopetemsnui.la
INCLUDES = $(all_includes)\
		-I$(top_srcdir)/kopete/library\
		-I$(top_srcdir)/kopete/library/ui\
		-I$(top_srcdir)/kopete/protocols/msn
CXXFLAGS = -g -O2 -Wall 
DEFS = -I. -I$(srcdir) -I$(top_srcdir)
libkopetemsnui_la_METASOURCES = AUTO
libkopetemsnui_la_SOURCES = msnadd.ui msnprefs.ui msnmessagebase.cpp empty.cpp
EXTRA_DIST = msnadd.ui msnprefs.ui empty.cpp
