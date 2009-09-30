
#add_subdirectory( cutestuff ) 
#add_subdirectory( iris ) 

FIND_PACKAGE(ZLIB REQUIRED)

remove_definitions(-DQT_NO_HTTP)
if(NOT MSVC)
  add_definitions(-fPIC)
endif(NOT MSVC)

include_directories(
${CMAKE_CURRENT_SOURCE_DIR}/iris
 ${CMAKE_CURRENT_SOURCE_DIR}/iris/xmpp/base
# ${CMAKE_CURRENT_SOURCE_DIR}/iris/xmpp/base/unittest
 ${CMAKE_CURRENT_SOURCE_DIR}/iris/xmpp/base
# ${CMAKE_CURRENT_SOURCE_DIR}/iris/xmpp/qa/qttestutil
 ${CMAKE_CURRENT_SOURCE_DIR}/iris/xmpp/jid
 ${CMAKE_CURRENT_SOURCE_DIR}/iris/xmpp/cutestuff
 ${CMAKE_CURRENT_SOURCE_DIR}/iris/xmpp/xmpp-im
 ${CMAKE_CURRENT_SOURCE_DIR}/iris/xmpp/sasl
 ${CMAKE_CURRENT_SOURCE_DIR}/iris/xmpp/zlib
 ${CMAKE_CURRENT_SOURCE_DIR}/iris/xmpp/base64
 ${CMAKE_CURRENT_SOURCE_DIR}/iris/xmpp/xmpp-core
 ${CMAKE_CURRENT_SOURCE_DIR}/iris/xmpp/jingle
 #${CMAKE_CURRENT_SOURCE_DIR}/iris/libidn
 ${CMAKE_CURRENT_SOURCE_DIR}/iris/jdns
 ${CMAKE_CURRENT_SOURCE_DIR}/iris/irisnet/noncore
 ${CMAKE_CURRENT_SOURCE_DIR}/iris/irisnet/noncore/legacy
 ${CMAKE_CURRENT_SOURCE_DIR}/iris/irisnet/corelib
 ${CMAKE_CURRENT_SOURCE_DIR}/iris/irisnet/appledns
 

${QCA2_INCLUDE_DIR} 
${IDN_INCLUDE_DIR}
${ZLIB_INCLUDE_DIR}
${DNSSD_INCLUDE_DIR}
)


# set(cutestuff_SRCS 
# cutestuff/legacy/ndns.cpp
# cutestuff/legacy/safedelete.cpp
# cutestuff/legacy/servsock.cpp
# cutestuff/legacy/srvresolver.cpp
# cutestuff/network/bsocket.cpp 
# cutestuff/network/httpconnect.cpp 
# cutestuff/network/httppoll.cpp
# cutestuff/network/socks.cpp 
# cutestuff/util/bytestream.cpp 
# )

set(iris_SRCS
iris/xmpp/base/randomnumbergenerator.cpp
# iris/xmpp/base/unittest/randomnumbergeneratortest.cpp
# iris/xmpp/base/unittest/randrandomnumbergeneratortest.cpp
# iris/xmpp/qa/qttestutil/testregistry.cpp
# iris/xmpp/qa/qttestutil/simplechecker.cpp
# iris/xmpp/qa/qttestutil/example/mysecondclasstest.cpp
# iris/xmpp/qa/qttestutil/example/myfirstclasstest.cpp
# iris/xmpp/qa/unittest.template/myclasstest.cpp
# iris/xmpp/jid/unittest/jidtest.cpp
iris/xmpp/jid/jid.cpp
iris/xmpp/cutestuff/httppoll.cpp
iris/xmpp/cutestuff/socks.cpp
iris/xmpp/cutestuff/bytestream.cpp
iris/xmpp/cutestuff/bsocket.cpp
iris/xmpp/cutestuff/httpconnect.cpp
iris/xmpp/xmpp-im/xmpp_discoitem.cpp
iris/xmpp/xmpp-im/client.cpp
iris/xmpp/xmpp-im/types.cpp
iris/xmpp/xmpp-im/xmpp_vcard.cpp
iris/xmpp/xmpp-im/xmpp_xmlcommon.cpp
iris/xmpp/xmpp-im/xmpp_ibb.cpp
iris/xmpp/xmpp-im/xmpp_xdata.cpp
iris/xmpp/xmpp-im/xmpp_task.cpp
iris/xmpp/xmpp-im/xmpp_features.cpp
iris/xmpp/xmpp-im/xmpp_discoinfotask.cpp
iris/xmpp/xmpp-im/s5b.cpp
iris/xmpp/xmpp-im/xmpp_tasks.cpp
iris/xmpp/xmpp-im/filetransfer.cpp
iris/xmpp/sasl/digestmd5proplist.cpp
# iris/xmpp/sasl/unittest/digestmd5responsetest.cpp
# iris/xmpp/sasl/unittest/plainmessagetest.cpp
iris/xmpp/sasl/digestmd5response.cpp
iris/xmpp/sasl/plainmessage.cpp
iris/xmpp/zlib/zlibcompressor.cpp
iris/xmpp/zlib/zlibdecompressor.cpp
# iris/xmpp/base64/unittest/base64test.cpp
iris/xmpp/base64/base64.cpp
iris/xmpp/xmpp-core/tlshandler.cpp
iris/xmpp/xmpp-core/xmpp_stanza.cpp
iris/xmpp/xmpp-core/stream.cpp
iris/xmpp/xmpp-core/securestream.cpp
iris/xmpp/xmpp-core/simplesasl.cpp
iris/xmpp/xmpp-core/xmlprotocol.cpp
iris/xmpp/xmpp-core/protocol.cpp
iris/xmpp/xmpp-core/compressionhandler.cpp
iris/xmpp/xmpp-core/parser.cpp
iris/xmpp/xmpp-core/connector.cpp
#iris/jdns/main.cpp
iris/jdns/qjdns.cpp
iris/jdns/qjdns_sock.cpp
iris/irisnet/noncore/ice176.cpp
iris/irisnet/noncore/stunallocate.cpp
iris/irisnet/noncore/legacy/ndns.cpp
iris/irisnet/noncore/legacy/srvresolver.cpp
iris/irisnet/noncore/legacy/safedelete.cpp
iris/irisnet/noncore/legacy/servsock.cpp
iris/irisnet/noncore/icelocaltransport.cpp
iris/irisnet/noncore/stunmessage.cpp
iris/irisnet/noncore/stunbinding.cpp
iris/irisnet/noncore/stuntransaction.cpp
iris/irisnet/noncore/processquit.cpp
iris/irisnet/corelib/netavailability.cpp
iris/irisnet/corelib/netnames_jdns.cpp
iris/irisnet/corelib/netnames.cpp
iris/irisnet/corelib/irisnetplugin.cpp
iris/irisnet/corelib/netinterface.cpp
iris/irisnet/corelib/jdnsshared.cpp
iris/irisnet/corelib/objectsession.cpp
iris/irisnet/corelib/irisnetglobal.cpp
#iris/irisnet/appledns/appledns.cpp
#iris/irisnet/appledns/qdnssd.cpp
#iris/irisnet/appledns/sdtest.cpp

# iris/libidn/profiles.c
# iris/libidn/rfc3454.c
# iris/libidn/stringprep.c
# iris/libidn/nfkc.c
iris/jdns/jdns.c
iris/jdns/jdns_util.c
iris/jdns/jdns_sys.c
iris/jdns/jdns_mdnsd.c
iris/jdns/jdns_packet.c


iris/xmpp/jingle/jingleaction.cpp
iris/xmpp/jingle/jinglecontent.cpp
iris/xmpp/jingle/jinglertpapplication.cpp
iris/xmpp/jingle/jinglesession.cpp
iris/xmpp/jingle/jingletasks.cpp
iris/xmpp/jingle/jingleapplication.cpp
iris/xmpp/jingle/jingleicetransport.cpp
#iris/xmpp/jingle/jingles5btransport.cpp (being developped)
iris/xmpp/jingle/jinglesessionmanager.cpp
iris/xmpp/jingle/jingletransport.cpp


)

if(WIN32)
  LIST(APPEND iris_SRCS iris/irisnet/corelib/netinterface_win.cpp)
else(WIN32)
  LIST(APPEND iris_SRCS iris/irisnet/corelib/netinterface_unix.cpp)
endif(WIN32)

set(libiris_SRCS 
    ${cutestuff_SRCS} 
    ${iris_SRCS} 
)

 kde4_moc_headers(iris_kopete


iris/xmpp/base/randrandomnumbergenerator.h
iris/xmpp/base/randomnumbergenerator.h
iris/xmpp/jid/jid.h
iris/xmpp/cutestuff/bytestream.h
iris/xmpp/cutestuff/socks.h
iris/xmpp/cutestuff/httpconnect.h
iris/xmpp/cutestuff/bsocket.h
iris/xmpp/cutestuff/httppoll.h
iris/xmpp/xmpp-im/xmpp_xdata.h
iris/xmpp/xmpp-im/xmpp_rosterx.h
iris/xmpp/xmpp-im/xmpp_tasks.h
iris/xmpp/xmpp-im/xmpp_discoitem.h
iris/xmpp/xmpp-im/xmpp_pubsubretraction.h
iris/xmpp/xmpp-im/xmpp_xmlcommon.h
iris/xmpp/xmpp-im/xmpp_roster.h
iris/xmpp/xmpp-im/xmpp_muc.h
iris/xmpp/xmpp-im/xmpp_htmlelement.h
iris/xmpp/xmpp-im/xmpp_receipts.h
iris/xmpp/xmpp-im/xmpp_message.h
iris/xmpp/xmpp-im/xmpp_discoinfotask.h
iris/xmpp/xmpp-im/xmpp_ibb.h
iris/xmpp/xmpp-im/xmpp_liverosteritem.h
iris/xmpp/xmpp-im/xmpp_httpauthrequest.h
iris/xmpp/xmpp-im/xmpp_pubsubitem.h
iris/xmpp/xmpp-im/xmpp_vcard.h
iris/xmpp/xmpp-im/xmpp_client.h
iris/xmpp/xmpp-im/s5b.h
iris/xmpp/xmpp-im/xmpp_url.h
iris/xmpp/xmpp-im/xmpp_chatstate.h
iris/xmpp/xmpp-im/xmpp_task.h
iris/xmpp/xmpp-im/xmpp_address.h
iris/xmpp/xmpp-im/xmpp_agentitem.h
iris/xmpp/xmpp-im/filetransfer.h
iris/xmpp/xmpp-im/xmpp_status.h
iris/xmpp/xmpp-im/xmpp_resourcelist.h
iris/xmpp/xmpp-im/xmpp_liveroster.h
iris/xmpp/xmpp-im/xmpp_resource.h
iris/xmpp/xmpp-im/xmpp_rosteritem.h
iris/xmpp/xmpp-im/xmpp_features.h
iris/xmpp/xmpp-im/im.h
iris/xmpp/sasl/digestmd5response.h
iris/xmpp/sasl/digestmd5proplist.h
iris/xmpp/sasl/plainmessage.h
iris/xmpp/zlib/common.h
iris/xmpp/zlib/zlibdecompressor.h
iris/xmpp/zlib/zlibcompressor.h
iris/xmpp/base64/base64.h
iris/xmpp/xmpp-core/xmpp_stanza.h
iris/xmpp/xmpp-core/simplesasl.h
iris/xmpp/xmpp-core/securestream.h
iris/xmpp/xmpp-core/protocol.h
iris/xmpp/xmpp-core/compressionhandler.h
iris/xmpp/xmpp-core/xmlprotocol.h
iris/xmpp/xmpp-core/xmpp.h
iris/xmpp/xmpp-core/xmpp_clientstream.h
iris/xmpp/xmpp-core/xmpp_stream.h
iris/xmpp/xmpp-core/td.h
iris/xmpp/xmpp-core/parser.h

iris/jdns/qjdns.h


iris/irisnet/noncore/stuntransaction.h
iris/irisnet/noncore/stunbinding.h
iris/irisnet/noncore/legacy/ndns.h
iris/irisnet/noncore/legacy/safedelete.h
iris/irisnet/noncore/legacy/servsock.h
iris/irisnet/noncore/legacy/srvresolver.h
iris/irisnet/noncore/stunallocate.h
iris/irisnet/noncore/ice176.h
iris/irisnet/noncore/processquit.h
iris/irisnet/noncore/icelocaltransport.h
iris/irisnet/corelib/irisnetglobal.h
iris/irisnet/corelib/objectsession.h
iris/irisnet/corelib/netavailability.h
iris/irisnet/corelib/netnames.h
iris/irisnet/corelib/irisnetplugin.h
iris/irisnet/corelib/netinterface.h
iris/irisnet/corelib/jdnsshared.h
#iris/irisnet/appledns/qdnssd.h

iris/xmpp/jingle/jingleaction.h
iris/xmpp/jingle/jinglecontent.h
iris/xmpp/jingle/jinglertpapplication.h
iris/xmpp/jingle/jinglesession.h
iris/xmpp/jingle/jingletasks.h
iris/xmpp/jingle/jingleapplication.h
iris/xmpp/jingle/jingleicetransport.h
#iris/xmpp/jingle/jingles5btransport.h (being developped)
iris/xmpp/jingle/jinglesessionmanager.h
iris/xmpp/jingle/jingletransport.h


 )

