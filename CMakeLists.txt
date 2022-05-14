project(bc7enc)

cmake_minimum_required(VERSION 3.12)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(BUILD_X64 "build 64-bit" ON)
option(SUPPORT_BC7E "support BC7E (requires ispc)" OFF)
option(CLI "enable build of CLI" ON)

set(SIMD "sse2;sse4;avx;avx2" CACHE STRING "ispc SIMD target")

set(BC7ENC_TARGET_CLI "bc7enc")
set(BC7ENC_TARGET_STATIC "bc7enc-static")

message("Initial BUILD_X64=${BUILD_X64}")
message("Initial SIMD=${SIMD}")
message("Initial CMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}")
message("Initial SUPPORT_BC7E=${SUPPORT_BC7E}")

if(NOT CMAKE_BUILD_TYPE)
  	set(CMAKE_BUILD_TYPE Release)
endif()

set(ARCH "x64")
if(NOT BUILD_X64)
	set(ARCH "x86")
endif()

message("${PROJECT_NAME} build type: ${CMAKE_BUILD_TYPE}")
message("target architecture: ${ARCH}")
message("simd: ${SIMD}")

if(SUPPORT_BC7E)
	if(NOT MSVC)
		set(OBJ_EXT ".o")
	else()
		set(OBJ_EXT ".obj")
	endif()

	set(BC7E_OBJ ${CMAKE_BINARY_DIR}/bc7e${OBJ_EXT})

	# if only single simd target ispc generates no individual _sse4, _avx2, ... objects and headers
	list(LENGTH SIMD SIMD_COUNT)
	if(SIMD_COUNT GREATER 1)
		list(TRANSFORM SIMD PREPEND "${CMAKE_BINARY_DIR}/bc7e_" OUTPUT_VARIABLE BC7E_SIMD_OBJ_LIST)
		list(TRANSFORM BC7E_SIMD_OBJ_LIST APPEND "${OBJ_EXT}")
	endif()

	set(BC7E_OBJ_LIST ${BC7E_OBJ} ${BC7E_SIMD_OBJ_LIST})

	list(JOIN SIMD "," SIMD_TARGETS)

	add_custom_command(
		OUTPUT ${BC7E_OBJ_LIST}
		COMMAND ispc -g -O2 ${CMAKE_SOURCE_DIR}/bc7e.ispc -o ${BC7E_OBJ} -h ${CMAKE_SOURCE_DIR}/bc7e_ispc.h --target=${SIMD_TARGETS} --opt=fast-math --opt=disable-assertions
		DEPENDS bc7e.ispc
		)

	add_definitions(-DSUPPORT_BC7E=1)
endif()

if(NOT MSVC)
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g")
	set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -g")
endif()

# -fno-strict-aliasing shouldn't be necessary, it's here because that is what MSVC uses by default and that's what I've tested with the most.
if(NOT MSVC)
	set(GCC_COMPILE_FLAGS "-fno-strict-aliasing -Wall -Wextra")
	if(NOT BUILD_X64)
		set(GCC_COMPILE_FLAGS "${GCC_COMPILE_FLAGS} -m32")
	endif()
endif()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${GCC_COMPILE_FLAGS}")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -D_DEBUG")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${GCC_COMPILE_FLAGS}")
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -D_DEBUG")

add_library(${BC7ENC_TARGET_STATIC} STATIC
	bc7enc.cpp
	bc7enc.h
	bc7decomp.cpp
	bc7decomp_ref.cpp
	bc7decomp.h
	rgbcx.cpp
	rgbcx.h
	rgbcx_rdo.h
	rgbcx_table4.h
	rgbcx_table4_small.h
	utils.cpp
	utils.h
	dds_defs.h
	ert.cpp
	ert.h
	bc7e.ispc
	${BC7E_OBJ_LIST}
	)

if(${CLI})
	add_executable(${BC7ENC_TARGET_CLI}
		lodepng.cpp
		lodepng.h
		miniz.h
		rdo_bc_encoder.cpp
		rdo_bc_encoder.h
		test.cpp
		# bc7enc_wrapper.cpp
		# bc7enc_wrapper.h
		)

	target_compile_options(${BC7ENC_TARGET_CLI} PRIVATE
		# MSVC compiler flags
		$<$<CXX_COMPILER_ID:MSVC>:/W4>

		# G++ and Clang++ compiler flags
		)

	find_package(OpenMP REQUIRED)
	target_link_libraries(${BC7ENC_TARGET_CLI} PUBLIC OpenMP::OpenMP_CXX)

	target_link_libraries(${BC7ENC_TARGET_CLI} PRIVATE
		${BC7ENC_TARGET_STATIC}
		$<$<NOT:$<CXX_COMPILER_ID:MSVC>>:m>
		)
endif()
