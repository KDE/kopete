METASOURCES = AUTO

SUBDIRS = ui . icons

AM_CPPFLAGS = $(KOPETE_INCLUDES) -I$(srcdir)/ui -Iui $(all_includes)

noinst_HEADERS = wlmprotocol.h wlmcontact.h wlmaccount.h wlmaddcontactpage.h wlmeditaccountwidget.h wlmincomingmessage.h wlmlibmsn.h wlmchatmanager.h wlmchatsession.h wlmtransfersession.h wlmtransfermanager.h
kde_module_LTLIBRARIES =  kopete_wlm.la
kopete_wlm_la_SOURCES = wlmprotocol.cpp wlmcontact.cpp wlmaccount.cpp wlmaddcontactpage.cpp wlmaddui.ui wlmeditaccountwidget.cpp wlmaccountpreferences.ui wlmincomingmessage.cpp wlmlibmsn.cpp wlmsocket.cpp wlmserver.cpp wlmchatmanager.cpp wlmchatsession.cpp wlmtransfersession.cpp wlmtransfermanager.cpp
kopete_wlm_la_LDFLAGS = -module $(KDE_PLUGIN) $(all_libraries) /usr/lib/libmsn.so.0.0.0
kopete_wlm_la_LIBADD = ../../libkopete/avdevice/libkopete_videodevice.la \
	ui/libkopetewlmui.la ../../libkopete/libkopete.la $(LIB_KIO)

service_DATA = kopete_wlm.desktop
servicedir= $(kde_servicesdir)

mydatadir = $(kde_datadir)/kopete_wlm
mydata_DATA = wlmchatui.rc

