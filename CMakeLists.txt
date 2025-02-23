cmake_minimum_required(VERSION 3.5)
project(iris)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(PkgConfig REQUIRED)
pkg_check_modules(MOSQUITTO REQUIRED libmosquitto libmosquittopp)

# Find OpenSSL
find_package(OpenSSL REQUIRED)

include_directories(${MOSQUITTO_INCLUDE_DIRS})
link_directories(${MOSQUITTO_LIBRARY_DIRS})

# Add all source files
set(SOURCES
    iris.cpp
    src/irismqttclient.cpp 
)

# Define the executable
add_executable(iris ${SOURCES})


# Link Drogon and PostgreSQL
target_link_libraries(${PROJECT_NAME} PRIVATE pqxx mosquittopp pthread jsoncpp ${SODIUM_LIBRARY} OpenSSL::Crypto)

# Include headers
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} /usr/local/include)
