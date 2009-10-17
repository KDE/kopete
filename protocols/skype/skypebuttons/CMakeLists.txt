########### next target ###############

include( ${QT_USE_FILE} )
add_definitions(${QT_DEFINITIONS})

include_directories(
	${QT_INCLUDE_DIR}
	${QT_QTDBUS_INCLUDE_DIR}
)

set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fvisibility=default" )
add_definitions(-DQT_PLUGIN -DQT_SHARED)

string( REGEX MATCHALL ^$ENV{HOME}.*$ IS_HOME ${CMAKE_INSTALL_PREFIX} )
string( COMPARE EQUAL /usr ${CMAKE_INSTALL_PREFIX} IS_USR )
string( COMPARE EQUAL /usr/local ${CMAKE_INSTALL_PREFIX} IS_LOCAL )

if( NOT MOZPLUGIN_INSTALL_DIR )
	if( IS_HOME AND NOT IS_USR AND NOT IS_LOCAL )
		set( MOZPLUGIN_INSTALL_DIR $ENV{HOME}/.mozilla/plugins )
	endif( IS_HOME AND NOT IS_USR AND NOT IS_LOCAL )
	if( NOT IS_HOME AND IS_USR AND NOT IS_LOCAL )
		set( MOZPLUGIN_INSTALL_DIR /usr/lib${LIB_SUFFIX}/mozilla/plugins )
	endif( NOT IS_HOME AND IS_USR AND NOT IS_LOCAL )
	if( NOT IS_HOME AND NOT IS_USR AND IS_LOCAL )
		set( MOZPLUGIN_INSTALL_DIR /usr/local/netscape/plugins )
	endif( NOT IS_HOME AND NOT IS_USR AND IS_LOCAL )
	if( NOT IS_HOME AND NOT IS_USR AND NOT IS_LOCAL )
		set( MOZPLUGIN_INSTALL_DIR ${CMAKE_INSTALL_PREFIX}/lib${LIB_SUFFIX}/mozilla/plugins )
	endif( NOT IS_HOME AND NOT IS_USR AND NOT IS_LOCAL )
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

