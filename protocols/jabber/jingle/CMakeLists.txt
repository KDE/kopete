add_definitions(-DQT3_SUPPORT -DQT3_SUPPORT_WARNINGS)

#add_subdirectory( ui ) 
#add_subdirectory( icons ) 
#add_subdirectory( libiris ) 

#message(STATUS "${CMAKE_CURRENT_SOURCE_DIR}: skipped subdir $(JINGLE)")
include_directories( 
${KOPETE_INCLUDES} 
${CMAKE_CURRENT_SOURCE_DIR}/../ui/ 
${CMAKE_CURRENT_SOURCE_DIR}/../libiris/iris/include/
${CMAKE_CURRENT_SOURCE_DIR}/../libiris/iris/jabber/ 
#${CMAKE_CURRENT_SOURCE_DIR}/libiris/iris/xmpp-core/ 
${CMAKE_CURRENT_SOURCE_DIR}/../libiris/iris/xmpp-im/ 
${CMAKE_CURRENT_SOURCE_DIR}/../libiris/cutestuff/util/ 
#${CMAKE_CURRENT_SOURCE_DIR}/libiris/cutestuff/network/ 
${CMAKE_CURRENT_SOURCE_DIR}/../libiris/cutestuff/ 
${CMAKE_CURRENT_SOURCE_DIR}/../libiris/iris/ 
${QCA2_INCLUDE_DIR} 
${CMAKE_CURRENT_SOURCE_DIR}/../ 
${CMAKE_CURRENT_SOURCE_DIR}/libjingle/ 
)

link_directories( ${CMAKE_CURRENT_BINARY_DIR}/../libiris/iris/xmpp-im ./)


########### next target ###############

set(kopete_jabber_jingle_ui_SRCS
	ui/jinglevoicesessiondialogbase.ui
)

kde4_add_ui_files(kopete_jabber_jingle_ui_SRCS)

set(kopete_jabber_jingle_PART_SRCS 
	${kopete_jabber_jingle_ui_SRCS}
	jinglefoosession.cpp
	jinglejabberxml.cpp
	jinglewatchsessiontask.cpp
	jinglesessionmanager.cpp
#	jinglevoicesession.cpp
	jinglefootransport.cpp
#	jinglenetwork.cpp
	jinglevoicesessiondialog.cpp
	jinglesession.cpp
	jinglevoicecaller.cpp
)

set (kopete_jingle_Q_OBJECT_FILES
	voicecaller.h
	jinglesession.h
	jinglevoicesession.h
	jinglevoicecaller.h
#	jinglenetwork.h
	jinglewatchsessiontask.h
	jinglevoicesessiondialog.h
	jingleconnectioncandidate.h
	jinglesessionmanager.h
	jinglefoosession.h
)

qt4_wrap_cpp(kopete_jingle_MOC_FILES ${kopete_jingle_Q_OBJECT_FILES})

kde4_add_library(kopete_jabberjingle STATIC
	${kopete_jabber_jingle_PART_SRCS}
	${kopete_jingle_MOC_FILES}
)

link_directories(
	${CMAKE_CURRENT_SOURCE_DIR}/libjingle/talk/session/phone/.libs/
	${CMAKE_CURRENT_SOURCE_DIR}/libjingle/talk/p2p/client/.libs/
	${CMAKE_CURRENT_SOURCE_DIR}/libjingle/talk/p2p/base/.libs/
	${CMAKE_CURRENT_SOURCE_DIR}/libjingle/talk/xmpp/.libs/
	${CMAKE_CURRENT_SOURCE_DIR}/libjingle/talk/xmllite/.libs/
	${CMAKE_CURRENT_SOURCE_DIR}/libjingle/talk/base/.libs/
	${CMAKE_CURRENT_SOURCE_DIR}/libjingle/talk/third_party/mediastreamer/.libs/
	${CMAKE_BINARY_DIR}/lib/
)

target_link_libraries(kopete_jabberjingle ${KDE4_KDE3SUPPORT_LIBS} ${QCA2_LIBRARIES}
	speex ortp glib-2.0 gmodule-2.0 gthread-2.0 asound expat kopete iris_kopete 
	${CMAKE_CURRENT_SOURCE_DIR}/libjingle/talk/session/phone/.libs/libcricketsessionphone.a
	${CMAKE_CURRENT_SOURCE_DIR}/libjingle/talk/p2p/client/.libs/libcricketp2pclient.a 
	${CMAKE_CURRENT_SOURCE_DIR}/libjingle/talk/p2p/base/.libs/libcricketp2pbase.a 
	${CMAKE_CURRENT_SOURCE_DIR}/libjingle/talk/xmllite/.libs/libcricketxmllite.a 
	${CMAKE_CURRENT_SOURCE_DIR}/libjingle/talk/xmpp/.libs/libcricketxmpp.a 
	${CMAKE_CURRENT_SOURCE_DIR}/libjingle/talk/base/.libs/libcricketbase.a 
	${CMAKE_CURRENT_SOURCE_DIR}/libjingle/talk/third_party/mediastreamer/.libs/libmediastreamer.a
)


install(TARGETS kopete_jabberjingle  DESTINATION ${LIB_INSTALL_DIR})


########### install files ###############

install( FILES kopete_jabber.desktop xmpp.protocol DESTINATION ${SERVICES_INSTALL_DIR})
install( FILES jabberchatui.rc  DESTINATION ${DATA_INSTALL_DIR}/kopete_jabber)
