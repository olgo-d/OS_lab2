
cmake_minimum_required(VERSION 3.16)

project(OS_lab3 C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

find_package(Threads REQUIRED)

add_executable(mergesort mergesort.c)

add_executable(paralel_merge_sort paralel_merge_sort.c)
target_link_libraries(paralel_merge_sort PRIVATE Threads::Threads)


add_library(seq_obj OBJECT mergesort.c)
target_compile_definitions(seq_obj PRIVATE
    mergeSort=seq_mergeSort
    merge=seq_merge
    binSearch=seq_binSearch
    main=seq_main
)

add_library(par_obj OBJECT paralel_merge_sort.c)
target_compile_definitions(par_obj PRIVATE
    mergeSort=par_mergeSort
    merge=par_merge
    binSearch=par_binSearch
    main=par_main
)

add_executable(benchmark
    benchmark.c
    $<TARGET_OBJECTS:seq_obj>
    $<TARGET_OBJECTS:par_obj>
)

target_link_libraries(benchmark PRIVATE Threads::Threads)
