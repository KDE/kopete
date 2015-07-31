set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )




########### next target ###############
set(kopetetabwidgettest_SRCS kopetetabwidgettest.cpp )



add_executable(kopetetabwidgettest ${kopetetabwidgettest_SRCS})
ecm_mark_as_test(kopetetabwidgettest)

target_link_libraries(kopetetabwidgettest  KF5::KDELibs4Support kopetechatwindow )


########### install files ###############





