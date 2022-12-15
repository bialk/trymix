# Get all propreties that cmake supports
if(NOT CMAKE_PROPERTY_LIST)
    execute_process(COMMAND cmake --help-property-list OUTPUT_VARIABLE CMAKE_PROPERTY_LIST)

    # Convert command output into a CMake list
    string(REGEX REPLACE ";" "\\\\;" CMAKE_PROPERTY_LIST "${CMAKE_PROPERTY_LIST}")
    string(REGEX REPLACE "\n" ";" CMAKE_PROPERTY_LIST "${CMAKE_PROPERTY_LIST}")

    list(REMOVE_DUPLICATES CMAKE_PROPERTY_LIST)
endif()

function(print_target_properties target)
    if(NOT TARGET ${target})
      message(STATUS "There is no target named '${target}'")
      return()
    endif()

    foreach(property ${CMAKE_PROPERTY_LIST})
        string(REPLACE "<CONFIG>" "${CMAKE_BUILD_TYPE}" property ${property})
        get_property(was_set TARGET ${target} PROPERTY ${property} SET)
        if(was_set)
            get_target_property(value ${target} ${property})
            message("${target} ${property} = ${value}")
        endif()
    endforeach()
endfunction()

function(add_target_dll tgt)
    get_target_property(extra_dll_path_in ${tgt} LOCATION_${CMAKE_BUILD_TYPE})

    add_custom_command(
      TARGET omi_triang
      POST_BUILD
      COMMAND "${CMAKE_COMMAND}" -E copy_if_different  "${extra_dll_path_in}" "$<TARGET_FILE_DIR:omi_triang>"
      )
endfunction()

function(add_dll_by_path tgt)
    add_custom_command(
      TARGET omi_triang
      POST_BUILD
      COMMAND "${CMAKE_COMMAND}" -E copy_if_different  "${GMP_DLL}" "$<TARGET_FILE_DIR:omi_triang>"
      )
endfunction()
