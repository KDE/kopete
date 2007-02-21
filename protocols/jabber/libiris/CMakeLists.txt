
#add_subdirectory( cutestuff ) 
#add_subdirectory( iris ) 

FIND_PACKAGE(ZLIB REQUIRED)

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
cutestuff/legacy/ndns.cpp
cutestuff/legacy/safedelete.cpp
cutestuff/legacy/servsock.cpp
cutestuff/legacy/srvresolver.cpp 

cutestuff/network/bsocket.cpp 
cutestuff/network/httpconnect.cpp 
cutestuff/network/socks.cpp 
cutestuff/network/httppoll.cpp 

cutestuff/util/bytestream.cpp 
cutestuff/util/safedelete.cpp 

iris/jabber/filetransfer.cpp 
iris/jabber/s5b.cpp 
iris/jabber/xmpp_ibb.cpp 
iris/jabber/xmpp_jidlink.cpp 

iris/xmpp-core/compress.cpp
iris/xmpp-core/compressionhandler.cpp
iris/xmpp-core/securestream.cpp

iris/xmpp-im/xmpp_tasks.cpp 
)

qt4_wrap_cpp(libiris_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/iris/include/im.h)
qt4_wrap_cpp(libiris_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/iris/include/xmpp.h)
qt4_wrap_cpp(libiris_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/iris/include/xmpp_task.h)

qt4_generate_moc(${CMAKE_CURRENT_SOURCE_DIR}/iris/jabber/s5b.cpp ${CMAKE_CURRENT_BINARY_DIR}/moc_s5b.cxx)
qt4_generate_moc(${CMAKE_CURRENT_SOURCE_DIR}/iris/xmpp-im/types.cpp ${CMAKE_CURRENT_BINARY_DIR}/moc_types.cxx)
qt4_generate_moc(${CMAKE_CURRENT_SOURCE_DIR}/iris/xmpp-core/securestream.cpp ${CMAKE_CURRENT_BINARY_DIR}/moc_securestream.cxx)

kde4_automoc(${libiris_needmoc_SRCS})

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

