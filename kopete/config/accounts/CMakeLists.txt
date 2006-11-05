
include_directories( 
${KOPETE_INCLUDES} 
${CMAKE_SOURCE_DIR}/kopete/kopete/addaccountwizard 
${CMAKE_BINARY_DIR}/kopete/kopete/addaccountwizard 
)

link_directories( ${CMAKE_BINARY_DIR}/kopete/kopete/addaccountwizard )

########### next target ###############

set(kcm_kopete_accountconfig_PART_SRCS kopeteaccountconfig.cpp )

kde4_automoc(${kcm_kopete_accountconfig_PART_SRCS})

kde4_add_ui_files(kcm_kopete_accountconfig_PART_SRCS kopeteaccountconfigbase.ui )

kde4_add_plugin(kcm_kopete_accountconfig ${kcm_kopete_accountconfig_PART_SRCS})


target_link_libraries(kcm_kopete_accountconfig  ${KDE4_KUTILS_LIBS} kopete kopeteaddaccountwizard )

install(TARGETS kcm_kopete_accountconfig  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES kopete_accountconfig.desktop  DESTINATION ${SERVICES_INSTALL_DIR})




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
