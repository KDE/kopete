METASOURCES = AUTO

AM_CPPFLAGS = $(KOPETE_INCLUDES) -I$(top_srcdir)/kopete/kopete/addcontactwizard -I$(top_srcdir)/kopete/libkopete/private $(all_includes)

noinst_LTLIBRARIES = libkopetechatwindow.la

libkopetechatwindow_la_SOURCES = kopeteviewmanager.cpp kopeteemailwindow.cpp \
	kopetechatwindow.cpp chatview.cpp \
	ktabwidget.cpp \
	emoticonselector.cpp \
	kopeteemoticonaction.cpp


libkopetechatwindow_la_LDFLAGS = $(all_libraries)
libkopetechatwindow_la_LIBADD = ../../libkopete/libkopete.la $(LIB_KDEUI)

rcdir = $(kde_datadir)/kopete
rc_DATA = kopetechatwindow.rc kopeteemailwindow.rc


# vim: set noet:

