
if(NOT WIN32)
  add_definitions(-fPIC)
else(NOT WIN32)
  if(MINGW)
    add_definitions(-DWIN32)
  endif(MINGW)
endif(NOT WIN32)

include_directories(
${CMAKE_CURRENT_SOURCE_DIR}/src
${CMAKE_CURRENT_SOURCE_DIR}/src/xmpp/base
${CMAKE_CURRENT_SOURCE_DIR}/src/xmpp/base
${CMAKE_CURRENT_SOURCE_DIR}/src/xmpp/jid
${CMAKE_CURRENT_SOURCE_DIR}/src/irisnet/noncore/cutestuff
${CMAKE_CURRENT_SOURCE_DIR}/src/xmpp/xmpp-im
${CMAKE_CURRENT_SOURCE_DIR}/src/xmpp/sasl
${CMAKE_CURRENT_SOURCE_DIR}/src/xmpp/zlib
${CMAKE_CURRENT_SOURCE_DIR}/src/xmpp/base64
${CMAKE_CURRENT_SOURCE_DIR}/src/xmpp/xmpp-core
${CMAKE_CURRENT_SOURCE_DIR}/src/xmpp/jingle
${CMAKE_CURRENT_SOURCE_DIR}/src/jdns/include/jdns
${CMAKE_CURRENT_SOURCE_DIR}/src/irisnet/noncore
${CMAKE_CURRENT_SOURCE_DIR}/src/irisnet/noncore/legacy
${CMAKE_CURRENT_SOURCE_DIR}/src/irisnet/corelib
${CMAKE_CURRENT_SOURCE_DIR}/src/irisnet/appledns

${QCA2_INCLUDE_DIR}
${IDN_INCLUDE_DIR}
${ZLIB_INCLUDE_DIR}
${DNSSD_INCLUDE_DIR}
)

set(iris_SRCS
src/xmpp/base/randomnumbergenerator.cpp
src/xmpp/jid/jid.cpp
src/irisnet/noncore/cutestuff/httppoll.cpp
src/irisnet/noncore/cutestuff/socks.cpp
src/irisnet/noncore/cutestuff/bytestream.cpp
src/irisnet/noncore/cutestuff/bsocket.cpp
src/irisnet/noncore/cutestuff/httpconnect.cpp
src/xmpp/xmpp-im/xmpp_discoitem.cpp
src/xmpp/xmpp-im/client.cpp
src/xmpp/xmpp-im/types.cpp
src/xmpp/xmpp-im/xmpp_vcard.cpp
src/xmpp/xmpp-im/xmpp_xmlcommon.cpp
src/xmpp/xmpp-im/xmpp_ibb.cpp
src/xmpp/xmpp-im/xmpp_xdata.cpp
src/xmpp/xmpp-im/xmpp_task.cpp
src/xmpp/xmpp-im/xmpp_features.cpp
src/xmpp/xmpp-im/xmpp_discoinfotask.cpp
src/xmpp/xmpp-im/xmpp_bitsofbinary.cpp
src/xmpp/xmpp-im/xmpp_bytestream.cpp
src/xmpp/xmpp-im/s5b.cpp
src/xmpp/xmpp-im/xmpp_tasks.cpp
src/xmpp/xmpp-im/filetransfer.cpp
src/xmpp/sasl/digestmd5proplist.cpp
src/xmpp/sasl/digestmd5response.cpp
src/xmpp/sasl/plainmessage.cpp
src/xmpp/sasl/scramsha1message.cpp
src/xmpp/sasl/scramsha1signature.cpp
src/xmpp/sasl/scramsha1response.cpp
src/xmpp/zlib/zlibcompressor.cpp
src/xmpp/zlib/zlibdecompressor.cpp
src/xmpp/base64/base64.cpp
src/xmpp/xmpp-core/tlshandler.cpp
src/xmpp/xmpp-core/xmpp_stanza.cpp
src/xmpp/xmpp-core/stream.cpp
src/xmpp/xmpp-core/securestream.cpp
src/xmpp/xmpp-core/simplesasl.cpp
src/xmpp/xmpp-core/xmlprotocol.cpp
src/xmpp/xmpp-core/protocol.cpp
src/xmpp/xmpp-core/compressionhandler.cpp
src/xmpp/xmpp-core/parser.cpp
src/xmpp/xmpp-core/connector.cpp
src/irisnet/noncore/ice176.cpp
src/irisnet/noncore/icecomponent.cpp
src/irisnet/noncore/icetransport.cpp
src/irisnet/noncore/iceturntransport.cpp
src/irisnet/noncore/stunallocate.cpp
src/irisnet/noncore/legacy/ndns.cpp
src/irisnet/noncore/legacy/srvresolver.cpp
src/irisnet/noncore/legacy/safedelete.cpp
src/irisnet/noncore/legacy/servsock.cpp
src/irisnet/noncore/icelocaltransport.cpp
src/irisnet/noncore/stunmessage.cpp
src/irisnet/noncore/stunbinding.cpp
src/irisnet/noncore/stuntransaction.cpp
src/irisnet/noncore/stuntypes.cpp
src/irisnet/noncore/stunutil.cpp
src/irisnet/noncore/processquit.cpp
src/irisnet/noncore/turnclient.cpp
src/irisnet/noncore/udpportreserver.cpp
src/irisnet/corelib/netavailability.cpp
src/irisnet/corelib/netnames_jdns.cpp
src/irisnet/corelib/netnames.cpp
src/irisnet/corelib/irisnetplugin.cpp
src/irisnet/corelib/netinterface.cpp
src/irisnet/corelib/objectsession.cpp
src/irisnet/corelib/irisnetglobal.cpp

src/jdns/src/jdns/jdns_util.c
src/jdns/src/jdns/jdns_packet.c
src/jdns/src/jdns/jdns_mdnsd.c
src/jdns/src/jdns/jdns_sys.c
src/jdns/src/jdns/jdns.c
src/jdns/src/qjdns/qjdns_sock.cpp
src/jdns/src/qjdns/qjdns.cpp
src/jdns/src/qjdns/qjdnsshared.cpp

)

