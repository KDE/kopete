



########### next target ###############

set(kcm_kopete_msn_PART_SRCS msnpreferences.cpp )

kde4_automoc(${kcm_kopete_msn_PART_SRCS})

kde4_add_ui_files(kcm_kopete_msn_PART_SRCS msnprefs.ui )

kde4_add_plugin(kcm_kopete_msn ${kcm_kopete_msn_PART_SRCS})


target_link_libraries(kcm_kopete_msn  ${KDE4_KDECORE_LIBS} kopete ${KDE4_KUTILS_LIBS} )

install(TARGETS kcm_kopete_msn  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES kopete_msn_config.desktop  DESTINATION ${SERVICES_INSTALL_DIR}/kconfiguredialog)




#original Makefile.am contents follow:

#METASOURCES = AUTO
#AM_CPPFLAGS = $(KOPETE_INCLUDES) $(all_includes)
#
#kde_module_LTLIBRARIES = kcm_kopete_msn.la
#
#kcm_kopete_msn_la_SOURCES = msnprefs.ui msnpreferences.cpp
#kcm_kopete_msn_la_LDFLAGS = -no-undefined -module $(KDE_PLUGIN) $(all_libraries)
#kcm_kopete_msn_la_LIBADD  = ../../../libkopete/libkopete.la $(LIB_KUTILS)
#
#service_DATA = kopete_msn_config.desktop
#servicedir = $(kde_servicesdir)/kconfiguredialog
#
