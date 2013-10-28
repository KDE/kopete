set( GROUPWISE_DEBUG_LIBGW 14191 )
add_definitions( -DKDE_DEFAULT_DEBUG_AREA=${GROUPWISE_DEBUG_LIBGW} )

include_directories(
	${CMAKE_CURRENT_SOURCE_DIR}/tasks
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
	bytestream.cpp
	chatroommanager.cpp
	client.cpp
	compressionhandler.cpp
	compress.cpp
	connector.cpp
	coreprotocol.cpp
	eventprotocol.cpp
	eventtransfer.cpp
	gwclientstream.cpp
	gwerror.cpp
	gwfield.cpp
	gwglobal.cpp
	inputprotocolbase.cpp
	privacymanager.cpp
	qcatlshandler.cpp
	request.cpp
	requestfactory.cpp
	response.cpp
	responseprotocol.cpp
	rtf.cc
	safedelete.cpp
	securestream.cpp
	stream.cpp
	task.cpp
	tlshandler.cpp
	transfer.cpp
	transferbase.cpp
	userdetailsmanager.cpp
	usertransfer.cpp
)


kde4_add_library( qgroupwise SHARED ${libqgroupwise_tasks_SRCS} ${libqgroupwise_SRCS} )

target_link_libraries( qgroupwise ${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY} ${QT_QTNETWORK_LIBRARY} ${QT_QT3SUPPORT_LIBRARY} ${QCA2_LIBRARIES} ${ZLIB_LIBRARIES} ${KDE4_KDECORE_LIBS})

########### install files ###############

install(TARGETS qgroupwise ${INSTALL_TARGETS_DEFAULT_ARGS})




