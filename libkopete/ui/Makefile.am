METASOURCES = AUTO
AM_CPPFLAGS = -DKDE_NO_COMPAT -DQT_NO_COMPAT -DQT_NO_CAST_ASCII -DQT_NO_ASCII_CAST \
    -I$(top_srcdir)/kopete/libkopete $(all_includes)

noinst_LTLIBRARIES = libkopeteui.la
noinst_HEADERS = configmodule.h \
	preferencesdialog.h \
	addcontactpage.h  \
	kopetehistorydialog.h kopetestdaction.h  fileconfirmbase.h \
	kopetehistorywidget.h chatview.h kopetefileconfirmdialog.h \
        kopeteemailwindow.h userinfodialog.h

libkopeteui_la_SOURCES = configmodule.cpp \
	preferencesdialog.cpp kopetecontactaction.cpp \
	addcontactpage.cpp \
	kopetehistorydialog.cpp \
	kopetestdaction.cpp \
	kopetehistorywidget.cpp kopeteemailwindow.cpp \
	kopeteawaydialogbase.ui \
	kopetefiletransferui.ui kopetefileconfirmdialog.cpp fileconfirmbase.ui \
	kopetechatwindow.cpp chatview.cpp userinfodialog.cpp ktabwidget.cpp

libkopeteui_la_LDFLAGS = $(all_libraries)
libkopeteui_la_LIBADD = $(LIB_KDEUI) $(LIB_KIO) $(LIB_KHTML)

rcdir = $(kde_datadir)/kopete
rc_DATA = kopetechatwindow.rc kopeteemailwindow.rc
