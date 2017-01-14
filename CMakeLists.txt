project(Network_Speed_Monitor)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c11 -Wall")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall")

set(SOURCE_FILES main.cpp classes/TrafficMonitor.cpp classes/TrafficMonitor.h classes/I2CLcd1602.cpp classes/I2CLcd1602.h i2c.c includes/i2c.h lcd.c includes/lcd.h)
add_executable(Network_Speed_Monitor ${SOURCE_FILES})