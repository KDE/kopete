SUBDIRS = ui pixmaps oscarsocket data
METASOURCES = AUTO
INCLUDES = -I$(srcdir)/ui \
    -I./ui \
    -I$(top_srcdir)/kopete/libkopete \
    -I$(top_srcdir)/kopete/libkopete/ui \
    -I../../libkopete/ui \
    -I$(srcdir)/oscarsocket \
    $(all_includes)

kde_module_LTLIBRARIES = kopete_oscar.la

#kopete_aim_la_SOURCES = aimprotocol.cpp \
#    aimcontact.cpp aimchangestatus.cpp aimuserinfo.cpp \
#    aimmergebuddies.cpp aimpreferences.cpp \
#    aimprocessbuddies.cpp aimdeleteevent.cpp

kopete_oscar_la_SOURCES = oscarprotocol.cpp oscarcontact.cpp oscarpreferences.cpp oscarchangestatus.cpp oscaruserinfo.cpp

kopete_oscar_la_LDFLAGS = -no-undefined -module $(KDE_PLUGIN) $(all_libraries)
kopete_oscar_la_LIBADD  = ./oscarsocket/libkopeteoscarsocket.la \
    ./ui/libkopeteoscarui.la $(top_builddir)/kopete/libkopete/libkopete.la \
    $(LIB_KHTML)

data_DATA = oscar.plugin
datadir = $(kde_datadir)/kopete

