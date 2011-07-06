# cmake macro to test msiLBC

# Copyright (c) 2009-2010 Pali Rohár <pali.rohar@gmail.com>
#
# MSILBC_FOUND
# MSILBC_LIBRARY

include ( FindPackageHandleStandardArgs )

if ( MSILBC_LIBRARY )
	set ( MSILBC_FOUND true )
else ( MSILBC_LIBRARY )

	find_library ( MSILBC_LIBRARY NAMES msilbc PATH_SUFFIXES mediastreamer/plugins )

	if ( MSILBC_LIBRARY )
		set ( MSILBC_FOUND true )
		message ( STATUS "Found msiLBC: ${MSILBC_LIBRARY}" )
	else ( MSILBC_LIBRARY )
		set ( MSILBC_FOUND false )
		message ( STATUS "Not found msiLBC" )
	endif ( MSILBC_LIBRARY )

	mark_as_advanced ( MSILBC_LIBRARY )

endif ( MSILBC_LIBRARY )

