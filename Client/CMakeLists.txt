#TcpIso8583Client

cmake_minimum_required(VERSION 3.0)

project(TcpIso8583Client)

set(TARGET TcpIso8583Client)

set(Boost_USE_STATIC_LIBS       OFF) 
set(Boost_USE_MULTITHREADED     ON)
set(Boost_USE_STATIC_RUNTIME    OFF)
find_package(Boost 1.72.0 REQUIRED)
if(Boost_FOUND)
  include_directories(${Boost_INCLUDE_DIRS} ${COMMON_SRC_DIR})  
  link_directories(${Boost_LIBRARY_DIR} $ENV{BOOST_ROOT}/lib)
  
  message("Boost_INCLUDE_DIRS=${Boost_INCLUDE_DIRS}" )  
  message("Boost_LIBRARY_DIR=${Boost_LIBRARY_DIR}" )  
  message("Boost_LIBRARIES=${Boost_LIBRARIES}" )  
  message("Boost_ROOT=$ENV{BOOST_ROOT}" )  
  
  add_executable(${TARGET} ClientMain.cpp TcpClient.cpp TcpClient.h ${ISO8583_SOURCES})
  set_target_properties(${TARGET} PROPERTIES CXX_STANDARD 17 CXX_STANDARD_REQUIRED ON)
  target_link_libraries(${TARGET} ${Boost_LIBRARIES})
else()
    message(SEND_ERROR "Failed to find boost.")
	return()  
endif()




