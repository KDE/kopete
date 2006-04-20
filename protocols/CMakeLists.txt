kde4_header()

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )

OPTION(WITH_testbed "Enable Kopete testbed protocol" ON)
OPTION(WITH_messenger "Enable Kopete MSN/Windows Live Messenger protocol" ON)

add_subdirectory( msn ) 
add_subdirectory( oscar ) 
add_subdirectory( yahoo ) 
add_subdirectory( winpopup )
if(WITH_messenger)
  add_subdirectory( messenger )
endif(WITH_messenger)
if(WITH_testbed)
  add_subdirectory( testbed )
endif(WITH_testbed)
#add_subdirectory( jabber )
add_subdirectory( gadu )

message(STATUS "${CMAKE_CURRENT_SOURCE_DIR}: Can someone fix linking problems with libiris_kopete in Jabber ? (-DarkShock)")
message(STATUS "${CMAKE_CURRENT_SOURCE_DIR}: Disabled IRC, Groupwise, SMS and Meanwhile for compiling, because they are not ported right now. I leave that to the protocol maintainer (-DarkShock)")

########### install files ###############


kde4_footer()



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
