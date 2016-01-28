
add_subdirectory( icons ) 

include_directories( 
${KOPETE_INCLUDES}
)


########### next target ###############

set(kopete_highlight_PART_SRCS highlightplugin.cpp highlightconfig.cpp filter.cpp )

add_library(kopete_highlight MODULE ${kopete_highlight_PART_SRCS})


target_link_libraries(kopete_highlight 
	kopete
	KF5::KDELibs4Support
	KF5::Parts
	Qt5::Xml
)

install(TARGETS kopete_highlight  DESTINATION ${PLUGIN_INSTALL_DIR})


########### next target ###############

set(kcm_kopete_highlight_PART_SRCS highlightpreferences.cpp filter.cpp highlightconfig.cpp )

ki18n_wrap_ui(kcm_kopete_highlight_PART_SRCS highlightprefsbase.ui )


add_library(kcm_kopete_highlight MODULE ${kcm_kopete_highlight_PART_SRCS})

target_link_libraries(kcm_kopete_highlight
	KF5::KDELibs4Support
	KF5::NotifyConfig
	KF5::Completion
	KF5::KCMUtils
	KF5::Parts
	Qt5::Widgets
	Qt5::Xml
)

install(TARGETS kcm_kopete_highlight  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES kopete_highlight.desktop  DESTINATION ${SERVICES_INSTALL_DIR})
install( FILES kopete_highlight_config.desktop  DESTINATION ${SERVICES_INSTALL_DIR}/kconfiguredialog)




