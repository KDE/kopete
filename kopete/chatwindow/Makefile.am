kde_module_LTLIBRARIES = libkrichtexteditpart.la kopete_chatwindow.la kopete_emailwindow.la
METASOURCES = AUTO

AM_CPPFLAGS = $(KOPETE_INCLUDES) $(KOPETE_COMPAT_INCLUDES) -I$(top_srcdir)/kopete/libkopete/private    -I$(top_srcdir)/kopete/kopete $(all_includes)

kopete_chatwindow_la_SOURCES = chatmessagepart.cpp emoticonselector.cpp kopeteemoticonaction.cpp \
	chattexteditpart.cpp krichtexteditpart.cpp chatview.cpp kopetechatwindow.cpp chatmemberslistwidget.cpp
kopete_chatwindow_la_LDFLAGS = -no-undefined -module $(KDE_PLUGIN) $(all_libraries)
kopete_chatwindow_la_LIBADD = ../../libkopete/libkopete.la $(LIB_KOPETECOMPAT) $(LIB_KDEUI)

kopete_emailwindow_la_SOURCES = chatmessagepart.cpp emoticonselector.cpp kopeteemoticonaction.cpp \
	chattexteditpart.cpp krichtexteditpart.cpp kopeteemailwindow.cpp
kopete_emailwindow_la_LDFLAGS = -no-undefined -module $(KDE_PLUGIN) $(all_libraries)
kopete_emailwindow_la_LIBADD = ../../libkopete/libkopete.la $(LIB_KOPETECOMPAT) $(LIB_KDEUI)

libkrichtexteditpart_la_SOURCES = krichtexteditpart.cpp
libkrichtexteditpart_la_LDFLAGS = -module $(KDE_PLUGIN) $(all_libraries) $(KDE_RPATH)
libkrichtexteditpart_la_LIBADD = ../../libkopete/libkopete.la $(LIB_KPARTS) $(LIB_KSPELL) $(LIB_KDEPRINT)

partdir = $(kde_datadir)/kopeterichtexteditpart
part_DATA = kopeterichtexteditpartfull.rc

rcdir = $(kde_datadir)/kopete
rc_DATA = kopetechatwindow.rc kopeteemailwindow.rc

service_DATA = chatwindow.desktop emailwindow.desktop
servicedir = $(kde_servicesdir)

