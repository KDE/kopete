
include_directories( ${KOPETE_INCLUDES} )

set(libkirc_SRCS
  kircconst.cpp 
  kircentity.cpp 
  kircentitymanager.cpp 
  kircevent.cpp 
  kircmessage.cpp 
  kircmessageutil.cpp 
  kircsocket.cpp 
  kircstdmessages.cpp 
)

kde4_automoc(${libkirc_SRCS})

kde4_add_library( kirc SHARED ${libkirc_SRCS} )

target_link_libraries( kirc ${KDE4_KIO_LIBS} ${QT_QTNETWORK_LIBRARY} )

install(TARGETS kirc  DESTINATION ${LIB_INSTALL_DIR})

