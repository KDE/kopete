METASOURCES = AUTO

SUBDIRS = icons libkirc ui
AM_CPPFLAGS = -I$(srcdir)/ui $(KOPETE_INCLUDES) \
		-I./ui \
		-I$(srcdir)/libkirc \
		$(all_includes)
kde_module_LTLIBRARIES = kopete_irc.la

kopete_irc_la_SOURCES = \
	ircaccount.cpp  \
	ircaddcontactpage.cpp \
	ircchannelcontact.cpp \
	irccontact.cpp \
	ircguiclient.cpp \
	ircprotocol.cpp \
	ircservercontact.cpp \
	ircsignalhandler.cpp \
	irctransferhandler.cpp \
	ircusercontact.cpp \
	irccontactmanager.cpp \
	kcodecaction.cpp \
	ksparser.cpp

kopete_irc_la_LDFLAGS     = -module $(KDE_PLUGIN) $(all_libraries)
kopete_irc_la_LIBADD      = ../../libkopete/libkopete.la \
		./ui/libkopeteircui.la \
		./libkirc/libkirc.la \
		$(LIB_KIO)

service_DATA = kopete_irc.desktop irc.protocol
servicedir = $(kde_servicesdir)

xmldata_DATA = ircnetworks.xml
xmldatadir = $(kde_datadir)/kopete

EXTRA_DIST = $(xmldata_DATA)

mydatadir = $(kde_datadir)/kopete
mydata_DATA = ircchatui.rc
