#pragma once

#include <stddef.h>
#include <stdint.h>

#ifndef SUPPORT_BC7E
#define SUPPORT_BC7E 0
#endif

#include "dds_defs.h"

enum bc7enc_error {
	/** @brief The call was successful. */
	BC7ENC_SUCCESS,
    /** @brief The call failed due to low memory, or undersized I/O buffers. */
	BC7ENC_ERR_OUT_OF_MEM,
    /** @brief The call failed due to an out-of-spec parameter. */
    BC7ENC_ERR_BAD_PARAM,
    /** @brief The call failed due to the context not supporting the operation. */
	BC7ENC_ERR_BAD_CONTEXT,
    BC7ENC_ERR_INIT_CONTEXT_FAILED
};

// #ifndef BC7ENC_BLOCK_SIZE
// #define BC7ENC_BLOCK_SIZE (16)
// #endif
// #ifndef BC7ENC_MAX_PARTITIONS
// #define BC7ENC_MAX_PARTITIONS (64)
// #endif
// #ifndef BC7ENC_MAX_UBER_LEVEL
// #define BC7ENC_MAX_UBER_LEVEL (4)
// #endif

enum bc7enc_bc1_approx_mode {
    // The default mode. No rounding for 4-color colors 2,3. My older tools/compressors use this mode.
    // This matches the D3D10 docs on BC1.
    BC7ENC_BC1_APPROX_MODE_IDEAL = 0,

    // NVidia GPU mode.
    BC7ENC_BC1_APPROX_MODE_NVIDIA = 1,

    // AMD GPU mode.
    BC7ENC_BC1_APPROX_MODE_AMD = 2,

    // This mode matches AMD Compressonator's output. It rounds 4-color colors 2,3 (not 3-color color 2).
    // This matches the D3D9 docs on DXT1.
    BC7ENC_BC1_APPROX_MODE_ROUND_4 = 3
};

enum BC7ENC_BC1_LEVEL {
    BC7ENC_BC1_LEVEL_MIN = 0,
    BC7ENC_BC1_LEVEL_MAX = 18
};

enum BC7ENC_BC4_USE {
    BC7ENC_BC4_USE_MODE8_FLAG = 1,
	BC7ENC_BC4_USE_MODE6_FLAG = 2,
	BC7ENC_BC4_USE_ALL_MODES = BC7ENC_BC4_USE_MODE8_FLAG | BC7ENC_BC4_USE_MODE8_FLAG
};

struct bc7enc_params {
    int bc7_uber_level;
    int bc7enc_max_partitions_to_scan;
    bool perceptual;
    bool y_flip;
    uint32_t bc45_channel0;
    uint32_t bc45_channel1;

    bc7enc_bc1_approx_mode bc1_mode;
    bool use_bc1_3color_mode;

    bool use_bc1_3color_mode_for_black;

    int bc1_quality_level;

    DXGI_FORMAT dxgi_format;

    float rdo_lambda;
    bool rdo_debug_output;
    float rdo_smooth_block_error_scale;
    bool custom_rdo_smooth_block_error_scale;
    uint32_t lookback_window_size;
    bool custom_lookback_window_size;
    bool bc7enc_rdo_bc7_quant_mode6_endpoints;
    bool bc7enc_rdo_bc7_weight_modes;
    bool bc7enc_rdo_bc7_weight_low_frequency_partitions;
    bool bc7enc_rdo_bc7_pbit1_weighting;
    float rdo_max_smooth_block_std_dev;
    bool rdo_allow_relative_movement;
    bool rdo_try_2_matches;
    bool rdo_ultrasmooth_block_handling;

    bool use_hq_bc345;
    int bc345_search_rad;
    uint32_t bc345_mode_mask;

    bool bc7enc_mode6_only;
    bool rdo_multithreading;

    bool bc7enc_reduce_entropy;

    bool use_bc7e;

    bool status_output;

    uint32_t rdo_max_threads;
};

bc7enc_error bc7enc_params_init(bc7enc_params* params);

// opaque structure
struct bc7enc_context;

bc7enc_error bc7enc_context_alloc(const bc7enc_params* params, bc7enc_context** context);
bc7enc_error bc7enc_context_free(bc7enc_context* context);

bc7enc_error bc7enc_compress_image(
	bc7enc_context* context,
    const uint8_t* src_pixels,
    uint32_t width,
    uint32_t height,
    uint8_t* data_out,
    size_t data_len
);

bc7enc_error bc7enc_compress_reset(bc7enc_context* context);
