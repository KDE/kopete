
set(libkirc_SRCS
  kircconst.cpp 
  kircentity.cpp 
  kircentitymanager.cpp 
  kircevent.cpp 
  kircmessage.cpp 
  kircmessageutil.cpp 
  kircsocket.cpp 
  kircstdmessages.cpp 
  kirctask.cpp

  tasks/kircsimpletask.cpp  
)

kde4_automoc(${libkirc_SRCS})

kde4_add_library(kirc SHARED ${libkirc_SRCS})

target_link_libraries(kirc ${QT_QTCORE_LIBRARY} ${QT_QTNETWORK_LIBRARY})
set_target_properties(kirc PROPERTIES VERSION 1.0.0 SOVERSION 1)

install(TARGETS kirc  DESTINATION ${LIB_INSTALL_DIR})

