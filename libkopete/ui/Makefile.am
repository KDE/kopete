INCLUDES = -I$(top_srcdir)/kopete/libkopete $(all_includes)
noinst_LTLIBRARIES = libkopeteui.la

noinst_HEADERS = contactlist.h\
		pluginmodule.h\
		configmodule.h\
		kopetelistview.h\
		preferencesdialog.h\
		kopetewindow.h\
		statusbaricon.h\
		protocolboxitem.h\
		addcontactpage.h\
		systemtray.h \
		chatview.h \
		appearanceconfig.h\
		groupseparator.h\
		balloonbase.h\
		kopeteballoon.h \
		historydialog.h \
		kopetecontactviewitem.h \
		kopetestdaction.h \
		historywidget.h

#CXXFLAGS = -g -O2 -Wall
#DEFS = -I. -I$(srcdir) -I$(top_srcdir)

libkopeteui_la_SOURCES = configmodule.cpp\
			kopetewindow.cpp preferencesdialog.cpp\
			contactlist.cpp \
			pluginmodule.cpp statusbaricon.cpp\
			addwizard.ui addwizardimpl.cpp\
			protocolboxitem.cpp addcontactpage.cpp\
			systemtray.cpp chatview.cpp appearanceconfig.cpp\
			groupseparator.cpp balloonbase.cpp kopeteballoon.cpp \
			historydialog.cpp kopetechatwindow.cpp kopetecontactviewitem.cpp \
			kopetestdaction.cpp historywidget.cpp

libkopeteui_la_METASOURCES = AUTO
