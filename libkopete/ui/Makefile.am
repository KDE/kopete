METASOURCES = AUTO
AM_CPPFLAGS = -DKDE_NO_COMPAT -DQT_NO_COMPAT -DQT_NO_CAST_ASCII -DQT_NO_ASCII_CAST \
	$(KOPETE_INCLUDES) -I$(top_srcdir)/kopete/libkopete/private $(all_includes)

noinst_LTLIBRARIES = libkopeteui.la

libkopeteui_la_SOURCES = configmodule.cpp preferencesdialog.cpp \
	kopetecontactaction.cpp addcontactpage.cpp \
	editaccountwidget.cpp kopetehistorydialog.cpp \
	kopetepassworddialog.ui kopetestdaction.cpp \
	kopetehistorywidget.cpp kopeteemailwindow.cpp \
	kopeteawaydialogbase.ui kopetefiletransferui.ui \
	kopetefileconfirmdialog.cpp fileconfirmbase.ui \
	kopetechatwindow.cpp chatview.cpp \
	userinfodialog.cpp ktabwidget.cpp \
	kopeteview.cpp emoticonselector.cpp \
	kopeteemoticonaction.cpp

libkopeteui_la_LDFLAGS = $(all_libraries)
libkopeteui_la_LIBADD = $(LIB_KDEUI) $(LIB_KIO) $(LIB_KHTML)

rcdir = $(kde_datadir)/kopete
rc_DATA = kopetechatwindow.rc kopeteemailwindow.rc

# vim: set noet:

