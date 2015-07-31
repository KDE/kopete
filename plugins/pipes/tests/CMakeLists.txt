


set (kopete_pipes_test1_SRCS
  main.cpp
  test1.cpp )

add_executable ( kopete_pipes_test1 ${kopete_pipes_test1_SRCS} )
add_test(kopete_pipes_test1 kopete_pipes_test1)
ecm_mark_as_test(kopete_pipes_test1)

target_link_libraries ( kopete_pipes_test1 Qt5::Core Qt5::Xml )
