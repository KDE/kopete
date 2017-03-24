set(jdns_tool_MOC_HDRS
    main.h
)

if(NOT Qt5Core_FOUND)
  qt5_wrap_cpp(jdns_tool_MOC_SRCS ${jdns_tool_MOC_HDRS})
endif()

set(jdns_tool_SRCS
    main.cpp
)

add_executable(jdns-tool ${jdns_tool_SRCS} ${jdns_tool_MOC_SRCS} ${jdns_tool_MOC_SRCS})

target_link_libraries(jdns-tool jdns qjdns)

set_target_properties(jdns-tool PROPERTIES
                      OUTPUT_NAME jdns
)

install(TARGETS jdns-tool
        LIBRARY DESTINATION ${KDE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${KDE_INSTALL_BINDIR}
        ARCHIVE DESTINATION ${KDE_INSTALL_LIBDIR}
#       FRAMEWORK DESTINATION ${FRAMEWORK_INSTALL_DIR}
)

if(MSVC)
  get_target_property(LOCATION jdns-tool LOCATION_DEBUG)
  string(REGEX REPLACE "\\.[^.]*$" ".pdb" LOCATION "${LOCATION}")
  install(FILES ${LOCATION} DESTINATION ${KDE_INSTALL_BINDIR} CONFIGURATIONS Debug)

  get_target_property(LOCATION jdns-tool LOCATION_RELWITHDEBINFO)
  string(REGEX REPLACE "\\.[^.]*$" ".pdb" LOCATION "${LOCATION}")
  install(FILES ${LOCATION} DESTINATION ${KDE_INSTALL_BINDIR} CONFIGURATIONS RelWithDebInfo)
endif(MSVC)
