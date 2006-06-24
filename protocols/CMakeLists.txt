macro_optional_find_package(OpenSSL)

option(WITH_testbed "Enable Kopete testbed protocol" ON)
option(WITH_messenger "Enable Kopete MSN/Windows Live Messenger protocol" ON)

include_directories(${KOPETE_INCLUDES})

add_subdirectory( msn ) 
add_subdirectory( oscar ) 
add_subdirectory( yahoo ) 
add_subdirectory( winpopup )
add_subdirectory( qq )
#add_subdirectory( irc )
if(OPENSSL_FOUND)
	add_subdirectory( gadu )
endif(OPENSSL_FOUND)

if(QCA2_FOUND)
  if(WITH_messenger)
    add_subdirectory( messenger )
  endif(WITH_messenger)
  add_subdirectory( jabber )
endif(QCA2_FOUND)

if(WITH_testbed)
  add_subdirectory( testbed )
endif(WITH_testbed)

message(STATUS "${CMAKE_CURRENT_SOURCE_DIR}: Disabled IRC, Groupwise, SMS and Meanwhile for compiling, because they are not ported right now. I leave that to the protocol maintainer (-DarkShock)")


#original Makefile.am contents follow:

#if include_meanwhile
#MEANWHILE=meanwhile
#endif
#
#if include_gadu
#GADU=gadu
#endif
#
#if include_jabber
#JABBER=jabber
#endif
#
#if include_testbed
#TESTBED=testbed
#endif
#
#if include_smsgsm
#SMS=sms
#endif
#
##SUBDIRS = $(TESTBED) groupwise msn irc oscar yahoo winpopup $(SMS) $(JABBER) $(GADU) $(MEANWHILE)
#
## Disabled IRC, Groupwise, SMS and Meanwhile for compiling, because they are not ported right now
## I leave that to the protocol maintainer (-DarkShock)
#SUBDIRS = $(TESTBED) msn oscar yahoo winpopup $(GADU) messenger $(JABBER)
