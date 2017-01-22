project(Network_Speed_Monitor)
project(lcm1602)
# set(CMAKE_BUILD_TYPE Release)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c11 -Wall")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall")

set(SOURCE_FILES main.cpp classes/TrafficMonitor.cpp classes/TrafficMonitor.h classes/I2CLcd1602.cpp classes/I2CLcd1602.h )
add_executable(Network_Speed_Monitor ${SOURCE_FILES})
add_library(lcm1602 SHARED i2c.c lcd.c)
target_link_libraries(Network_Speed_Monitor lcm1602)