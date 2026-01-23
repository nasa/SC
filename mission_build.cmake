###########################################################
#
# SC App mission build setup
#
# This file is evaluated as part of the "prepare" stage
# and can be used to set up prerequisites for the build,
# such as generating header files
#
###########################################################

# Add stand alone documentation
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/docs/dox_src ${MISSION_BINARY_DIR}/docs/sc-usersguide)

# The list of header files that control the SC configuration
set(SC_MISSION_CONFIG_FILE_LIST
  sc_extern_typedefs.h
  sc_fcncode_values.h
  sc_interface_cfg_values.h
  sc_mission_cfg.h
  sc_msgdefs.h
  sc_msg.h
  sc_msgstruct.h
  sc_tbldefs.h
  sc_tbl.h
  sc_tblstruct.h
  sc_topicid_values.h
)

generate_configfile_set(${SC_MISSION_CONFIG_FILE_LIST})
