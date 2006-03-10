kde4_header()

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### install files ###############

install_files( ${DATA_INSTALL_DIR}/kopete/styles/Hacker/Contents/Resources/Outgoing FILES Content.html NextContent.html Context.html NextContext.html buddy_icon.png )

kde4_footer()



#original Makefile.am contents follow:

#style_DATA = Content.html NextContent.html Context.html NextContext.html buddy_icon.png
#styledir = $(kde_datadir)/kopete/styles/Hacker/Contents/Resources/Outgoing
#
#EXTRA_DIST = $(style_DATA)
