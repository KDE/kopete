noinst_LTLIBRARIES = libkopetemsnui.la
INCLUDES = -I$(top_srcdir)/kopete/library \
		-I$(top_srcdir)/kopete/library/ui \
		-I$(top_srcdir)/kopete/protocols/msn \
		-I$(top_srcdir)/kopete/protocols/msn/libkmerlin \
		-I$(top_builddir)/kopete/protocols/msn/ui \
		-I$(top_builddir)/kopete/protocols/msn/libkmerlin \
		$(all_includes)

CXXFLAGS = -g -O2 -Wall 
DEFS = -I. -I$(srcdir) -I$(top_srcdir)
libkopetemsnui_la_METASOURCES = AUTO
libkopetemsnui_la_SOURCES = msnadd.ui msnprefs.ui msnmessagedialog.cpp empty.cpp
EXTRA_DIST = msnadd.ui msnprefs.ui empty.cpp
