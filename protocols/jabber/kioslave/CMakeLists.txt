



########### next target ###############

set(kio_jabberdisco_PART_SRCS jabberdisco.cpp )

add_library(kio_jabberdisco MODULE ${kio_jabberdisco_PART_SRCS})


target_link_libraries(kio_jabberdisco  KF5::KIOCore jabberclient iris_local  )

install(TARGETS kio_jabberdisco  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES jabberdisco.protocol  DESTINATION ${SERVICES_INSTALL_DIR})




