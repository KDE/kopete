
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

${IDN_INCLUDE_DIR}
${ZLIB_INCLUDE_DIR}
${DNSSD_INCLUDE_DIR}
)

set(iris_SRCS
src/xmpp/base/randomnumbergenerator.cpp
src/xmpp/base/timezone.cpp
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
src/xmpp/xmpp-im/xmpp_caps.cpp
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
src/xmpp/xmpp-core/tlshandler.cpp
src/xmpp/xmpp-core/xmpp_stanza.cpp
src/xmpp/xmpp-core/stream.cpp
src/xmpp/xmpp-core/securestream.cpp
src/xmpp/xmpp-core/simplesasl.cpp
src/xmpp/xmpp-core/xmlprotocol.cpp
src/xmpp/xmpp-core/protocol.cpp
src/xmpp/xmpp-core/sm.cpp
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

add_definitions(-DIRISNET_STATIC)
add_definitions(-DJDNS_STATIC)

if(CMAKE_COMPILER_IS_GNUCXX)
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=gnu90")
endif(CMAKE_COMPILER_IS_GNUCXX)

add_library(iris_kopete STATIC ${libiris_SRCS})

set(iris_kopete_libs Qt5::Core Qt5::Network Qt5::Xml
                     ${IDN_LIBRARIES} qca-qt5 ${ZLIB_LIBRARIES} )

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

