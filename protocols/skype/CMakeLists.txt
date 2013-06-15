project( kopete_skype )

########### next target ###############

include_directories(
	${QT_QTDBUS_INCLUDE_DIR}

	${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}/libskype
	${CMAKE_CURRENT_SOURCE_DIR}/libskype/skypedbus
	${CMAKE_CURRENT_SOURCE_DIR}/skypebuttons
	${CMAKE_CURRENT_SOURCE_DIR}/skypebuttons/qtbrowserplugin

	${CMAKE_CURRENT_BUILD_DIR}
)

add_definitions(-DSKYPE_DEBUG_GLOBAL=14311)

########### next target ###############

set(
	skypedbus_SRCS
	libskype/skypedbus/skypeconnection.cpp
)

QT4_ADD_DBUS_ADAPTOR(
	skypedbus_SRCS
	libskype/skypedbus/com.Skype.API.Client.xml
	libskype/skypedbus/skypeconnection.h
	SkypeConnection
	skypeadaptor
	SkypeAdaptor
)

set(
	libskype_SRCS

	libskype/skype.cpp
	libskype/skypewindow.cpp
)

set(
	kopete_skype_ui_SRCS
)

kde4_add_ui_files(
	kopete_skype_ui_SRCS
	ui/skypeaddcontactbase.ui
	ui/skypecalldialogbase.ui
	ui/skypedetailsbase.ui
	ui/skypeeditaccountbase.ui
)

set(
	kopete_skype_SRCS
	${skypedbus_SRCS}
	${libskype_SRCS}
	${kopete_skype_ui_SRCS}
	skypeaccount.cpp
	skypeaddcontact.cpp
	skypecalldialog.cpp
#	skypeconference.cpp
	skypecontact.cpp
	skypedetails.cpp
	skypeeditaccount.cpp
	skypechatsession.cpp
	skypeprotocol.cpp
)

kde4_add_plugin(
	kopete_skype
	${kopete_skype_SRCS}
)

target_link_libraries(
	kopete_skype
	kopete
	${KDE4_KDEUI_LIBS}
	${X11_X11_LIB}
)

########### next target ###############

add_subdirectory( icons )
macro_optional_add_subdirectory( skypebuttons )

########### install files ###############

install( TARGETS kopete_skype DESTINATION ${PLUGIN_INSTALL_DIR} )
install( FILES kopete_skype.desktop skype.protocol callto.protocol tel.protocol DESTINATION ${SERVICES_INSTALL_DIR} )
install( FILES skypeui.rc skypechatui.rc DESTINATION ${DATA_INSTALL_DIR}/kopete_skype )

#Enable this for system bus dbus type communication with skype public api
#Linux skype client (version 2.0.0.72) still does not support it, so we do not need it now maybe in new skype version
#if (DBUS_SYSTEM_POLICY_DIR)
#	install( FILES libskype/skypedbus/com.Skype.API.Client.conf DESTINATION ${DBUS_SYSTEM_POLICY_DIR})
#else (DBUS_SYSTEM_POLICY_DIR)
#	message(STATUS "It is necessary to specify the directory where the client policy file for com.Skype.API.Client.conf is installed as DBUS_SYSTEM_POLICY_DIR.  Normally this is /etc/dbus-1/system.d on a Linux system")
#endif (DBUS_SYSTEM_POLICY_DIR)
