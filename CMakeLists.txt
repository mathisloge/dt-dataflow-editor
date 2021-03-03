cmake_minimum_required(VERSION 3.15)
project(DtDataflowEditor
        VERSION 0.1.0
        DESCRIPTION "dataflow plugin editor"
        HOMEPAGE_URL https://github.com/mathisloge/dt-dataflow-editor
        LANGUAGES CXX
)
include(GenerateExportHeader)

option(BUILD_SHARED_LIBS "build as a shared library" ON)

find_package(Magnum REQUIRED GL)
find_package(Corrade REQUIRED PluginManager)
find_package(imgui CONFIG REQUIRED)
find_package(Boost REQUIRED COMPONENTS graph)
find_package(fmt CONFIG REQUIRED)
find_package(DtDataFlow CONFIG REQUIRED)
add_library(DtDataflowEditor 
    src/editor.cpp
    src/gui.cpp
    src/data_flow_graph.cpp
    src/graph_impl.cpp
    src/node_display_tree.cpp
    src/priv_types.cpp
)
add_library(dt::DtDataflowEditor ALIAS DtDataflowEditor)
set_property(TARGET DtDataflowEditor PROPERTY CXX_STANDARD 20)


set_directory_properties(PROPERTIES CORRADE_USE_PEDANTIC_FLAGS ON)

generate_export_header(DtDataflowEditor)

target_include_directories(
    DtDataflowEditor PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}>
    $<INSTALL_INTERFACE:include>
)
target_link_libraries(DtDataflowEditor PRIVATE 
    Corrade::PluginManager
    Magnum::Magnum
    Magnum::GL
    Boost::headers
    Boost::graph
    fmt::fmt
    imgui::imgui
    dt::imnodes
    dt::DtDataflowCore
    dt::DtDataflowPlugin
)
install(DIRECTORY include/ TYPE INCLUDE)
install(FILES
    ${PROJECT_BINARY_DIR}/dtdatafloweditor_export.h
    DESTINATION include/dt/df/editor
)

install(TARGETS DtDataflowEditor
    EXPORT DtDataflowEditorTargets
)

install(EXPORT DtDataflowEditorTargets 
    FILE DtDataflowEditorTargets.cmake
    DESTINATION lib/cmake/DtDataflowEditor
)

include(CMakePackageConfigHelpers)
configure_package_config_file(${CMAKE_CURRENT_SOURCE_DIR}/Config.cmake.in
    "${CMAKE_CURRENT_BINARY_DIR}/DtDataflowEditorConfig.cmake"
    INSTALL_DESTINATION lib/cmake/DtDataflowEditor
    NO_SET_AND_CHECK_MACRO
    NO_CHECK_REQUIRED_COMPONENTS_MACRO
)
write_basic_package_version_file(
    DtDataflowEditorConfigVersion.cmake
    VERSION ${PACKAGE_VERSION}
    COMPATIBILITY AnyNewerVersion
)

install(EXPORT DtDataflowEditorTargets
    FILE DtDataflowEditorTargets.cmake
    NAMESPACE dt::
    DESTINATION lib/cmake/DtDataflowEditor
)

install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/DtDataflowEditorConfig.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/DtDataflowEditorConfigVersion.cmake
    DESTINATION lib/cmake/DtDataflowEditor
)
