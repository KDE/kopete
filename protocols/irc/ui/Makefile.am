noinst_LTLIBRARIES = libkopeteircui.la
INCLUDES = $(all_includes)\
		-I$(top_srcdir)/kopete/library\
		-I$(top_srcdir)/kopete/library/ui\
		-I$(top_srcdir)/kopete/protocols/irc\
		-I$(top_srcdir)/kopete/protocols/irc/libkirc
CXXFLAGS = -g -O2 -Wall 
DEFS = -I. -I$(srcdir) -I$(top_srcdir)
libkopeteircui_la_METASOURCES = AUTO
libkopeteircui_la_SOURCES = ircadd.ui ircprefs.ui ircmessagebase.ui ircchatviewbase.ui ircchatwindow.cpp ircconsoleviewbase.ui ircqueryview.cpp empty.cpp
EXTRA_DIST = ircadd.ui ircprefs.ui empty.cpp
