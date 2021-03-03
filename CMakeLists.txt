cmake_minimum_required(VERSION 3.15)
project(DtDataflowEditor
        VERSION 0.1.0
        DESCRIPTION "dataflow plugin editor"
        HOMEPAGE_URL https://github.com/mathisloge/dt-dataflow-editor
        LANGUAGES CXX
)
include(GenerateExportHeader)

option(BUILD_SHARED_LIBS "build as a shared library" ON)

add_subdirectory(dt-dataflow)
find_package(Magnum REQUIRED GL)
find_package(Corrade REQUIRED PluginManager)
find_package(imgui CONFIG REQUIRED)
find_package(Boost REQUIRED COMPONENTS graph)
find_package(fmt CONFIG REQUIRED)
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
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/include>
    $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}>
)
target_link_libraries(DtDataflowEditor PRIVATE 
    Corrade::PluginManager
    Magnum::Magnum
    Magnum::GL
    Boost::headers
    Boost::graph
    fmt::fmt
    imgui::imgui
    imgui::imnodes
    dt::DtDataflowCore
    dt::DtDataflowPlugin
)
install(DIRECTORY include/ TYPE INCLUDE)
