find_package(ZLIB REQUIRED)

add_subdirectory( tasks )

include_directories(
	${CMAKE_SOURCE_DIR}/kopete/protocols/groupwise/libgroupwise/tasks
	${QCA2_INCLUDE_DIR}
)

########### tasks target ###############

set( libqgroupwise_tasks_SRCS
	tasks/chatcountstask.cpp
	tasks/chatpropertiestask.cpp
	tasks/conferencetask.cpp
	tasks/connectiontask.cpp
	tasks/createconferencetask.cpp
	tasks/createcontacttask.cpp
	tasks/createcontactinstancetask.cpp
	tasks/createfoldertask.cpp
	tasks/deleteitemtask.cpp
	tasks/eventtask.cpp
	tasks/getchatsearchresultstask.cpp
	tasks/getdetailstask.cpp
	tasks/getstatustask.cpp
	tasks/joinchattask.cpp
	tasks/joinconferencetask.cpp
	tasks/keepalivetask.cpp
	tasks/logintask.cpp
	tasks/leaveconferencetask.cpp
	tasks/modifycontactlisttask.cpp
	tasks/movecontacttask.cpp
	tasks/needfoldertask.cpp
	tasks/pollsearchresultstask.cpp
	tasks/privacyitemtask.cpp
	tasks/rejectinvitetask.cpp
	tasks/requesttask.cpp
	tasks/searchchattask.cpp
	tasks/searchusertask.cpp
	tasks/setstatustask.cpp
	tasks/sendinvitetask.cpp
	tasks/sendmessagetask.cpp
	tasks/statustask.cpp
	tasks/typingtask.cpp
	tasks/updatecontacttask.cpp
	tasks/updatefoldertask.cpp
	tasks/updateitemtask.cpp
)

set( libqgroupwise_SRCS
	connector.cpp
	bytestream.cpp
	tlshandler.cpp
	qcatlshandler.cpp
	gwclientstream.cpp
	securestream.cpp
	stream.cpp
	coreprotocol.cpp
	gwfield.cpp
	transferbase.cpp
	usertransfer.cpp
	eventtransfer.cpp
	transfer.cpp
	request.cpp
	requestfactory.cpp
	safedelete.cpp
	response.cpp
	client.cpp
	task.cpp
	rtf.cc
	userdetailsmanager.cpp
	eventprotocol.cpp
	inputprotocolbase.cpp
	responseprotocol.cpp
	privacymanager.cpp
	gwglobal.cpp
	chatroommanager.cpp
	compressionhandler.cpp
	compress.cpp
)


kde4_add_library( qgroupwise SHARED ${libqgroupwise_tasks_SRCS} ${libqgroupwise_SRCS} )

target_link_libraries( qgroupwise ${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY} ${QT_QTNETWORK_LIBRARY} ${QT_QT3SUPPORT_LIBRARY} ${QCA2_LIBRARIES} ${ZLIB_LIBRARIES} ${KDE4_KDECORE_LIBS})

########### install files ###############

install(TARGETS qgroupwise  DESTINATION ${LIB_INSTALL_DIR})




