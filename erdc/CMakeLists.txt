cmake_minimum_required(VERSION 3.12)

project(dummy_adios2_coupling C)

find_package(MPI REQUIRED)
find_package(ADIOS2 REQUIRED)

add_executable(a a.c)
target_link_libraries(a MPI::MPI_C adios2::adios2)

add_executable(b b.c)
target_link_libraries(b MPI::MPI_C adios2::adios2)
