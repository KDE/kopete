



########### next target ###############
set(kopetetabwidgettest_SRCS kopetetabwidgettest.cpp )

kde4_automoc(${kopetetabwidgettest_SRCS})


kde4_add_test_executable(kopetetabwidgettest ${kopetetabwidgettest_SRCS})

target_link_libraries(kopetetabwidgettest  ${KDE4_KDECORE_LIBS} kopetechatwindow )


########### install files ###############





