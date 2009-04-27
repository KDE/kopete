########### next target ###############

include( ${QT_USE_FILE} )

include_directories(
	${QT_INCLUDE_DIR}
	${QT_QTDBUS_INCLUDE_DIR}
)

set( skypeactionhandler_SRCS
	skypeactionhandler.cpp
)

add_executable( skypeactionhandler ${skypeactionhandler_SRCS} )
set_target_properties( skypeactionhandler PROPERTIES OUTPUT_NAME skype-action-handler )
target_link_libraries( skypeactionhandler ${QT_LIBRARIES} ${QT_QTDBUS_LIBRARY} )

########### next target ###############

set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fvisibility=default" )

if( NOT MOZPLUGIN_INSTALL_DIR )
	set( MOZPLUGIN_INSTALL_DIR /usr/lib/firefox/plugins )
endif( NOT MOZPLUGIN_INSTALL_DIR )

message( STATUS "Mozilla plugin install dir is \"${MOZPLUGIN_INSTALL_DIR}\" to change use parm -DMOZPLUGIN_INSTALL_DIR=<directory>" )

#TODO: Check if QtBrowerPlugin (or QtNsPlugin?) installed and use it

set( skypebuttons_SRCS
	skypebuttons.cpp
	qtbrowserplugin/qtbrowserplugin.cpp
	qtbrowserplugin/qtbrowserplugin_x11.cpp
)

QT4_GENERATE_MOC( ${CMAKE_CURRENT_SOURCE_DIR}/skypebuttons.cpp ${CMAKE_CURRENT_BINARY_DIR}/skypebuttons.moc )

add_library( skypebuttons SHARED ${skypebuttons_SRCS} ${qtbrowserplugin_SRC})
target_link_libraries( skypebuttons ${QT_LIBRARIES} )

########### install files ###############

install( TARGETS skypebuttons DESTINATION ${MOZPLUGIN_INSTALL_DIR} )
install( TARGETS skypeactionhandler ${INSTALL_TARGETS_DEFAULT_ARGS} )
install( FILES skype.protocol DESTINATION ${SERVICES_INSTALL_DIR} )
install( FILES callto.protocol DESTINATION ${SERVICES_INSTALL_DIR} )
install( FILES tel.protocol DESTINATION ${SERVICES_INSTALL_DIR} )
