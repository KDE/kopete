METASOURCES = AUTO

SUBDIRS = . icons

AM_CPPFLAGS = $(KOPETE_INCLUDES) $(all_includes)

noinst_HEADERS = testbedprotocol.h testbedcontact.h testbedaccount.h testbedaddcontactpage.h testbededitaccountwidget.h testbedfakeserver.h testbedincomingmessage.h
kde_module_LTLIBRARIES =  kopete_testbed.la
kopete_testbed_la_SOURCES = testbedprotocol.cpp testbedcontact.cpp testbedaccount.cpp testbedaddcontactpage.cpp testbedaddui.ui testbededitaccountwidget.cpp testbedaccountpreferences.ui testbedfakeserver.cpp testbedincomingmessage.cpp
kopete_testbed_la_LDFLAGS = -no-undefined -module $(KDE_PLUGIN) $(all_libraries)
kopete_testbed_la_LIBADD = ../../libkopete/libkopete.la $(LIB_KIO)

service_DATA = testbed.desktop
servicedir= $(kde_servicesdir)
