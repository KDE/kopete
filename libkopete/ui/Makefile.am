METASOURCES = AUTO
AM_CPPFLAGS = -I$(top_srcdir)/kopete/libkopete $(all_includes)

noinst_LTLIBRARIES = libkopeteui.la
noinst_HEADERS = configmodule.h \
	preferencesdialog.h \
	addcontactpage.h systemtray.h \
	kopeteballoon.h \
	kopetehistorydialog.h kopetestdaction.h  fileconfirmbase.h \
	kopetehistorywidget.h chatview.h kopetefileconfirmdialog.h

libkopeteui_la_SOURCES = configmodule.cpp \
	preferencesdialog.cpp kopetecontactaction.cpp \
	addcontactpage.cpp systemtray.cpp \
	kopeteballoon.cpp \
	kopetehistorydialog.cpp \
	kopetestdaction.cpp \
	kopetehistorywidget.cpp kopeteemailwindow.cpp emailwindow.ui \
	kopeteawaydialogbase.ui \
	kopetefiletransferui.ui kopetefileconfirmdialog.cpp fileconfirmbase.ui \
	kopeteawayconfigbase.ui kopeteawayconfigui.cpp kopetechatwindow.cpp chatview.cpp

libkopeteui_la_LDFLAGS = $(all_libraries)
libkopeteui_la_LIBADD = $(LIB_KDEUI) $(LIB_KIO) $(LIB_KHTML)

rcdir = $(kde_datadir)/kopete
rc_DATA = kopetechatwindow.rc
