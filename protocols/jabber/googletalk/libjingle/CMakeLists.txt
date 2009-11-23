# Copyright (c) 2009, Pali Rohár <pali.rohar@gmail.com>
# autotools files was converted to cmake build system using am2cmake

########### next target ###############

set ( CMAKE_INCLUDE_CURRENT_DIR OFF )
add_definitions ( -DHAVE_CONFIG_H )

#macro_bool_to_01 ( ILBC_FOUND HAVE_ILBC )
#macro_bool_to_01 ( ILBC_FOUND HAVE_ILBC_DECODE_H ) # Fixme: need this?
#macro_bool_to_01 ( LIBORTP_FOUND HAVE_ORTP )
macro_bool_to_01 ( SPEEX_FOUND HAVE_SPEEX )
#macro_bool_to_01 ( SPEEX_FOUND HAVE_SPEEX_SPEEX_H ) # Fixme: need this?

#New mediastreamer doesnt have native iLBC support
set(HAVE_ILBC 0)

if ( NOT WIN32 )
	set ( POSIX 1 )
	add_definitions ( -DPOSIX )
	if ( NOT APPLE )
		#add_definitions ( -DLINUX ) #Fixme: need this?
		set ( WIN32 0 )
		set ( LINUX 1 )
		set ( OSX 0 )
	else ( NOT APPLE )
		set ( WIN32 0 )
		set ( LINUX 0 )
		set ( OSX 1 )
	endif ( NOT APPLE )
else ( NOT WIN32 )
	set ( POSIX 0 )
	set ( WIN32 1 )
	set ( LINUX 0 )
	set ( OSX 0 )
endif ( NOT WIN32 )

configure_file ( ${CMAKE_CURRENT_SOURCE_DIR}/config.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config.h )

########### next target ###############

if ( NOT EXPAT_FOUND AND NOT GLIB2_FOUND AND ( NOT SSL_FOUND OR NOT WIN32 ) )
	message ( FATAL "Expat or SSl not found!" )
endif ( NOT EXPAT_FOUND AND NOT GLIB2_FOUND AND ( NOT SSL_FOUND OR NOT WIN32 ) )

if ( BUILD_libjingle_call )
	if ( LIBORTP_FOUND AND MEDIASTREAMER_FOUND )
		message ( STATUS "Building Google Talk libjingle voice call support, supported codecs:" )
		if ( SPEEX_FOUND )
			message ( STATUS " Speex: yes" )
		else ( SPEEX_FOUND )
			message ( STATUS " Speex: no" )
		endif ( SPEEX_FOUND )
#		if ( ILBC_FOUND )
#			message ( STATUS " iLBC: yes" )
#		else ( ILBC_FOUND )
#			message ( STATUS " iLBC: no" )
#		endif ( ILBC_FOUND )
		message ( STATUS " MULAW: yes" )
	else ( LIBORTP_FOUND AND MEDIASTREAMER_FOUND )
		set ( BUILD_libjingle_call false )
		message ( STATUS "Not building Google Talk libjingle voice call support, because oRTP or Mediastreamer not found!" )
	endif ( LIBORTP_FOUND AND MEDIASTREAMER_FOUND )
else ( BUILD_libjingle_call )
	message ( STATUS "Not building Google Talk libjingle voice call support" )
endif ( BUILD_libjingle_call )

if ( BUILD_libjingle_pcp )
	message ( STATUS "Building Google Talk libjingle filetransfer support" )
else ( BUILD_libjingle_pcp )
	message ( STATUS "Not building Google Talk libjingle filetransfer support" )
endif ( BUILD_libjingle_pcp )

########### next target ###############

if ( BUILD_libjingle_natserver OR BUILD_libjingle_nat_unittest OR BUILD_libjingle_virtualsocket_unittest OR BUILD_libjingle_relayserver OR BUILD_libjingle_stunserver OR BUILD_libjingle_stunserver_unittest OR BUILD_libjingle_session_unittest OR BUILD_libjingle_port_unittest OR BUILD_libjingle_login OR BUILD_libjingle_pcp OR BUILD_libjingle_call )
	set ( BUILD_libjingle_cricketbase true )
else ( BUILD_libjingle_natserver OR BUILD_libjingle_nat_unittest OR BUILD_libjingle_virtualsocket_unittest OR BUILD_libjingle_relayserver OR BUILD_libjingle_stunserver OR BUILD_libjingle_stunserver_unittest OR BUILD_libjingle_session_unittest OR BUILD_libjingle_port_unittest OR BUILD_libjingle_login OR BUILD_libjingle_pcp OR BUILD_libjingle_call )
	set ( BUILD_libjingle_cricketbase false )
