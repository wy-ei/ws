cmake_minimum_required(VERSION 3.5)
project(ws)

set(CMAKE_CXX_STANDARD 11)
set(BIN_NAME ws)

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --coverage -fprofile-arcs -ftest-coverage")

include_directories(${PROJECT_SOURCE_DIR})


file(GLOB BASE_SRC "base/*.cpp")
file(GLOB NET_SRC "net/*.cpp" "net/**/*.cpp")
file(GLOB HTTP_SRC "http/**/*.cpp" "http/*.cpp")
file(GLOB LOG_SRC "log/*.cpp")
file(GLOB UTILS_SRC "utils/*.cpp")

add_library(net STATIC
    ${BASE_SRC}
    ${NET_SRC}
    ${LOG_SRC}
    ${UTILS_SRC}
)

set(HEADER_DIRS
    ${PROJECT_SOURCE_DIR}/base
    ${PROJECT_SOURCE_DIR}/net
    ${PROJECT_SOURCE_DIR}/log
    ${PROJECT_SOURCE_DIR}/utils
)

target_include_directories(net
    PUBLIC
        ${HEADER_DIRS}
)

add_executable(${BIN_NAME} ws.cpp ${HTTP_SRC})
target_include_directories(${BIN_NAME}  PUBLIC
    ${PROJECT_SOURCE_DIR}/http

)

find_package(Threads REQUIRED)
target_link_libraries(${BIN_NAME} Threads::Threads)
target_link_libraries(${BIN_NAME} net)


# Coverage
set(TARGET_NAME "ws")
add_custom_target(${TARGET_NAME}-ccov-preprocessing
    COMMAND LLVM_PROFILE_FILE=${TARGET_NAME}.profraw $<TARGET_FILE:${TARGET_NAME}>
    COMMAND llvm-profdata merge -sparse ${TARGET_NAME}.profraw -o ${TARGET_NAME}.profdata
    DEPENDS ${TARGET_NAME})

##############################################
# install
install(TARGETS ${BIN_NAME} DESTINATION bin)

target_compile_definitions(${BIN_NAME} PRIVATE DDD)


#############################################
# Unit tests

add_subdirectory(third_party/googletest)

# enable CTest testing
enable_testing()

# Add a testing executable

function(ws_test test_file)
    get_filename_component(test_target_name "${test_file}" NAME_WE)
    message(STATUS "${test_file}")

    add_executable("${test_target_name}" "${test_file}")
    target_link_libraries("${test_target_name}" net gtest)

    add_test(NAME "${test_target_name}" COMMAND "${EXECUTABLE_OUTPUT_PATH}/${test_target_name}")
endfunction(ws_test)


ws_test("test/socket-test.cpp")
ws_test("test/formdata-test.cpp")


