
include_directories( 
    ${KOPETE_INCLUDES}
    ${KOPETE_SOURCE_DIR}/kopete/statusmenu
    ${KOPETE_BINARY_DIR}/kopete/statusmenu
    ${KOPETE_SOURCE_DIR}/kopete/addaccountwizard
    ${KOPETE_BINARY_DIR}/kopete/addaccountwizard
    )

link_directories( ${KOPETE_BINARY_DIR}/kopete/addaccountwizard )

########### next target ###############

set( kopeteidentity_SRCS
    accountidentitydialog.cpp
    identitydialog.cpp
    identitystatuswidget.cpp
    )

ki18n_wrap_ui( kopeteidentity_SRCS
    accountidentitybase.ui
    identitygeneral.ui
    identitydetailed.ui
    identitystatusbase.ui
    )

add_library( kopeteidentity SHARED ${kopeteidentity_SRCS} )
generate_export_header(kopeteidentity BASE_NAME kopeteidentity)
target_link_libraries( kopeteidentity kopeteaddaccountwizard kopete  kopetestatusmenu )
set_target_properties( kopeteidentity PROPERTIES VERSION 1.0.0 SOVERSION 1 )
install(TARGETS kopeteidentity ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})

########### install files ###############





