#pragma once

#include <stdlib.h>

namespace ert
{
	struct color_rgba { uint8_t m_c[4]; };

	struct reduce_entropy_params
	{
		// m_lambda: The post-processor tries to reduce distortion*smooth_block_scale + rate*lambda (rate is approximate LZ bits and distortion is scaled MS error multiplied against the smooth block MSE weighting factor).
		// Larger values push the postprocessor towards optimizing more for lower rate, and smaller values more for distortion. 0=minimal distortion.
		float m_lambda;

		// m_lookback_window_size: The number of bytes the encoder can look back from each block to find matches. The larger this value, the slower the encoder but the higher the quality per LZ compressed bit.
		uint32_t m_lookback_window_size;

		// m_max_allowed_rms_increase_ratio: How much the RMS error of a block is allowed to increase before a trial is rejected. 1.0=no increase allowed, 1.05=5% increase allowed, etc.
		float m_max_allowed_rms_increase_ratio;

		float m_max_smooth_block_std_dev;
		float m_smooth_block_max_mse_scale;

		uint32_t m_color_weights[4];

		bool m_try_two_matches;
		bool m_allow_relative_movement;
		bool m_skip_zero_mse_blocks;
		bool m_debug_output;
	};

	void clear_params(reduce_entropy_params* params);
	void print_params(const reduce_entropy_params* params);

	typedef bool (*pUnpack_block_func)(const void* pBlock, color_rgba* pPixels, uint32_t block_index, void* pUser_data);

	// BC7 entropy reduction transform with Deflate/LZMA/LZHAM optimizations
	bool reduce_entropy(void* pBlocks, uint32_t num_blocks,
		uint32_t total_block_stride_in_bytes, uint32_t block_size_to_optimize_in_bytes, uint32_t block_width, uint32_t block_height, uint32_t num_comps,
		const color_rgba* pBlock_pixels, const reduce_entropy_params* params, uint32_t* total_modified,
		pUnpack_block_func pUnpack_block_func, void* pUnpack_block_func_user_data,
		const float* const* pBlock_mse_scales = 0);

} // namespace ert
