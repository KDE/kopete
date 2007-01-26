
add_subdirectory( latex ) 
add_subdirectory( autoreplace ) 
add_subdirectory( history ) 
add_subdirectory( contactnotes ) 
add_subdirectory( cryptography ) 
add_subdirectory( connectionstatus ) 
add_subdirectory( translator ) 
add_subdirectory( nowlistening ) 
add_subdirectory( texteffect ) 
add_subdirectory( highlight ) 
add_subdirectory( alias ) 
add_subdirectory( addbookmarks ) 
#add_subdirectory( statistics ) 
add_subdirectory( privacy )

if( LIBXML2_FOUND AND LIBXSLT_FOUND )
  add_subdirectory( webpresence )
endif( LIBXML2_FOUND AND LIBXSLT_FOUND )

#add_subdirectory(smpppdcs)

message(STATUS "${CMAKE_CURRENT_SOURCE_DIR}: skipped subdir smpppdcs (SuSE Meta PPP Daemon)")
message(STATUS "${CMAKE_CURRENT_SOURCE_DIR}: skipped subdir statistics (will be rewritten)")
message(STATUS "${CMAKE_CURRENT_SOURCE_DIR}: skipped subdir $(MOTIONAUTOAWAY_SUBDIR)")

