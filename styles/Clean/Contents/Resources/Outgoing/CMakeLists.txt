kde4_header()

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### install files ###############

install_files( ${DATA_INSTALL_DIR}/kopete/styles/Clean/Contents/Resources/Outgoing FILES buddy_icon.png Content.html NextContent.html )

kde4_footer()



#original Makefile.am contents follow:

#style_DATA = buddy_icon.png Content.html NextContent.html
#styledir = $(kde_datadir)/kopete/styles/Clean/Contents/Resources/Outgoing
#
#EXTRA_DIST = $(style_DATA)
