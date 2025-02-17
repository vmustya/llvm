
# The find_package changes these variables. This leaves the build in an odd
# state. Calling cmake a second time tries to write site config information in
# the system's libc++. Restoring these setting after testing fixes this issue.
set(LLVM_DIR_SAVE ${LLVM_DIR})
set(Clang_DIR_SAVE ${Clang_DIR})

# TODO LLVM 17 prefer to use teh stable release again instead of ToT.
# libc++ normally prefers the use the last stable release instead of the ToT.
# For modules we need ToT since they are still under heavy development. So
# temporary the ToT version is preferred.
find_package(Clang 17)
if (NOT Clang_FOUND)
  find_package(Clang 16)
endif()

set(SOURCES
    abi_tag_on_virtual.cpp
    hide_from_abi.cpp
    proper_version_checks.cpp
    qualify_declval.cpp
    robust_against_adl.cpp
    uglify_attributes.cpp

    libcpp_module.cpp
   )

if(NOT Clang_FOUND)
  message(STATUS "Could not find a suitable version of the Clang development package;
                  custom libc++ clang-tidy checks will not be available.")
  return()
endif()

set(LLVM_DIR "${LLVM_DIR_SAVE}" CACHE PATH "The directory containing a CMake configuration file for LLVM." FORCE)
set(Clang_DIR "${Clang_DIR_SAVE}" CACHE PATH "The directory containing a CMake configuration file for Clang." FORCE)

message(STATUS "Found system-installed LLVM ${LLVM_PACKAGE_VERSION} with headers in ${LLVM_INCLUDE_DIRS}")

set(CMAKE_CXX_STANDARD 20)

# Link only against clangTidy itself, not anything that clangTidy uses; otherwise we run setup code multiple times
# which results in clang-tidy crashing
set_target_properties(clangTidy PROPERTIES INTERFACE_LINK_LIBRARIES "")
# ClangTargets.cmake doesn't set the include paths, so we have to do it
target_include_directories(clangTidy INTERFACE
                           ${CLANG_INCLUDE_DIRS}
                           ${LLVM_INCLUDE_DIRS}
                          )
target_compile_options(clangTidy INTERFACE
                       -fno-rtti
                       -fno-sanitize=address,hwaddress,undefined,thread,leak # ignore any sanitizers
                      )

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
  target_compile_options(clangTidy INTERFACE
                         -fno-sanitize=memory,dataflow
                        )
endif()

add_library(cxx-tidy MODULE ${SOURCES})
target_link_libraries(cxx-tidy clangTidy)

set_target_properties(cxx-tidy PROPERTIES
                      CXX_STANDARD 20
                      CXX_STANDARD_REQUIRED YES
                      CXX_EXTENSIONS NO)

set_target_properties(cxx-tidy PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
set(CMAKE_SHARED_MODULE_SUFFIX_CXX .plugin) # Use a portable suffix to simplify how we can find it from Lit
