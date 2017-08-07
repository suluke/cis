cmake_minimum_required (VERSION 3.0.0)

##
## PROJECT
## name and version
##
project (cis_hpp VERSION 1.0.0 LANGUAGES CXX)

##
## OPTIONS
##
option(CIS_BuildTests "Build the unit tests" ON)

##
## CONFIGURATION
##
set(CIS_TARGET_NAME               ${PROJECT_NAME})
set(CIS_SOURCE_DIR                ${PROJECT_SOURCE_DIR})
set(CIS_CONFIG_INSTALL_DIR        "lib/cmake/${PROJECT_NAME}")
set(CIS_INCLUDE_INSTALL_DIR       "include")
set(CIS_HEADER_INSTALL_DIR        "${CIS_INCLUDE_INSTALL_DIR}/nlohmann")
set(CIS_TARGETS_EXPORT_NAME       "${PROJECT_NAME}Targets")
set(CIS_CMAKE_CONFIG_TEMPLATE     "cmake/config.cmake.in")
set(CIS_CMAKE_CONFIG_DIR          "${CMAKE_CURRENT_BINARY_DIR}/cmake_config")
set(CIS_CMAKE_VERSION_CONFIG_FILE "${CIS_CMAKE_CONFIG_DIR}/${PROJECT_NAME}ConfigVersion.cmake")
set(CIS_CMAKE_PROJECT_CONFIG_FILE "${CIS_CMAKE_CONFIG_DIR}/${PROJECT_NAME}Config.cmake")


##
## TARGET
## create target and add include path
##
add_library(${CIS_TARGET_NAME} INTERFACE)
target_include_directories(
  ${CIS_TARGET_NAME}
  INTERFACE $<INSTALL_INTERFACE:include/
)

##
## TESTS
## create and configure the unit test target
##
if(CIS_BuildTests)
  enable_testing()
  include_directories(${CIS_SOURCE_DIR})
  add_subdirectory(test)
endif()


##
## INSTALL
## install header files, generate and install cmake config files for find_package()
##
include(CMakePackageConfigHelpers)
write_basic_package_version_file(
  ${CIS_CMAKE_VERSION_CONFIG_FILE} COMPATIBILITY SameMajorVersion
)
configure_package_config_file(
  ${CIS_CMAKE_CONFIG_TEMPLATE}
  ${CIS_CMAKE_PROJECT_CONFIG_FILE}
  INSTALL_DESTINATION ${CIS_CONFIG_INSTALL_DIR}
)
install(
  DIRECTORY ${CIS_SOURCE_DIR}
  DESTINATION ${CIS_HEADER_INSTALL_DIR}
)
install(
  FILES ${CIS_CMAKE_PROJECT_CONFIG_FILE} ${CIS_CMAKE_VERSION_CONFIG_FILE}
  DESTINATION ${CIS_CONFIG_INSTALL_DIR}
)
install(
  TARGETS ${CIS_TARGET_NAME}
  EXPORT ${CIS_TARGETS_EXPORT_NAME}
  INCLUDES DESTINATION ${CIS_INCLUDE_INSTALL_DIR}
)
install(
  EXPORT ${CIS_TARGETS_EXPORT_NAME}
  DESTINATION ${CIS_CONFIG_INSTALL_DIR}
)

##
## SCRIPTS
## custom targets to facilitate development, such as
## * run clang-format on cis.hpp
##
add_custom_target(cis-update-format
  clang-format -i -style=LLVM ./cis.hpp
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  COMMENT "Format all source files. Requires clang-format to be installed"
)
