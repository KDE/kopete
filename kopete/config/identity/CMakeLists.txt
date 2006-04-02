kde4_header()

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} 
${KOPETE_INCLUDES} 
)

########### next target ###############

set(kcm_kopete_identityconfig_PART_SRCS kopeteidentityconfig.cpp globalidentitiesmanager.cpp )

kde4_add_ui3_files(kcm_kopete_identityconfig_PART_SRCS kopeteidentityconfigbase.ui )

kde4_add_kcfg_files(kcm_kopete_identityconfig_PART_SRCS kopeteidentityconfigpreferences.kcfgc )

kde4_automoc(${kcm_kopete_identityconfig_PART_SRCS})

kde4_add_plugin(kcm_kopete_identityconfig ${kcm_kopete_identityconfig_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kcm_kopete_identityconfig )

target_link_libraries(kcm_kopete_identityconfig  ${KDE4_KUTILS_LIBS} kabc kopete)

install_targets(${LIB_INSTALL_DIR}/kde4 kcm_kopete_identityconfig )


########### install files ###############

install_files( ${KCFG_INSTALL_DIR} FILES kopeteidentityconfigpreferences.kcfg )
install_files( ${SERVICES_INSTALL_DIR} FILES kopete_identityconfig.desktop )

kde4_footer()



#original Makefile.am contents follow:

#METASOURCES = AUTO
#AM_CPPFLAGS =  $(KOPETE_INCLUDES) $(KOPETE_COMPAT_INCLUDES) $(all_includes)
#
#kde_module_LTLIBRARIES = kcm_kopete_identityconfig.la
#
#kcm_kopete_identityconfig_la_SOURCES = kopeteidentityconfigbase.ui \
#		kopeteidentityconfig.cpp globalidentitiesmanager.cpp kopeteidentityconfigpreferences.kcfgc
#kcm_kopete_identityconfig_la_LDFLAGS = -no-undefined -module $(KDE_PLUGIN) $(all_libraries)
#kcm_kopete_identityconfig_la_LIBADD  = -lkabc ../../../libkopete/libkopete.la $(LIB_KOPETECOMPAT) $(LIB_KUTILS)
#
#service_DATA = kopete_identityconfig.desktop
#servicedir = $(kde_servicesdir)
#
## vim: set noet:
#kde_kcfg_DATA = kopeteidentityconfigpreferences.kcfg
