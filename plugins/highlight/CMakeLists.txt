
add_subdirectory( icons ) 

include_directories( 
${KOPETE_INCLUDES}
)


########### next target ###############

set(kopete_highlight_PART_SRCS highlightplugin.cpp highlightconfig.cpp filter.cpp )

kde4_add_plugin(kopete_highlight ${kopete_highlight_PART_SRCS})


target_link_libraries(kopete_highlight  ${KDE4_KDECORE_LIBS} kopete )

install(TARGETS kopete_highlight  DESTINATION ${PLUGIN_INSTALL_DIR})


########### next target ###############

set(kcm_kopete_highlight_PART_SRCS highlightpreferences.cpp filter.cpp highlightconfig.cpp )

kde4_add_ui_files(kcm_kopete_highlight_PART_SRCS highlightprefsbase.ui )

kde4_automoc(${kcm_kopete_highlight_PART_SRCS})

kde4_add_plugin(kcm_kopete_highlight ${kcm_kopete_highlight_PART_SRCS})


target_link_libraries(kcm_kopete_highlight ${KDE4_KUTILS_LIBS}  ${KDE4_KNOTIFYCONFIG_LIBS} ${QT_QT3SUPPORT_LIBRARY} )

install(TARGETS kcm_kopete_highlight  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES kopete_highlight.desktop  DESTINATION ${SERVICES_INSTALL_DIR})
install( FILES kopete_highlight_config.desktop  DESTINATION ${SERVICES_INSTALL_DIR}/kconfiguredialog)




