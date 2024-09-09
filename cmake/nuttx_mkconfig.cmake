# ##############################################################################
# cmake/nuttx_mkconfig.cmake
#
# SPDX-License-Identifier: Apache-2.0
#
# Licensed to the Apache Software Foundation (ASF) under one or more contributor
# license agreements.  See the NOTICE file distributed with this work for
# additional information regarding copyright ownership.  The ASF licenses this
# file to you under the Apache License, Version 2.0 (the "License"); you may not
# use this file except in compliance with the License.  You may obtain a copy of
# the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
# WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  See the
# License for the specific language governing permissions and limitations under
# the License.
#
# ##############################################################################

if(NOT EXISTS ${CMAKE_BINARY_DIR}/.config)
  return()
endif()

if(NOT EXISTS ${CMAKE_BINARY_DIR}/.config.prev)
  execute_process(
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_BINARY_DIR}/.config
            ${CMAKE_BINARY_DIR}/.config.prev
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR})
endif()

execute_process(
  COMMAND ${CMAKE_COMMAND} -E compare_files ${CMAKE_BINARY_DIR}/.config
          ${CMAKE_BINARY_DIR}/.config.prev
  RESULT_VARIABLE COMPARE_RESULT
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR})

set(CONFIG_H ${CMAKE_BINARY_DIR}/include/nuttx/config.h)
if(COMPARE_RESULT EQUAL 0 AND EXISTS ${CONFIG_H})
  return()
endif()

set(BASE_DEFCONFIG "${NUTTX_BOARD}/${NUTTX_CONFIG}")
execute_process(
  COMMAND ${CMAKE_COMMAND} -E compare_files ${CMAKE_BINARY_DIR}/.config
          ${CMAKE_BINARY_DIR}/.config.orig RESULT_VARIABLE COMPARE_RESULT)
if(COMPARE_RESULT)
  string(APPEND BASE_DEFCONFIG "-dirty")
endif()

set(DEQUOTELIST
    # NuttX
    "CONFIG_DEBUG_OPTLEVEL" # Custom debug level
    "CONFIG_EXECFUNCS_NSYMBOLS_VAR" # Variable holding number of symbols in the
                                    # table
    "CONFIG_EXECFUNCS_SYMTAB_ARRAY" # Symbol table array used by exec[l|v]
    "CONFIG_INIT_ARGS" # Argument list of entry point
    "CONFIG_INIT_SYMTAB" # Global symbol table
    "CONFIG_INIT_NEXPORTS" # Global symbol table size
    "CONFIG_INIT_ENTRYPOINT" # Name of entry point function
    "CONFIG_MODLIB_SYMTAB_ARRAY" # Symbol table array used by modlib functions
    "CONFIG_MODLIB_NSYMBOLS_VAR" # Variable holding number of symbols in the
                                 # table
    "CONFIG_PASS1_BUILDIR" # Pass1 build directory
    "CONFIG_PASS1_TARGET" # Pass1 build target
    "CONFIG_PASS1_OBJECT" # Pass1 build object
    "CONFIG_TTY_LAUNCH_ENTRYPOINT" # Name of entry point from tty launch
    "CONFIG_TTY_LAUNCH_ARGS" # Argument list of entry point from tty launch
    # NxWidgets/NxWM
    "CONFIG_NXWM_BACKGROUND_IMAGE" # Name of bitmap image class
    "CONFIG_NXWM_CALIBRATION_ICON" # Name of bitmap image class
    "CONFIG_NXWM_HEXCALCULATOR_ICON" # Name of bitmap image class
    "CONFIG_NXWM_MINIMIZE_BITMAP" # Name of bitmap image class
    "CONFIG_NXWM_NXTERM_ICON" # Name of bitmap image class
    "CONFIG_NXWM_STARTWINDOW_ICON" # Name of bitmap image class
    "CONFIG_NXWM_STOP_BITMAP" # Name of bitmap image class
    # apps/ definitions
    "CONFIG_NSH_SYMTAB_ARRAYNAME" # Symbol table array name
    "CONFIG_NSH_SYMTAB_COUNTNAME" # Name of the variable holding the
    # number of symbols
)

