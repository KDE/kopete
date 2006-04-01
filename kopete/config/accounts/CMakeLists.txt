kde4_header()

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} 
${CMAKE_SOURCE_DIR}/kopete/libkopete/ 
${CMAKE_BINARY_DIR}/kopete/libkopete/ 
${CMAKE_SOURCE_DIR}/kopete/libkopete/ui/ 
${CMAKE_BINARY_DIR}/kopete/libkopete/ui/ 
${CMAKE_SOURCE_DIR}/kopete/libkopete/private 
${CMAKE_SOURCE_DIR}/kopete/kopete/addaccountwizard 
)

link_directories( ${CMAKE_BINARY_DIR}/kopete/kopete/addaccountwizard )

########### next target ###############

set(kcm_kopete_accountconfig_PART_SRCS kopeteaccountconfig.cpp )

kde4_automoc(${kcm_kopete_accountconfig_PART_SRCS})

kde4_add_ui3_files(kcm_kopete_accountconfig_PART_SRCS kopeteaccountconfigbase.ui )

kde4_add_plugin(kcm_kopete_accountconfig ${kcm_kopete_accountconfig_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kcm_kopete_accountconfig )

target_link_libraries(kcm_kopete_accountconfig  ${KDE4_KDECORE_LIBS} kopete kopeteaddaccountwizard kutils )

install_targets(${LIB_INSTALL_DIR}/kde4 kcm_kopete_accountconfig )


########### install files ###############

install_files( ${SERVICES_INSTALL_DIR} FILES kopete_accountconfig.desktop )

kde4_footer()



#original Makefile.am contents follow:

#kopete_srcdir=$(top_srcdir)/kopete
#kopete_builddir=$(top_builddir)/kopete
#
#METASOURCES = AUTO
#AM_CPPFLAGS = $(KOPETE_INCLUDES) $(KOPETE_COMPAT_INCLUDES) $(all_includes) -I$(kopete_srcdir)/kopete/addaccountwizard
#			  
#
#kde_module_LTLIBRARIES = kcm_kopete_accountconfig.la
#
#kcm_kopete_accountconfig_la_SOURCES = kopeteaccountconfigbase.ui kopeteaccountconfig.cpp
#kcm_kopete_accountconfig_la_LDFLAGS = -no-undefined -module $(KDE_PLUGIN) $(all_libraries)
#kcm_kopete_accountconfig_la_LIBADD  = ../../../libkopete/libkopete.la ../../addaccountwizard/libkopeteaddaccountwizard.la $(LIB_KOPETECOMPAT) $(LIB_KUTILS)
#
#service_DATA = kopete_accountconfig.desktop
#servicedir = $(kde_servicesdir)
#
## vim: set noet:
#
