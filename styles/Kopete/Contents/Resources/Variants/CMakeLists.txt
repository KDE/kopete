kde4_header()

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### install files ###############

install_files( ${DATA_INSTALL_DIR}/kopete/styles/Kopete/Contents/Resources/Variants FILES Big_pictures.css )

kde4_footer()



#original Makefile.am contents follow:

#style_DATA = Big_pictures.css
#
#styledir = $(kde_datadir)/kopete/styles/Kopete/Contents/Resources/Variants
#
#EXTRA_DIST = $(style_DATA)
