# cmake macro to test Expat

# Copyright (c) 2009, Pali Rohár <pali.rohar@gmail.com>
#
# EXPAT_FOUND
# EXPAT_INCLUDE_DIR
# EXPAT_LIBRARY

include ( FindPackageHandleStandardArgs )

if ( EXPAT_INCLUDE_DIR AND EXPAT_LIBRARY )
	set ( EXPAT_FOUND true )
else ( EXPAT_INCLUDE_DIR AND EXPAT_LIBRARY )
	find_path ( EXPAT_INCLUDE_DIR expat.h )
	find_library ( EXPAT_LIBRARY NAMES expat )

	if ( EXPAT_INCLUDE_DIR AND EXPAT_LIBRARY )
		set ( EXPAT_FOUND true )
		message ( STATUS "Found Expat: ${EXPAT_LIBRARY}" )
	else ( EXPAT_INCLUDE_DIR AND EXPAT_LIBRARY )
		set ( EXPAT_FOUND false )
		message ( STATUS "Not found Expat" )
	endif ( EXPAT_INCLUDE_DIR AND EXPAT_LIBRARY )

	mark_as_advanced ( EXPAT_INCLUDE_DIR EXPAT_LIBRARY )

endif ( EXPAT_INCLUDE_DIR AND EXPAT_LIBRARY )

