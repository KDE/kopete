METASOURCES = AUTO
if include_libggcopy
LIBGADU_COPY=libgadu
GG_INCLUDES=-Ilibgadu -I$(srcdir)/libgadu $(SSL_INCLUDES)
GG_LIBS=$(top_builddir)/kopete/protocols/gadu/libgadu/libgadu_copy.la \
	   $(LIBPTHREAD)
else
LIBGADU_COPY=
GG_INCLUDES=$(LIBGG_INCLUDES)
GG_LIBS=$(LIBGG_LIBS)
endif


SUBDIRS = ui icons $(LIBGADU_COPY)
AM_CPPFLAGS = -I$(srcdir)/ui \
	-I./ui \
	-I./lib \
	-I$(srcdir)/lib \
	$(KOPETE_INCLUDES) \
	$(all_includes) \
	$(GG_INCLUDES)

kde_module_LTLIBRARIES = kopete_gadu.la

kopete_gadu_la_SOURCES = gaduaway.cpp gadueditcontact.cpp gaducommands.cpp \
			 gadueditaccount.cpp gadusession.cpp gaducontact.cpp \
			 gaduaddcontactpage.cpp gaduprotocol.cpp gaduaccount.cpp \
			 gadupubdir.cpp gaduregisteraccount.cpp \
			 gaducontactlist.cpp gadurichtextformat.cpp \
			 gadudccserver.cpp gadudcctransaction.cpp gadudcc.cpp

kopete_gadu_la_LDFLAGS = -no-undefined -module $(KDE_PLUGIN) $(all_libraries)
kopete_gadu_la_LIBADD  = ./ui/libgaduui.la \
		       $(top_builddir)/kopete/libkopete/libkopete.la \
		       $(GG_LIBS)

service_DATA = kopete_gadu.desktop
servicedir = $(kde_servicesdir)
