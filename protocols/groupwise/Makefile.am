METASOURCES = ignore_global_warming.moc

SUBDIRS = . icons libgroupwise ui

AM_CPPFLAGS = $(KOPETE_INCLUDES) $(all_includes)

noinst_HEADERS = gwprotocol.h gwcontact.h gwaccount.h gwaddcontactpage.h \
	gweditaccountwidget.h gwfakeserver.h gwincomingmessage.h gwconnection.h \
	gwbytestream.h gwconnector.h
kde_module_LTLIBRARIES =  kopete_groupwise.la
kopete_groupwise_la_SOURCES = gwprotocol.cpp gwcontact.cpp gwaccount.cpp \
	gwfakeserver.cpp gwincomingmessage.cpp gwconnection.cpp gwbytestream.cpp gwconnector.cpp
kopete_groupwise_la_LDFLAGS = -module -no-undefined $(KDE_PLUGIN) \
	$(all_libraries)
kopete_groupwise_la_LIBADD = libgroupwise/libgroupwise.la \
	../../libkopete/libkopete.la $(LIB_KIO)

service_DATA = kopete_groupwise.desktop
servicedir = $(kde_servicesdir)
INCLUDES = -I$(top_srcdir)/kopete/protocols/groupwise/libgroupwise

