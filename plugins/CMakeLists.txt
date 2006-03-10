kde4_header()

add_subdirectory( latex ) 
add_subdirectory( autoreplace ) 
add_subdirectory( history ) 
add_subdirectory( contactnotes ) 
add_subdirectory( cryptography ) 
add_subdirectory( connectionstatus ) 
add_subdirectory( translator ) 
add_subdirectory( nowlistening ) 
add_subdirectory( webpresence ) 
add_subdirectory( texteffect ) 
add_subdirectory( highlight ) 
add_subdirectory( alias ) 
add_subdirectory( netmeeting ) 
add_subdirectory( addbookmarks ) 
add_subdirectory( statistics ) 

message(STATUS "${CMAKE_CURRENT_SOURCE_DIR}: skipped subdir $(MOTIONAUTOAWAY_SUBDIR)")
message(STATUS "${CMAKE_CURRENT_SOURCE_DIR}: skipped subdir $(SMPPPDCS_SUBDIR)")
include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


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
