SUBDIRS = ui pixmaps icons libkirc
AM_CPPFLAGS = -I$(srcdir)/ui $(KOPETE_INCLUDES) \
		-I./ui \
		-I$(srcdir)/libkirc \
		$(all_includes)
kde_module_LTLIBRARIES = kopete_irc.la

kopete_irc_la_SOURCES     = ircaddcontactpage.cpp ircchannelcontact.cpp \
	irccontact.cpp ircidentity.cpp ircpreferences.cpp \
	ircprotocol.cpp ircusercontact.cpp ksparser.cpp

kopete_irc_la_METASOURCES = AUTO
kopete_irc_la_LDFLAGS     = -module $(KDE_PLUGIN) $(all_libraries) 
kopete_irc_la_LIBADD      = ../../libkopete/libkopete.la \
		./ui/libkopeteircui.la \
		./libkirc/libkirc.la \
		$(LIB_KIO)

data_DATA = irc.desktop
datadir = $(kde_datadir)/kopete
