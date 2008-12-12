
add_subdirectory( icons ) 

include_directories( 
${KOPETE_INCLUDES} 
)


########### next target ###############

set(kopete_texteffect_PART_SRCS texteffectplugin.cpp texteffectconfig.cpp )

kde4_add_plugin(kopete_texteffect ${kopete_texteffect_PART_SRCS})


target_link_libraries(kopete_texteffect  ${KDE4_KDEUI_LIBS} kopete )

install(TARGETS kopete_texteffect  DESTINATION ${PLUGIN_INSTALL_DIR})


########### next target ###############

set(kcm_kopete_texteffect_PART_SRCS texteffectconfig.cpp texteffectpreferences.cpp )

kde4_add_ui_files(kcm_kopete_texteffect_PART_SRCS texteffectprefs.ui )


kde4_add_plugin(kcm_kopete_texteffect ${kcm_kopete_texteffect_PART_SRCS})


target_link_libraries(kcm_kopete_texteffect  ${KDE4_KUTILS_LIBS} ${KDE4_KDE3SUPPORT_LIBS} kopete )

install(TARGETS kcm_kopete_texteffect  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES kopete_texteffect.desktop  DESTINATION ${SERVICES_INSTALL_DIR})
install( FILES kopete_texteffect_config.desktop  DESTINATION ${SERVICES_INSTALL_DIR}/kconfiguredialog)




