
include_directories( 
${KOPETE_INCLUDES} 
)


########### next target ###############

set(kopete_translator_PART_SRCS 
   translatorplugin.cpp 
   translatordialog.cpp 
   translatorguiclient.cpp 
   translatorlanguages.cpp )

kde4_automoc(${kopete_translator_PART_SRCS})

kde4_add_plugin(kopete_translator ${kopete_translator_PART_SRCS})


target_link_libraries(kopete_translator  ${KDE4_KIO_LIBS} kopete )

install(TARGETS kopete_translator  DESTINATION ${PLUGIN_INSTALL_DIR})


########### next target ###############

set(kcm_kopete_translator_PART_SRCS translatorprefs.cpp translatorlanguages.cpp )

kde4_add_ui_files(kcm_kopete_translator_PART_SRCS translatorprefsbase.ui )

kde4_automoc(${kcm_kopete_translator_PART_SRCS})

kde4_add_plugin(kcm_kopete_translator ${kcm_kopete_translator_PART_SRCS})


target_link_libraries(kcm_kopete_translator  ${KDE4_KUTILS_LIBS} kopete )

install(TARGETS kcm_kopete_translator  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES kopete_translator.desktop  DESTINATION ${SERVICES_INSTALL_DIR})
install( FILES translatorui.rc translatorchatui.rc  DESTINATION ${DATA_INSTALL_DIR}/kopete_translator)
install( FILES kopete_translator_config.desktop  DESTINATION ${SERVICES_INSTALL_DIR}/kconfiguredialog)




#original Makefile.am contents follow:

#METASOURCES = AUTO
#
#AM_CPPFLAGS = $(KOPETE_INCLUDES) $(all_includes)
#
#kde_module_LTLIBRARIES = kopete_translator.la kcm_kopete_translator.la
#
#kopete_translator_la_SOURCES     = translatorplugin.cpp  \
#    translatordialog.cpp translatorguiclient.cpp translatorlanguages.cpp
#kopete_translator_la_LDFLAGS     = -module -no-undefined $(KDE_PLUGIN) $(all_libraries)
#kopete_translator_la_LIBADD      = ../../libkopete/libkopete.la $(LIB_KIO)
#
#kcm_kopete_translator_la_SOURCES = translatorprefsbase.ui translatorprefs.cpp translatorlanguages.cpp
#kcm_kopete_translator_la_LDFLAGS = -module -no-undefined $(KDE_PLUGIN) $(all_libraries)
#kcm_kopete_translator_la_LIBADD  = ../../libkopete/libkopete.la $(LIB_KUTILS)
#
#service_DATA = kopete_translator.desktop
#servicedir = $(kde_servicesdir)
#
#kcm_DATA = kopete_translator_config.desktop
#kcmdir = $(kde_servicesdir)/kconfiguredialog
#
#mydatadir = $(kde_datadir)/kopete_translator
#mydata_DATA = translatorui.rc translatorchatui.rc
#
#
