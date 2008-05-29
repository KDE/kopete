
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/libgadu-config.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/libgadu-config.h )



set(libgadu_SRCS common.c dcc.c events.c http.c libgadu.c pubdir50.c pubdir.c)

kde4_add_library(gadu_kopete SHARED ${libgadu_SRCS})

target_link_libraries(gadu_kopete pthread ${OPENSSL_LIBRARIES} )

if(HAVE_NSL_LIBRARY)
  # This is probably Solaris, and we need libnsl for gethostbyname
  target_link_libraries(gadu_kopete nsl)
endif(HAVE_NSL_LIBRARY)


set_target_properties(gadu_kopete PROPERTIES VERSION 1.0.0 SOVERSION 1)

install(TARGETS gadu_kopete ${INSTALL_TARGETS_DEFAULT_ARGS})

########### install files ###############





