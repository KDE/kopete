METASOURCES = AUTO

SUBDIRS = . icons

AM_CPPFLAGS = $(KOPETE_INCLUDES) $(all_includes)

noinst_HEADERS = gwprotocol.h gwcontact.h gwaccount.h gwaddcontactpage.h gweditaccountwidget.h gwfakeserver.h gwincomingmessage.h
kde_module_LTLIBRARIES =  kopete_groupwise.la
kopete_groupwise_la_SOURCES = gwprotocol.cpp gwcontact.cpp gwaccount.cpp gwaddcontactpage.cpp gwaddui.ui gweditaccountwidget.cpp gwaccountpreferences.ui gwfakeserver.cpp gwincomingmessage.cpp
kopete_groupwise_la_LDFLAGS = -no-undefined -module $(KDE_PLUGIN) $(all_libraries)
kopete_groupwise_la_LIBADD = ../../libkopete/libkopete.la $(LIB_KIO)

service_DATA = kopete_groupwise.desktop
servicedir= $(kde_servicesdir)
