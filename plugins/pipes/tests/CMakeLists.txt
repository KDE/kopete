


set (kopete_pipes_test1_SRCS
  main.cpp
  test1.cpp )

kde4_add_unit_test ( kopete_pipes_test1 ${kopete_pipes_test1_SRCS} )

target_link_libraries ( kopete_pipes_test1 ${QT_QTCORE_LIBRARY} ${QT_QTXML_LIBRARY} )
