METASOURCES = AUTO

AM_CPPFLAGS = $(KOPETE_INCLUDES) \
	-I$(top_srcdir)/kopete/libkopete/private \
	-I$(top_srcdir)/kopete/libkopete/ui \
	-I$(top_srcdir)/kopete/kopete \
	-I$(top_srcdir)/kopete/kopete/chatwindow \
	-I$(top_srcdir)/kopete/kopete/addcontactwizard \
	-I$(top_builddir)/kopete/kopete/addcontactwizard \
	$(all_includes)

noinst_LTLIBRARIES = libkopetecontactlist.la

libkopetecontactlist_la_SOURCES = kopetemetacontactlvi.cpp \
	kopetestatusgroupviewitem.cpp kopetegroupviewitem.cpp kopetecontactlistview.cpp \
	kopetegvipropswidget.ui kopetemetalvipropswidget.ui kopetelviprops.cpp \
	kopeteaddrbookexport.cpp kopeteaddrbookexportui.ui customnotifications.ui \
	customnotificationprops.cpp kopetegrouplistaction.cpp kabcexport.cpp \
	kabcexport_base.ui 

libkopetecontactlist_la_LDFLAGS = $(all_libraries)
libkopetecontactlist_la_LIBADD = -lkabc ../../libkopete/libkopete.la ../addcontactwizard/libkopeteaddcontactwizard.la $(LIB_KDEUI) $(LIB_XRENDER)

noinst_HEADERS = kopeteaddrbookexport.h customnotificationprops.h kabcexport.h 

KDE_OPTIONS = nofinal

# vim: set noet:
