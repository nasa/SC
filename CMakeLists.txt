project(CFS_SC C)

set(APP_SRC_FILES
  fsw/src/sc_rtsrq.c
  fsw/src/sc_utils.c
  fsw/src/sc_app.c
  fsw/src/sc_atsrq.c
  fsw/src/sc_state.c
  fsw/src/sc_loads.c
  fsw/src/sc_cmds.c
  fsw/src/sc_dispatch.c
)

# Create the app module
add_cfe_app(sc ${APP_SRC_FILES})

# This permits direct access to public headers in the fsw/inc directory
target_include_directories(sc PUBLIC fsw/inc)

set(APP_TABLE_FILES
  fsw/tables/sc_rts001.c
  fsw/tables/sc_ats1.c
  fsw/tables/sc_rts002.c
)

add_cfe_tables(sc ${APP_TABLE_FILES})

# If UT is enabled, then add the tests from the subdirectory
# Note that this is an app, and therefore does not provide
# stub functions, as other entities would not typically make
# direct function calls into this application.
if(ENABLE_UNIT_TESTS)
  add_subdirectory(unit-test)
endif()
