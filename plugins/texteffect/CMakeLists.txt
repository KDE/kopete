
add_subdirectory( icons ) 

include_directories( 
${KOPETE_INCLUDES} 
)


########### next target ###############

set(kopete_texteffect_PART_SRCS texteffectplugin.cpp texteffectconfig.cpp )

kde4_automoc(${kopete_texteffect_PART_SRCS})

kde4_add_plugin(kopete_texteffect ${kopete_texteffect_PART_SRCS})


target_link_libraries(kopete_texteffect  ${KDE4_KDECORE_LIBS} kopete )

install(TARGETS kopete_texteffect  DESTINATION ${PLUGIN_INSTALL_DIR})


########### next target ###############

set(kcm_kopete_texteffect_PART_SRCS texteffectconfig.cpp texteffectpreferences.cpp )

kde4_add_ui_files(kcm_kopete_texteffect_PART_SRCS texteffectprefs.ui )

kde4_automoc(${kcm_kopete_texteffect_PART_SRCS})

kde4_add_plugin(kcm_kopete_texteffect ${kcm_kopete_texteffect_PART_SRCS})


target_link_libraries(kcm_kopete_texteffect  ${KDE4_KUTILS_LIBS} kopete )

install(TARGETS kcm_kopete_texteffect  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES kopete_texteffect.desktop  DESTINATION ${SERVICES_INSTALL_DIR})
install( FILES kopete_texteffect_config.desktop  DESTINATION ${SERVICES_INSTALL_DIR}/kconfiguredialog)




#original Makefile.am contents follow:

#METASOURCES = AUTO
#
#SUBDIRS = icons
#INCLUDES = $(KOPETE_INCLUDES) $(all_includes)
#
#kde_module_LTLIBRARIES = kopete_texteffect.la kcm_kopete_texteffect.la
#
#kopete_texteffect_la_SOURCES     = texteffectplugin.cpp texteffectconfig.cpp
#kopete_texteffect_la_LDFLAGS     = -module -no-undefined $(KDE_PLUGIN) $(all_libraries)
#kopete_texteffect_la_LIBADD      = ../../libkopete/libkopete.la
#
#kcm_kopete_texteffect_la_SOURCES = texteffectconfig.cpp texteffectprefs.ui texteffectpreferences.cpp
#kcm_kopete_texteffect_la_LDFLAGS = -module -no-undefined $(KDE_PLUGIN) $(all_libraries)
#kcm_kopete_texteffect_la_LIBADD  = ../../libkopete/libkopete.la $(LIB_KUTILS)
#
#service_DATA = kopete_texteffect.desktop
#servicedir = $(kde_servicesdir)
#
#kcm_DATA = kopete_texteffect_config.desktop
#kcmdir = $(kde_servicesdir)/kconfiguredialog
