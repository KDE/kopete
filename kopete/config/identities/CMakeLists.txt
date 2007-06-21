
include_directories(
	${KOPETE_INCLUDES}
	${CMAKE_SOURCE_DIR}/kopete/kopete/addidentitywizard
	${CMAKE_BINARY_DIR}/kopete/kopete/addidentitywizard
	)

link_directories( ${CMAKE_BINARY_DIR}/kopete/kopete/addidentitywizard )

########### next target ###############

set(kcm_kopete_identityconfig_PART_SRCS kopeteidentityconfig.cpp )

kde4_add_ui_files(kcm_kopete_identityconfig_PART_SRCS kopeteidentityconfigbase.ui )

kde4_automoc(${kcm_kopete_identityconfig_PART_SRCS})

kde4_add_plugin(kcm_kopete_identityconfig ${kcm_kopete_identityconfig_PART_SRCS})


target_link_libraries(kcm_kopete_identityconfig  ${KDE4_KUTILS_LIBS} ${KDE4_KABC_LIBS} kopete kopeteaddidentitywizard)

install(TARGETS kcm_kopete_identityconfig  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES kopete_identityconfig.desktop  DESTINATION ${SERVICES_INSTALL_DIR})

