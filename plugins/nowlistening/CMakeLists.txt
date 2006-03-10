kde4_header()

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############

set(kopete_nowlistening_PART_SRCS 
   nowlisteningplugin.cpp 
   nlkscd.cpp 
   nlnoatun.cpp 
   nlxmms.cpp 
   nowlisteningguiclient.cpp 
   nljuk.cpp 
   nlamarok.cpp 
   nlkaffeine.cpp )

kde4_automoc(${kopete_nowlistening_PART_SRCS})

kde4_add_kcfg_files(kopete_nowlistening_PART_SRCS nowlisteningconfig.kcfgc )

kde4_add_plugin(kopete_nowlistening ${kopete_nowlistening_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kopete_nowlistening )

target_link_libraries(kopete_nowlistening  ${QT_AND_KDECORE_LIBS} kopete )

install_targets(${LIB_INSTALL_DIR}/kde4 kopete_nowlistening )


########### next target ###############

set(kcm_kopete_nowlistening_PART_SRCS nowlisteningpreferences.cpp )

kde4_automoc(${kcm_kopete_nowlistening_PART_SRCS})

kde4_add_ui_files(kcm_kopete_nowlistening_PART_SRCS nowlisteningprefs.ui )

kde4_add_kcfg_files(kcm_kopete_nowlistening_PART_SRCS nowlisteningconfig.kcfgc )

kde4_add_plugin(kcm_kopete_nowlistening ${kcm_kopete_nowlistening_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kcm_kopete_nowlistening )

target_link_libraries(kcm_kopete_nowlistening  ${QT_AND_KDECORE_LIBS} kutils )

install_targets(${LIB_INSTALL_DIR}/kde4 kcm_kopete_nowlistening )


########### install files ###############

install_files( ${KCFG_INSTALL_DIR} FILES nowlisteningconfig.kcfg )
install_files( ${SERVICES_INSTALL_DIR} FILES kopete_nowlistening.desktop )
install_files( ${DATA_INSTALL_DIR}/kopete FILES nowlisteningui.rc nowlisteningchatui.rc )
install_files( ${SERVICES_INSTALL_DIR}/kconfiguredialog FILES kopete_nowlistening_config.desktop )

kde4_footer()



#original Makefile.am contents follow:

#METASOURCES = AUTO
#
#AM_CPPFLAGS = $(KOPETE_INCLUDES) $(XMMS_INCLUDES) $(all_includes)
#
#kde_module_LTLIBRARIES = kopete_nowlistening.la kcm_kopete_nowlistening.la
#
#kopete_nowlistening_la_SOURCES = nowlisteningconfig.kcfgc nowlisteningplugin.cpp nlkscd.cpp nlnoatun.cpp nlxmms.cpp nowlisteningguiclient.cpp nljuk.cpp nlamarok.cpp nlkaffeine.cpp
#kopete_nowlistening_la_LDFLAGS     = -module $(KDE_PLUGIN) $(XMMS_LDFLAGS) $(all_libraries)
#kopete_nowlistening_la_LIBADD =  ../../libkopete/libkopete.la $(XMMS_LIBS)
#
#kcm_kopete_nowlistening_la_SOURCES = nowlisteningprefs.ui nowlisteningpreferences.cpp nowlisteningconfig.kcfgc
#kcm_kopete_nowlistening_la_LDFLAGS = -module -no-undefined $(KDE_PLUGIN) $(all_libraries)
#kcm_kopete_nowlistening_la_LIBADD  = $(LIB_KOPETECOMPAT) $(LIB_KUTILS)
#
#service_DATA = kopete_nowlistening.desktop
#servicedir = $(kde_servicesdir)
#
#kcm_DATA = kopete_nowlistening_config.desktop
#kcmdir = $(kde_servicesdir)/kconfiguredialog
#
#kde_kcfg_DATA = nowlisteningconfig.kcfg
#
#mydatadir = $(kde_datadir)/kopete
#mydata_DATA = nowlisteningui.rc nowlisteningchatui.rc
#noinst_HEADERS =  nlkaffeine.h
