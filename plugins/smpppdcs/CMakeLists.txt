kde4_header()

add_subdirectory( icons ) 
add_subdirectory( libsmpppdclient ) 
add_subdirectory( unittest ) 

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############

set(kopete_smpppdcs_PART_SRCS 
   smpppdcsplugin.cpp 
   onlineinquiry.cpp 
   detectordcop.cpp 
   detectorsmpppd.cpp 
   detectornetstat.cpp 
   detectornetworkstatus.cpp )

kde4_automoc(${kopete_smpppdcs_PART_SRCS})

kde4_add_dcop_skels(kopete_smpppdcs_PART_SRCS smpppdcsiface.h )

kde4_add_dcop_stubs(kopete_smpppdcs_PART_SRCS kinternetiface.h )

kde4_add_kcfg_files(kopete_smpppdcs_PART_SRCS smpppdcsconfig.kcfgc )

kde4_add_plugin(kopete_smpppdcs ${kopete_smpppdcs_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kopete_smpppdcs )

target_link_libraries(kopete_smpppdcs  ${QT_AND_KDECORE_LIBS} smpppdclient kopete )

install_targets(${LIB_INSTALL_DIR}/kde4 kopete_smpppdcs )


########### next target ###############

set(kcm_kopete_smpppdcs_PART_SRCS 
   smpppdcspreferences.cpp 
   smpppdsearcher.cpp 
   smpppdcsprefsimpl.cpp 
   smpppdlocationwidget.cpp )

kde4_automoc(${kcm_kopete_smpppdcs_PART_SRCS})

kde4_add_ui_files(kcm_kopete_smpppdcs_PART_SRCS smpppdcsprefs.ui smpppdlocationui.ui )

kde4_add_kcfg_files(kcm_kopete_smpppdcs_PART_SRCS smpppdcsconfig.kcfgc )

kde4_add_plugin(kcm_kopete_smpppdcs ${kcm_kopete_smpppdcs_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kcm_kopete_smpppdcs )

target_link_libraries(kcm_kopete_smpppdcs  ${QT_AND_KDECORE_LIBS} smpppdclient kopete kutils )

install_targets(${LIB_INSTALL_DIR}/kde4 kcm_kopete_smpppdcs )


########### install files ###############

install_files( ${KCFG_INSTALL_DIR} FILES smpppdcs.kcfg )
install_files( ${SERVICES_INSTALL_DIR} FILES kopete_smpppdcs.desktop )
install_files( ${SERVICES_INSTALL_DIR}/kconfiguredialog FILES kopete_smpppdcs_config.desktop )

kde4_footer()



#original Makefile.am contents follow:

#METASOURCES = AUTO
#
#SUBDIRS = icons libsmpppdclient unittest
#
#EXTRA_DIST = Changelog.smpppdcs
#
#AM_CPPFLAGS = $(KOPETE_INCLUDES) $(all_includes) -Ilibsmpppdclient
#
#kde_module_LTLIBRARIES = kopete_smpppdcs.la kcm_kopete_smpppdcs.la
#
#kopete_smpppdcs_la_SOURCES = kinternetiface.stub smpppdcsplugin.cpp \
#	onlineinquiry.cpp 	smpppdcsiface.skel detectordcop.cpp detectorsmpppd.cpp \
#	detectornetstat.cpp detectornetworkstatus.cpp smpppdcsconfig.kcfgc
#kopete_smpppdcs_la_LDFLAGS = -module $(KDE_PLUGIN) $(all_libraries)
#kopete_smpppdcs_la_LIBADD = \
#	libsmpppdclient/libsmpppdclient.la ../../libkopete/libkopete.la
#
#kcm_kopete_smpppdcs_la_SOURCES = smpppdcsprefs.ui smpppdcspreferences.cpp \
#	smpppdsearcher.cpp smpppdcsprefsimpl.cpp smpppdlocationui.ui smpppdlocationwidget.cpp \
#	smpppdcsconfig.kcfgc
#kcm_kopete_smpppdcs_la_LDFLAGS = -module -no-undefined $(KDE_PLUGIN) $(all_libraries)
#kcm_kopete_smpppdcs_la_LIBADD  = libsmpppdclient/libsmpppdclient.la \
#	../../libkopete/libkopete.la $(LIB_KUTILS)
#
#service_DATA = kopete_smpppdcs.desktop
#servicedir = $(kde_servicesdir)
#
#kcm_DATA = kopete_smpppdcs_config.desktop
#kcmdir = $(kde_servicesdir)/kconfiguredialog
#
#kde_kcfg_DATA = smpppdcs.kcfg
#
#noinst_HEADERS = smpppdcsiface.h detectordcop.h detectorsmpppd.h \
#		detectornetstat.h kinternetiface.h detectornetworkstatus.h \
#		smpppdsearcher.h smpppdcsprefsimpl.h smpppdlocationwidget.h
