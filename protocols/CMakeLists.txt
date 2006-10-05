macro_optional_find_package(OpenSSL)

option(WITH_testbed "Enable Kopete testbed protocol" ON)
option(WITH_messenger "Enable Kopete MSN/Windows Live Messenger protocol" ON)
option(WITH_sms "Enable Kopete SMS protocol" ON)
option(WITH_telepathy "Enable Telepathy testbed protocol" OFF)

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

if(WITH_sms)
  add_subdirectory( sms )
endif(WITH_sms)

if(WITH_telepathy AND DECIBEL_FOUND)
  add_subdirectory( telepathy )
endif(WITH_telepathy AND DECIBEL_FOUND)

message(STATUS "${CMAKE_CURRENT_SOURCE_DIR}: Disabled Groupwise, Meanwhile for compiling, because they are not ported right now. I leave that to the protocol maintainer (-DarkShock)")
