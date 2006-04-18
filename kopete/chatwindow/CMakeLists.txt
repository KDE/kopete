kde4_header()

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} 
${KOPETE_INCLUDES}
${CMAKE_CURRENT_SOURCE_DIR}/../ 
)

link_directories( ${CMAKE_CURRENT_BINARY_DIR} )
########### next target ###############

set( kopetechatwindow_shared_SRCS 
chatmessagepart.cpp 
emoticonselector.cpp 
kopeteemoticonaction.cpp 
chattexteditpart.cpp 
krichtexteditpart.cpp 
kopetechatwindowstylemanager.cpp 
kopetechatwindowstyle.cpp )

kde4_automoc(${kopetechatwindow_shared_SRCS})
kde4_add_library(kopetechatwindow_shared SHARED ${kopetechatwindow_shared_SRCS})
target_link_libraries(kopetechatwindow_shared kopete ${KDE4_KDEUI_LIBS} ${KDE4_KHTML_LIBS} )
install_targets(${LIB_INSTALL_DIR} kopetechatwindow_shared )

########### next target ###############

set(krichtexteditpart_PART_SRCS krichtexteditpart.cpp )

kde4_automoc(${krichtexteditpart_PART_SRCS})

kde4_add_plugin(krichtexteditpart WITH_PREFIX ${krichtexteditpart_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} krichtexteditpart )

target_link_libraries(krichtexteditpart  ${KDE4_KDECORE_LIBS} kopete ${KDE4_KPARTS_LIBS} ${KDE4_KSPELL2_LIBS} ${KDE4_KDEPRINT_LIBS} )

install_targets(${PLUGIN_INSTALL_DIR} krichtexteditpart )


########### next target ###############

set(kopete_chatwindow_PART_SRCS chatview.cpp kopetechatwindow.cpp chatmemberslistwidget.cpp )

kde4_automoc(${kopete_chatwindow_PART_SRCS})

kde4_add_plugin(kopete_chatwindow ${kopete_chatwindow_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kopete_chatwindow )

target_link_libraries(kopete_chatwindow  ${KDE4_KDEUI_LIBS} kopete kopetechatwindow_shared)

install_targets(${PLUGIN_INSTALL_DIR} kopete_chatwindow )


########### next target ###############

set(kopete_emailwindow_PART_SRCS kopeteemailwindow.cpp )

kde4_automoc(${kopete_emailwindow_PART_SRCS})

kde4_add_plugin(kopete_emailwindow ${kopete_emailwindow_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kopete_emailwindow )

target_link_libraries(kopete_emailwindow  ${KDE4_KDEUI_LIBS} kopete kopetechatwindow_shared)

install_targets(${PLUGIN_INSTALL_DIR} kopete_emailwindow )


########### next target ###############


########### install files ###############

install_files( ${DATA_INSTALL_DIR}/kopeterichtexteditpart FILES kopeterichtexteditpartfull.rc )
install_files( ${SERVICES_INSTALL_DIR} FILES chatwindow.desktop emailwindow.desktop )
install_files( ${DATA_INSTALL_DIR}/kopete FILES kopetechatwindow.rc kopeteemailwindow.rc )

kde4_footer()



#original Makefile.am contents follow:

#kde_module_LTLIBRARIES = libkrichtexteditpart.la kopete_chatwindow.la kopete_emailwindow.la
#noinst_LTLIBRARIES = libkopetechatwindow.la
#METASOURCES = AUTO
#
#AM_CPPFLAGS = $(KOPETE_INCLUDES) $(KOPETE_COMPAT_INCLUDES) -I$(top_srcdir)/kopete/libkopete/private    -I$(top_srcdir)/kopete/kopete $(all_includes)
#
#kopete_chatwindow_la_SOURCES = chatview.cpp kopetechatwindow.cpp chatmemberslistwidget.cpp
#kopete_chatwindow_la_LDFLAGS = -no-undefined -module $(KDE_PLUGIN) $(all_libraries)
#kopete_chatwindow_la_LIBADD = ../../libkopete/libkopete.la ./libkopetechatwindow.la $(LIB_KOPETECOMPAT) $(LIB_KDEUI)
#
#kopete_emailwindow_la_SOURCES = kopeteemailwindow.cpp
#kopete_emailwindow_la_LDFLAGS = -no-undefined -module $(KDE_PLUGIN) $(all_libraries)
#kopete_emailwindow_la_LIBADD = ../../libkopete/libkopete.la ./libkopetechatwindow.la $(LIB_KOPETECOMPAT) $(LIB_KDEUI)
#
#libkrichtexteditpart_la_SOURCES = krichtexteditpart.cpp
#libkrichtexteditpart_la_LDFLAGS = -module $(KDE_PLUGIN) $(all_libraries) $(KDE_RPATH)
#libkrichtexteditpart_la_LIBADD = ../../libkopete/libkopete.la $(LIB_KPARTS) $(LIB_KSPELL2) $(LIB_KDEPRINT)
#
#libkopetechatwindow_la_SOURCES = chatmessagepart.cpp emoticonselector.cpp kopeteemoticonaction.cpp \
#	chattexteditpart.cpp krichtexteditpart.cpp kopetechatwindowstylemanager.cpp \
#	kopetechatwindowstyle.cpp
#libkopetechatwindow_la_LDFLAGS = $(all_libraries) -no-undefined
#libkopetechatwindow_la_LIBADD = ../../libkopete/libkopete.la $(LIB_KOPETECOMPAT) $(LIB_KDEUI)
#
#partdir = $(kde_datadir)/kopeterichtexteditpart
#part_DATA = kopeterichtexteditpartfull.rc
#
#rcdir = $(kde_datadir)/kopete
#rc_DATA = kopetechatwindow.rc kopeteemailwindow.rc
#
#service_DATA = chatwindow.desktop emailwindow.desktop
#servicedir = $(kde_servicesdir)
#
