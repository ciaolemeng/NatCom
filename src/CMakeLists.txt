project(NatCom)
cmake_minimum_required(VERSION 2.8.4)

add_subdirectory(NatUdp)
#add_subdirectory(PathChirp)
#add_subdirectory(BandDeamon)
#add_subdirectory(TransChirp)

if (WIN32)
#    target_link_libraries(MeasureBand ws2_32)
endif ()