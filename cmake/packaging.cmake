# reference: https://cmake.org/cmake/help/book/mastering-cmake/chapter/Packaging%20With%20CPack.html

set (CPACK_PACKAGE_NAME ${PROJECT_NAME})
set (CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set (CPACK_PACKAGE_VENDOR "msh31")
set (CPACK_PACKAGE_DESCRIPTION_SUMMARY ${PROJECT_DESCRIPTION})

set (CPACK_PACKAGE_EXECUTABLES "savemanager" "SaveManager" )
set (CPACK_PACKAGE_INSTALL_DIRECTORY "SaveManager")

if(WIN32)
    set (CPACK_GENERATOR "NSIS")
    set (CPACK_NSIS_MUI_ICON "${CMAKE_CURRENT_SOURCE_DIR}/assets/app_icon/smicon.ico")
    set (CPACK_NSIS_MUI_UNIICON "${CMAKE_CURRENT_SOURCE_DIR}/assets/app_icon/smicon.ico")
    set (CPACK_NSIS_INSTALLED_ICON_NAME "bin\\savemanager.exe")
    include (InstallRequiredSystemLibraries)
elseif(APPLE)
    set (CPACK_GENERATOR "DragNDrop")
    # CPACK_BUNDLE_* vars here
else()
    set (CPACK_GENERATOR "TGZ")
endif()

include (CPack)