# Check if QT_MAJOR_VERSION was not defined
if(NOT DEFINED QT_MAJOR_VERSION)
    message(FATAL_ERROR "QT_MAJOR_VERSION not set.")
endif()

# Use appropriate wrap_cpp command based on QT_MAJOR_VERSION
if("${QT_MAJOR_VERSION}" STREQUAL "5")
    qt5_wrap_cpp(QTERM_TEST_MOC qterminal_test.h)
    set(QT_TEST_LIB Qt5::Test)
elseif("${QT_MAJOR_VERSION}" STREQUAL "6")
    qt6_wrap_cpp(QTERM_TEST_MOC qterminal_test.h)
    set(QT_TEST_LIB Qt6::Test)
else()
    message(FATAL_ERROR "Unsupported QT_MAJOR_VERSION: ${QT_MAJOR_VERSION}")
endif()

add_executable(qterminal_test
    qterminal_test.cpp
    ${CMAKE_SOURCE_DIR}/src/qterminalutils.cpp
    ${QTERM_TEST_MOC})

target_link_libraries(qterminal_test ${QT_TEST_LIB})

add_test(NAME qterminal_test COMMAND qterminal_test)
