project(lcm1602)
project(i2clcd1602)
project(nsm)
project(dht11)
# set(CMAKE_BUILD_TYPE Release)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c11 -Wall")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall")

add_library(lcm1602 SHARED i2c.c lcd.c)
add_library(i2clcd1602 classes/I2CLcd1602.cpp classes/Dht11.cpp classes/Dht11.h)
add_executable(nsm network-speed-monitor.cpp classes/TrafficMonitor.cpp)
add_executable(dht11 dht11.cpp)

target_link_libraries(nsm lcm1602 i2clcd1602)
target_link_libraries(dht11 lcm1602 i2clcd1602 wiringPi)