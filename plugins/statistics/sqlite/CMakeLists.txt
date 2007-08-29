



########### next target ###############

set (sqlite_PART_SRCS
  attach.c
  auth.c
  btree.c
  build.c
  date.c
  delete.c
  encode.c
  expr.c
  func.c
  hash.c
  insert.c
  legacy.c
  lempar.c
  main.c
  opcodes.c
  os_mac.c
  os_unix.c
  os_win.c
  pager.c
  parse.c
  pragma.c
  printf.c
  random.c
  select.c
  shell.c
  table.c
  tokenize.c
  trigger.c
  update.c
  utf.c
  util.c
  vacuum.c
  vdbeapi.c
  vdbeaux.c
  vdbemem.c
  where.c )
  
kde4_add_library (sqlite ${sqlite_PART_SRCS})

install (TARGETS sqlite DESTINATION ${LIB_INSTALL_DIR})


########### install files ###############





