include_directories (${CMAKE_CURRENT_SOURCE_DIR}
                     ${CMAKE_CURRENT_BINARY_DIR}
)

add_subdirectory (KopeteTelepathy)

project (kopete-protocol-telepathy)

add_subdirectory( icons )

REMOVE_DEFINITIONS(-DQT_NO_STL)

include_directories( 
        ${KDE4_INCLUDES}
        ${KOPETE_INCLUDES}
        ${KCMTELEPATHYACCOUNTS_INCLUDE_DIR}
        ${TELEPATHY_QT4_INCLUDE_DIR}
)

set(kopete_protocol_telepathy_SRCS 
    kopete-protocol-telepathy.cpp
)

kde4_add_plugin(kopete_telepathy ${kopete_protocol_telepathy_SRCS})

target_link_libraries(kopete_telepathy
                      kopetetelepathy
)

install (TARGETS kopete_telepathy
         DESTINATION ${PLUGIN_INSTALL_DIR})

########### install files ###############

install( FILES kopete_telepathy.desktop  DESTINATION ${SERVICES_INSTALL_DIR})
