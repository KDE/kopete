kde4_header()

add_subdirectory( Kopete ) 
add_subdirectory( Hacker ) 
add_subdirectory( Clean ) 
add_subdirectory( Clear ) 
add_subdirectory( Konqi ) 
add_subdirectory( Retropete ) 

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### install files ###############


kde4_footer()



#original Makefile.am contents follow:

#SUBDIRS = Kopete Hacker Clean Clear Konqi Retropete
#
