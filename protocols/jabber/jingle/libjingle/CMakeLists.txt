kde4_header()

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config.h )

add_subdirectory( talk ) 




########### install files ###############


kde4_footer()



#original Makefile.am contents follow:

#SUBDIRS=talk
#
#dist-hook:
#	sed -i -f talk/sanitize.sed `find $(distdir) -type f`