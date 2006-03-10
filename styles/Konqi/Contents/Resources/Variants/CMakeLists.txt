kde4_header()

add_subdirectory( konqui ) 

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### install files ###############

install_files( ${DATA_INSTALL_DIR}/kopete/styles/Konqi/Contents/Resources/Variants FILES Side_blue_moon.css Side_blue_without_transparency.css Side_green_without_trans.css Side_blue.css  Side_blue_moon_without_transparency.css Side_green.css Side_green_without_transparency.css )

kde4_footer()



#original Makefile.am contents follow:

#SUBDIRS = konqui
#style_DATA = Side_blue_moon.css Side_blue_without_transparency.css Side_green_without_trans.css Side_blue.css  Side_blue_moon_without_transparency.css Side_green.css Side_green_without_transparency.css
#styledir = $(kde_datadir)/kopete/styles/Konqi/Contents/Resources/Variants
#
#EXTRA_DIST = $(style_DATA)