set(CORE_SOURCES
  main.cpp
  graphics.cpp
  font.cpp
  log.cpp
  lang.cpp
  entity.cpp
  file_browser.cpp
  utils/string.cpp
  utils/time.cpp
  emulator/emulator.cpp
)

set(NETWORK_SOURCES
  network/dns.cpp
  network/cert.cpp
  network/http_get.cpp
  network/download_file.cpp
)

set(GUI_SOURCES
  gui/download_emulator_gui.cpp
  gui/main_gui.cpp
)

set(H8PKS_SOURCES ${CORE_SOURCES} ${NETWORK_SOURCES} ${GUI_SOURCES})

# ref https://stackoverflow.com/a/27630120
FUNCTION(PREPEND var prefix)
  SET(listVar "")
  FOREACH(f ${ARGN})
    LIST(APPEND listVar "${prefix}/${f}")
  ENDFOREACH(f)
  SET(${var} "${listVar}" PARENT_SCOPE)
ENDFUNCTION(PREPEND)

PREPEND(H8PKS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR} ${H8PKS_SOURCES})
set(H8PKS_SOURCES ${H8PKS_SOURCES} PARENT_SCOPE)
