kde4_header()

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### install files ###############

install_files( ${DATA_INSTALL_DIR}/kopete/styles/Clean/Contents/Resources/images FILES action.png important.png internal.png )

kde4_footer()



#original Makefile.am contents follow:

#style_DATA = action.png important.png internal.png
#styledir = $(kde_datadir)/kopete/styles/Clean/Contents/Resources/images
#
#EXTRA_DIST = $(style_DATA)