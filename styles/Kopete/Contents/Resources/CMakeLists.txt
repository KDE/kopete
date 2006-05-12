
add_subdirectory( images ) 
add_subdirectory( Incoming ) 
add_subdirectory( Outgoing ) 
add_subdirectory( Variants ) 

########### install files ###############

install_files( ${DATA_INSTALL_DIR}/kopete/styles/Kopete/Contents/Resources FILES main.css Footer.html Header.html Status.html )

