cmake_minimum_required(VERSION 3.5)

project(chatgml_cpp_qt VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wall")

if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif ()

# third-party libraries
add_compile_definitions(GGML_CUDA_MMV_Y=2)  # for large vocab
include_directories(third_party/ggml/include/ggml third_party/ggml/src)
add_subdirectory(third_party/ggml)

set(SPM_ENABLE_SHARED OFF CACHE BOOL "chatglm: disable sentencepiece shared libraries by default")
set(SPM_ENABLE_TCMALLOC OFF CACHE BOOL "chatglm: disable tcmalloc by default")
include_directories(third_party/sentencepiece/src)
add_subdirectory(third_party/sentencepiece)

if (GGML_CUBLAS)
    add_compile_definitions(GGML_USE_CUBLAS)
    set(CUDA_ARCHITECTURES "52;61;70;75;80;86" CACHE STRING "chatglm: cuda architectures to compile")
    set_property(TARGET ggml PROPERTY CUDA_ARCHITECTURES ${CUDA_ARCHITECTURES})
endif ()

if (GGML_METAL)
    add_compile_definitions(GGML_USE_METAL)
    configure_file(third_party/ggml/src/ggml-metal.metal ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/ggml-metal.metal COPYONLY)
endif ()

if (GGML_PERF)
    add_compile_definitions(GGML_PERF)
endif ()

file(GLOB CPP_SOURCES
    ${PROJECT_SOURCE_DIR}/*.h
    ${PROJECT_SOURCE_DIR}/*.cpp)

set_source_files_properties(${CPP_SOURCES} PROPERTIES COMPILE_FLAGS "-pedantic-errors")

add_library(chatglm STATIC chatglm.cpp)
target_link_libraries(chatglm PUBLIC ggml sentencepiece-static)


#add_executable(main main.cpp)
#target_link_libraries(main PRIVATE chatglm)


find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)

set(PROJECT_SOURCES
        main.cpp
        widget.cpp
        widget.h
        chat_thread.cpp
        chat_thread.h
        widget.ui
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(chatgml_cpp_qt
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}

    )

# qt_add_binary_resources(resources resource.qrc DESTINATION resource.rcc)

# add_dependencies(chatgml_cpp_qt resources)

# Define target properties for Android with Qt 6 as:
#    set_property(TARGET chatgml_cpp_qt APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
else()
    if(ANDROID)
        add_library(chatgml_cpp_qt SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(chatgml_cpp_qt
            ${PROJECT_SOURCES}
        )
    endif()
endif()

target_link_libraries(chatgml_cpp_qt PRIVATE chatglm Qt${QT_VERSION_MAJOR}::Widgets)
#target_link_libraries(chatgml_cpp_qt PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
if(${QT_VERSION} VERSION_LESS 6.1.0)
  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.chatgml_cpp_qt)
endif()
set_target_properties(chatgml_cpp_qt PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS chatgml_cpp_qt
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(chatgml_cpp_qt)
endif()
