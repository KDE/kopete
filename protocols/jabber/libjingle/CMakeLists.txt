# Copyright (c) 2009-2013 Pali Rohár <pali.rohar@gmail.com>

########### next target ###############

add_definitions (
	-DFEATURE_ENABLE_SSL
	-DFEATURE_ENABLE_VOICEMAIL
	-DFEATURE_ENABLE_PSTN
	-DHAVE_SRTP
	-DHAVE_LINPHONE
	-DSRTP_RELATIVE_PATH
	-DEXPAT_RELATIVE_PATH
	-DGTEST_RELATIVE_PATH
	-DJSONCPP_RELATIVE_PATH
)

if ( NOT WIN32 )
	add_definitions (
		-DHASHNAMESPACE=__gnu_cxx
		-DHASH_NAMESPACE=__gnu_cxx
		-DPOSIX
		-DDISABLE_DYNAMIC_CAST
		-DHAVE_OPENSSL_SSL_H
		-D_REENTRANT
	)
	if ( NOT APPLE )
		add_definitions ( -DLINUX )
	endif ( NOT APPLE )
endif ( NOT WIN32 )

if ( MEDIASTREAMER_OLD )
	add_definitions ( -DMEDIASTREAMER_OLD )
endif ( MEDIASTREAMER_OLD )

if ( CMAKE_BUILD_TYPE STREQUAL "Debug" )
	add_definitions ( -D_DEBUG )
endif ( CMAKE_BUILD_TYPE STREQUAL "Debug" )

set ( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-undef -Wno-unused-parameter" )
set ( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-undef -Wno-unused-parameter -Wno-non-virtual-dtor -Wno-delete-non-virtual-dtor -std=gnu++98" )

########### next target ###############

include_directories (
	${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}/talk/third_party/libudev
	${EXPAT_INCLUDE_DIR}
	${MEDIASTREAMER_INCLUDE_DIR}
	${LIBORTP_INCLUDE_DIR}
	${SRTP_INCLUDE_DIR}
	${JSONCPP_INCLUDE_DIR}
	${ALSA_INCLUDES}
)

if ( NOT WIN32 )
	include_directories (
		${OPENSSL_INCLUDE_DIR}
	)
endif ( NOT WIN32 )

########### next target ###############

set ( call_SRCS
	talk/base/latebindingsymboltable.cc
	talk/base/linux.cc
	talk/base/linuxfdwalk.cc
	talk/base/libdbusglibsymboltable.cc
	talk/session/phone/libudevsymboltable.cc
	talk/session/phone/linuxdevicemanager.cc
	talk/session/phone/v4llookup.cc
	talk/sound/alsasoundsystem.cc
	talk/sound/alsasymboltable.cc
	talk/sound/linuxsoundsystem.cc
	talk/sound/pulseaudiosoundsystem.cc
	talk/sound/pulseaudiosymboltable.cc
	talk/base/unixfilesystem.cc
	talk/base/openssladapter.cc
	talk/base/openssldigest.cc
	talk/base/opensslidentity.cc
	talk/base/opensslstreamadapter.cc
	talk/base/posix.cc
	talk/base/sslidentity.cc
	talk/base/sslstreamadapter.cc
	talk/session/phone/linphonemediaengine.cc
	talk/base/asyncfile.cc
	talk/base/asynchttprequest.cc
	talk/base/asyncsocket.cc
	talk/base/asynctcpsocket.cc
	talk/base/asyncudpsocket.cc
	talk/base/autodetectproxy.cc
	talk/base/bandwidthsmoother.cc
	talk/base/base64.cc
	talk/base/basicpacketsocketfactory.cc
	talk/base/bytebuffer.cc
	talk/base/checks.cc
	talk/base/common.cc
	talk/base/cpuid.cc
	talk/base/cpumonitor.cc
	talk/base/diskcache.cc
	talk/base/event.cc
	talk/base/filelock.cc
	talk/base/fileutils.cc
	talk/base/firewallsocketserver.cc
	talk/base/flags.cc
	talk/base/helpers.cc
	talk/base/host.cc
	talk/base/httpbase.cc
	talk/base/httpclient.cc
	talk/base/httpcommon.cc
	talk/base/httprequest.cc
	talk/base/httpserver.cc
	talk/base/ipaddress.cc
	talk/base/logging.cc
	talk/base/md5c.c
	talk/base/messagedigest.cc
	talk/base/messagehandler.cc
	talk/base/messagequeue.cc
	talk/base/multipart.cc
	talk/base/natserver.cc
	talk/base/natsocketfactory.cc
	talk/base/nattypes.cc
	talk/base/nethelpers.cc
	talk/base/network.cc
	talk/base/optionsfile.cc
	talk/base/pathutils.cc
	talk/base/physicalsocketserver.cc
	talk/base/proxydetect.cc
	talk/base/proxyinfo.cc
	talk/base/proxyserver.cc
	talk/base/ratelimiter.cc
	talk/base/ratetracker.cc
	talk/base/sha1.c
	talk/base/sharedexclusivelock.cc
	talk/base/signalthread.cc
	talk/base/socketadapters.cc
	talk/base/socketaddress.cc
	talk/base/socketaddresspair.cc
	talk/base/socketpool.cc
	talk/base/socketstream.cc
	talk/base/ssladapter.cc
	talk/base/sslsocketfactory.cc
	talk/base/stream.cc
	talk/base/stringencode.cc
	talk/base/stringutils.cc
	talk/base/systeminfo.cc
	talk/base/task.cc
	talk/base/taskparent.cc
	talk/base/taskrunner.cc
	talk/base/testclient.cc
	talk/base/thread.cc
	talk/base/timeutils.cc
	talk/base/timing.cc
	talk/base/transformadapter.cc
	talk/base/urlencode.cc
	talk/base/versionparsing.cc
	talk/base/virtualsocketserver.cc
	talk/base/worker.cc
	talk/p2p/base/constants.cc
	talk/p2p/base/p2ptransport.cc
	talk/p2p/base/p2ptransportchannel.cc
	talk/p2p/base/parsing.cc
	talk/p2p/base/port.cc
	talk/p2p/base/portallocator.cc
	talk/p2p/base/portallocatorsessionproxy.cc
	talk/p2p/base/portproxy.cc
	talk/p2p/base/pseudotcp.cc
	talk/p2p/base/relayport.cc
	talk/p2p/base/relayserver.cc
	talk/p2p/base/rawtransport.cc
	talk/p2p/base/rawtransportchannel.cc
	talk/p2p/base/session.cc
	talk/p2p/base/sessiondescription.cc
	talk/p2p/base/sessionmanager.cc
	talk/p2p/base/sessionmessages.cc
	talk/p2p/base/stun.cc
	talk/p2p/base/stunport.cc
	talk/p2p/base/stunrequest.cc
	talk/p2p/base/stunserver.cc
	talk/p2p/base/tcpport.cc
	talk/p2p/base/transport.cc
	talk/p2p/base/transportchannel.cc
	talk/p2p/base/transportchannelproxy.cc
	talk/p2p/base/udpport.cc
	talk/p2p/client/basicportallocator.cc
	talk/p2p/client/connectivitychecker.cc
	talk/p2p/client/httpportallocator.cc
	talk/p2p/client/socketmonitor.cc
	talk/session/tunnel/pseudotcpchannel.cc
	talk/session/tunnel/tunnelsessionclient.cc
	talk/session/tunnel/securetunnelsessionclient.cc
	talk/session/phone/audiomonitor.cc
	talk/session/phone/call.cc
	talk/session/phone/channel.cc
	talk/session/phone/channelmanager.cc
	talk/session/phone/codec.cc
	talk/session/phone/constants.cc
	talk/session/phone/currentspeakermonitor.cc
	talk/session/phone/dataengine.cc
	talk/session/phone/devicemanager.cc
	talk/session/phone/filemediaengine.cc
	talk/session/phone/filevideocapturer.cc
	talk/session/phone/mediaengine.cc
	talk/session/phone/mediamessages.cc
	talk/session/phone/mediamonitor.cc
	talk/session/phone/mediarecorder.cc
	talk/session/phone/mediasession.cc
	talk/session/phone/mediasessionclient.cc
	talk/session/phone/rtpdump.cc
	talk/session/phone/rtputils.cc
	talk/session/phone/rtcpmuxfilter.cc
	talk/session/phone/soundclip.cc
	talk/session/phone/srtpfilter.cc
	talk/session/phone/ssrcmuxfilter.cc
	talk/session/phone/streamparams.cc
	talk/session/phone/videoadapter.cc
	talk/session/phone/videocapturer.cc
	talk/session/phone/videocommon.cc
	talk/session/phone/videoframe.cc
	talk/sound/nullsoundsystem.cc
	talk/sound/nullsoundsystemfactory.cc
	talk/sound/platformsoundsystem.cc
	talk/sound/platformsoundsystemfactory.cc
	talk/sound/soundsysteminterface.cc
	talk/sound/soundsystemproxy.cc
	talk/xmllite/qname.cc
	talk/xmllite/xmlbuilder.cc
	talk/xmllite/xmlconstants.cc
	talk/xmllite/xmlelement.cc
	talk/xmllite/xmlnsstack.cc
	talk/xmllite/xmlparser.cc
	talk/xmllite/xmlprinter.cc
	talk/xmpp/constants.cc
	talk/xmpp/hangoutpubsubclient.cc
	talk/xmpp/iqtask.cc
	talk/xmpp/jid.cc
	talk/xmpp/moduleimpl.cc
	talk/xmpp/mucroomconfigtask.cc
	talk/xmpp/mucroomhistorytask.cc
	talk/xmpp/mucroomlookuptask.cc
	talk/xmpp/pubsubclient.cc
	talk/xmpp/pubsub_task.cc
	talk/xmpp/pubsubtasks.cc
	talk/xmpp/receivetask.cc
	talk/xmpp/saslmechanism.cc
	talk/xmpp/xmppclient.cc
	talk/xmpp/xmppengineimpl.cc
	talk/xmpp/xmppengineimpl_iq.cc
	talk/xmpp/xmpplogintask.cc
	talk/xmpp/xmppstanzaparser.cc
	talk/xmpp/xmpptask.cc
	talk/examples/call/call_main.cc
	talk/examples/call/callclient.cc
	talk/examples/call/console.cc
	talk/examples/call/friendinvitesendtask.cc
	talk/examples/call/mediaenginefactory.cc
	talk/examples/call/mucinviterecvtask.cc
	talk/examples/call/mucinvitesendtask.cc
	talk/examples/call/presenceouttask.cc
	talk/examples/call/presencepushtask.cc
	talk/examples/login/jingleinfotask.cc
	talk/examples/login/xmppauth.cc
	talk/examples/login/xmpppump.cc
	talk/examples/login/xmppsocket.cc
)

set ( call_LIBS
	${SRTP_LIBRARY}
	${JSONCPP_LIBRARY}
	${EXPAT_LIBRARY}
	${MEDIASTREAMER_LIBRARIES}
	${LIBORTP_LIBRARY}
	${CMAKE_DL_LIBS}
)

if ( NOT WIN32 )
	#timeutils calls CLOCK_MONOLITHIC stuff, so we need the rt library
	FIND_LIBRARY(RT_LIBRARY NAMES rt)
	set ( call_LIBS ${call_LIBS}
		${OPENSSL_LIBRARIES}
		${RT_LIBRARY}
	)
endif ( NOT WIN32 )

add_executable ( call ${call_SRCS} )
set_target_properties ( call PROPERTIES OUTPUT_NAME libjingle-call )
target_link_libraries ( call ${call_LIBS} )

########### install files ###############

install ( TARGETS call DESTINATION bin )
