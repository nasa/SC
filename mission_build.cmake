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
  sc_fcncodes.h
  sc_interface_cfg.h
  sc_mission_cfg.h
  sc_msgdefs.h
  sc_msg.h
  sc_msgstruct.h
  sc_tbldefs.h
  sc_tbl.h
  sc_tblstruct.h
)

if (CFE_EDS_ENABLED_BUILD)

  # In an EDS-based build, these files come generated from the EDS tool
  set(SC_CFGFILE_SRC_sc_interface_cfg "sc_eds_designparameters.h")
  set(SC_CFGFILE_SRC_sc_fcncodes      "sc_eds_cc.h")
  set(SC_CFGFILE_SRC_sc_msgstruct     "sc_eds_typedefs.h")
  set(SC_CFGFILE_SRC_sc_tblstruct     "sc_eds_typedefs.h")

endif(CFE_EDS_ENABLED_BUILD)

# Create wrappers around the all the config header files
# This makes them individually overridable by the missions, without modifying
# the distribution default copies
foreach(SC_CFGFILE ${SC_MISSION_CONFIG_FILE_LIST})
  get_filename_component(CFGKEY "${SC_CFGFILE}" NAME_WE)
  if (DEFINED SC_CFGFILE_SRC_${CFGKEY})
    set(DEFAULT_SOURCE GENERATED_FILE "${SC_CFGFILE_SRC_${CFGKEY}}")
  else()
    set(DEFAULT_SOURCE FALLBACK_FILE "${CMAKE_CURRENT_LIST_DIR}/config/default_${SC_CFGFILE}")
  endif()
  generate_config_includefile(
    FILE_NAME           "${SC_CFGFILE}"
    ${DEFAULT_SOURCE}
  )
endforeach()
