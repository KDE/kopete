METASOURCES = AUTO
AM_CPPFLAGS = -DKDE_NO_COMPAT -DQT_NO_COMPAT -DQT_NO_CAST_ASCII -DQT_NO_ASCII_CAST \
	$(KOPETE_INCLUDES) -I$(top_srcdir)/kopete/libkopete/private $(all_includes)

noinst_LTLIBRARIES = libkopeteui.la

libkopeteui_la_SOURCES = kopetecontactaction.cpp addcontactpage.cpp \
	editaccountwidget.cpp kopetepassworddialog.ui kopetestdaction.cpp \
	kopeteawaydialogbase.ui kopetefiletransferui.ui kopetefileconfirmdialog.cpp \
	fileconfirmbase.ui kopeteview.cpp kopetepasswordwidgetbase.ui \
	kopetepasswordwidget.cpp

libkopeteui_la_LDFLAGS = $(all_libraries)
libkopeteui_la_LIBADD = ../private/libkopeteprivate.la $(LIB_KHTML) $(LIBXML_LIBS) $(LIBXSLT_LIBS)

# vim: set noet:

