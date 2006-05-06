METASOURCES = AUTO

SUBDIRS = ui . icons

AM_CPPFLAGS = $(KOPETE_INCLUDES) -I$(srcdir)/ui -Iui $(all_includes)

noinst_HEADERS = testbedprotocol.h testbedcontact.h testbedaccount.h testbedaddcontactpage.h testbededitaccountwidget.h testbedfakeserver.h testbedincomingmessage.h
kde_module_LTLIBRARIES =  kopete_testbed.la
kopete_testbed_la_SOURCES = testbedprotocol.cpp testbedcontact.cpp testbedaccount.cpp testbedaddcontactpage.cpp testbedaddui.ui testbededitaccountwidget.cpp testbedaccountpreferences.ui testbedfakeserver.cpp testbedincomingmessage.cpp
kopete_testbed_la_LDFLAGS = -module $(KDE_PLUGIN) $(all_libraries)
kopete_testbed_la_LIBADD = ../../libkopete/avdevice/libkopete_videodevice.la \
	ui/libkopetetestbedui.la ../../libkopete/libkopete.la $(LIB_KIO)

service_DATA = kopete_testbed.desktop
servicedir= $(kde_servicesdir)
