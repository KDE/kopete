kde4_header()

add_subdirectory( Incoming ) 
add_subdirectory( Outgoing ) 

########### install files ###############

install_files( ${DATA_INSTALL_DIR}/kopete/styles/Retropete/Contents/Resources FILES main.css Footer.html Header.html Status.html )

kde4_footer()
