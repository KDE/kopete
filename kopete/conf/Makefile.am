METASOURCES = AUTO

AM_CPPFLAGS = $(KOPETE_INCLUDES) \
        -I$(top_srcdir)/kopete/libkopete/private \
        -I$(top_srcdir)/kopete/kopete/conf \
        -I$(top_builddir)/kopete/kopete/conf \
        -I$(top_builddir)/kopete/libkopete $(all_includes)

noinst_LTLIBRARIES = libkopeteconf.la
noinst_HEADERS = pluginconfig.h appearanceconfig.h  \
	behaviorconfig.h

libkopeteconf_la_SOURCES = pluginconfig.cpp \
	styleeditdialog.ui appearanceconfig_chatwindow.ui \
	appearanceconfig_colors.ui appearanceconfig.cpp \
	behaviorconfig_general.ui behaviorconfig_chat.ui behaviorconfig.cpp \
	kopeteawayconfigbase.ui kopeteawayconfigui.cpp
libkopeteconf_la_LDFLAGS = $(all_libraries) -no-undefined
libkopeteconf_la_LIBADD = ../../libkopete/libkopete.la -lktexteditor $(LIB_KDEUI)

# vim: set noet:
