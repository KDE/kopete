KDE_OPTIONS = nofinal
noinst_LTLIBRARIES = libkirc.la

#	kircengine_commands.cpp \
#	kircengine_ctcp.cpp \
#	kircengine_numericreplies.cpp \

libkirc_la_SOURCES = \
	kircclient.cpp \
	kirccommand.cpp \
	kirccommandhandler.cpp \
	kircconst.cpp \
	kircentity.cpp \
	kircentitymanager.cpp \
	kircmessage.cpp \
	kircsocket.cpp \
	kircstdcommands.cpp
#	kircclientcommands.cpp \
#	kircclientcommandhandler.cpp \
#	kirctransfer.cpp \
#	kirctransferhandler.cpp \
#	kirctransferserver.cpp
 
libkirc_la_LDFLAGS = -no-undefined $(KDE_PLUGIN) $(all_libraries)
libkirc_la_LIBADD = $(LIB_KIO)

AM_CPPFLAGS = -I$(top_srcdir)/kopete/protocols/irc $(KOPETE_INCLUDES) $(all_includes)
METASOURCES = AUTO
