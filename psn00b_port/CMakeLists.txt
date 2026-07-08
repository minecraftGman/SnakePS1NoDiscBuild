cmake_minimum_required(VERSION 3.21)

project(snake_nocd LANGUAGES C)

get_filename_component(SDK_DIR "${CMAKE_TOOLCHAIN_FILE}" DIRECTORY)
set(LD_SCRIPT_PATH "${SDK_DIR}/../ldscripts/exe.ld")
if(EXISTS "${LD_SCRIPT_PATH}")
    file(READ "${LD_SCRIPT_PATH}" LD_CONTENT)
    string(REPLACE "0x80010000" "0x80040000" LD_CONTENT "${LD_CONTENT}")
    file(WRITE "${LD_SCRIPT_PATH}" "${LD_CONTENT}")
endif()

psn00bsdk_add_executable(snake STATIC
	src/main.c
	src/Controller.c
	src/AssetsTextures.c
	src/AssetsAudio.c
	src/tmd.c
)

target_link_libraries(snake PRIVATE psxapi)
