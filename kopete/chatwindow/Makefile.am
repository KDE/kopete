kde_module_LTLIBRARIES = libkrichtexteditpart.la kopete_chatwindow.la kopete_emailwindow.la
noinst_LTLIBRARIES = libkopetechatwindow.la
METASOURCES = AUTO

AM_CPPFLAGS = $(KOPETE_INCLUDES) $(KOPETE_COMPAT_INCLUDES) -I$(top_srcdir)/kopete/libkopete/private    -I$(top_srcdir)/kopete/kopete $(all_includes)

kopete_chatwindow_la_SOURCES = chatview.cpp kopetechatwindow.cpp chatmemberslistwidget.cpp
kopete_chatwindow_la_LDFLAGS = -no-undefined -module $(KDE_PLUGIN) $(all_libraries)
kopete_chatwindow_la_LIBADD = ../../libkopete/libkopete.la ./libkopetechatwindow.la $(LIB_KOPETECOMPAT) $(LIB_KDEUI)

kopete_emailwindow_la_SOURCES = kopeteemailwindow.cpp
kopete_emailwindow_la_LDFLAGS = -no-undefined -module $(KDE_PLUGIN) $(all_libraries)
kopete_emailwindow_la_LIBADD = ../../libkopete/libkopete.la ./libkopetechatwindow.la $(LIB_KOPETECOMPAT) $(LIB_KDEUI)

libkrichtexteditpart_la_SOURCES = krichtexteditpart.cpp
libkrichtexteditpart_la_LDFLAGS = -module $(KDE_PLUGIN) $(all_libraries) $(KDE_RPATH)
libkrichtexteditpart_la_LIBADD = ../../libkopete/libkopete.la $(LIB_KPARTS) $(LIB_KSPELL) $(LIB_KDEPRINT)

libkopetechatwindow_la_SOURCES = chatmessagepart.cpp emoticonselector.cpp kopeteemoticonaction.cpp \
	chattexteditpart.cpp krichtexteditpart.cpp kopetechatwindowstylemanager.cpp \
	kopetechatwindowstyle.cpp 
libkopetechatwindow_la_LDFLAGS = $(all_libraries) -no-undefined
libkopetechatwindow_la_LIBADD = ../../libkopete/libkopete.la $(LIB_KOPETECOMPAT) $(LIB_KDEUI)

partdir = $(kde_datadir)/kopeterichtexteditpart
part_DATA = kopeterichtexteditpartfull.rc

rcdir = $(kde_datadir)/kopete
rc_DATA = kopetechatwindow.rc kopeteemailwindow.rc

service_DATA = chatwindow.desktop emailwindow.desktop
servicedir = $(kde_servicesdir)

