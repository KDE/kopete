kde4_header()

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### install files ###############

install_files( ${DATA_INSTALL_DIR}/kopete/styles/Hacker/Contents/Resources/Variants FILES Dark2.css Dark.css Light2.css Light.css Dark2-Noback.css Dark-Noback.css Light2-Noback.css Light-Noback.css )

kde4_footer()



#original Makefile.am contents follow:

#style_DATA = Dark2.css Dark.css Light2.css Light.css Dark2-Noback.css Dark-Noback.css Light2-Noback.css Light-Noback.css
#
#styledir = $(kde_datadir)/kopete/styles/Hacker/Contents/Resources/Variants
#
#EXTRA_DIST = $(style_DATA)
