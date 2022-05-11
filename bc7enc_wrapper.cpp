#include "bc7enc_wrapper.h"
#include "rdo_bc_encoder.h"
#include <cstring>

bc7enc_error bc7enc_params_init(bc7enc_params* params) {
    if (!params) {
        return BC7ENC_ERR_BAD_PARAM;
    }

    params->bc7_uber_level = 6; // BC7ENC_MAX_UBER_LEVEL;
    params->bc7enc_max_partitions_to_scan = BC7ENC_MAX_PARTITIONS;
    params->perceptual = false;
    params->y_flip = false;
    params->bc45_channel0 = 0;
    params->bc45_channel1 = 1;

    params->bc1_mode = BC7ENC_BC1_APPROX_MODE_IDEAL;
    params->use_bc1_3color_mode = true;

    params->use_bc1_3color_mode_for_black = true;

    params->bc1_quality_level = BC7ENC_BC1_LEVEL_MAX;

    params->dxgi_format = DXGI_FORMAT_BC7_UNORM;

    params->rdo_lambda = 0.0f;
    params->rdo_debug_output = false;
    params->rdo_smooth_block_error_scale = 15.0f;
    params->custom_rdo_smooth_block_error_scale = false;
    params->lookback_window_size = 128;
    params->custom_lookback_window_size = false;
    params->bc7enc_rdo_bc7_quant_mode6_endpoints = true;
    params->bc7enc_rdo_bc7_weight_modes = true;
    params->bc7enc_rdo_bc7_weight_low_frequency_partitions = true;
    params->bc7enc_rdo_bc7_pbit1_weighting = true;
    params->rdo_max_smooth_block_std_dev = 18.0f;
    params->rdo_allow_relative_movement = false;
    params->rdo_try_2_matches = true;
    params->rdo_ultrasmooth_block_handling = true;

    params->use_hq_bc345 = true;
    params->bc345_search_rad = 5;
    params->bc345_mode_mask = BC7ENC_BC4_USE_ALL_MODES;

    params->bc7enc_mode6_only = false;
    params->rdo_multithreading = true;

    params->bc7enc_reduce_entropy = false;

#if SUPPORT_BC7E
    params->use_bc7e = true;
#else
    params->use_bc7e = false;
#endif

    params->status_output = false;

    params->rdo_max_threads = 128;

    return BC7ENC_SUCCESS;
}

struct bc7enc_context {
    rdo_bc::rdo_bc_encoder encoder;
    rdo_bc::rdo_bc_params params;
    utils::image_u8 src_image;
};

bc7enc_error bc7enc_context_alloc(const bc7enc_params* params, bc7enc_context** context) {
    if (!context || *context) {
        return BC7ENC_ERR_BAD_PARAM;
    }

    *context = new bc7enc_context();

    rdo_bc::rdo_bc_params& p = (*context)->params;

    p.m_bc7_uber_level = params->bc7_uber_level;
    p.m_bc7enc_max_partitions_to_scan = params->bc7enc_max_partitions_to_scan;
    p.m_perceptual = params->perceptual;
    p.m_y_flip = params->y_flip;
    p.m_bc45_channel0 = params->bc45_channel0;
    p.m_bc45_channel1 = params->bc45_channel1;
    p.m_bc1_mode = static_cast<rgbcx::bc1_approx_mode>(params->bc1_mode);
    p.m_use_bc1_3color_mode = params->use_bc1_3color_mode;
    p.m_use_bc1_3color_mode_for_black = params->use_bc1_3color_mode_for_black;
    p.m_bc1_quality_level = params->bc1_quality_level;
    p.m_dxgi_format = params->dxgi_format;
    p.m_rdo_lambda = params->rdo_lambda;
    p.m_rdo_debug_output = params->rdo_debug_output;
    p.m_rdo_smooth_block_error_scale = params->rdo_smooth_block_error_scale;
    p.m_custom_rdo_smooth_block_error_scale = params->custom_rdo_smooth_block_error_scale;
    p.m_lookback_window_size = params->lookback_window_size;
    p.m_custom_lookback_window_size = params->custom_lookback_window_size;
    p.m_bc7enc_rdo_bc7_quant_mode6_endpoints = params->bc7enc_rdo_bc7_quant_mode6_endpoints;
    p.m_bc7enc_rdo_bc7_weight_modes = params->bc7enc_rdo_bc7_weight_modes;
    p.m_bc7enc_rdo_bc7_weight_low_frequency_partitions = params->bc7enc_rdo_bc7_weight_low_frequency_partitions;
    p.m_bc7enc_rdo_bc7_pbit1_weighting = params->bc7enc_rdo_bc7_pbit1_weighting;
    p.m_rdo_max_smooth_block_std_dev = params->rdo_max_smooth_block_std_dev;
    p.m_rdo_allow_relative_movement = params->rdo_allow_relative_movement;
    p.m_rdo_try_2_matches = params->rdo_try_2_matches;
    p.m_rdo_ultrasmooth_block_handling = params->rdo_ultrasmooth_block_handling;
    p.m_use_hq_bc345 = params->use_hq_bc345;
    p.m_bc345_search_rad = params->bc345_search_rad;
    p.m_bc345_mode_mask = params->bc345_mode_mask;
    p.m_bc7enc_mode6_only = params->bc7enc_mode6_only;
    p.m_rdo_multithreading = params->rdo_multithreading;
    p.m_bc7enc_reduce_entropy = params->bc7enc_reduce_entropy;
    p.m_use_bc7e = params->use_bc7e;
    p.m_status_output = params->status_output;
    p.m_rdo_max_threads = params->rdo_max_threads;

    return BC7ENC_SUCCESS;
}

bc7enc_error bc7enc_context_free(bc7enc_context* context) {
    if (!context) {
        return BC7ENC_ERR_BAD_CONTEXT;
    }

    delete context;

    return BC7ENC_SUCCESS;
}

bc7enc_error bc7enc_compress_image(
	bc7enc_context* context,
    const uint8_t* src_pixels,
    uint32_t width,
    uint32_t height,
    uint8_t* data_out,
    size_t data_len
) {
    if (!context) {
        return BC7ENC_ERR_BAD_CONTEXT;
    }

    context->src_image.init(width, height);
    std::memcpy(&context->src_image.get_pixels(), src_pixels, width * height * 4);

    if (!context->encoder.init(context->src_image, context->params)) {
        return BC7ENC_ERR_INIT_CONTEXT_FAILED; // todo
    }

    if (!context->encoder.encode()) {
        return BC7ENC_ERR_INIT_CONTEXT_FAILED; // todo
    }

    const uint32_t output_data_size = context->encoder.get_total_blocks_size_in_bytes();
    if (data_len < output_data_size) {
        return BC7ENC_ERR_OUT_OF_MEM;
    }

    std::memcpy(data_out, context->encoder.get_blocks(), output_data_size);

    return BC7ENC_SUCCESS;
}

bc7enc_error bc7enc_compress_reset(bc7enc_context* context) {
    if (!context) {
        return BC7ENC_ERR_BAD_CONTEXT;
    }

    context->src_image.clear();
    context->encoder.clear();

    return BC7ENC_SUCCESS;
}
