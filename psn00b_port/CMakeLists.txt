cmake_minimum_required(VERSION 3.21)

project(
	snake_nocd
	LANGUAGES C
)

get_filename_component(SDK_DIR "${CMAKE_TOOLCHAIN_FILE}" DIRECTORY)
set(LD_SCRIPT_PATH "${SDK_DIR}/../ldscripts/exe.ld")
if(EXISTS "${LD_SCRIPT_PATH}")
    file(READ "${LD_SCRIPT_PATH}" LD_CONTENT)
    string(REPLACE "0x80010000" "0x80040000" LD_CONTENT "${LD_CONTENT}")
    file(WRITE "${LD_SCRIPT_PATH}" "${LD_CONTENT}")
endif()

# Ensure the directory exists before generating the file
add_custom_command(
    OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/src/AssetsAudio.c
    COMMAND bash -c "mkdir -p src && xxd -i bite1.vag > src/AssetsAudio.c && xxd -i die1.vag >> src/AssetsAudio.c && xxd -i jungle.vag >> src/AssetsAudio.c"
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    DEPENDS bite1.vag die1.vag jungle.vag
)

psn00bsdk_add_executable(snake STATIC
	src/main.c
	src/Controller.c
	src/AssetsTextures.c
	src/AssetsAudio.c
	src/tmd.c
)

target_link_libraries(snake PRIVATE psxapi)
