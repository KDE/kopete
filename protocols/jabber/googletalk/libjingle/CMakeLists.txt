# Copyright (c) 2009-2010 Pali Rohár <pali.rohar@gmail.com>
# autotools files was converted to cmake build system using am2cmake

########### next target ###############

find_package ( LiboRTP )
find_package ( Mediastreamer )
find_package ( Speex )
find_package ( Expat )

if ( NOT WIN32 )
	find_package ( SSL )
endif ( NOT WIN32 )

if ( MEDIASTREAMER_FOUND )
        set ( HAVE_MEDIASTREAMER 1 )
else (MEDIAST REAMER_FOUND)
        set ( HAVE_MEDIASTREAMER 0 )
endif ( MEDIASTREAMER_FOUND )

if ( SPEEX_FOUND )
	set ( HAVE_SPEEX 1 )
else ( SPEEX_FOUND )
	set ( HAVE_SPEEX 0 )
endif ( SPEEX_FOUND )

#New mediastreamer doesnt have native iLBC support
set ( HAVE_ILBC 0 )

########### next target ###############

if ( NOT DEFINED BUILD_libjingle_natserver )
	set ( BUILD_libjingle_natserver true )
endif ( NOT DEFINED BUILD_libjingle_natserver )

if ( NOT DEFINED BUILD_libjingle_nat_unittest )
	set ( BUILD_libjingle_nat_unittest true )
endif ( NOT DEFINED BUILD_libjingle_nat_unittest )

if ( NOT DEFINED BUILD_libjingle_virtualsocket_unittest )
	set ( BUILD_libjingle_virtualsocket_unittest true )
endif ( NOT DEFINED BUILD_libjingle_virtualsocket_unittest )

if ( NOT DEFINED BUILD_libjingle_relayserver )
	set ( BUILD_libjingle_relayserver true )
endif ( NOT DEFINED BUILD_libjingle_relayserver )

if ( NOT DEFINED BUILD_libjingle_stunserver )
	set ( BUILD_libjingle_stunserver true )
endif ( NOT DEFINED BUILD_libjingle_stunserver )

if ( NOT DEFINED BUILD_libjingle_stunserver_unittest )
	set ( BUILD_libjingle_stunserver_unittest true )
endif ( NOT DEFINED BUILD_libjingle_stunserver_unittest )

if ( NOT DEFINED BUILD_libjingle_session_unittest )
	set ( BUILD_libjingle_session_unittest true )
endif ( NOT DEFINED BUILD_libjingle_session_unittest )

if ( NOT DEFINED BUILD_libjingle_port_unittest )
	set ( BUILD_libjingle_port_unittest true )
endif ( NOT DEFINED BUILD_libjingle_port_unittest )

if ( NOT DEFINED BUILD_libjingle_login )
	set ( BUILD_libjingle_login true )
endif ( NOT DEFINED BUILD_libjingle_login )

if ( NOT DEFINED BUILD_libjingle_pcp )
	set ( BUILD_libjingle_pcp true )
endif ( NOT DEFINED BUILD_libjingle_pcp )

if ( NOT DEFINED BUILD_libjingle_call )
	set ( BUILD_libjingle_call true )
endif ( NOT DEFINED BUILD_libjingle_call )

if ( NOT DEFINED BUILD_libjingle_pcp )
	set ( BUILD_libjingle_pcp true )
endif ( NOT DEFINED BUILD_libjingle_pcp )

########### next target ###############

set ( CMAKE_INCLUDE_CURRENT_DIR OFF )
add_definitions ( -DHAVE_CONFIG_H )

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