if(WIN32)
  LIST(APPEND iris_SRCS src/irisnet/corelib/netinterface_win.cpp)
else(WIN32)
  LIST(APPEND iris_SRCS src/irisnet/corelib/netinterface_unix.cpp)
endif(WIN32)

set(libiris_SRCS 
    ${cutestuff_SRCS} 
    ${iris_SRCS} 
)

 kde4_moc_headers(iris_kopete


src/xmpp/base/randrandomnumbergenerator.h
src/xmpp/base/randomnumbergenerator.h
src/xmpp/jid/jid.h
src/irisnet/noncore/cutestuff/bytestream.h
src/irisnet/noncore/cutestuff/socks.h
src/irisnet/noncore/cutestuff/httpconnect.h
src/irisnet/noncore/cutestuff/bsocket.h
src/irisnet/noncore/cutestuff/httppoll.h
src/irisnet/noncore/iceturntransport.h
src/irisnet/noncore/icecomponent.h
src/irisnet/noncore/icetransport.h
src/irisnet/noncore/turnclient.h
src/irisnet/noncore/udpportreserver.h
src/xmpp/xmpp-im/xmpp_xdata.h
src/xmpp/xmpp-im/xmpp_rosterx.h
src/xmpp/xmpp-im/xmpp_tasks.h
src/xmpp/xmpp-im/xmpp_discoitem.h
src/xmpp/xmpp-im/xmpp_pubsubretraction.h
src/xmpp/xmpp-im/xmpp_xmlcommon.h
src/xmpp/xmpp-im/xmpp_roster.h
src/xmpp/xmpp-im/xmpp_muc.h
src/xmpp/xmpp-im/xmpp_htmlelement.h
src/xmpp/xmpp-im/xmpp_receipts.h
src/xmpp/xmpp-im/xmpp_message.h
src/xmpp/xmpp-im/xmpp_discoinfotask.h
src/xmpp/xmpp-im/xmpp_ibb.h
src/xmpp/xmpp-im/xmpp_liverosteritem.h
src/xmpp/xmpp-im/xmpp_httpauthrequest.h
src/xmpp/xmpp-im/xmpp_pubsubitem.h
src/xmpp/xmpp-im/xmpp_vcard.h
src/xmpp/xmpp-im/xmpp_client.h
src/xmpp/xmpp-im/s5b.h
src/xmpp/xmpp-im/xmpp_url.h
src/xmpp/xmpp-im/xmpp_chatstate.h
src/xmpp/xmpp-im/xmpp_task.h
src/xmpp/xmpp-im/xmpp_address.h
src/xmpp/xmpp-im/xmpp_agentitem.h
src/xmpp/xmpp-im/filetransfer.h
src/xmpp/xmpp-im/xmpp_status.h
src/xmpp/xmpp-im/xmpp_resourcelist.h
src/xmpp/xmpp-im/xmpp_liveroster.h
src/xmpp/xmpp-im/xmpp_resource.h
src/xmpp/xmpp-im/xmpp_rosteritem.h
src/xmpp/xmpp-im/xmpp_features.h
src/xmpp/xmpp-im/im.h
src/xmpp/sasl/digestmd5response.h
src/xmpp/sasl/digestmd5proplist.h
src/xmpp/sasl/plainmessage.h
src/xmpp/sasl/scramsha1response.h
src/xmpp/zlib/common.h
src/xmpp/zlib/zlibdecompressor.h
src/xmpp/zlib/zlibcompressor.h
src/xmpp/base64/base64.h
src/xmpp/xmpp-core/xmpp_stanza.h
src/xmpp/xmpp-core/simplesasl.h
src/xmpp/xmpp-core/securestream.h
src/xmpp/xmpp-core/protocol.h
src/xmpp/xmpp-core/compressionhandler.h
src/xmpp/xmpp-core/xmlprotocol.h
src/xmpp/xmpp-core/xmpp.h
src/xmpp/xmpp-core/xmpp_clientstream.h
src/xmpp/xmpp-core/xmpp_stream.h
src/xmpp/xmpp-core/td.h
src/xmpp/xmpp-core/parser.h

src/irisnet/noncore/stuntransaction.h
src/irisnet/noncore/stunbinding.h
src/irisnet/noncore/legacy/ndns.h
src/irisnet/noncore/legacy/safedelete.h
src/irisnet/noncore/legacy/servsock.h
src/irisnet/noncore/legacy/srvresolver.h
src/irisnet/noncore/stunallocate.h
src/irisnet/noncore/ice176.h
src/irisnet/noncore/processquit.h
src/irisnet/noncore/icelocaltransport.h
src/irisnet/corelib/irisnetglobal.h
src/irisnet/corelib/objectsession.h
src/irisnet/corelib/netavailability.h
src/irisnet/corelib/netnames.h
src/irisnet/corelib/irisnetplugin.h
src/irisnet/corelib/netinterface.h

src/jdns/src/jdns/jdns_packet.h
src/jdns/src/jdns/jdns_mdnsd.h
src/jdns/src/jdns/jdns_p.h
src/jdns/include/jdns/jdns.h
src/jdns/src/qjdns/qjdns_sock.h
src/jdns/src/qjdns/qjdns_p.h
src/jdns/src/qjdns/qjdnsshared_p.h
src/jdns/include/jdns/qjdns.h
src/jdns/include/jdns/qjdnsshared.h
src/jdns/include/jdns/jdns_export.h

)

