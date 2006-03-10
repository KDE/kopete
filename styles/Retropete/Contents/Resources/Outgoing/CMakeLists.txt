kde4_header()

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### install files ###############

install_files( ${DATA_INSTALL_DIR}/kopete/styles/Retropete/Contents/Resources/Outgoing FILES Action.html Content.html NextContent.html )

kde4_footer()



#original Makefile.am contents follow:

#style_DATA = Action.html Content.html NextContent.html
#styledir = $(kde_datadir)/kopete/styles/Retropete/Contents/Resources/Outgoing
#
#EXTRA_DIST = $(style_DATA)