file(WRITE ${CONFIG_H} "/* config.h -- Autogenerated! Do not edit. */\n\n")
file(APPEND ${CONFIG_H} "#ifndef __INCLUDE_NUTTX_CONFIG_H\n")
file(APPEND ${CONFIG_H} "#define __INCLUDE_NUTTX_CONFIG_H\n\n")
file(APPEND ${CONFIG_H}
     "/* Used to represent the values of tristate options */\n\n")
file(APPEND ${CONFIG_H} "#define CONFIG_y 1\n")
file(APPEND ${CONFIG_H} "#define CONFIG_m 2\n\n")
file(APPEND ${CONFIG_H}
     "/* General Definitions ***********************************/\n")
file(APPEND ${CONFIG_H} "#define CONFIG_BASE_DEFCONFIG \"${BASE_DEFCONFIG}\"\n")

file(STRINGS ${CMAKE_BINARY_DIR}/.config ConfigContents)
encode_brackets(ConfigContents)
foreach(NameAndValue ${ConfigContents})
  decode_brackets(NameAndValue)
  encode_semicolon(NameAndValue)
  string(REGEX REPLACE "^[ ]+" "" NameAndValue ${NameAndValue})
  string(REGEX MATCH "^CONFIG[^=]+" NAME ${NameAndValue})
  # skip BASE_DEFCONFIG here as it is handled above
  if("${NAME}" STREQUAL "CONFIG_BASE_DEFCONFIG")
    continue()
  endif()
  string(REPLACE "${NAME}=" "" VALUE ${NameAndValue})
  if(NAME AND NOT "${VALUE}" STREQUAL "")
    if(${VALUE} STREQUAL "y")
      file(APPEND ${CONFIG_H} "#define ${NAME} 1\n")
    elseif(${VALUE} STREQUAL "m")
      file(APPEND ${CONFIG_H} "#define ${NAME} 2\n")
    elseif(${VALUE} STREQUAL "n")
      file(APPEND ${CONFIG_H} "#undef ${NAME}\n")
    else()
      foreach(dequote ${DEQUOTELIST})
        if("${NAME}" STREQUAL "${dequote}")
          if(NOT "${VALUE}" STREQUAL "\"\"")
            string(REGEX REPLACE "^\"(.*)\"$" "\\1" VALUE "${VALUE}")
            string(REGEX REPLACE "\\\\\\\"" "\"" VALUE "${VALUE}")
          else()
            set(VALUE)
            file(APPEND ${CONFIG_H} "#undef ${NAME}\n")
          endif()
          break()
        endif()
      endforeach()
      if(NOT "${VALUE}" STREQUAL "")
        decode_semicolon(VALUE)
        file(APPEND ${CONFIG_H} "#define ${NAME} ${VALUE}\n")
      endif()
    endif()
  endif()
endforeach()

file(APPEND ${CONFIG_H}
     "\n/* Sanity Checks *****************************************/\n\n")
file(APPEND ${CONFIG_H}
     "/* If the end of RAM is not specified then it is assumed to be\n")
file(APPEND ${CONFIG_H} " * the beginning of RAM plus the RAM size.\n")
file(APPEND ${CONFIG_H} " */\n\n")
file(APPEND ${CONFIG_H} "#ifndef CONFIG_RAM_END\n")
file(APPEND ${CONFIG_H}
     "#  define CONFIG_RAM_END (CONFIG_RAM_START+CONFIG_RAM_SIZE)\n")
file(APPEND ${CONFIG_H} "#endif\n\n")
file(APPEND ${CONFIG_H} "#ifndef CONFIG_RAM_VEND\n")
file(APPEND ${CONFIG_H}
     "#  define CONFIG_RAM_VEND (CONFIG_RAM_VSTART+CONFIG_RAM_SIZE)\n")
file(APPEND ${CONFIG_H} "#endif\n\n")
file(APPEND ${CONFIG_H}
     "/* If the end of FLASH is not specified then it is assumed to be\n")
file(APPEND ${CONFIG_H} " * the beginning of FLASH plus the FLASH size.\n")
file(APPEND ${CONFIG_H} " */\n\n")
file(APPEND ${CONFIG_H} "#ifndef CONFIG_FLASH_END\n")
file(APPEND ${CONFIG_H}
     "#  define CONFIG_FLASH_END (CONFIG_FLASH_START+CONFIG_FLASH_SIZE)\n")
file(APPEND ${CONFIG_H} "#endif\n\n")
file(APPEND ${CONFIG_H} "#endif /* __INCLUDE_NUTTX_CONFIG_H */\n")
