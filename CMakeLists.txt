# Describe project build

cmake_minimum_required(VERSION 3.17 FATAL_ERROR)

project(inf_security VERSION 0.1 LANGUAGES CXX)

find_package(Python3 REQUIRED)

set(CMAKE_CXX_FLAGS "-Wall -Wextra -Wpedantic -Werror")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS 1)

set(PROJECT_THIRD_PARTY ${CMAKE_CURRENT_SOURCE_DIR}/third-party)
set(PROJECT_INFRA_SCRIPTS ${CMAKE_CURRENT_SOURCE_DIR}/scripts/infra)
set(PROJECT_RESEARCH_SCRIPTS ${CMAKE_CURRENT_SOURCE_DIR}/scripts/research)
set(DATASET_FOLDER ${PROJECT_SOURCE_DIR}/datasets)

include(${DATASET_FOLDER}/datasets.cmake)

execute_process(
    COMMAND ${Python3_EXECUTABLE} ${PROJECT_INFRA_SCRIPTS}/install_third_party.py
        ${PROJECT_INFRA_SCRIPTS}/third_party_list.txt ${PROJECT_THIRD_PARTY}
)

add_custom_target(
    run-clang-format
    COMMENT Run clang-format
    COMMAND ${Python3_EXECUTABLE} ${PROJECT_INFRA_SCRIPTS}/run_clang_format.py ${PROJECT_SOURCE_DIR}
)

add_custom_target(
    run-clang-tidy
    COMMENT Run clang-tidy
    COMMAND ${Python3_EXECUTABLE} ${PROJECT_INFRA_SCRIPTS}/run_clang_tidy.py ${PROJECT_SOURCE_DIR}
    DEPENDS all
)

set(HASH_SRC
    src/main.cpp
    src/sha256.cpp
    src/keccak.cpp
    src/blake256.cpp
    src/gost.cpp
)

add_subdirectory(third-party/cli11)

add_executable(hash_stat ${HASH_SRC})
target_include_directories(hash_stat PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(hash_stat PRIVATE CLI11)

add_executable(hash_time_stat ${HASH_SRC})
target_compile_definitions(hash_time_stat PRIVATE -DMEASURE_TIME)
target_include_directories(hash_time_stat PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(hash_time_stat PRIVATE CLI11)

foreach(dataset_name ${DATASETS})
    add_custom_target(collect_time_${dataset_name}
        COMMENT Collecting times
        COMMAND ${Python3_EXECUTABLE} ${PROJECT_RESEARCH_SCRIPTS}/collector.py
                                    --name-of-program ${PROJECT_BINARY_DIR}/hash_time_stat
                                    --dataset-file ${DATASET_FOLDER}/${dataset_name}
        DEPENDS hash_stat ${DATASET_FOLDER}
    )

    add_custom_target(collect_data_${dataset_name}
        COMMENT Collecting hash functions data
        COMMAND ${Python3_EXECUTABLE} ${PROJECT_RESEARCH_SCRIPTS}/collector.py
                                      --name-of-program ${PROJECT_BINARY_DIR}/hash_stat
                                      --dataset-file ${DATASET_FOLDER}/${dataset_name}
                                      --execute-in-parallel True
        DEPENDS hash_stat ${DATASET_FOLDER}
    )

    add_custom_target(convert_one_bit_in_${dataset_name}
        COMMENT Convert one random bit in random character in ${dataset_name} file
        COMMAND ${Python3_EXECUTABLE} ${PROJECT_RESEARCH_SCRIPTS}/converter.py
                                      --output ${DATASET_FOLDER}/converted_${dataset_name}
                                       --input ${DATASET_FOLDER}/${dataset_name}
    )

endforeach()