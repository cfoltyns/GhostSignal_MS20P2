# ============================================================
# CPack post-install cleanup
# ============================================================
#
# CPack runs `cmake --install`, which also installs the vendored
# JUCE framework files (headers, cmake modules, juceaide tool)
# into the staging directory because JUCE registers its own
# unconditional install() rules. Those files are not part of the
# shipped product, so we purge them from the staging prefix here.
#
# This script is executed by CPack via `cmake -P` after staging
# with CMAKE_INSTALL_PREFIX pointing at the staging directory.

if(NOT DEFINED CMAKE_INSTALL_PREFIX)
    message(FATAL_ERROR "CMAKE_INSTALL_PREFIX not defined")
endif()

# CPACK_TEMPORARY_DIRECTORY points at the exact staging directory
# where CPack rolled the install; fall back to CMAKE_INSTALL_PREFIX.
set(_staging "${CMAKE_INSTALL_PREFIX}")
if(DEFINED CPACK_TEMPORARY_DIRECTORY AND CPACK_TEMPORARY_DIRECTORY)
    set(_staging "${CPACK_TEMPORARY_DIRECTORY}")
endif()
string(STRIP "${_staging}" _staging)
message(STATUS "CPack cleanup: staging = ${_staging}")

file(GLOB _juce_lib "${_staging}/lib/cmake/JUCE-*")
file(GLOB _juce_include "${_staging}/include/JUCE-*")
file(GLOB _juce_bin "${_staging}/bin/JUCE-*")

foreach(_entry IN LISTS _juce_lib _juce_include _juce_bin)
    if(EXISTS "${_entry}")
        message(STATUS "CPack cleanup: removing ${_entry}")
        file(REMOVE_RECURSE "${_entry}")
    endif()
endforeach()

# Also drop the empty JUCE-tainted parent dirs if they remain empty.
foreach(_dir "${_staging}/lib/cmake" "${_staging}/include" "${_staging}/bin" "${_staging}/lib")
    if(EXISTS "${_dir}")
        file(GLOB _stuff "${_dir}/*")
        list(LENGTH _stuff _n)
        if(_n EQUAL 0)
            file(REMOVE_RECURSE "${_dir}")
        endif()
    endif()
endforeach()