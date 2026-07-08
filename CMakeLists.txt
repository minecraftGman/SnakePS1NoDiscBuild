cmake_minimum_required(VERSION 3.21)

project(
	snake_nocd
	LANGUAGES C
)

# STATIC = no runtime DLL loading, smallest/simplest executable.
psn00bsdk_add_executable(snake STATIC
	src/main.c
	src/Controller.c
	src/AssetsTextures.c
	src/AssetsAudio.c
)
