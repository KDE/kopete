kde4_header()

add_subdirectory( images ) 
add_subdirectory( Incoming ) 
add_subdirectory( Outgoing ) 

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### install files ###############

install_files( ${DATA_INSTALL_DIR}/kopete/styles/Clean/Contents/Resources FILES main.css Footer.html Header.html Status.html )

kde4_footer()



#original Makefile.am contents follow:

#SUBDIRS = images Incoming Outgoing
#style_DATA = main.css Footer.html Header.html Status.html
#styledir = $(kde_datadir)/kopete/styles/Clean/Contents/Resources
#
#EXTRA_DIST = $(style_DATA)