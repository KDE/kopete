kde4_header()

add_subdirectory( Incoming ) 
add_subdirectory( Outgoing ) 
add_subdirectory( Variants ) 

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### install files ###############

install_files( ${DATA_INSTALL_DIR}/kopete/styles/Konqi/Contents/Resources FILES main.css Footer.html Header.html Status.html puce.png )

kde4_footer()



#original Makefile.am contents follow:

#SUBDIRS = Incoming Outgoing Variants
#style_DATA = main.css Footer.html Header.html Status.html puce.png
#styledir = $(kde_datadir)/kopete/styles/Konqi/Contents/Resources
#
#EXTRA_DIST = $(style_DATA)