METASOURCES = AUTO
INCLUDES = -I$(top_srcdir)/kopete/libkopete $(all_includes)

noinst_LTLIBRARIES = libkopeteui.la
noinst_HEADERS = kopetecontactlistview.h pluginmodule.h configmodule.h \
	kopetelistview.h preferencesdialog.h kopetewindow.h statusbaricon.h \
	protocolboxitem.h addcontactpage.h systemtray.h \
	appearanceconfig.h kopeteballoonbase.h kopeteballoon.h \
	kopetehistorydialog.h kopetecontactviewitem.h kopetestdaction.h  fileconfirmbase.h \
	kopetehistorywidget.h chatview.h kopetegroupviewitem.h kopetefileconfirmdialog.h

libkopeteui_la_SOURCES = configmodule.cpp kopetewindow.cpp \
	preferencesdialog.cpp kopetecontactlistview.cpp pluginmodule.cpp \
	statusbaricon.cpp addwizardui.ui addwizardimpl.cpp protocolboxitem.cpp \
	addcontactpage.cpp systemtray.cpp appearanceconfig.cpp \
	kopeteballoonbase.cpp kopeteballoon.cpp \
	kopetehistorydialog.cpp \
	kopetecontactviewitem.cpp kopetestdaction.cpp \
	kopetehistorywidget.cpp kopeteemailwindow.cpp emailwindow.ui \
	kopetemetacontactlvi.cpp kopeteawaydialogbase.ui \
	kopetefiletransferui.ui kopetegroupviewitem.cpp kopetefileconfirmdialog.cpp fileconfirmbase.ui \
	kopeteawayconfigbase.ui kopeteawayconfigui.cpp kopetechatwindow.cpp chatview.cpp prfschatwindow_ui.ui

libkopeteui_la_LDFLAGS = $(all_libraries)
libkopeteui_la_LIBADD = $(LIB_KDEUI) $(LIB_KIO)

rcdir = $(kde_datadir)/kopete
rc_DATA = kopetechatwindow.rc
