#add_subdirectory(libjingle)

MACRO (QMAKE_WRAP filename outfiles )
  # get include dirs
  QT4_GET_MOC_INC_DIRS(moc_includes)

  FOREACH (it ${ARGN})
    GET_FILENAME_COMPONENT(it ${it} ABSOLUTE)
    GET_FILENAME_COMPONENT(outfile ${it} NAME_WE)

    SET(outfile ${CMAKE_CURRENT_BINARY_DIR}/${filename})
    ADD_CUSTOM_COMMAND(OUTPUT ${outfile}
      COMMAND ${QT_MOC_EXECUTABLE}
      ARGS ${moc_includes} -o ${outfile} ${it}
      DEPENDS ${it})
	SET(${outfiles} ${${outfiles}} ${outfile})
  ENDFOREACH(it)
ENDMACRO (QMAKE_WRAP)

MACRO (QMAKE_WRAP_CPP outfiles )
   QMAKE_WRAP("\${outfile}.moc" ${outfiles} ${ARGN})
ENDMACRO (QMAKE_WRAP_CPP)

MACRO (QMAKE_WRAP_H outfiles )
   QMAKE_WRAP("moc_\${outfile}.cpp" ${outfiles} ${ARGN})
ENDMACRO (QMAKE_WRAP_H)

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

link_directories( ${CMAKE_CURRENT_BINARY_DIR}/../libiris/iris/xmpp-im )

########### next target ###############

set(kopete_jingle_PART_SRCS 

  jinglefoosession.cpp
  jinglejabberxml.cpp
  jinglesession.cpp 
  jinglesessionmanager.cpp 
  #jinglevoicesession.cpp  this one'll get put back
  jinglevoicesessiondialog.cpp 
  voicecaller.h
  jinglewatchsessiontask.cpp 
  #jinglenetwork.cpp
  #${CMAKE_CURRENT_SOURCE_DIR}/../libiris/iris/xmpp-im/xmpp_xmlcommon.cpp
  #${CMAKE_CURRENT_SOURCE_DIR}/../libiris/iris/xmpp-core/stream.cpp
)

kde4_add_ui_files(
  kopete_jingle_PART_SRCS
  jinglevoicesessiondialogbase.ui
)

set(moc_source jinglevoicecaller.cpp)
set(moc_head voicecaller.h)


QMAKE_WRAP_CPP(kopete_jingle_PART_SRCS ${moc_source})
QMAKE_WRAP_H(kopete_jingle_PART_SRCS ${moc_head})


kde4_automoc(${kopete_jingle_PART_SRCS})

kde4_add_library(kopete_jabberjingle STATIC ${kopete_jingle_PART_SRCS})

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


#target_link_libraries(kopete_jabberjingle ${KDE4_KDE3SUPPORT_LIBS} ${QCA2_LIBRARIES} kopete iris_kopete jinglebase-0.3 jinglep2pbase-0.3 jinglep2pclient-0.3 jinglesessiontunnel-0.3 jinglexmpp-0.3)


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

#install( FILES kopete_jabber.desktop xmpp.protocol DESTINATION ${SERVICES_INSTALL_DIR})
#install( FILES jabberchatui.rc  DESTINATION ${DATA_INSTALL_DIR}/kopete_jabber)
