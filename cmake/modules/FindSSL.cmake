# cmake macro to test SSL and CRYPTO

# Copyright (c) 2009, Pali Rohár <pali.rohar@gmail.com>
#
# SSL_FOUND
# SSL_LIBRARY
# CRYPTO_LIBRARY

include ( FindPackageHandleStandardArgs )

if ( SSL_LIBRARY AND CRYPTO_LIBRARY )
	set ( SSL_FOUND true )
else ( SSL_LIBRARY AND CRYPTO_LIBRARY )
	find_library ( SSL_LIBRARY NAMES ssl )
	find_library ( CRYPTO_LIBRARY NAMES crypto )

	if ( SSL_LIBRARY AND CRYPTO_LIBRARY )
		set ( SSL_FOUND true )
		message ( STATUS "Found SSL and CRYPTO libraries: ${SSL_LIBRARY}; ${CRYPTO_LIBRARY}" )
	else ( SSL_LIBRARY AND CRYPTO_LIBRARY )
		set ( SSL_FOUND false )
	endif ( SSL_LIBRARY AND CRYPTO_LIBRARY )

	mark_as_advanced ( SSL_LIBRARY CRYPTO_LIBRARY )

endif ( SSL_LIBRARY AND CRYPTO_LIBRARY )

