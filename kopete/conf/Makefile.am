METASOURCES = AUTO

AM_CPPFLAGS = $(KOPETE_INCLUDES) \
        -I$(top_srcdir)/kopete/libkopete/private $(all_includes) \
        -I$(top_srcdir)/kopete/kopete/conf \
        -I$(top_builddir)/kopete/kopete/conf
        
noinst_LTLIBRARIES = libkopeteconf.la
noinst_HEADERS = pluginconfig.h appearanceconfig.h accountconfig.h addaccountwizard.h

libkopeteconf_la_SOURCES = pluginconfig.cpp \
	appearanceconfig_general.ui appearanceconfig_contactlist.ui \
	appearanceconfig_chatwindow.ui appearanceconfig_chatappearance.ui \
	appearanceconfig.cpp kopeteawayconfigbase.ui kopeteawayconfigui.cpp \
	accountconfigbase.ui accountconfig.cpp addaccountwizard_base.ui \
	addaccountwizard.cpp styleeditdialog.ui

libkopeteconf_la_LDFLAGS = $(all_libraries)
libkopeteconf_la_LIBADD = ../../libkopete/libkopete.la $(LIB_KDEUI)

# vim: set noet:

