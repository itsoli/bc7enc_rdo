project(bc7enc)

cmake_minimum_required(VERSION 3.12)

option(BUILD_X64 "build 64-bit" TRUE)
option(SUPPORT_BC7E "support BC7E (requires ispc)" FALSE)

set(SIMD "sse2;sse4;avx;avx2" CACHE STRING "ispc SIMD target")
set(LLVM_PATH "" CACHE PATH "llvm path") # -DLLVM_PATH:FILEPATH=$(brew --prefix llvm)

set(BC7ENC_TARGET_BIN "bc7enc")
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

if(NOT MSVC)
	if(NOT LLVM_PATH STREQUAL "")
		set(CMAKE_C_COMPILER "${LLVM_PATH}/bin/clang")
		set(CMAKE_CXX_COMPILER "${LLVM_PATH}/bin/clang++")
		link_directories("${LLVM_PATH}/lib")
	endif()

	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fopenmp")
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g")
	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE}")
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fopenmp")
	set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -g")
	set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE}")
else()
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /openmp /W4")
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}")
	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE}")
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /openmp /W4")
	set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG}")
	set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE}")
endif()

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
endif()

# -fno-strict-aliasing shouldn't be necessary, it's here because that is what MSVC uses by default and that's what I've tested with the most.
if(NOT MSVC)
	set(GCC_COMPILE_FLAGS "-fno-strict-aliasing -Wall -Wextra")
	if(NOT BUILD_X64)
		set(GCC_COMPILE_FLAGS "${GCC_COMPILE_FLAGS} -m32")
	endif()
endif()

if(SUPPORT_BC7E)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${GCC_COMPILE_FLAGS} -DSUPPORT_BC7E=1")
endif()

set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} ${GCC_COMPILE_FLAGS}")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} ${GCC_COMPILE_FLAGS} -D_DEBUG")

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${GCC_COMPILE_FLAGS}")
set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} ${GCC_COMPILE_FLAGS}")
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} ${GCC_COMPILE_FLAGS} -D_DEBUG")

set(BC7ENC_COMMON_SRC_LIST
	bc7enc.cpp
	bc7enc.h
	bc7decomp.cpp
	bc7decomp_ref.cpp
	bc7decomp.h
	rgbcx.cpp
	rgbcx.h
	rgbcx_table4.h
	rgbcx_table4_small.h
	utils.cpp
	utils.h
	dds_defs.h
	ert.cpp
	ert.h
	rdo_bc_encoder.cpp
	rdo_bc_encoder.h
	bc7e.ispc
	)

set(BC7ENC_SRC_LIST
	${COMMON_SRC_LIST}
	${BC7ENC_COMMON_SRC_LIST}
	lodepng.cpp
	lodepng.h
	test.cpp
	miniz.h
	)

add_executable(${BC7ENC_TARGET_BIN} ${BC7ENC_SRC_LIST} ${BC7E_OBJ_LIST})

if(NOT MSVC)
	target_link_libraries(${BC7ENC_TARGET_BIN} m)
endif()

add_library(${BC7ENC_TARGET_STATIC} STATIC ${BC7ENC_COMMON_SRC_LIST} ${BC7E_OBJ_LIST})
