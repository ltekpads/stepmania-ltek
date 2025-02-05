set(SQLITE_SRC
  "sqlite3/sqlite3.c"
)

set(SQLITE_HPP
  "sqlite3/sqlite3.h"
)

source_group("" FILES ${SQLITE_SRC})
source_group("" FILES ${SQLITE_HPP})

add_library("sqlite3" ${SQLITE_SRC} ${SQLITE_HPP})

set_property(TARGET "sqlite3" PROPERTY FOLDER "External Libraries")
