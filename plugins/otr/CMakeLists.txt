project( kopete-otr )

find_package(KDE4 REQUIRED)

set(CMAKE_VERBOSE_MAKEFILE ON)

add_definitions( ${QT_DEFINITIONS} ${KDE4_DEFINITIONS} )

link_directories( ${KDE4_LIB_DIR} ${CMAKE_CURRENT_BINARY_DIR} )

add_subdirectory( icons )

include_directories( ${KDE4_INCLUDE_DIR} ${KDE4_INCLUDE_DIR}/kopete ${QT_INCLUDES} ${CMAKE_CURRENT_BINARY_DIR} )


########### next target ###############

set(kotr_LIB_SRCS otrlchatinterface.cpp otrlconfinterface.cpp smppopup.cpp verifydialog.cpp privkeypopup.cpp )

kde4_add_library(kotr SHARED ${kotr_LIB_SRCS} )

kde4_add_ui_files(kotr_LIB_SRCS privkeypopup.ui smppopup.ui verifydialog.ui )

target_link_libraries(kotr kopete kdeui ${QT_QT3SUPPORT_LIBRARY} Qt3Support ${KDE4_KDECORE_LIBS} otr )

set_target_properties(kotr PROPERTIES VERSION 1.0.0 SOVERSION 1 )
install(TARGETS kotr DESTINATION ${LIB_INSTALL_DIR} )

########### next target ###############

set(kcm_kopete_otr_PART_SRCS
  ${kotr_LIB_SRCS} otrpreferences.cpp )

kde4_add_ui_files(kcm_kopete_otr_PART_SRCS otrprefs.ui)

kde4_add_kcfg_files( kcm_kopete_otr_PART_SRCS ${kotr_PART_SRCS} 
kopete_otr.kcfgc )

kde4_add_plugin(kcm_kopete_otr ${kcm_kopete_otr_PART_SRCS})

target_link_libraries(kcm_kopete_otr ${QT_QT3SUPPORT_LIBRARY} ${KDE4_KDECORE_LIBS} kopete otr ${KDE4_KUTILS_LIBS} )

install(TARGETS kcm_kopete_otr DESTINATION ${PLUGIN_INSTALL_DIR} )


########### next target ###############

set(kopete_otr_PART_SRCS
  ${kotr_LIB_SRCS} otrguiclient.cpp otrplugin.cpp)

kde4_add_kcfg_files(kopete_otr_PART_SRCS kopete_otr.kcfgc )

kde4_add_plugin(kopete_otr ${kopete_otr_PART_SRCS})

target_link_libraries(kopete_otr  ${QT_QT3SUPPORT_LIBRARY} ${KDE4_KDECORE_LIBS} ${KDE4_KDEUI_LIBS} kopete otr )

install(TARGETS kopete_otr DESTINATION ${PLUGIN_INSTALL_DIR} )




########### install files ###############

install( FILES  kopete_otr.kcfg DESTINATION ${KCFG_INSTALL_DIR} )
install( FILES  kopete_otr.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
install( FILES  otrchatui.rc otrui.rc DESTINATION ${DATA_INSTALL_DIR}/kopete_otr )
install( FILES  kopete_otr_config.desktop DESTINATION ${SERVICES_INSTALL_DIR}/kconfiguredialog )



#original Makefile.am contents follow:

#METASOURCES = AUTO
#
#messages:
#	extractrc `find . -name \*.ui -o -name \*.rc` > rc.cpp
#	$(XGETTEXT) --keyword=i18n *.cpp -o $(top_srcdir)/po/kopete_otr.pot
#
#INCLUDES = $(all_includes) -I$(kde_includes)/kopete -I$(kde_includes)/kopete/ui
#
#kde_module_LTLIBRARIES = kcm_kopete_otr.la kopete_otr.la
#
#kopete_otr_la_SOURCES = otrguiclient.cpp otrplugin.cpp
#	
#
#kopete_otr_la_LDFLAGS = -module -no-undefined $(KDE_PLUGIN) $(all_libraries) \
#	-lkopete
#
#kcm_kopete_otr_la_SOURCES = otrpreferences.cpp otrprefs.ui
#
#kcm_kopete_otr_la_LDFLAGS = -module -no-undefined $(KDE_PLUGIN) \
#	$(all_libraries) -lkopete
#kcm_kopete_otr_la_LIBADD = libkotr.la -lotr $(LIB_KUTILS)
#
#service_DATA = kopete_otr.desktop
#servicedir = $(kde_servicesdir)
#
#kde_kcfg_DATA = kopete_otr.kcfg
#
#kcm_DATA = kopete_otr_config.desktop
#kcmdir = $(kde_servicesdir)/kconfiguredialog
#
#kopete_otr_la_LIBADD = libkotr.la -lotr
#
#noinst_HEADERS = otrguiclient.h otrlchatinterface.h otrlconfinterface.h \
#	otrplugin.h otrpreferences.h otrprefs.h privkeypopup.h
#
#
#rcdir = $(kde_datadir)/kopete_otr
#rc_DATA = otrchatui.rc otrui.rc
#
#lib_LTLIBRARIES = libkotr.la
#libkotr_la_LDFLAGS = $(all_libraries)
#libkotr_la_SOURCES = otrlchatinterface.cpp otrlconfinterface.cpp \
#		privkeypopupui.ui privkeypopup.cpp
#libkotr_la_LIBADD = -lotr
#
#SUBDIRS = pics
#
