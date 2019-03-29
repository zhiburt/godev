# seaboltConfig.cmake
# -------------------
#
# seabolt cmake module.
# This module sets the following variables in your project:
#
# ::
#
#   seabolt_FOUND - true if seabolt found on the system
#   seabolt_VERSION - seabolt version in format Major.Minor.Release
#
#
# Exported targets:
#
# ::
#
# If message is found, this module defines the following :prop_tgt:`IMPORTED`
# targets. ::
#   seabolt17::seabolt-shared - the seabolt shared library with header & defs attached.
#   seabolt17::seabolt-static - the seabolt static library with header & defs attached.
#
#
# Suggested usage:
#
# ::
#
#   find_package(seabolt17)
#   find_package(seabolt17 1.7.0 CONFIG REQUIRED)
#
#
# The following variables can be set to guide the search for this package:
#
# ::
#
#   seabolt17_DIR - CMake variable, set to directory containing this Config file
#   CMAKE_PREFIX_PATH - CMake variable, set to root directory of this package
#   PATH - environment variable, set to bin directory of this package


####### Expanded from @PACKAGE_INIT@ by configure_package_config_file() #######
####### Any changes to this file will be overwritten by the next CMake run ####
####### The input file was seaboltConfig.cmake.in                            ########

get_filename_component(PACKAGE_PREFIX_DIR "${CMAKE_CURRENT_LIST_DIR}/../../../" ABSOLUTE)

macro(set_and_check _var _file)
  set(${_var} "${_file}")
  if(NOT EXISTS "${_file}")
    message(FATAL_ERROR "File or directory ${_file} referenced by variable ${_var} does not exist !")
  endif()
endmacro()

macro(check_required_components _NAME)
  foreach(comp ${${_NAME}_FIND_COMPONENTS})
    if(NOT ${_NAME}_${comp}_FOUND)
      if(${_NAME}_FIND_REQUIRED_${comp})
        set(${_NAME}_FOUND FALSE)
      endif()
    endif()
  endforeach()
endmacro()

####################################################################################

include("${CMAKE_CURRENT_LIST_DIR}/seabolt17Targets.cmake")
check_required_components(
  "seabolt-shared"
  )
