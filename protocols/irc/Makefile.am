SUBDIRS = ui pixmaps libkirc
INCLUDES = -I$(srcdir)/ui -I$(srcdir)/../../library \
		-I$(srcdir)/../../library/ui \
		-I$./ui \
		-I$(srcdir)/libkirc \
		$(all_includes)
lib_LTLIBRARIES = libkopeteirc.la

libkopeteirc_la_SOURCES     = ircaddcontactpage.cpp irccontact.cpp ircmessage.cpp ircpreferences.cpp ircprotocol.cpp protocol.cpp ircchatview.cpp ircservermanager.cpp ircservercontact.cpp ircconsoleview.cpp
libkopeteirc_la_METASOURCES = AUTO
libkopeteirc_la_LDFLAGS     = 
libkopeteirc_la_LIBADD      = ../../library/libkopete.la \
		./ui/libkopeteircui.la \
		./libkirc/libkirc.la

data_DATA = irc.plugin
datadir = $(kde_datadir)/kopete
