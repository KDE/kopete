METASOURCES = AUTO
INCLUDES = -I$(top_srcdir)/kopete/libkopete $(all_includes)

noinst_LTLIBRARIES = libkopeteui.la
noinst_HEADERS = pluginmodule.h configmodule.h \
	preferencesdialog.h \
	protocolboxitem.h addcontactpage.h systemtray.h \
	appearanceconfig.h kopeteballoon.h \
	kopetehistorydialog.h kopetestdaction.h  fileconfirmbase.h \
	kopetehistorywidget.h chatview.h kopetefileconfirmdialog.h

libkopeteui_la_SOURCES = configmodule.cpp \
	preferencesdialog.cpp pluginmodule.cpp \
	addwizardui.ui addwizardimpl.cpp protocolboxitem.cpp \
	addcontactpage.cpp systemtray.cpp appearanceconfig.cpp \
	kopeteballoon.cpp \
	kopetehistorydialog.cpp \
	kopetestdaction.cpp \
	kopetehistorywidget.cpp kopeteemailwindow.cpp emailwindow.ui \
	kopeteawaydialogbase.ui \
	kopetefiletransferui.ui kopetefileconfirmdialog.cpp fileconfirmbase.ui \
	kopeteawayconfigbase.ui kopeteawayconfigui.cpp kopetechatwindow.cpp chatview.cpp prfschatwindow_ui.ui

libkopeteui_la_LDFLAGS = $(all_libraries)
libkopeteui_la_LIBADD = $(LIB_KDEUI) $(LIB_KIO)

rcdir = $(kde_datadir)/kopete
rc_DATA = kopetechatwindow.rc
