kde4_header()

OPTION(WITH_smpppd "enable support for the SuSE Meta PPP Daemon (smpppd)" OFF)

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
if( LIBXML2_FOUND AND LIBXSLT_FOUND )
  add_subdirectory( webpresence )
endif( LIBXML2_FOUND AND LIBXSLT_FOUND )
if(WITH_smpppd)
	add_subdirectory(smpppdcs)
endif(WITH_smpppd)

message(STATUS "${CMAKE_CURRENT_SOURCE_DIR}: skipped subdir statistics (should be removed or rewritten -DarkShock)")
message(STATUS "${CMAKE_CURRENT_SOURCE_DIR}: skipped subdir netmeeting (should be removed -DarkShock)")
message(STATUS "${CMAKE_CURRENT_SOURCE_DIR}: skipped subdir $(MOTIONAUTOAWAY_SUBDIR)")


########### install files ###############


kde4_footer()



#original Makefile.am contents follow:

#if include_motionautoaway
#MOTIONAUTOAWAY_SUBDIR=motionautoaway
#endif
#
#if include_smpppdcs
#SMPPPDCS_SUBDIR=smpppdcs
#endif
#
#SUBDIRS = latex autoreplace history contactnotes cryptography\
#	connectionstatus translator nowlistening webpresence texteffect\
#	highlight alias $(MOTIONAUTOAWAY_SUBDIR) netmeeting addbookmarks\
#	statistics $(SMPPPDCS_SUBDIR)
