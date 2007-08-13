METASOURCES = AUTO

SUBDIRS = . icons liblig ui

AM_CPPFLAGS = $(KOPETE_INCLUDES) -I$(srcdir)/ui -Iui $(all_includes)

noinst_HEADERS = ligprotocol.h ligcontact.h ligaccount.h ligaddcontactpage.h ligeditaccountwidget.h ligserver.h ligincomingmessage.h
kde_module_LTLIBRARIES =  kopete_lig.la
kopete_lig_la_SOURCES = ligprotocol.cpp ligcontact.cpp ligaccount.cpp ligaddcontactpage.cpp ligaddui.ui ligeditaccountwidget.cpp ligaccountpreferences.ui ligserver.cpp ligincomingmessage.cpp
kopete_lig_la_LDFLAGS = -module $(KDE_PLUGIN) $(all_libraries)
kopete_lig_la_LIBADD = ../../libkopete/avdevice/libkopete_videodevice.la \
	ui/libkopeteligui.la ../../libkopete/libkopete.la $(LIB_KIO)

service_DATA = kopete_lig.desktop
servicedir= $(kde_servicesdir)
