METASOURCES = AUTO

AM_CPPFLAGS = $(KOPETE_INCLUDES) -I$(top_srcdir)/kopete/libkopete/private $(all_includes) -I.. -I../chatwindow -I../addcontactwizard

noinst_LTLIBRARIES = libkopetecontactlist.la

libkopetecontactlist_la_SOURCES = kopetewindow.cpp kopetemetacontactlvi.cpp kopetestatusgroupviewitem.cpp kopetegroupviewitem.cpp \
	statusbaricon.cpp kopetecontactlistview.cpp

libkopetecontactlist_la_LDFLAGS = $(all_libraries)
libkopetecontactlist_la_LIBADD = ../../libkopete/libkopete.la ../addcontactwizard/libkopeteaddcontactwizard.la $(LIB_KDEUI)

# vim: set noet:

