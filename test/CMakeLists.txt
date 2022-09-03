qt5_wrap_cpp(QTERM_TEST_MOC qterminal_test.h)
add_executable(qterminal_test
    qterminal_test.cpp
    ${CMAKE_SOURCE_DIR}/src/qterminalutils.cpp
    ${QTERM_TEST_MOC})
target_link_libraries(qterminal_test Qt5::Test)
add_test(NAME qterminal_test COMMAND qterminal_test)
