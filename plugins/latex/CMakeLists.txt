
add_subdirectory( icons ) 

include_directories( 
${KOPETE_INCLUDES} 
)


########### next target ###############

set(kopete_latex_PART_SRCS latexplugin.cpp latexguiclient.cpp )

kde4_add_kcfg_files(kopete_latex_PART_SRCS latexconfig.kcfgc )

kde4_add_plugin(kopete_latex ${kopete_latex_PART_SRCS})


target_link_libraries(kopete_latex  ${KDE4_KDEUI_LIBS} kopete )

install(TARGETS kopete_latex  DESTINATION ${PLUGIN_INSTALL_DIR})


########### next target ###############

set(kcm_kopete_latex_PART_SRCS latexpreferences.cpp )

kde4_add_ui_files(kcm_kopete_latex_PART_SRCS latexprefsbase.ui )

kde4_add_kcfg_files(kcm_kopete_latex_PART_SRCS latexconfig.kcfgc )


kde4_add_plugin(kcm_kopete_latex ${kcm_kopete_latex_PART_SRCS})


target_link_libraries(kcm_kopete_latex  ${KDE4_KUTILS_LIBS} ${KDE4_KIO_LIBS})

install(TARGETS kcm_kopete_latex  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES latexconfig.kcfg  DESTINATION ${KCFG_INSTALL_DIR})
install( FILES kopete_latex.desktop  DESTINATION ${SERVICES_INSTALL_DIR})
install( FILES latexchatui.rc  DESTINATION ${DATA_INSTALL_DIR}/kopete_latex)
install( FILES kopete_latex_config.desktop  DESTINATION ${SERVICES_INSTALL_DIR}/kconfiguredialog)
install( PROGRAMS kopete_latexconvert.sh DESTINATION ${BIN_INSTALL_DIR})



