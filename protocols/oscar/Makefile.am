SUBDIRS = ui pixmaps icons oscarsocket data icq aim
METASOURCES = AUTO
AM_CPPFLAGS = -I$(srcdir)/ui \
    -I./ui \
    -I../../libkopete/ui \
	$(KOPETE_INCLUDES) \
    -I$(srcdir)/oscarsocket \
    $(all_includes)

lib_LTLIBRARIES = kopete_oscar.la

#kopete_aim_la_SOURCES = aimprotocol.cpp \
#    aimcontact.cpp aimchangestatus.cpp aimuserinfo.cpp \
#    aimmergebuddies.cpp aimpreferences.cpp \
#    aimprocessbuddies.cpp aimdeleteevent.cpp

kopete_oscar_la_SOURCES = oscarprotocol.cpp oscarcontact.cpp \
			oscarpreferences.cpp oscarchangestatus.cpp \
			oscaruserinfo.cpp oscaraccount.cpp
#			icquserinfo.cpp icqaddcontactpage.cpp

kopete_oscar_la_LDFLAGS = -no-undefined -module $(KDE_PLUGIN) $(all_libraries)
kopete_oscar_la_LIBADD  = ./oscarsocket/libkopeteoscarsocket.la \
    ./ui/libkopeteoscarui.la $(top_builddir)/kopete/libkopete/libkopete.la