add_definitions(-DIRISNET_STATIC)
add_definitions(-DJDNS_STATIC)

if(CMAKE_COMPILER_IS_GNUCXX)
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=gnu90")
endif(CMAKE_COMPILER_IS_GNUCXX)

kde4_add_library(iris_kopete STATIC ${libiris_SRCS})

set(iris_kopete_libs ${QT_QTCORE_LIBRARY} ${QT_QTNETWORK_LIBRARY} ${QT_QTXML_LIBRARY}
                     ${IDN_LIBRARIES} ${QCA2_LIBRARIES} ${ZLIB_LIBRARIES} )

if(WIN32)
  set(iris_kopete_libs ${iris_kopete_libs} ws2_32)
endif(WIN32)

# On Solaris, some of the name resolution functions are in libnsl;
# this needs to be linked in if found. Copied from kdelibs.
include(CheckLibraryExists)
check_library_exists(nsl gethostbyname "" HAVE_NSL_LIBRARY)

if(HAVE_NSL_LIBRARY)
  # This is probably Solaris, and libiris needs to link
  # to libnsl for gethostbyname
  set(iris_kopete_libs ${iris_kopete_libs} nsl)
endif(HAVE_NSL_LIBRARY)
target_link_libraries(iris_kopete ${iris_kopete_libs})

########### install files ###############

