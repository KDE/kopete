METASOURCES = AUTO
INCLUDES = -I$(top_srcdir)/kopete/libkopete $(all_includes)

noinst_LTLIBRARIES = libkopeteui.la
noinst_HEADERS = kopetecontactlistview.h pluginmodule.h configmodule.h \
	kopetelistview.h preferencesdialog.h kopetewindow.h statusbaricon.h \
	protocolboxitem.h addcontactpage.h systemtray.h chatview.h \
	appearanceconfig.h kopeteballoonbase.h kopeteballoon.h \
	kopetehistorydialog.h kopetecontactviewitem.h kopetestdaction.h \
	kopetehistorywidget.h

libkopeteui_la_SOURCES = configmodule.cpp kopetewindow.cpp \
	preferencesdialog.cpp kopetecontactlistview.cpp pluginmodule.cpp \
	statusbaricon.cpp addwizard.ui addwizardimpl.cpp protocolboxitem.cpp \
	addcontactpage.cpp systemtray.cpp chatview.cpp appearanceconfig.cpp \
	kopeteballoonbase.cpp kopeteballoon.cpp \
	kopetehistorydialog.cpp kopetechatwindow.cpp \
	kopetecontactviewitem.cpp kopetestdaction.cpp \
	kopetehistorywidget.cpp kopeteemailwindow.cpp emailwindow.ui \
	kopetemetacontactlvi.cpp kopeteawaydialogbase.ui \
	emoticonbutton.cpp
libkopeteui_la_LDFLAGS = $(all_libraries)
libkopeteui_la_LIBADD = $(LIB_KDEUI)

