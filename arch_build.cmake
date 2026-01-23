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
  sc_internal_cfg_values.h
  sc_platform_cfg.h
  sc_perfids.h
  sc_msgids.h
  sc_msgid_values.h
)

generate_configfile_set(${SC_PLATFORM_CONFIG_FILE_LIST})
