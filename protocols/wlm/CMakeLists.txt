project(wlm)

include_directories(
    ${Boost_INCLUDE_DIRS}
)

ADD_SUBDIRECTORY(icons)

set(kopete_wlm_include_DIRS
    ${KOPETE_INCLUDES}
    ${CMAKE_CURRENT_SOURCE_DIR}/ui
    ${LIBMSN_INCLUDE_DIR}
)

set(kopete_wlm_ui_SRCS
   ui/wlmaddcontactpage.cpp
   ui/wlmeditaccountwidget.cpp
)

ki18n_wrap_ui(kopete_wlm_ui_SRCS
    ui/wlmaddui.ui
    ui/wlminfo.ui 
    ui/wlmaccountpreferences.ui
    ui/wlmchatsessioninkpopup.ui
)

set(kopete_wlm_PART_SRCS
   ${kopete_wlm_ui_SRCS}
   wlmprotocol.cpp
   wlmcontact.cpp
   wlmaccount.cpp
   wlmlibmsn.cpp
   wlmsocket.cpp
   wlmserver.cpp
   wlmchatmanager.cpp
   wlmchatsession.cpp
   wlmchatsessioninkarea.cpp
   wlmchatsessioninkaction.cpp
   wlmtransfermanager.cpp
)

add_library(kopete_wlm MODULE ${kopete_wlm_PART_SRCS})

set(kopete_wlm_link_LIBS
     
    
    KF5::KIOCore 
    ${KDE4_KMIME_LIBS}
    Qt5::Xml 
    Qt5::Network 
    kopete 
    ${LIBMSN_LIBRARIES}
)

if(BUILD_WLM_MEDIASTREAMER)
    set(kopete_wlm_link_LIBS ${kopete_wlm_link_LIBS} ${MEDIASTREAMER_LIBRARIES} ${LIBORTP_LIBRARY})
    set(kopete_wlm_include_DIRS ${kopete_wlm_include_DIRS} ${MEDIASTREAMER_INCLUDE_DIR})
    add_definitions(-DHAVE_MEDIASTREAMER)
endif(BUILD_WLM_MEDIASTREAMER)

if(GIF_FOUND)
    set(kopete_wlm_link_LIBS ${kopete_wlm_link_LIBS} ${GIF_LIBRARIES})
    set(kopete_wlm_include_DIRS ${kopete_wlm_include_DIRS} ${GIF_INCLUDE_DIR})
    add_definitions(-DHAVE_GIFLIB)

    # giflib 4.2.0 removed PrintGifError() in favor of GifError() and GifErrorString().
    # We need to check which functions are present.
    # FIXME: Once we depend on CMake 2.8.6+ we can use CMAKE_{PUSH,POP}_CHECK_STATE().
    set(_OLD_CMAKE_REQUIRED_LIBRARIES "${CMAKE_REQUIRED_LIBRARIES}")
    set(_OLD_CMAKE_REQUIRED_INCLUDES "${CMAKE_REQUIRED_INCLUDES}")
    set(CMAKE_REQUIRED_LIBRARIES "${GIF_LIBRARIES}")
    set(CMAKE_REQUIRED_INCLUDES "${GIF_INCLUDE_DIR}")
    include(CheckFunctionExists)
    check_function_exists(GifErrorString HAVE_GIF_ERROR_STRING)
    set(CMAKE_REQUIRED_LIBRARIES "${_OLD_CMAKE_REQUIRED_LIBRARIES}")
    set(CMAKE_REQUIRED_INCLUDES "${_OLD_CMAKE_REQUIRED_INCLUDES}")

    if(HAVE_GIF_ERROR_STRING)
        add_definitions(-DHAVE_GIF_ERROR_STRING)
    endif(HAVE_GIF_ERROR_STRING)
endif(GIF_FOUND)

include_directories(${kopete_wlm_include_DIRS})
target_link_libraries(kopete_wlm ${kopete_wlm_link_LIBS})

install(TARGETS kopete_wlm  DESTINATION ${PLUGIN_INSTALL_DIR})

install(FILES  kopete_wlm.desktop DESTINATION ${SERVICES_INSTALL_DIR})
install(FILES  wlmchatui.rc DESTINATION ${DATA_INSTALL_DIR}/kopete_wlm)
