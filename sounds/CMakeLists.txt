kde4_header()

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### install files ###############

install_files( ${SOUND_INSTALL_DIR} FILES Kopete_Event.ogg Kopete_Received.ogg Kopete_Sent.ogg Kopete_User_is_Online.ogg )

kde4_footer()



#original Makefile.am contents follow:

#kde_sound_DATA = Kopete_Event.ogg Kopete_Received.ogg Kopete_Sent.ogg Kopete_User_is_Online.ogg
#
#EXTRA_DIST = $(kde_sound_DATA)
#
