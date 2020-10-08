#PROJECT_MAIN

cmake_minimum_required(VERSION 3.0)

set(COMMON_SRC_DIR "../src")
set(ISO8583_SOURCES ${COMMON_SRC_DIR}/MessageISO8583.h ${COMMON_SRC_DIR}/MessageISO8583.cpp ${COMMON_SRC_DIR}/ByteConversionHelper.h ${COMMON_SRC_DIR}/ByteConversionHelper.cpp)

project(ClientServerIso8583Protocol VERSION 1.0)

add_subdirectory(Client)
add_subdirectory(Server)