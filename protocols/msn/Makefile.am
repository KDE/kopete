SUBDIRS = libkmerlin ui pixmaps
INCLUDES = -I$(srcdir)/ui \
        -I./ui \
        -I$(srcdir)/libkmerlin \
        -I./libkmerlin \
        -I$(srcdir)/../../library \
        -I$(srcdir)/../../library/ui \
        $(all_includes)
kde_module_LTLIBRARIES = kopete_msn.la

kopete_msn_la_SOURCES     = msnprotocol.cpp msnpreferences.cpp protocol.cpp msnaddcontactpage.cpp msncontact.cpp msnmessage.cpp msnuser.cpp
kopete_msn_la_METASOURCES = AUTO
kopete_msn_la_LDFLAGS     = -module $(KDE_PLUGIN)
kopete_msn_la_LIBADD      = ./ui/libkopetemsnui.la \
        ./libkmerlin/libkmerlin.la ../../library/libkopete.la 

data_DATA = msn.plugin
datadir = $(kde_datadir)/kopete

