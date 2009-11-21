# cmake macro to test iLBC

# Copyright (c) 2009, Pali Rohár <pali.rohar@gmail.com>
#
# ILBC_FOUND
# ILBC_INCLUDE_DIR
# ILBC_LIBRARY

include ( FindPackageHandleStandardArgs )

if ( ILBC_INCLUDE_DIR AND ILBC_LIBRARY )
	set ( ILBC_FOUND true )
else ( ILBC_INCLUDE_DIR AND ILBC_LIBRARY )
	find_path ( ILBC_INCLUDE_DIR iLBC_decode.h PATHS ${INCLUDE_INSTALL_DIR}/ilbc ${CMAKE_INCLUDE_PATH}/ilbc )
	find_library ( ILBC_LIBRARY NAMES ilbc )

	if ( ILBC_INCLUDE_DIR AND ILBC_LIBRARY )
		set ( ILBC_FOUND true )
		message ( STATUS "Found iLBC: ${ILBC_LIBRARY}" )
	else ( ILBC_INCLUDE_DIR AND ILBC_LIBRARY )
		set ( ILBC_FOUND false )
		message ( STATUS "Not found iLBC" )
	endif ( ILBC_INCLUDE_DIR AND ILBC_LIBRARY )

	mark_as_advanced ( ILBC_INCLUDE_DIR ILBC_LIBRARY )

endif ( ILBC_INCLUDE_DIR AND ILBC_LIBRARY )

