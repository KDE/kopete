METASOURCES = AUTO
SUBDIRS = ui icons
AM_CPPFLAGS = -I$(srcdir)/ui \
	-I./ui \
	-I./lib \
	-I$(srcdir)/lib \
	$(KOPETE_INCLUDES) \
	$(all_includes) \
	$(LIBGG_INCLUDES)

kde_module_LTLIBRARIES = kopete_gadu.la

kopete_gadu_la_SOURCES = gaducommands.cpp gadusession.cpp	\
		       gaducontact.cpp gaduaddcontactpage.cpp	\
		       gaduprotocol.cpp gaduaccount.cpp		\
		       gadueditaccount.cpp gaduaway.cpp		\
		       gadupubdir.cpp gaduregisteraccount.cpp	\
		       gadueditcontact.cpp gaducontactlist.cpp

kopete_gadu_la_LDFLAGS = -no-undefined -module $(KDE_PLUGIN) $(all_libraries)
kopete_gadu_la_LIBADD  = ./ui/libgaduui.la \
		       $(top_builddir)/kopete/libkopete/libkopete.la \
		       $(LIBGG_LIBS)

service_DATA = kopete_gadu.desktop
servicedir = $(kde_servicesdir)
