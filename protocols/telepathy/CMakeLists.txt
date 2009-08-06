
add_subdirectory( icons )

REMOVE_DEFINITIONS(-DQT_NO_STL)

include_directories( 
        ${KDE4_INCLUDES}
        ${KOPETE_INCLUDES}
        ${KCMTELEPATHYACCOUNTS_INCLUDE_DIR}
        ${TELEPATHY_QT4_INCLUDE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/ui
        ${CMAKE_CURRENT_BINARY_dir}/ui
        ${CMAKE_CURRENT_SOURCE_DIR}/accounts_kcm
        ${CMAKE_CURRENT_BINARY_DIR}/accounts_kcm
        ${CMAKE_CURRENT_SOURCE_DIR}/accounts_kcm/defaultaccountui
        ${CMAKE_CURRENT_BINARY_DIR}/accounts_kcm/defaultaccountui
)

# UI classes
set(kopete_telepathy_ui_SRCS
ui/telepathyeditaccountwidget.cpp
ui/telepathyaddcontactpage.cpp
)

# Generated UI files
kde4_add_ui_files(kopete_telepathy_ui_SRCS
ui/telepathyaddcontactpage.ui
                   accounts_kcm/protocol-select-widget.ui
                   accounts_kcm/defaultaccountui/parameter-edit-widget.ui
)

# Normal protocol classes
set(kopete_telepathy_PART_SRCS 
telepathyprotocol.cpp
telepathyaccount.cpp
telepathycontact.cpp
telepathycontactmanager.cpp
telepathychatsession.cpp
telepathyclienthandler.cpp
telepathychannelmanager.cpp
${kopete_telepathy_ui_SRCS}
     accounts_kcm/protocol-select-widget.cpp
     accounts_kcm/protocol-list-model.cpp
     accounts_kcm/connection-manager-item.cpp
     accounts_kcm/protocol-item.cpp
     accounts_kcm/defaultaccountui/parameter-edit-widget.cpp
     accounts_kcm/defaultaccountui/parameter-edit-delegate.cpp
     accounts_kcm/defaultaccountui/parameter-edit-model.cpp
     accounts_kcm/defaultaccountui/parameter-item.cpp
     accounts_kcm/defaultaccountui/integer-edit.cpp
     accounts_kcm/defaultaccountui/unsigned-integer-edit.cpp
     accounts_kcm/defaultaccountui/mandatory-parameter-edit-widget.cpp
     accounts_kcm/defaultaccountui/optional-parameter-edit-widget.cpp
     accounts_kcm/dictionary.cpp
     accounts_kcm/plugin-manager.cpp
)

kde4_add_plugin(kopete_telepathy ${kopete_telepathy_PART_SRCS})

target_link_libraries(kopete_telepathy
                      ${KDE4_KIO_LIBS}
                      kopete
                      ${TELEPATHY_QT4_LIBRARIES}
                      ${KCMTELEPATHYACCOUNTS_LIBRARIES}
)

install(TARGETS kopete_telepathy  DESTINATION ${PLUGIN_INSTALL_DIR})

########### install files ###############

install( FILES kopete_telepathy.desktop  DESTINATION ${SERVICES_INSTALL_DIR})
