



########### next target ###############

set(kopete_motionaway_PART_SRCS motionawayplugin.cpp )

kde4_automoc(kopete_motionaway ${kopete_motionaway_PART_SRCS})

kde4_add_kcfg_files(kopete_motionaway_PART_SRCS motionawayconfig.kcfgc )

kde4_add_plugin(kopete_motionaway ${kopete_motionaway_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kopete_motionaway )

target_link_libraries(kopete_motionaway  ${KDE4_KDECORE_LIBS} kopete )

install(TARGETS kopete_motionaway  DESTINATION ${PLUGIN_INSTALL_DIR})


########### next target ###############

set(kcm_kopete_motionaway_PART_SRCS motionawaypreferences.cpp )

kde4_automoc(kcm_kopete_motionaway ${kcm_kopete_motionaway_PART_SRCS})

kde4_add_ui_files(kcm_kopete_motionaway_PART_SRCS motionawayprefs.ui )

kde4_add_kcfg_files(kcm_kopete_motionaway_PART_SRCS motionawayconfig.kcfgc )

kde4_add_plugin(kcm_kopete_motionaway ${kcm_kopete_motionaway_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kcm_kopete_motionaway )

target_link_libraries(kcm_kopete_motionaway  ${KDE4_KDECORE_LIBS} ${KDE4_KUTILS_LIBS} )

install(TARGETS kcm_kopete_motionaway  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES motionawayconfig.kcfg  DESTINATION ${KCFG_INSTALL_DIR})
install( FILES kopete_motionaway.desktop  DESTINATION ${SERVICES_INSTALL_DIR})
install( FILES kopete_motionaway_config.desktop  DESTINATION ${SERVICES_INSTALL_DIR}/kconfiguredialog)




#original Makefile.am contents follow:

#METASOURCES = AUTO
#
#AM_CPPFLAGS = $(KOPETE_INCLUDES) $(all_includes)
#
#kde_module_LTLIBRARIES = kopete_motionaway.la kcm_kopete_motionaway.la
#
#kopete_motionaway_la_SOURCES     = motionawayplugin.cpp motionawayconfig.kcfgc
#kopete_motionaway_la_LDFLAGS     = -module $(KDE_PLUGIN)
#kopete_motionaway_la_LIBADD      = ../../libkopete/libkopete.la
#
#kcm_kopete_motionaway_la_SOURCES = motionawayprefs.ui motionawaypreferences.cpp motionawayconfig.kcfgc
#kcm_kopete_motionaway_la_LDFLAGS = -module -no-undefined $(KDE_PLUGIN) $(all_libraries)
#kcm_kopete_motionaway_la_LIBADD  = $(LIB_KOPETECOMPAT) $(LIB_KUTILS)
#
#
#
#service_DATA = kopete_motionaway.desktop
#servicedir = $(kde_servicesdir)
#
#kcm_DATA = kopete_motionaway_config.desktop
#kcmdir = $(kde_servicesdir)/kconfiguredialog
#kde_kcfg_DATA = motionawayconfig.kcfg
#
#
