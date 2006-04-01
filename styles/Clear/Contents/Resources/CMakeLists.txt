kde4_header()

add_subdirectory( images ) 
add_subdirectory( Incoming ) 
add_subdirectory( Outgoing ) 

########### install files ###############

install_files( ${DATA_INSTALL_DIR}/kopete/styles/Clear/Contents/Resources FILES main.css Footer.html Header.html Status.html )

kde4_footer()
