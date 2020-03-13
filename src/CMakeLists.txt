SET(APP_HEADERS
        "${CMAKE_CURRENT_SOURCE_DIR}/transformation_helpers.h"
        )

SET(APP_SOURCES
        "${CMAKE_CURRENT_SOURCE_DIR}/transformation_helpers.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/extract_mkv_k4a.cpp"
        )

add_executable(extract_mkv_k4a ${APP_SOURCES} ${APP_HEADERS} )
set_property(TARGET extract_mkv_k4a PROPERTY CXX_STANDARD 11)
set_target_properties(extract_mkv_k4a PROPERTIES LINKER_LANGUAGE CXX)

target_link_libraries(extract_mkv_k4a PRIVATE
        CONAN_PKG::magnum
        CONAN_PKG::corrade
        CONAN_PKG::opencv
        CONAN_PKG::kinect-azure-sensor-sdk
        )

target_include_directories(extract_mkv_k4a PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
        PRIVATE
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/src>
        )

install(TARGETS extract_mkv_k4a DESTINATION ${MAGNUM_BINARY_INSTALL_DIR})