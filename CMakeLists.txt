# mahi::com CMakeLists.txt
# Evan Pezent (epezent@rice.edu)
# Updated: 2/2020

cmake_minimum_required(VERSION 3.13.0)

#===============================================================================
# USER OPTIONS
#===============================================================================

# General
option(MAHI_COM_EXAMPLES "Turn ON to build example executable(s)" OFF)
option(MAHI_COM_OUTPUT_LOGS "Turn ON to output log messages" ON)

#===============================================================================
# FRONT MATTER
#===============================================================================

# create the project
project(mahi-com VERSION 1.0.0 LANGUAGES C CXX)

# add mahi-util to the project
include(FetchContent)
# add_subdirectory("mahi-gui")
FetchContent_Declare(mahi-util GIT_REPOSITORY https://github.com/mahilab/mahi-util.git)
FetchContent_MakeAvailable(mahi-util)

# FetchContent_Declare(mahi-util
#                     GIT_REPOSITORY https://github.com/mahilab/mahi-util.git)
# FetchContent_MakeAvailable(mahi-util)

# tell user they can't have shared version if they try
if (BUILD_SHARED_LIBS)
    message(FATAL_ERROR "mahi::com does not support shared libaries")
endif()

# add ./cmake to CMake module path so our .cmake files can be found
list(INSERT CMAKE_MODULE_PATH 0 ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
include(GNUInstallDirs) # defines conventional GNU isntallation directories

# Enable IDE folders and set them for predefined CMake projects
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set_property(GLOBAL PROPERTY PREDEFINED_TARGETS_FOLDER "CMake")

if (CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -Wall -O3") # ALL WARNINGS
elseif(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
    set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} /MP")    # MULTICORE BUILDS
endif()

#===============================================================================
# CREATE LIBRARY
#===============================================================================

add_library(com "")
add_library(mahi::com ALIAS com)
set_target_properties(com PROPERTIES DEBUG_POSTFIX -d)
target_compile_features(com PUBLIC cxx_std_11)
install(TARGETS com EXPORT mahi-com-targets LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})
set_target_properties(com PROPERTIES OUTPUT_NAME "mahi-com")

# defines
target_compile_definitions(com PUBLIC MAHI_COM) # for compatibility checks
if (MAHI_COM_OUTPUT_LOGS)    
    target_compile_definitions(com PUBLIC MAHI_COM_OUTPUT_LOGS)
endif()

# add source files
add_subdirectory(src/Mahi/Com)

# add include files
file(GLOB_RECURSE MAHI_COM_HEADERS "include/*.hpp" "include/*.h" "include/*.inl")
target_sources(com PRIVATE ${MAHI_COM_HEADERS}) # for intellisense
target_include_directories(com
    PUBLIC
        $<INSTALL_INTERFACE:include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src # DIFFERENT THEN ADD_SUBDIRECTORY ABOVE?
)

#===============================================================================
# WINDOWS ONLY
#===============================================================================

if(WIN32)

    target_compile_definitions(com
        PUBLIC
            -D_CRT_SECURE_NO_WARNINGS          # remove secure warnings (e.g sprinft_s)
            -DNOMINMAX                         # remove min/max macros
            -D_WINSOCK_DEPRECATED_NO_WARNINGS  # remove winsock deprecated warnings
    )

endif(WIN32)

#===============================================================================
# NI LINUX REAL-TIME ONLY
#===============================================================================

if (NI_LRT)
    set(CMAKE_BUILD_WITH_INSTALL_RPATH ON)
    target_link_libraries(com
        PRIVATE
            dl # Dynamic Linking Library
            rt # Realtime Extensions library
    )
    if (CMAKE_SYSTEM_PROCESSOR MATCHES ARM)
        add_subdirectory(src/mahi/com/NI/MyRio)
    endif()
endif()

#===============================================================================
# EXAMPLE EXECUTABLES
#===============================================================================

if(MAHI_COM_EXAMPLES)
    message("Building mahi::com examples")
    add_subdirectory(examples)
endif()

#===============================================================================
# INSTALL
#===============================================================================

if (MSVC)
    if (CMAKE_SIZEOF_VOID_P EQUAL 8)
        set(MAHI_INSTALL_POSTFIX "") # 64 bit Windows is default
    else()
        set(MAHI_INSTALL_POSTFIX "-x86")
    endif()
elseif(NI_LRT AND CMAKE_SYSTEM_PROCESSOR MATCHES ARM)
    set(MAHI_INSTALL_POSTFIX "-nilrt-arm")
elseif(NI_LRT AND CMAKE_SYSTEM_PROCESSOR MATCHES ARM)
    set(MAHI_INSTALL_POSTFIX "-nilrt-x64")
endif()

set(CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}${MAHI_INSTALL_POSTFIX}")

# install headers
install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

# set where we want to install our config
set(INSTALL_CONFIGDIR ${CMAKE_INSTALL_LIBDIR}/cmake/mahi-com)

# export the targets to a script
install(EXPORT mahi-com-targets
	FILE
		mahi-com-targets.cmake
	NAMESPACE
		mahi::
	DESTINATION
		${INSTALL_CONFIGDIR}
)

# include helper functions for creating config files that can be included by other projects to find and use a package
include(CMakePackageConfigHelpers)

# generate a package configuration file and a package version file
configure_package_config_file(${CMAKE_CURRENT_LIST_DIR}/cmake/mahi-com-config.cmake.in
	${CMAKE_CURRENT_BINARY_DIR}/mahi-com-config.cmake
	INSTALL_DESTINATION ${INSTALL_CONFIGDIR}
)

write_basic_package_version_file(
	${CMAKE_CURRENT_BINARY_DIR}/mahi-com-config-version.cmake
	VERSION ${PROJECT_VERSION}
	COMPATIBILITY AnyNewerVersion
)

# install the config and configversion modules
install(FILES
	${CMAKE_CURRENT_BINARY_DIR}/mahi-com-config.cmake
	${CMAKE_CURRENT_BINARY_DIR}/mahi-com-config-version.cmake
	DESTINATION ${INSTALL_CONFIGDIR}
)

# export from the build tree
export(EXPORT mahi-com-targets
	   NAMESPACE mahi::
	   FILE ${CMAKE_CURRENT_BINARY_DIR}/mahi-com-targets.cmake)

