kde4_header()

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### install files ###############

install_files( ${DATA_INSTALL_DIR}/kopete/styles/Konqi/Contents/Resources/Variants/konqui FILES cadre1.png cadre2.png cadre3.png cadre4.png cadre5.png cadre6.png konqui-blue.png konqui-green.png konqui-moon.jpg )

kde4_footer()



#original Makefile.am contents follow:

#style_DATA = cadre1.png cadre2.png cadre3.png cadre4.png cadre5.png cadre6.png konqui-blue.png konqui-green.png konqui-moon.jpg
#styledir = $(kde_datadir)/kopete/styles/Konqi/Contents/Resources/Variants/konqui
#
#EXTRA_DIST = $(style_DATA)