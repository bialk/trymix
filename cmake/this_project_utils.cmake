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

function(add_dll_by_path path)
    add_custom_command(
      TARGET omi_triang
      POST_BUILD
      COMMAND "${CMAKE_COMMAND}" -E copy_if_different  "${path}" "$<TARGET_FILE_DIR:omi_triang>"
      )
endfunction()

# example:
# find_path_by_regex(VARIABLE_TO_RETURN_FOUND_PATH "boost_*[0-9]_*[0-9]_*[0-9]"
# PATHS ${LIST_OF_PATHS} )
function(find_path_by_regex var_out regex_in)
    foreach(item ${ARGN})
        if(item STREQUAL "PATHS")
            set(itemmode "PATHS")
            continue()
        endif()
        if(itemmode STREQUAL "PATHS")
           FILE(GLOB found_path "${item}/${regex_in}")
           message(STATUS found_path:${found_path})
           if(found_path)
               set(${var_out} ${found_path} PARENT_SCOPE)
               return()
           endif()
        endif()
    endforeach()
endfunction()

