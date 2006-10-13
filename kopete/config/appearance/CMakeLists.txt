
include_directories( 
${KOPETE_INCLUDES} 

)


########### next target ###############

set(kcm_kopete_appearanceconfig_PART_SRCS
   appearanceconfig.cpp 
   appearanceconfig_colors.cpp 
   appearanceconfig_contactlist.cpp 
   appearanceconfig_emoticons.cpp 
   tooltipeditdialog.cpp )

kde4_add_ui_files(kcm_kopete_appearanceconfig_PART_SRCS 
   appearanceconfig_colors.ui
   appearanceconfig_contactlist.ui 
   appearanceconfig_emoticons.ui 
   tooltipeditwidget.ui )

kde4_automoc(kcm_kopete_appearanceconfig ${kcm_kopete_appearanceconfig_PART_SRCS})

kde4_add_plugin(kcm_kopete_appearanceconfig ${kcm_kopete_appearanceconfig_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kcm_kopete_appearanceconfig )

target_link_libraries(kcm_kopete_appearanceconfig ${KDE4_KUTILS_LIBS} kopete ${KDE4_KNEWSTUFF_LIBS} )

install(TARGETS kcm_kopete_appearanceconfig  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES kopete_appearanceconfig.desktop  DESTINATION ${SERVICES_INSTALL_DIR})



