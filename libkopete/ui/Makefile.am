INCLUDES = -I$(top_srcdir)/kopete/library $(all_includes)
noinst_LTLIBRARIES = libkopeteui.la

noinst_HEADERS = aboutplugins.h\     
		contactlist.h\   
		pluginmodule.h\
		configmodule.h\     
		kopetelistview.h\  
		preferencesdialog.h\
		connectionlabel.h\
		kopetewindow.h\
		statusbaricon.h
#CXXFLAGS = -g -O2 -Wall
DEFS = -I. -I$(srcdir) -I$(top_srcdir)

libkopeteui_la_SOURCES = aboutplugins.cpp configmodule.cpp kopetewindow.cpp preferencesdialog.cpp contactlist.cpp connectionlabel.cpp pluginmodule.cpp statusbaricon.cpp addwizard.ui addwizardimpl.cpp
libkopeteui_la_METASOURCES = AUTO
libkopeteui_la_LDFLAGS     = 
libkopeteui_la_LIBADD      =  
