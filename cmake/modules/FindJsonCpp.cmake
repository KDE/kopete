# cmake macro to test JSONCPP

# Copyright (c) 2013, Pali Rohár <pali.rohar@gmail.com>
#
# JSONCPP_FOUND
# JSONCPP_INCLUDE_DIR
# JSONCPP_LIBRARY

include ( FindPackageHandleStandardArgs )

if ( JSONCPP_INCLUDE_DIR AND JSONCPP_LIBRARY )
	set ( JSONCPP_FOUND true )
else ( JSONCPP_INCLUDE_DIR AND JSONCPP_LIBRARY )
	find_path ( JSONCPP_INCLUDE_DIR json/json.h PATH_SUFFIXES jsoncpp )
	find_library ( JSONCPP_LIBRARY NAMES jsoncpp )

	if ( JSONCPP_INCLUDE_DIR AND JSONCPP_LIBRARY )
		set ( JSONCPP_FOUND true )
		message ( STATUS "Found JSONCPP: ${JSONCPP_LIBRARY}" )
	else ( JSONCPP_INCLUDE_DIR AND JSONCPP_LIBRARY )
		set ( JSONCPP_FOUND false )
		message ( STATUS "Not found JSONCPP" )
	endif ( JSONCPP_INCLUDE_DIR AND JSONCPP_LIBRARY )

	mark_as_advanced ( JSONCPP_INCLUDE_DIR JSONCPP_LIBRARY )

endif ( JSONCPP_INCLUDE_DIR AND JSONCPP_LIBRARY )

