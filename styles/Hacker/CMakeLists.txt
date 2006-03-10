kde4_header()

add_subdirectory( Contents ) 

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### install files ###############

install_files( ${DATA_INSTALL_DIR}/kopete/styles/Hacker FILES COPYRIGHT README gpl.txt )

kde4_footer()



#original Makefile.am contents follow:

#SUBDIRS = Contents
#style_DATA = COPYRIGHT README gpl.txt
#styledir = $(kde_datadir)/kopete/styles/Hacker
#
#EXTRA_DIST = $(style_DATA)