#TcpIso8583Server

cmake_minimum_required(VERSION 3.0)

project(TcpIso8583Server)

set(TARGET TcpIso8583Server)

set(Boost_USE_STATIC_LIBS       OFF) 
set(Boost_USE_MULTITHREADED     ON)
set(Boost_USE_STATIC_RUNTIME    OFF)
find_package(Boost 1.72.0 REQUIRED)
if(Boost_FOUND)
  include_directories(${Boost_INCLUDE_DIRS} ${COMMON_SRC_DIR}) 
  link_directories(${Boost_LIBRARY_DIR} $ENV{BOOST_ROOT}/lib)   
else()
    message(SEND_ERROR "Failed to find boost.")
	return()  
endif()

message ("OCCI_PATH = '$ENV{OCCI_PATH}'")
set(OCCI_PATH $ENV{OCCI_PATH})

include_directories(${OCCI_PATH}/include) 
link_directories(${OCCI_PATH}/lib)
  
add_executable(${TARGET} ServerMain.cpp TcpServer.cpp TcpServer.h ResponseCodeSettingsProvider.h ResponseCodeSettingsProvider.cpp OracleDbClient.h OracleDbClient.cpp ${ISO8583_SOURCES})
set_target_properties(${TARGET} PROPERTIES CXX_STANDARD 17 CXX_STANDARD_REQUIRED ON)
target_link_libraries(${TARGET} optimized ${Boost_LIBRARIES} oraocci18)
target_link_libraries(${TARGET} debug ${Boost_LIBRARIES} oraocci18d)


