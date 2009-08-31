project (libkopetetelepathy)

include_directories (${KCMTELEPATHYACCOUNTS_INCLUDE_DIR}
                     ${KDE4_INCLUDES}
                     ${KOPETE_INCLUDES}
                     ${TELEPATHY_QT4_INCLUDE_DIR}
                     ${CMAKE_CURRENT_BINARY_DIR}
                     ${CMAKE_CURRENT_SOURCE_DIR}
)

# TelepathyQt4 requires this definition removal.
remove_definitions (-DQT_NO_STL)

set (libkopetetelepathy_SRCS
     telepathyaccount.cpp
     telepathyaddedinfoevent.cpp
     telepathychannelmanager.cpp
     telepathychatsession.cpp
     telepathyclienthandler.cpp
     telepathycontact.cpp
     telepathycontactmanager.cpp
     telepathyprotocolinternal.cpp
)

set (libkopetetelepathy_HDRS
     telepathyaccount.h
     telepathyaddedinfoevent.h
     telepathychannelmanager.h
     telepathychatsession.h
     telepathyclienthandler.h
     telepathycontact.h
     telepathycontactmanager.h
     telepathyprotocolinternal.h
)

set (libkopetetelepathy_PRETTY_HDRS
     include/TelepathyAccount
     include/TelepathyAddedInfoEvent
     include/TelepathyChannelManager
     include/TelepathyChatSession
     include/TelepathyClientHandler
     include/TelepathyContact
     include/TelepathyContactManager
     include/TelepathyProtocol
)

kde4_add_library (kopetetelepathy
                  SHARED
                  ${libkopetetelepathy_SRCS}                 
)

target_link_libraries (kopetetelepathy
                       ${QT_QTCORE_LIBRARY}
                       ${QT_QTGUI_LIBRARY}
                       ${KDE4_KDEUI_LIBRARY}
                       ${KOPETE_LIBRARIES}
                       ${TELEPATHY_QT4_LIBRARIES}
                       ${KCMTELEPATHYACCOUNTS_LIBRARIES}
)

set_target_properties (kopetetelepathy PROPERTIES
                       VERSION ${GENERIC_LIB_VERSION}
                       SOVERSION ${GENERIC_LIB_SOVERSION}
)

if (MSVC)
    set_target_properties (kopetetelepathy PROPERTIES
                           OUTPUT_NAME libkopetetelepathy
    )
endif (MSVC)

# Install:
install (TARGETS kopetetelepathy
         ${INSTALL_TARGETS_DEFAULT_ARGS}
)

install (FILES
         ${libkopetetelepathy_HDRS}
         ${libkopetetelepathy_PRETTY_HDRS}
         DESTINATION ${INCLUDE_INSTALL_DIR}/KopeteTelepathy COMPONENT Devel
)

