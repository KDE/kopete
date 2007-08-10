



########### next target ###############

set(kio_jabberdisco_PART_SRCS jabberdisco.cpp )

kde4_add_plugin(kio_jabberdisco ${kio_jabberdisco_PART_SRCS})


target_link_libraries(kio_jabberdisco  ${KDE4_KIO_LIBS} jabberclient iris_local  )

install(TARGETS kio_jabberdisco  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES jabberdisco.protocol  DESTINATION ${SERVICES_INSTALL_DIR})




