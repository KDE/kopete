METASOURCES = AUTO
SUBDIRS = lib ui pixmaps
AM_CPPFLAGS = -I$(srcdir)/ui \
	 -I./ui \
	 -I$(srcdir)/lib  \
	 -I$(top_srcdir)/kopete/libkopete \
	 -I$(top_srcdir)/kopete/libkopete/ui \
	 -I../../libkopete/ui \
	 -I$(srcdir)/lib \
	 $(all_includes)

kde_module_LTLIBRARIES = kopete_gadu.la

kopete_gadu_la_SOURCES = gaducommands.cpp gadusession.cpp     \
		       gaducontact.cpp gaduaddcontactpage.cpp  \
		       gadupreferences.cpp gaduprotocol.cpp

kopete_gadu_la_LDFLAGS = -no-undefined -module $(KDE_PLUGIN) $(all_libraries)
kopete_gadu_la_LIBADD  = ./ui/libgaduui.la \
		       ./lib/libgadu.la      \
		       $(top_builddir)/kopete/libkopete/libkopete.la 

data_DATA = gadu.plugin
datadir = $(kde_datadir)/kopete
