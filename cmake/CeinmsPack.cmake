
if (WIN32)

find_file(xerces_dll NAMES "xerces-c_3_1_vc100.dll" PATHS ENV PATH)
if (xerces_dll)
    install(PROGRAMS ${xerces_dll} DESTINATION ${CMAKE_INSTALL_BINDIR} COMPONENT ThirdParty)
endif()
#list(APPEND CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS ${xerces_dll})
include(InstallRequiredSystemLibraries)
endif()
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Calibrated EMG-Informed NeuroMusculoSkeletal modeling")
set(CPACK_PACKAGE_VENDOR "The CEINMS team")
set(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_SOURCE_DIR}/README.md")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/LICENSE.txt")
set(CPACK_PACKAGE_VERSION_MAJOR ${CEINMS_MAJOR_VERSION})
set(CPACK_PACKAGE_VERSION_MINOR ${CEINMS_MINOR_VERSION})
set(CPACK_PACKAGE_VERSION_PATCH ${CEINMS_PATCH_VERSION})
set(CPACK_PACKAGE_INSTALL_DIRECTORY "CEINMS ${CEINMS_MAJOR_VERSION}.${CEINMS_MINOR_VERSION}")
#set(CPACK_PACKAGE_ICON "${CMAKE_SOURCE_DIR}/docs/static/ceinms-favicon.ico")

set(CPACK_COMPONENTS_ALL Applications ThirdParty Unspecified)
set(CPACK_COMPONENT_UNSPECIFIED_DISPLAY_NAME "Additional files")
set(CPACK_COMPONENT_UNSPECIFIED_GROUP "Application Bundle")
set(CPACK_COMPONENT_THIRDPARTY_DISPLAY_NAME "Third parties")
set(CPACK_COMPONENT_THIRDPARTY_GROUP "Application Bundle")
set(CPACK_COMPONENT_THIRDPARTY_DEPENDS Unspecified)
set(CPACK_COMPONENT_APPLICATIONS_DISPLAY_NAME "CEINMS Application")
set(CPACK_COMPONENT_APPLICATIONS_GROUP "Application Bundle")
set(CPACK_COMPONENT_APPLICATIONS_DEPENDS ThirdParty)


set(CPACK_PACKAGE_CONTACT "Monica Reggiani <monica.reggiani@gmail.com>")
set(CPACK_DEB_COMPONENT_INSTALL ON)
set(CPACK_RPM_COMPONENT_INSTALL ON)

# On Windows replace slashes in file name with escaped backslashes, as NSIS is picky about Unix paths
# ans using file(TO_NATIVE_PATH) is not enough, as \s are not escaped
if(WIN32)
    string(REPLACE "/" "\\\\" CPACK_PACKAGE_ICON "${CPACK_PACKAGE_ICON}")
    # Produce an installer for 64bit windows
    if("${CMAKE_GENERATOR}" MATCHES "Win64")
        set(CMAKE_CL_64 TRUE)
        set(CPACK_WIX_SIZEOF_VOID_P 8)
    endif()
endif()

if(CMAKE_CL_64)
    set(CPACK_NSIS_INSTALL_ROOT "$PROGRAMFILES64")
    set(CPACK_NSIS_PACKAGE_NAME "${CPACK_PACKAGE_INSTALL_DIRECTORY} (Win64)")
    set(CPACK_PACKAGE_INSTALL_REGISTRY_KEY "${CPACK_PACKAGE_NAME}
    ${CPACK_PACKAGE_VERSION} (Win64)")
else()
    set(CPACK_NSIS_INSTALL_ROOT "$PROGRAMFILES")
    set(CPACK_NSIS_PACKAGE_NAME "${CPACK_PACKAGE_INSTALL_DIRECTORY}")
    set(CPACK_PACKAGE_INSTALL_REGISTRY_KEY "${CPACK_PACKAGE_NAME}
            ${CPACK_PACKAGE_VERSION}")
endif()

set(CPACK_NSIS_MODIFY_PATH ON)
set(CPACK_NSIS_ENABLE_UNINSTALL_BEFORE_INSTALL ON)

include(CPack)
