METASOURCES = AUTO
SUBDIRS = ui . icons config
AM_CPPFLAGS = -I$(srcdir)/ui \
	-I./ui \
	$(KOPETE_INCLUDES) \
	$(all_includes)

kde_module_LTLIBRARIES = kopete_msn.la
lib_LTLIBRARIES = libkopete_msn_shared.la

CLEANFILES = dummy.cpp

libkopete_msn_shared_la_SOURCES = msnprotocol.cpp msnaccount.cpp msnaddcontactpage.cpp msncontact.cpp \
	msnsocket.cpp msnmessagemanager.cpp msndebugrawcmddlg.cpp \
	msnnotifysocket.cpp msnswitchboardsocket.cpp msnfiletransfersocket.cpp msninvitation.cpp \
	msnp2p.cpp msnp2pdisplatcher.cpp msnp2pincoming.cpp msnp2poutgoing.cpp sha1.cpp

libkopete_msn_shared_la_LIBADD  = ./ui/libkopetemsnui.la ../../libkopete/libkopete.la $(LIB_KIO)
libkopete_msn_shared_la_LDFLAGS = -version-info 0:0:0 -no-undefined $(all_libraries)

kopete_msn_la_SOURCES = dummy.cpp
kopete_msn_la_LIBADD = libkopete_msn_shared.la

kopete_msn_la_LDFLAGS = -no-undefined -module $(KDE_PLUGIN) $(all_libraries)

dummy.cpp: $(srcdir)/Makefile.am
	echo '#include "kdemacros.h"' > $@
	echo 'extern "C" KDE_EXPORT void *init_libkopete_msn_shared();' >> $@
	echo 'extern "C" KDE_EXPORT void *init_kopete_msn() { return init_libkopete_msn_shared(); }' >> $@

service_DATA = kopete_msn.desktop
servicedir = $(kde_servicesdir)

mydatadir = $(kde_datadir)/kopete_msn
mydata_DATA = msnchatui.rc

