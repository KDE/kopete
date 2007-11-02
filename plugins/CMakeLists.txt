
# Laurent this plugins use shell script which will not work on windows
if (UNIX)
  add_subdirectory( latex ) 
endif (UNIX)
add_subdirectory( autoreplace ) 
add_subdirectory( history ) 
add_subdirectory( contactnotes ) 
# chehrlic connectionstatus is using k3process 
if(NOT WIN32)
  add_subdirectory( connectionstatus ) 
endif(NOT WIN32)
add_subdirectory( translator ) 
add_subdirectory( nowlistening ) 
add_subdirectory( texteffect ) 
add_subdirectory( highlight ) 
add_subdirectory( alias ) 
add_subdirectory( addbookmarks ) 
add_subdirectory( privacy ) 
add_subdirectory( urlpicpreview ) 
if (SQLITE_FOUND)
  add_subdirectory ( statistics ) 
endif (SQLITE_FOUND)

if( LIBXML2_FOUND AND LIBXSLT_FOUND )
  add_subdirectory( webpresence )
endif( LIBXML2_FOUND AND LIBXSLT_FOUND )


