# cmake macro to test SRTP

# Copyright (c) 2013, Pali Rohár <pali.rohar@gmail.com>
#
# SRTP_FOUND
# SRTP_INCLUDE_DIR
# SRTP_LIBRARY

include ( FindPackageHandleStandardArgs )

if ( SRTP_INCLUDE_DIR AND SRTP_LIBRARY )
	set ( SRTP_FOUND true )
else ( SRTP_INCLUDE_DIR AND SRTP_LIBRARY )
	find_path ( SRTP_INCLUDE_DIR srtp.h PATH_SUFFIXES srtp )
	find_library ( SRTP_LIBRARY NAMES srtp )

	if ( SRTP_INCLUDE_DIR AND SRTP_LIBRARY )
		set ( SRTP_FOUND true )
		message ( STATUS "Found SRTP: ${SRTP_LIBRARY}" )
	else ( SRTP_INCLUDE_DIR AND SRTP_LIBRARY )
		set ( SRTP_FOUND false )
		message ( STATUS "Not found SRTP" )
	endif ( SRTP_INCLUDE_DIR AND SRTP_LIBRARY )

	mark_as_advanced ( SRTP_INCLUDE_DIR SRTP_LIBRARY )

endif ( SRTP_INCLUDE_DIR AND SRTP_LIBRARY )

