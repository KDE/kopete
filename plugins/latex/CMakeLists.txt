
add_subdirectory( icons ) 

include_directories( 
${KOPETE_INCLUDES} 
)


########### next target ###############

set(kopete_latex_PART_SRCS latexplugin.cpp latexguiclient.cpp )

kconfig_add_kcfg_files(kopete_latex_PART_SRCS latexconfig.kcfgc )

add_library(kopete_latex MODULE ${kopete_latex_PART_SRCS})


target_link_libraries(kopete_latex
    kopete
    KF5::KDELibs4Support
)

install(TARGETS kopete_latex  DESTINATION ${PLUGIN_INSTALL_DIR})


########### next target ###############

set(kcm_kopete_latex_PART_SRCS latexpreferences.cpp )

ki18n_wrap_ui(kcm_kopete_latex_PART_SRCS latexprefsbase.ui )

kconfig_add_kcfg_files(kcm_kopete_latex_PART_SRCS latexconfig.kcfgc )


add_library(kcm_kopete_latex MODULE ${kcm_kopete_latex_PART_SRCS})


target_link_libraries(kcm_kopete_latex
    KF5::KDELibs4Support
    KF5::KIOCore
    KF5::KIOWidgets
    KF5::KCMUtils
    Qt5::Widgets
    Qt5::Gui
)

install(TARGETS kcm_kopete_latex  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES latexconfig.kcfg  DESTINATION ${KCFG_INSTALL_DIR})
install( FILES kopete_latex.desktop  DESTINATION ${SERVICES_INSTALL_DIR})
install( FILES latexchatui.rc  DESTINATION ${DATA_INSTALL_DIR}/kopete_latex)
install( FILES kopete_latex_config.desktop  DESTINATION ${SERVICES_INSTALL_DIR}/kconfiguredialog)
install( PROGRAMS kopete_latexconvert.sh DESTINATION ${BIN_INSTALL_DIR})



