INCLUDES = -I$(srcdir)/.. $(all_includes)
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
		historydialog.h

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
			historydialog.cpp

libkopeteui_la_METASOURCES = AUTO
libkopeteui_la_LDFLAGS     =
libkopeteui_la_LIBADD      = $(LIB_KDEUI)
