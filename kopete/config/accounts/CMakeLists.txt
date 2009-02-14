
include_directories( 
${KOPETE_INCLUDES} 
${CMAKE_SOURCE_DIR}/kopete/kopete/addaccountwizard 
${CMAKE_BINARY_DIR}/kopete/kopete/addaccountwizard 
${CMAKE_SOURCE_DIR}/kopete/kopete/identity
${CMAKE_BINARY_DIR}/kopete/kopete/identity
)

link_directories( ${CMAKE_BINARY_DIR}/kopete/kopete/addaccountwizard ${CMAKE_BINARY_DIR}/kopete/kopete/identity )

########### next target ###############

set(kcm_kopete_accountconfig_PART_SRCS accounttreewidget.cpp kopeteaccountconfig.cpp )


kde4_add_ui_files(kcm_kopete_accountconfig_PART_SRCS kopeteaccountconfigbase.ui )

kde4_add_plugin(kcm_kopete_accountconfig ${kcm_kopete_accountconfig_PART_SRCS})


target_link_libraries(kcm_kopete_accountconfig  ${KDE4_KUTILS_LIBS} kopete kopeteaddaccountwizard kopeteidentity)

install(TARGETS kcm_kopete_accountconfig  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES kopete_accountconfig.desktop  DESTINATION ${SERVICES_INSTALL_DIR})