endif ( BUILD_libjingle_natserver OR BUILD_libjingle_nat_unittest OR BUILD_libjingle_virtualsocket_unittest OR BUILD_libjingle_relayserver OR BUILD_libjingle_stunserver OR BUILD_libjingle_stunserver_unittest OR BUILD_libjingle_session_unittest OR BUILD_libjingle_port_unittest OR BUILD_libjingle_login OR BUILD_libjingle_pcp OR BUILD_libjingle_call )

if ( BUILD_libjingle_natserver OR BUILD_libjingle_nat_unittest OR BUILD_libjingle_virtualsocket_unittest OR BUILD_libjingle_stunserver_unittest OR BUILD_libjingle_session_unittest OR BUILD_libjingle_port_unittest )
	set ( BUILD_libjingle_crickettest true )
else ( BUILD_libjingle_natserver OR BUILD_libjingle_nat_unittest OR BUILD_libjingle_virtualsocket_unittest OR BUILD_libjingle_stunserver_unittest OR BUILD_libjingle_session_unittest OR BUILD_libjingle_port_unittest )
	set ( BUILD_libjingle_crickettest false )
endif ( BUILD_libjingle_natserver OR BUILD_libjingle_nat_unittest OR BUILD_libjingle_virtualsocket_unittest OR BUILD_libjingle_stunserver_unittest OR BUILD_libjingle_session_unittest OR BUILD_libjingle_port_unittest )

if ( BUILD_libjingle_relayserver OR BUILD_libjingle_stunserver OR BUILD_libjingle_stunserver_unittest OR BUILD_libjingle_session_unittest OR BUILD_libjingle_port_unittest OR BUILD_libjingle_pcp OR BUILD_libjingle_call )
	set ( BUILD_libjingle_cricketp2pbase true )
else ( BUILD_libjingle_relayserver OR BUILD_libjingle_stunserver OR BUILD_libjingle_stunserver_unittest OR BUILD_libjingle_session_unittest OR BUILD_libjingle_port_unittest OR BUILD_libjingle_pcp OR BUILD_libjingle_call )
	set ( BUILD_libjingle_cricketp2pbase false )
endif ( BUILD_libjingle_relayserver OR BUILD_libjingle_stunserver OR BUILD_libjingle_stunserver_unittest OR BUILD_libjingle_session_unittest OR BUILD_libjingle_port_unittest OR BUILD_libjingle_pcp OR BUILD_libjingle_call )

if ( BUILD_libjingle_session_unittest OR BUILD_libjingle_login OR BUILD_libjingle_pcp OR BUILD_libjingle_call )
	set ( BUILD_libjingle_cricketxmllite true )
	set ( BUILD_libjingle_cricketxmpp true )
else ( BUILD_libjingle_session_unittest OR BUILD_libjingle_login OR BUILD_libjingle_pcp OR BUILD_libjingle_call )
	set ( BUILD_libjingle_cricketxmllite false )
	set ( BUILD_libjingle_cricketxmpp false )
endif ( BUILD_libjingle_session_unittest OR BUILD_libjingle_login OR BUILD_libjingle_pcp OR BUILD_libjingle_call )

if ( BUILD_libjingle_pcp OR BUILD_libjingle_call )
	set ( BUILD_libjingle_cricketp2pclient true )
	set ( BUILD_libjingle_cricketexampleslogin true )
else ( BUILD_libjingle_pcp OR BUILD_libjingle_call )
	set ( BUILD_libjingle_cricketp2pclient false )
	set ( BUILD_libjingle_cricketexampleslogin false )
endif ( BUILD_libjingle_pcp OR BUILD_libjingle_call )

if ( BUILD_libjingle_call )
	set ( BUILD_libjingle_cricketsessionphone true )
else ( BUILD_libjingle_call )
	set ( BUILD_libjingle_cricketsessionphone false )
endif ( BUILD_libjingle_call )

if ( BUILD_libjingle_pcp )
	set ( BUILD_libjingle_cricketsessiontunnel true )
	set ( BUILD_libjingle_cricketsessionfileshare true )
else ( BUILD_libjingle_pcp )
	set ( BUILD_libjingle_cricketsessiontunnel false )
	set ( BUILD_libjingle_cricketsessionfileshare false )
endif ( BUILD_libjingle_pcp )

########### next target ###############

include_directories (
   ${CMAKE_CURRENT_SOURCE_DIR}
   ${CMAKE_CURRENT_BINARY_DIR}
)

add_subdirectory(talk)


########### install files ###############

