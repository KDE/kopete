METASOURCES = AUTO
SUBDIRS = ui . pixmaps
INCLUDES = -I$(srcdir)/ui \
	-I./ui \
	-I$(srcdir)/../../libkopete \
	-I$(srcdir)/../../libkopete/ui \
	-I../../libkopete/ui \
	$(all_includes)

kde_module_LTLIBRARIES = kopete_msn.la

kopete_msn_la_SOURCES = msnprotocol.cpp msnidentity.cpp \
	msnpreferences.cpp msnaddcontactpage.cpp \
	msncontact.cpp msnsocket.cpp msnauthsocket.cpp msndispatchsocket.cpp msnmessagemanager.cpp\
	msndebugrawcmddlg.cpp msnnotifysocket.cpp msnswitchboardsocket.cpp msnfiletransfersocket.cpp
kopete_msn_la_LDFLAGS = -no-undefined -module $(KDE_PLUGIN)
kopete_msn_la_LIBADD  = ./ui/libkopetemsnui.la \
	../../libkopete/libkopete.la  $(LIB_KIO)

data_DATA = msn.plugin
datadir = $(kde_datadir)/kopete
