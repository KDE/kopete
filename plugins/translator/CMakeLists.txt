kde4_header()

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} 
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

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kopete_translator )

target_link_libraries(kopete_translator  ${KDE4_KDECORE_LIBS} kopete kio )

install_targets(${PLUGIN_INSTALL_DIR} kopete_translator )


########### next target ###############

set(kcm_kopete_translator_PART_SRCS translatorprefs.cpp translatorlanguages.cpp )

kde4_add_ui_files(kcm_kopete_translator_PART_SRCS translatorprefsbase.ui )

kde4_automoc(${kcm_kopete_translator_PART_SRCS})

kde4_add_plugin(kcm_kopete_translator ${kcm_kopete_translator_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kcm_kopete_translator )

target_link_libraries(kcm_kopete_translator  ${KDE4_KUTILS_LIBS} kopete )

install_targets(${PLUGIN_INSTALL_DIR} kcm_kopete_translator )


########### install files ###############

install_files( ${SERVICES_INSTALL_DIR} FILES kopete_translator.desktop )
install_files( ${DATA_INSTALL_DIR}/kopete_translator FILES translatorui.rc translatorchatui.rc )
install_files( ${SERVICES_INSTALL_DIR}/kconfiguredialog FILES kopete_translator_config.desktop )

kde4_footer()



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
