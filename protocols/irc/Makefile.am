SUBDIRS = ui pixmaps libkirc
INCLUDES = -I$(top_srcdir)/kopete/protocols/irc/ui \
		-I$(top_srcdir)/kopete/library \
		-I$(top_srcdir)/kopete/library/ui \
		-I$(top_builddir)/kopete/protocols/irc/ui \
		-I$(top_srcdir)/kopete/protocols/irc/libkirc \
		$(all_includes)
lib_LTLIBRARIES = libkopeteirc.la

libkopeteirc_la_SOURCES     = ircaddcontactpage.cpp irccontact.cpp ircmessage.cpp ircpreferences.cpp ircprotocol.cpp protocol.cpp ircchatview.cpp ircservermanager.cpp ircservercontact.cpp
libkopeteirc_la_METASOURCES = AUTO
libkopeteirc_la_LDFLAGS     = 
libkopeteirc_la_LIBADD      = $(top_builddir)/kopete/library/libkopete.la \
		$(top_builddir)/kopete/protocols/irc/ui/libkopeteircui.la \
		$(top_builddir)/kopete/protocols/irc/libkirc/libkirc.la

data_DATA = irc.plugin
datadir = $(kde_datadir)/kopete
