kde4_header()

add_subdirectory( Resources ) 

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### install files ###############

install_files( ${DATA_INSTALL_DIR}/kopete/styles/Hacker/Contents FILES Info.plist )

kde4_footer()



#original Makefile.am contents follow:

#SUBDIRS = Resources
#style_DATA = Info.plist
#styledir = $(kde_datadir)/kopete/styles/Hacker/Contents
#
#EXTRA_DIST = $(style_DATA)