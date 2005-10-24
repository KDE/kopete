METASOURCES = AUTO
AM_CPPFLAGS = -DKDE_NO_COMPAT -DQT_NO_COMPAT -DQT_NO_CAST_ASCII -DQT_NO_ASCII_CAST \
	$(KOPETE_INCLUDES) -I$(top_srcdir)/kopete/libkopete/private $(all_includes)

noinst_LTLIBRARIES = libkopeteui.la
libkopeteui_la_SOURCES = kopetecontactaction.cpp addcontactpage.cpp \
	editaccountwidget.cpp kopetepassworddialog.ui kopetestdaction.cpp kopeteawaydialogbase.ui \
	kopetefileconfirmdialog.cpp fileconfirmbase.ui userinfodialog.cpp kopeteview.cpp \
	kopetepasswordwidgetbase.ui kopetepasswordwidget.cpp accountselector.cpp kopeteviewplugin.cpp \
	addresseeitem.cpp addressbookselectorwidget_base.ui addressbookselectordialog.cpp \
	addressbookselectorwidget.cpp metacontactselectorwidget_base.ui	metacontactselectorwidget.cpp \
	kopetelistview.cpp kopetelistviewitem.cpp kopetelistviewsearchline.cpp \
	contactaddednotifywidget.ui contactaddednotifydialog.cpp

libkopeteui_la_LDFLAGS = $(all_libraries) -lkabc
libkopeteui_la_LIBADD = ../private/libkopeteprivate.la $(LIB_KHTML) $(LIBXML_LIBS) $(LIBXSLT_LIBS)

kopeteincludedir = $(includedir)/kopete/ui
kopeteinclude_HEADERS = accountselector.h fileconfirmbase.h kopetefileconfirmdialog.h \
	kopetepasswordwidget.h kopeteview.h addcontactpage.h kopeteawaydialogbase.h  \
	kopetepasswordwidgetbase.h kopeteviewplugin.h \
	editaccountwidget.h kopetecontactaction.h kopetepassworddialog.h kopetestdaction.h \
	userinfodialog.h addressbookselectordialog.h addressbookselectorwidget.h kopetelistview.h kopetelistviewitem.h kopetelistviewsearchline.h

noinst_HEADERS = addresseeitem.h contactaddednotifywidget.h

kde_widget_LTLIBRARIES = kopetewidgets.la
kopetewidgets_la_CXXFLAGS = -DQT_PLUGIN
kopetewidgets_la_LDFLAGS = $(KDE_PLUGIN) -module $(all_libraries)
kopetewidgets_la_LIBADD = ../libkopete.la 
kopetewidgets_la_SOURCES = kopetewidgets.cpp

kopetewidgets.cpp: $(srcdir)/kopete.widgets
	makekdewidgets -o kopetewidgets.cpp $(srcdir)/kopete.widgets	

CLEANFILES = kopetewidgets.cpp

# vim: set noet:

