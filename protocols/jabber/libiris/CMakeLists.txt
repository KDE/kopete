kde4_header()

#add_subdirectory( cutestuff ) 
#add_subdirectory( iris ) 

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} 
${CMAKE_CURRENT_SOURCE_DIR}/cutestuff/util/ 
${CMAKE_CURRENT_SOURCE_DIR}/cutestuff/network/ 
${CMAKE_CURRENT_SOURCE_DIR}/iris/include/ 
${CMAKE_CURRENT_SOURCE_DIR}/iris/jabber/ 
${CMAKE_CURRENT_SOURCE_DIR}/iris/xmpp-core/ 
${CMAKE_CURRENT_SOURCE_DIR}/iris/xmpp-im/ 
${QCA2_INCLUDE_DIR} 
)

set(cutestuff_SRCS 
cutestuff/util/base64.cpp 
cutestuff/util/bytestream.cpp 
cutestuff/util/safedelete.cpp 
cutestuff/util/sha1.cpp 
cutestuff/network/bsocket.cpp 
cutestuff/network/httpconnect.cpp 
cutestuff/network/ndns.cpp 
cutestuff/network/servsock.cpp 
cutestuff/network/srvresolver.cpp 
)

set(iris_SRCS 
iris/xmpp-core/connector.cpp 
iris/xmpp-core/securestream.cpp 
iris/xmpp-core/tlshandler.cpp 
iris/xmpp-core/protocol.cpp 
iris/xmpp-core/stream.cpp 
iris/xmpp-core/xmlprotocol.cpp 
iris/xmpp-core/parser.cpp 
iris/xmpp-im/client.cpp 
iris/xmpp-im/types.cpp 
iris/xmpp-im/xmpp_tasks.cpp 
iris/xmpp-im/xmpp_vcard.cpp 
iris/xmpp-im/xmpp_xmlcommon.cpp 
iris/jabber/filetransfer.cpp 
iris/jabber/s5b.cpp 
iris/jabber/xmpp_ibb.cpp 
iris/jabber/xmpp_jidlink.cpp
)

set(libiris_SRCS 
    ${cutestuff_SRCS} 
    ${iris_SRCS} 
)

qt4_generate_moc( iris/xmpp-im/types.cpp ${CMAKE_CURRENT_BINARY_DIR}/types.moc )

kde4_automoc( ${libiris_SRCS} )

kde4_add_library(iris_kopete SHARED ${libiris_SRCS})

target_link_libraries(iris_kopete ${QT_QTCORE_LIBRARY} ${QT_QT3SUPPORT_LIBRARY} ${QT_QTNETWORK_LIBRARY} ${QT_QTXML_LIBRARY} idn ${QCA2_LIBRARIES})

########### install files ###############


kde4_footer()



#original Makefile.am contents follow:

#SUBDIRS = cutestuff iris
