METASOURCES = AUTO
SUBDIRS = libkmerlin ui pixmaps
INCLUDES = -I$(srcdir)/ui \
	-I./ui \
	-I$(srcdir)/libkmerlin \
	-I./libkmerlin \
	-I$(srcdir)/../../libkopete \
	-I$(srcdir)/../../libkopete/ui \
	-I../../libkopete/ui \
	$(all_includes)

kde_module_LTLIBRARIES = kopete_msn.la

kopete_msn_la_SOURCES = msnprotocol.cpp msnidentity.cpp \
	msnpreferences.cpp msnaddcontactpage.cpp \
	msncontact.cpp msnsocket.cpp msnauthsocket.cpp msndispatchsocket.cpp \
	msndebugrawcmddlg.cpp msnnotifysocket.cpp msnswitchboardsocket.cpp
kopete_msn_la_LDFLAGS = -no-undefined -module $(KDE_PLUGIN)
kopete_msn_la_LIBADD  = ./ui/libkopetemsnui.la \
	./libkmerlin/libkmerlin.la \
	../../libkopete/libkopete.la

data_DATA = msn.plugin
datadir = $(kde_datadir)/kopete

messages: rc.cpp
	$(EXTRACTRC) */*.ui >> rc.cpp
	$(XGETTEXT) -ki18n -C *.cpp *.h -o $(podir)/kopete_msn.pot