#set(testapp_SRCS
#xmpptest/xmpptest.cpp
#)

#qt4_wrap_ui(testapp_SRCS
#xmpptest/ui_test.ui
#)

#kde4_moc_headers(testapp
#xmpptest/xmpptest.cpp
#)

add_definitions(-DIRISNET_STATIC)

kde4_add_library(iris_kopete STATIC ${libiris_SRCS})

#kde4_add_executable(testapp ${testapp_SRCS})

set(iris_kopete_libs ${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY} ${QT_QT3SUPPORT_LIBRARY} ${QT_QTNETWORK_LIBRARY} ${QT_QTXML_LIBRARY}
                     ${IDN_LIBRARIES} ${QCA2_LIBRARIES} ${ZLIB_LIBRARIES} )

if(WIN32)
  set(iris_kopete_libs ${iris_kopete_libs} ws2_32)
endif(WIN32)

if(HAVE_NSL_LIBRARY)
  # This is probably Solaris, and libiris needs to link
  # to libnsl for gethostbyname
  set(iris_kopete_libs ${iris_kopete_libs} nsl)
endif(HAVE_NSL_LIBRARY)
target_link_libraries(iris_kopete ${iris_kopete_libs})

#set(iris_testapp_libs ${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY} ${QT_QT3SUPPORT_LIBRARY} ${QT_QTNETWORK_LIBRARY} ${QT_QTXML_LIBRARY}
#                     ${IDN_LIBRARIES} ${QCA2_LIBRARIES} ${ZLIB_LIBRARIES}
#		     iris_kopete)

#target_link_libraries(testapp ${iris_testapp_libs})

#set_target_properties(iris_kopete PROPERTIES VERSION 1.0.0 SOVERSION 1 )
#install(TARGETS iris_kopete ${INSTALL_TARGETS_DEFAULT_ARGS})

########### install files ###############

