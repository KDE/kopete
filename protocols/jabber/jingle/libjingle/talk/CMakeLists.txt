kde4_header()

add_subdirectory( base ) 
add_subdirectory( p2p ) 
add_subdirectory( xmllite ) 
add_subdirectory( xmpp ) 
add_subdirectory( session ) 
add_subdirectory( third_party ) 




########### install files ###############


kde4_footer()



#original Makefile.am contents follow:

#SUBDIRS=base p2p xmllite xmpp session third_party 
