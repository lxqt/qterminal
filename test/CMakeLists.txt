qt6_wrap_cpp(QTERM_TEST_MOC qterminal_test.h)
set(QT_TEST_LIB Qt6::Test)

add_executable(qterminal_test
    qterminal_test.cpp
    ${CMAKE_SOURCE_DIR}/src/qterminalutils.cpp
    ${QTERM_TEST_MOC})

target_link_libraries(qterminal_test ${QT_TEST_LIB})

add_test(NAME qterminal_test COMMAND qterminal_test)
