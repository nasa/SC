###########################################################
#
# SC App platform build setup
#
# This file is evaluated as part of the "prepare" stage
# and can be used to set up prerequisites for the build,
# such as generating header files
#
###########################################################

# The list of header files that control the SC configuration
set(SC_PLATFORM_CONFIG_FILE_LIST
  sc_internal_cfg.h
  sc_msgids.h
  sc_perfids.h
  sc_platform_cfg.h
)

if (CFE_EDS_ENABLED_BUILD)

  # In an EDS-based build, msgids come generated from the EDS tool
  set(SC_CFGFILE_SRC_sc_msgids "${CMAKE_CURRENT_LIST_DIR}/config/sc_eds_msg_topicids.h")

endif(CFE_EDS_ENABLED_BUILD)

# Create wrappers around the all the config header files
# This makes them individually overridable by the missions, without modifying
# the distribution default copies
foreach(SC_CFGFILE ${SC_PLATFORM_CONFIG_FILE_LIST})
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
