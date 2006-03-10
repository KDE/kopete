kde4_header()

add_subdirectory( include ) 
add_subdirectory( jabber ) 
add_subdirectory( xmpp-core ) 
add_subdirectory( xmpp-im ) 

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### install files ###############


kde4_footer()



#original Makefile.am contents follow:

#SUBDIRS = include jabber xmpp-core xmpp-im
