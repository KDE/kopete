
include_directories( 
${KOPETE_INCLUDES} 
${KOPETE_SOURCE_DIR}/kopete/addaccountwizard
${KOPETE_BINARY_DIR}/kopete/addaccountwizard
${KOPETE_SOURCE_DIR}/kopete/identity
${KOPETE_BINARY_DIR}/kopete/identity
)

link_directories( ${KOPETE_BINARY_DIR}/kopete/addaccountwizard ${KOPETE_BINARY_DIR}/kopete/identity )

########### next target ###############

set(kcm_kopete_accountconfig_PART_SRCS accounttreewidget.cpp kopeteaccountconfig.cpp )


ki18n_wrap_ui(kcm_kopete_accountconfig_PART_SRCS kopeteaccountconfigbase.ui )

add_library(kcm_kopete_accountconfig MODULE ${kcm_kopete_accountconfig_PART_SRCS})


target_link_libraries(kcm_kopete_accountconfig   kopete kopeteaddaccountwizard kopeteidentity)

install(TARGETS kcm_kopete_accountconfig  DESTINATION ${KDE_INSTALL_PLUGINDIR})


########### install files ###############

install( FILES kopete_accountconfig.desktop  DESTINATION ${KDE_INSTALL_KSERVICES5DIR})




