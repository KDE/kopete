kde4_header()

add_subdirectory( icons ) 

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############

set(kopete_cryptography_PART_SRCS 
   cryptographyplugin.cpp 
   kgpginterface.cpp 
   cryptographyguiclient.cpp 
   cryptographyselectuserkey.cpp 
   popuppublic.cpp 
   kgpgselkey.cpp )

kde4_automoc(${kopete_cryptography_PART_SRCS})

kde4_add_ui_files(kopete_cryptography_PART_SRCS cryptographyuserkey_ui.ui )

kde4_add_plugin(kopete_cryptography ${kopete_cryptography_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kopete_cryptography )

target_link_libraries(kopete_cryptography  ${KDE4_KDECORE_LIBS} kopete )

install_targets(${LIB_INSTALL_DIR}/kde4 kopete_cryptography )


########### next target ###############

set(kcm_kopete_cryptography_PART_SRCS cryptographypreferences.cpp kgpgselkey.cpp )

kde4_automoc(${kcm_kopete_cryptography_PART_SRCS})

kde4_add_ui_files(kcm_kopete_cryptography_PART_SRCS cryptographyprefsbase.ui )

kde4_add_plugin(kcm_kopete_cryptography ${kcm_kopete_cryptography_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kcm_kopete_cryptography )

target_link_libraries(kcm_kopete_cryptography  ${KDE4_KDECORE_LIBS} kopete kutils )

install_targets(${LIB_INSTALL_DIR}/kde4 kcm_kopete_cryptography )


########### install files ###############

install_files( ${SERVICES_INSTALL_DIR} FILES kopete_cryptography.desktop )
install_files( ${DATA_INSTALL_DIR}/kopete_cryptography FILES cryptographyui.rc cryptographychatui.rc )
install_files( ${SERVICES_INSTALL_DIR}/kconfiguredialog FILES kopete_cryptography_config.desktop )

kde4_footer()



#original Makefile.am contents follow:

#METASOURCES = AUTO
#
#SUBDIRS=icons
#
#AM_CPPFLAGS = $(KOPETE_INCLUDES) $(all_includes)
#
#kde_module_LTLIBRARIES = kopete_cryptography.la kcm_kopete_cryptography.la
#
#kopete_cryptography_la_SOURCES     = cryptographyplugin.cpp kgpginterface.cpp cryptographyguiclient.cpp cryptographyselectuserkey.cpp cryptographyuserkey_ui.ui popuppublic.cpp kgpgselkey.cpp
#kopete_cryptography_la_LDFLAGS     = -module -no-undefined $(KDE_PLUGIN) $(all_libraries)
#kopete_cryptography_la_LIBADD      = ../../libkopete/libkopete.la
#
#kcm_kopete_cryptography_la_SOURCES = cryptographypreferences.cpp cryptographyprefsbase.ui  kgpgselkey.cpp
#kcm_kopete_cryptography_la_LDFLAGS = -module -no-undefined $(KDE_PLUGIN) $(all_libraries)
#kcm_kopete_cryptography_la_LIBADD  = ../../libkopete/libkopete.la $(LIB_KUTILS)
#
#service_DATA = kopete_cryptography.desktop
#servicedir = $(kde_servicesdir)
#
#kcm_DATA = kopete_cryptography_config.desktop
#kcmdir = $(kde_servicesdir)/kconfiguredialog
#
#mydatadir = $(kde_datadir)/kopete_cryptography
#mydata_DATA = cryptographyui.rc cryptographychatui.rc
#
