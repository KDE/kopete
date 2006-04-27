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
cutestuff/network/httppoll.cpp
cutestuff/network/ndns.cpp 
cutestuff/network/servsock.cpp 
cutestuff/network/socks.cpp 
cutestuff/network/srvresolver.cpp 
)

set(iris_SRCS 
iris/xmpp-core/connector.cpp 
iris/xmpp-core/jid.cpp 
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
${CMAKE_CURRENT_BUILD_DIR}/im.cpp
${CMAKE_CURRENT_BUILD_DIR}/xmpp.cpp
)

set(libiris_SRCS 
    ${cutestuff_SRCS} 
    ${iris_SRCS} 
)

set(libiris_needmoc_SRCS 
cutestuff/network/servsock.cpp 
cutestuff/network/ndns.cpp 
cutestuff/network/httpconnect.cpp 
cutestuff/network/bsocket.cpp 
cutestuff/network/socks.cpp 
cutestuff/network/srvresolver.cpp 
cutestuff/network/httppoll.cpp 
cutestuff/util/bytestream.cpp 
cutestuff/util/safedelete.cpp 
iris/jabber/filetransfer.cpp 
iris/jabber/xmpp_ibb.cpp 
iris/jabber/xmpp_jidlink.cpp 
iris/xmpp-im/xmpp_tasks.cpp 
)

qt4_generate_moc( ${CMAKE_CURRENT_SOURCE_DIR}/iris/xmpp-core/connector.cpp ${CMAKE_CURRENT_BINARY_DIR}/connector.moc )
qt4_generate_moc( ${CMAKE_CURRENT_SOURCE_DIR}/iris/xmpp-core/tlshandler.cpp ${CMAKE_CURRENT_BINARY_DIR}/tlshandler.moc )
qt4_generate_moc( ${CMAKE_CURRENT_SOURCE_DIR}/iris/xmpp-core/stream.cpp ${CMAKE_CURRENT_BINARY_DIR}/stream.moc )
qt4_generate_moc( ${CMAKE_CURRENT_SOURCE_DIR}/iris/xmpp-core/jid.cpp ${CMAKE_CURRENT_BINARY_DIR}/jid.moc )
qt4_generate_moc( ${CMAKE_CURRENT_SOURCE_DIR}/iris/xmpp-im/types.cpp ${CMAKE_CURRENT_BINARY_DIR}/types.moc )
qt4_generate_moc( ${CMAKE_CURRENT_SOURCE_DIR}/iris/include/im.h ${CMAKE_CURRENT_BINARY_DIR}/im.cpp )
qt4_generate_moc( ${CMAKE_CURRENT_SOURCE_DIR}/iris/include/xmpp.h ${CMAKE_CURRENT_BINARY_DIR}/xmpp.cpp )

# BEGIN moc special case
# Those are special cases, both header and implementation contains a Q_OBJECT macro
# What we do is generate a moc file for both header and implementation and with CMake FILE command, create the final .moc
qt4_generate_moc( ${CMAKE_CURRENT_SOURCE_DIR}/iris/xmpp-core/securestream.h ${CMAKE_CURRENT_BINARY_DIR}/securestream_h.moc )
qt4_generate_moc( ${CMAKE_CURRENT_SOURCE_DIR}/iris/xmpp-core/securestream.cpp ${CMAKE_CURRENT_BINARY_DIR}/securestream_cpp.moc )
qt4_generate_moc( ${CMAKE_CURRENT_SOURCE_DIR}/iris/jabber/s5b.cpp ${CMAKE_CURRENT_BINARY_DIR}/s5b_cpp.moc )
qt4_generate_moc( ${CMAKE_CURRENT_SOURCE_DIR}/iris/jabber/s5b.h ${CMAKE_CURRENT_BINARY_DIR}/s5b_h.moc )

set(securestream_h_MOC)
set(securestream_cpp_MOC)
set(s5b_h_MOC)
set(s5b_cpp_MOC)

# Create securestream.moc
file(READ ${CMAKE_CURRENT_BINARY_DIR}/securestream_h.moc securestream_h_MOC)
file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/securestream.moc ${securestream_h_MOC})
file(READ ${CMAKE_CURRENT_BINARY_DIR}/securestream_cpp.moc securestream_cpp_MOC)
file(APPEND ${CMAKE_CURRENT_BINARY_DIR}/securestream.moc ${securestream_cpp_MOC})

# Create s5b.moc
file(READ ${CMAKE_CURRENT_BINARY_DIR}/s5b_h.moc s5b_h_MOC)
file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/s5b.moc ${s5b_h_MOC})
file(READ ${CMAKE_CURRENT_BINARY_DIR}/s5b_cpp.moc s5b_cpp_MOC)
file(APPEND ${CMAKE_CURRENT_BINARY_DIR}/s5b.moc ${s5b_cpp_MOC})

# END moc special case

kde4_automoc( ${libiris_needmoc_SRCS} )

kde4_add_library(iris_kopete SHARED ${libiris_SRCS})

target_link_libraries(iris_kopete ${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY} ${QT_QT3SUPPORT_LIBRARY} ${QT_QTNETWORK_LIBRARY} ${QT_QTXML_LIBRARY} idn ${QCA2_LIBRARIES})

install_targets( ${LIB_INSTALL_DIR} iris_kopete )

########### install files ###############


kde4_footer()



#original Makefile.am contents follow:

#SUBDIRS = cutestuff iris
