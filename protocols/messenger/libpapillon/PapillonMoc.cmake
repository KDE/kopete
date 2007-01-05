# Special macro to generate moc for libpapillon
# First parameter - the header file which contain the Q_OBJECT macro
# Second parameter - the source file to add the dependency on moc
MACRO(PAPILLON_MOC _header _originalFile)
  QT4_GET_MOC_INC_DIRS(_moc_INCS)

  GET_FILENAME_COMPONENT(_abs_FILE ${_header} ABSOLUTE)
  GET_FILENAME_COMPONENT(_basename ${_header} NAME_WE)
  SET(_moc ${CMAKE_CURRENT_BINARY_DIR}/${_basename}.moc)
  SET(extra_moc_argument)
  if(WIN32)
     SET(extra_moc_argument -DWIN32)
  endif(WIN32)
  ADD_CUSTOM_COMMAND(OUTPUT ${_moc}
                     COMMAND ${QT_MOC_EXECUTABLE}
                     ARGS ${extra_moc_argument} ${_moc_INCS} -o ${_moc} ${_abs_FILE}
                     DEPENDS ${_header}
                     )

  MACRO_ADD_FILE_DEPENDENCIES(${_originalFile} ${_moc})

ENDMACRO(PAPILLON_MOC)

# Papillon/Base
papillon_moc( ${CMAKE_CURRENT_SOURCE_DIR}/include/Papillon/Base/bytestream.h ${CMAKE_CURRENT_SOURCE_DIR}/base/bytestream.cpp )
papillon_moc( ${CMAKE_CURRENT_SOURCE_DIR}/include/Papillon/Base/connector.h ${CMAKE_CURRENT_SOURCE_DIR}/base/connector.cpp )
papillon_moc( ${CMAKE_CURRENT_SOURCE_DIR}/include/Papillon/Base/safedelete.h ${CMAKE_CURRENT_SOURCE_DIR}/base/safedelete.cpp )
papillon_moc( ${CMAKE_CURRENT_SOURCE_DIR}/include/Papillon/Base/stream.h ${CMAKE_CURRENT_SOURCE_DIR}/base/stream.cpp )

# Papillon/Http
papillon_moc( ${CMAKE_CURRENT_SOURCE_DIR}/include/Papillon/Http/httpcoreprotocol.h ${CMAKE_CURRENT_SOURCE_DIR}/http/httpcoreprotocol.cpp )
papillon_moc( ${CMAKE_CURRENT_SOURCE_DIR}/include/Papillon/Http/securestream.h ${CMAKE_CURRENT_SOURCE_DIR}/http/securestream.cpp )
papillon_moc( ${CMAKE_CURRENT_SOURCE_DIR}/include/Papillon/Http/tweenerhandler.h ${CMAKE_CURRENT_SOURCE_DIR}/http/tweenerhandler.cpp )
papillon_moc( ${CMAKE_CURRENT_SOURCE_DIR}/include/Papillon/Http/httpconnection.h ${CMAKE_CURRENT_SOURCE_DIR}/http/httpconnection.cpp )

