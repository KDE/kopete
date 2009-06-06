
option(WITH_history "Enable Kopete history plugin" ON)
if (WITH_history)
  add_subdirectory( history )
endif (WITH_history)
