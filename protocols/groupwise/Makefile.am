

SUBDIRS = icons libgroupwise ui

AM_CPPFLAGS = $(KOPETE_INCLUDES) $(all_includes)

METASOURCES = AUTO

noinst_HEADERS = gwprotocol.h gwcontact.h gwaccount.h gwfakeserver.h \
	gwincomingmessage.h gwbytestream.h gwconnector.h gwmessagemanager.h
kde_module_LTLIBRARIES =  kopete_groupwise.la
kopete_groupwise_la_SOURCES = gwprotocol.cpp gwcontact.cpp gwaccount.cpp \
	gwfakeserver.cpp gwincomingmessage.cpp gwbytestream.cpp gwconnector.cpp \
	gwmessagemanager.cpp
kopete_groupwise_la_LDFLAGS = -module -no-undefined $(KDE_PLUGIN) \
	$(all_libraries)
kopete_groupwise_la_LIBADD = ui/libkopetegroupwiseui.la \
	libgroupwise/libgroupwise.la ../../libkopete/libkopete.la $(LIB_KIO)

service_DATA = kopete_groupwise.desktop
servicedir = $(kde_servicesdir)
INCLUDES = -I$(top_srcdir)/kopete/protocols/groupwise/libgroupwise/tasks \
	-I$(top_srcdir)/kopete/protocols/groupwise/libgroupwise/qca/src -I$(top_srcdir)/kopete/protocols/groupwise/libgroupwise

mydatadir = $(kde_datadir)/kopete_groupwise
mydata_DATA = gwchatui.rc
