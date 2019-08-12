cmake_minimum_required(VERSION 3.6)

project(TestBlockInfo C CXX)

if(POLICY CMP0074)
  cmake_policy(SET CMP0074 NEW)
endif()

find_package(MPI REQUIRED)
find_package(ADIOS2 REQUIRED)

add_executable(writer writer.cxx)
target_link_libraries(writer MPI::MPI_C adios2::adios2)

add_executable(reader reader.cxx)
target_link_libraries(reader MPI::MPI_C adios2::adios2)
