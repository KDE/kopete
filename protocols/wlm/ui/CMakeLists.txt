INCLUDE_DIRECTORIES(
	${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_CURRENT_BINARY_DIR}
	${CMAKE_SOURCE_DIR}/$(srcdir)/..
	)

SET(kopetewlmui_STAT_SRCS
   wlmwebcamdialog.cpp
)

ADD_LIBRARY(kopetewlmui STATIC ${kopetewlmui_STAT_SRCS})
