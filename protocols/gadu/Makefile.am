METASOURCES = AUTO
SUBDIRS = lib ui pixmaps icons
AM_CPPFLAGS = -I$(srcdir)/ui \
	-I./ui \
	-I./lib \
	-I$(srcdir)/lib \
	$(KOPETE_INCLUDES) \
	$(all_includes)

kde_module_LTLIBRARIES = kopete_gadu.la

kopete_gadu_la_SOURCES = gaducommands.cpp gadusession.cpp      \
		       gaducontact.cpp gaduaddcontactpage.cpp  \
		       gadupreferences.cpp gaduprotocol.cpp    \
		       gaduaccount.cpp gadueditaccount.cpp     \
		       gaduaway.cpp

kopete_gadu_la_LDFLAGS = -no-undefined -module $(KDE_PLUGIN) $(all_libraries)
kopete_gadu_la_LIBADD  = ./ui/libgaduui.la \
		       ./lib/libgadu.la      \
		       $(top_builddir)/kopete/libkopete/libkopete.la 

service_DATA = gadu.desktop
servicedir = $(kde_servicesdir)