# Papillon/Tasks
papillon_moc( ${CMAKE_CURRENT_SOURCE_DIR}/include/Papillon/Tasks/challengetask.h ${CMAKE_CURRENT_SOURCE_DIR}/tasks/challengetask.cpp )
papillon_moc( ${CMAKE_CURRENT_SOURCE_DIR}/include/Papillon/Tasks/logintask.h ${CMAKE_CURRENT_SOURCE_DIR}/tasks/logintask.cpp )
papillon_moc( ${CMAKE_CURRENT_SOURCE_DIR}/include/Papillon/Tasks/notifymessagetask.h ${CMAKE_CURRENT_SOURCE_DIR}/tasks/notifymessagetask.cpp )
papillon_moc( ${CMAKE_CURRENT_SOURCE_DIR}/include/Papillon/Tasks/notifypresencetask.h ${CMAKE_CURRENT_SOURCE_DIR}/tasks/notifypresencetask.cpp )
papillon_moc( ${CMAKE_CURRENT_SOURCE_DIR}/include/Papillon/Tasks/notifystatusmessagetask.h ${CMAKE_CURRENT_SOURCE_DIR}/tasks/notifystatusmessagetask.cpp )
papillon_moc( ${CMAKE_CURRENT_SOURCE_DIR}/include/Papillon/Tasks/setpersonalinformationtask.h ${CMAKE_CURRENT_SOURCE_DIR}/tasks/setpersonalinformationtask.cpp )
papillon_moc( ${CMAKE_CURRENT_SOURCE_DIR}/include/Papillon/Tasks/setpresencetask.h ${CMAKE_CURRENT_SOURCE_DIR}/tasks/setpresencetask.cpp )
papillon_moc( ${CMAKE_CURRENT_SOURCE_DIR}/include/Papillon/Tasks/setstatusmessagetask.h ${CMAKE_CURRENT_SOURCE_DIR}/tasks/setstatusmessagetask.cpp )

# Papillon/contactlist
papillon_moc( ${CMAKE_CURRENT_SOURCE_DIR}/contactlist/fetchcontactlistjob.h ${CMAKE_CURRENT_SOURCE_DIR}/contactlist/fetchcontactlistjob.cpp )
papillon_moc( ${CMAKE_CURRENT_SOURCE_DIR}/contactlist/sharingservicebinding.h ${CMAKE_CURRENT_SOURCE_DIR}/contactlist/sharingservicebinding.cpp )

# Papillon/contactlist
papillon_moc( ${CMAKE_CURRENT_SOURCE_DIR}/contactlist/fetchcontactlistjob.h ${CMAKE_CURRENT_SOURCE_DIR}/contactlist/fetchcontactlistjob.cpp )
papillon_moc( ${CMAKE_CURRENT_SOURCE_DIR}/contactlist/sharingservicebinding.h ${CMAKE_CURRENT_SOURCE_DIR}/contactlist/sharingservicebinding.cpp )

# Papillon
papillon_moc( ${CMAKE_CURRENT_SOURCE_DIR}/include/Papillon/client.h ${CMAKE_CURRENT_SOURCE_DIR}/client.cpp )
papillon_moc( ${CMAKE_CURRENT_SOURCE_DIR}/include/Papillon/connection.h ${CMAKE_CURRENT_SOURCE_DIR}/connection.cpp )
papillon_moc( ${CMAKE_CURRENT_SOURCE_DIR}/include/Papillon/contact.h ${CMAKE_CURRENT_SOURCE_DIR}/contact.cpp )
papillon_moc( ${CMAKE_CURRENT_SOURCE_DIR}/include/Papillon/contactlist.h ${CMAKE_CURRENT_SOURCE_DIR}/contactlist.cpp )
papillon_moc( ${CMAKE_CURRENT_SOURCE_DIR}/include/Papillon/messengercoreprotocol.h ${CMAKE_CURRENT_SOURCE_DIR}/messengercoreprotocol.cpp )
papillon_moc( ${CMAKE_CURRENT_SOURCE_DIR}/include/Papillon/papillonclientstream.h ${CMAKE_CURRENT_SOURCE_DIR}/papillonclientstream.cpp )
papillon_moc( ${CMAKE_CURRENT_SOURCE_DIR}/include/Papillon/qtbytestream.h ${CMAKE_CURRENT_SOURCE_DIR}/qtbytestream.cpp )
papillon_moc( ${CMAKE_CURRENT_SOURCE_DIR}/include/Papillon/qtconnector.h ${CMAKE_CURRENT_SOURCE_DIR}/qtconnector.cpp )
papillon_moc( ${CMAKE_CURRENT_SOURCE_DIR}/include/Papillon/task.h ${CMAKE_CURRENT_SOURCE_DIR}/task.cpp )
papillon_moc( ${CMAKE_CURRENT_SOURCE_DIR}/include/Papillon/usercontact.h ${CMAKE_CURRENT_SOURCE_DIR}/usercontact.cpp )
