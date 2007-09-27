set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )




########### next target ###############
set(kopetetabwidgettest_SRCS kopetetabwidgettest.cpp )



kde4_add_executable(kopetetabwidgettest TEST ${kopetetabwidgettest_SRCS})

target_link_libraries(kopetetabwidgettest  ${KDE4_KDECORE_LIBS} kopetechatwindow )


########### install files ###############





