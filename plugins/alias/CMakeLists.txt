kde4_header()

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############

set(kopete_alias_PART_SRCS aliasplugin.cpp )

kde4_automoc(${kopete_alias_PART_SRCS})

kde4_add_plugin(kopete_alias ${kopete_alias_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kopete_alias )

target_link_libraries(kopete_alias  ${KDE4_KDECORE_LIBS} kopete )

install_targets(${LIB_INSTALL_DIR}/kde4 kopete_alias )


########### next target ###############

set(kcm_kopete_alias_PART_SRCS aliaspreferences.cpp editaliasdialog.cpp )

kde4_automoc(${kcm_kopete_alias_PART_SRCS})

kde4_add_ui_files(kcm_kopete_alias_PART_SRCS aliasdialogbase.ui aliasdialog.ui )

kde4_add_plugin(kcm_kopete_alias ${kcm_kopete_alias_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kcm_kopete_alias )

target_link_libraries(kcm_kopete_alias  ${KDE4_KDECORE_LIBS} kutils kopete )

install_targets(${LIB_INSTALL_DIR}/kde4 kcm_kopete_alias )


########### install files ###############

install_files( ${SERVICES_INSTALL_DIR} FILES kopete_alias.desktop )
install_files( ${SERVICES_INSTALL_DIR}/kconfiguredialog FILES kopete_alias_config.desktop )

kde4_footer()



#original Makefile.am contents follow:

#METASOURCES = AUTO
#AM_CPPFLAGS = $(KOPETE_INCLUDES) $(all_includes)
#
#kde_module_LTLIBRARIES = kopete_alias.la kcm_kopete_alias.la
#
#kopete_alias_la_SOURCES     = aliasplugin.cpp
#kopete_alias_la_LDFLAGS     = -module -no-undefined $(KDE_PLUGIN) $(all_libraries)
#kopete_alias_la_LIBADD      = ../../libkopete/libkopete.la
#
#kcm_kopete_alias_la_SOURCES     = aliaspreferences.cpp aliasdialogbase.ui aliasdialog.ui editaliasdialog.cpp
#kcm_kopete_alias_la_LDFLAGS     = -module -no-undefined $(KDE_PLUGIN) $(all_libraries)
#kcm_kopete_alias_la_LIBADD      = $(LIB_KOPETECOMPAT) $(LIB_KUTILS) ../../libkopete/libkopete.la
#
#service_DATA = kopete_alias.desktop
#servicedir = $(kde_servicesdir)
#
#kcm_DATA = kopete_alias_config.desktop
#kcmdir = $(kde_servicesdir)/kconfiguredialog
#
