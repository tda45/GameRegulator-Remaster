cmake_minimum_required(VERSION 3.16)

project(GameRegulatorRemaster VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Platform-specific Qt6 path
if(WIN32)
    set(CMAKE_PREFIX_PATH "C:/Users/hp/6.10.0/msvc2022_64")
elseif(UNIX AND NOT APPLE)
    # Linux - use system Qt6 or specify path
    # set(CMAKE_PREFIX_PATH "/usr/local/qt-6.10.0")
endif()

# Find Qt6 components including LinguistTools for translations
find_package(Qt6 REQUIRED COMPONENTS Core Widgets LinguistTools)

# Enable automatic MOC, UIC, and RCC processing
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# Source files
set(SOURCES
    main.cpp
    mainwindow.cpp
    filemanager.cpp
)

set(HEADERS
    mainwindow.h
    filemanager.h
)

# Translation files
set(TS_FILES
    translations/gameregulator_tr.ts
    translations/gameregulator_en.ts
)

# Create executable
add_executable(GameRegulatorRemaster ${SOURCES} ${HEADERS} ${TS_FILES} resources.qrc)

# Link Qt libraries
target_link_libraries(GameRegulatorRemaster Qt6::Core Qt6::Widgets)

# Windows specific settings
if(WIN32)
    set_target_properties(GameRegulatorRemaster PROPERTIES
        WIN32_EXECUTABLE TRUE
    )
endif()

# Set output directory
set_target_properties(GameRegulatorRemaster PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

# Generate .qm files from .ts files
qt6_add_lupdate(GameRegulatorRemaster ${SOURCES} ${HEADERS})
qt6_add_lrelease(GameRegulatorRemaster ${TS_FILES})

# Platform-specific deployment
if(WIN32)
    # Find windeployqt for automatic deployment
    find_program(WINDEPLOYQT_EXECUTABLE windeployqt HINTS ${Qt6_DIR}/../../../bin)
    if(WINDEPLOYQT_EXECUTABLE)
        # Add post-build command to deploy Qt dependencies
        add_custom_command(TARGET GameRegulatorRemaster POST_BUILD
            COMMAND ${WINDEPLOYQT_EXECUTABLE} $<TARGET_FILE:GameRegulatorRemaster> --release --no-translations --no-system-d3d-compiler --no-opengl-sw
            COMMENT "Deploying Qt dependencies"
        )
    else()
        message(WARNING "windeployqt not found. Qt dependencies will not be automatically deployed.")
    endif()
    
    # Install Visual C++ redistributable if needed
    find_program(VCREDIST_PATH vcredist_x64.exe HINTS "C:/Program Files (x86)/Microsoft Visual Studio/2022/BuildTools/VC/Redist/MSVC")
    if(VCREDIST_PATH)
        install(FILES ${VCREDIST_PATH} DESTINATION .)
    endif()
elseif(UNIX AND NOT APPLE)
    # Linux deployment
    find_program(LINUXDEPLOY_EXECUTABLE linuxdeploy)
    if(LINUXDEPLOY_EXECUTABLE)
        # Create AppImage
        add_custom_target(appimage
            COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target GameRegulatorRemaster
            COMMAND ${LINUXDEPLOY_EXECUTABLE} --appdir AppDir --executable $<TARGET_FILE:GameRegulatorRemaster> --create-desktop-file --output appimage
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
            COMMENT "Creating AppImage"
        )
    else()
        message(STATUS "linuxdeploy not found. Install it for AppImage creation: wget https://github.com/linuxdeploy/linuxdeploy/releases/download/continuous/linuxdeploy-x86_64.AppImage && chmod +x linuxdeploy-x86_64.AppImage")
    endif()
endif()
