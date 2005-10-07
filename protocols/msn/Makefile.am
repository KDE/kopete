if include_msn_webcam
WEBCAM = webcam
WEBCAM_LIBMINICWRAPPER = webcam/libmimicwrapper.la
endif

KDE_OPTIONS = nofinal
METASOURCES = AUTO
SUBDIRS = ui $(WEBCAM) . icons config
AM_CPPFLAGS = -Iui \
	-I$(srcdir)/webcam \
	-I$(srcdir)/ui \
	$(KOPETE_INCLUDES) \
	$(all_includes)

kde_module_LTLIBRARIES = kopete_msn.la
lib_LTLIBRARIES = libkopete_msn_shared.la

CLEANFILES = dummy.cpp

libkopete_msn_shared_la_SOURCES = msnprotocol.cpp msnaccount.cpp \
	msnaddcontactpage.cpp msncontact.cpp msnsocket.cpp msnchatsession.cpp msndebugrawcmddlg.cpp \
	msnnotifysocket.cpp msnswitchboardsocket.cpp msnfiletransfersocket.cpp msninvitation.cpp \
	sha1.cpp msnsecureloginhandler.cpp msnchallengehandler.cpp dispatcher.cpp \
	p2p.cpp messageformatter.cpp incomingtransfer.cpp outgoingtransfer.cpp \
	webcam.cpp

libkopete_msn_shared_la_LIBADD  = ./ui/libkopetemsnui.la ../../libkopete/libkopete.la $(WEBCAM_LIBMINICWRAPPER) ../../libkopete/avdevice/libkopete_videodevice.la   $(LIB_KIO)
libkopete_msn_shared_la_LDFLAGS = -version-info 0:0:0 -no-undefined $(all_libraries)

kopete_msn_la_SOURCES = dummy.cpp webcam.cpp
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
noinst_HEADERS = p2p.h dispatcher.h messageformatter.h incomingtransfer.h \
	outgoingtransfer.h webcam.h
