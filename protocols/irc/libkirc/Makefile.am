KDE_OPTIONS = nofinal
noinst_LTLIBRARIES = libkirc.la

libkirc_la_SOURCES = \
	kircengine.cpp \
	kircengine_commands.cpp \
	kircengine_ctcp.cpp \
	kircengine_numericreplies.cpp \
	kircentity.cpp \
	kircmessage.cpp \
	kircmessageredirector.cpp \
	kirctransfer.cpp \
	kirctransferhandler.cpp \
	kirctransferserver.cpp \
	ksslsocket.cpp
libkirc_la_LDFLAGS = -no-undefined $(KDE_PLUGIN) $(all_libraries)
libkirc_la_LIBADD = $(LIB_KIO)

AM_CPPFLAGS = -I$(top_srcdir)/kopete/protocols/irc $(KOPETE_INCLUDES) $(all_includes)
METASOURCES = AUTO
