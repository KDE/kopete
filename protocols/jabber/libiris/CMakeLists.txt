
#add_subdirectory( cutestuff ) 
#add_subdirectory( iris ) 

FIND_PACKAGE(ZLIB REQUIRED)

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

if(MSVC)
  add_definitions(-DIRIS_FULL_TEMPLATE_EXPORT_INSTANTIATION)
endif(MSVC)

include_directories( 
${CMAKE_CURRENT_SOURCE_DIR}/cutestuff/ 
${CMAKE_CURRENT_SOURCE_DIR}/cutestuff/legacy/ 
${CMAKE_CURRENT_SOURCE_DIR}/cutestuff/network/ 
${CMAKE_CURRENT_SOURCE_DIR}/cutestuff/util/ 
${CMAKE_CURRENT_SOURCE_DIR}/iris/include/ 
${CMAKE_CURRENT_SOURCE_DIR}/iris/jabber/ 
${CMAKE_CURRENT_SOURCE_DIR}/iris/xmpp-core/ 
${CMAKE_CURRENT_SOURCE_DIR}/iris/xmpp-im/ 
${CMAKE_CURRENT_SOURCE_DIR}/iris/ 
${QCA2_INCLUDE_DIR} 
${IDN_INCLUDE_DIR}
${ZLIB_INCLUDE_DIR}
)

set(cutestuff_SRCS 
cutestuff/legacy/ndns.cpp
cutestuff/legacy/safedelete.cpp
cutestuff/legacy/servsock.cpp
cutestuff/legacy/srvresolver.cpp
cutestuff/network/bsocket.cpp 
cutestuff/network/httpconnect.cpp 
cutestuff/network/httppoll.cpp
cutestuff/network/socks.cpp 
cutestuff/util/bytestream.cpp 
)

set(iris_SRCS 
iris/jabber/filetransfer.cpp
iris/jabber/s5b.cpp
iris/jabber/xmpp_ibb.cpp

iris/xmpp-core/compress.cpp
iris/xmpp-core/compressionhandler.cpp
iris/xmpp-core/connector.cpp
iris/xmpp-core/jid.cpp
iris/xmpp-core/securestream.cpp
iris/xmpp-core/tlshandler.cpp
iris/xmpp-core/protocol.cpp 
iris/xmpp-core/stream.cpp
iris/xmpp-core/xmlprotocol.cpp
iris/xmpp-core/parser.cpp
iris/xmpp-core/simplesasl.cpp

iris/xmpp-im/client.cpp
iris/xmpp-im/types.cpp
iris/xmpp-im/xmpp_tasks.cpp
iris/xmpp-im/xmpp_vcard.cpp
iris/xmpp-im/xmpp_xmlcommon.cpp
iris/xmpp-im/xmpp_xdata.cpp
)

set(libiris_SRCS 
    ${cutestuff_SRCS} 
    ${iris_SRCS} 
)

set(libiris_needmoc_SRCS
	iris/jabber/s5b.cpp
	iris/xmpp-core/securestream.cpp
	iris/xmpp-im/types.cpp
)

set(libiris_needmoc_HDRS
	cutestuff/legacy/ndns.h
	cutestuff/legacy/safedelete.h
	cutestuff/legacy/servsock.h
	cutestuff/legacy/srvresolver.h

	cutestuff/network/bsocket.h
	cutestuff/network/httpconnect.h
	cutestuff/network/socks.h
	cutestuff/network/httppoll.h

	cutestuff/util/bytestream.h

	iris/include/im.h
	iris/include/xmpp.h
	iris/include/xmpp_task.h

	iris/jabber/filetransfer.h
	iris/jabber/s5b.h
	iris/jabber/xmpp_ibb.h

	iris/xmpp-core/compress.h
	iris/xmpp-core/compressionhandler.h
	iris/xmpp-core/securestream.h

	iris/xmpp-im/xmpp_tasks.h
)

QMAKE_WRAP_CPP(libiris_SRCS ${libiris_needmoc_SRCS})
QMAKE_WRAP_H(libiris_SRCS ${libiris_needmoc_HDRS})

kde4_add_library(iris_kopete SHARED ${libiris_SRCS})

set(iris_kopete_libs ${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY} ${QT_QT3SUPPORT_LIBRARY} ${QT_QTNETWORK_LIBRARY} ${QT_QTXML_LIBRARY}
                     ${IDN_LIBRARY} ${QCA2_LIBRARIES} ${ZLIB_LIBRARIES})
if(WIN32)
  set(iris_kopete_libs ${iris_kopete_libs} ws2_32)
endif(WIN32)

target_link_libraries(iris_kopete ${iris_kopete_libs})

set_target_properties(iris_kopete PROPERTIES VERSION 1.0.0 SOVERSION 1 )
install(TARGETS iris_kopete  DESTINATION ${LIB_INSTALL_DIR})

########### install files ###############

