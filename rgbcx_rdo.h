#ifndef RGBCX_RDO_INCLUDE_H
#define RGBCX_RDO_INCLUDE_H

#ifdef _MSC_VER
#pragma warning (disable:4201) //nameless struct/union
#endif

#include <stdlib.h>
#include <stdint.h>
#include <algorithm>
#include <assert.h>
#include <limits.h>
#include "rgbcx.h"

namespace rgbcx
{
	enum class eNoClamp { cNoClamp };
	static inline uint8_t clamp255(int32_t i) { return (uint8_t)((i & 0xFFFFFF00U) ? (~(i >> 31)) : i); }

	template <typename S> inline S maximum(S a, S b) { return (a > b) ? a : b; }
	template <typename S> inline S maximum(S a, S b, S c) { return maximum(maximum(a, b), c); }
	template <typename S> inline S maximum(S a, S b, S c, S d) { return maximum(maximum(maximum(a, b), c), d); }

	template <typename S> inline S minimum(S a, S b) { return (a < b) ? a : b; }
	template <typename S> inline S minimum(S a, S b, S c) { return minimum(minimum(a, b), c); }
	template <typename S> inline S minimum(S a, S b, S c, S d) { return minimum(minimum(minimum(a, b), c), d); }

	struct color32
	{
		union
		{
			struct
			{
				uint8_t r;
				uint8_t g;
				uint8_t b;
				uint8_t a;
			};

			uint8_t c[4];

			uint32_t m;
		};

		color32() { }

		color32(uint32_t vr, uint32_t vg, uint32_t vb, uint32_t va) { set(vr, vg, vb, va); }
		color32(eNoClamp unused, uint32_t vr, uint32_t vg, uint32_t vb, uint32_t va) { (void)unused; set_noclamp_rgba(vr, vg, vb, va); }

		void set(uint32_t vr, uint32_t vg, uint32_t vb, uint32_t va) { c[0] = static_cast<uint8_t>(vr); c[1] = static_cast<uint8_t>(vg); c[2] = static_cast<uint8_t>(vb); c[3] = static_cast<uint8_t>(va); }

		void set_noclamp_rgb(uint32_t vr, uint32_t vg, uint32_t vb) { c[0] = static_cast<uint8_t>(vr); c[1] = static_cast<uint8_t>(vg); c[2] = static_cast<uint8_t>(vb); }
		void set_noclamp_rgba(uint32_t vr, uint32_t vg, uint32_t vb, uint32_t va) { set(vr, vg, vb, va); }

		void set_clamped(int vr, int vg, int vb, int va) { c[0] = clamp255(vr); c[1] = clamp255(vg);	c[2] = clamp255(vb); c[3] = clamp255(va); }

		uint8_t operator[] (uint32_t idx) const { assert(idx < 4); return c[idx]; }
		uint8_t& operator[] (uint32_t idx) { assert(idx < 4); return c[idx]; }

		bool operator== (const color32& rhs) const { return m == rhs.m; }

		void set_rgb(const color32& other) { c[0] = static_cast<uint8_t>(other.c[0]); c[1] = static_cast<uint8_t>(other.c[1]); c[2] = static_cast<uint8_t>(other.c[2]); }

		static color32 comp_min(const color32& a, const color32& b) { return color32(eNoClamp::cNoClamp, std::min(a[0], b[0]), std::min(a[1], b[1]), std::min(a[2], b[2]), std::min(a[3], b[3])); }
		static color32 comp_max(const color32& a, const color32& b) { return color32(eNoClamp::cNoClamp, std::max(a[0], b[0]), std::max(a[1], b[1]), std::max(a[2], b[2]), std::max(a[3], b[3])); }
	};

	bool unpack_bc1_block_colors(const void* pBlock_bits, color32* c, bc1_approx_mode mode = bc1_approx_mode::cBC1Ideal);

	// Rate Distortion Optimization (RDO)
	enum dxt_constants
	{
		cDXT1SelectorBits = 2U, cDXT1SelectorValues = 1U << cDXT1SelectorBits, cDXT1SelectorMask = cDXT1SelectorValues - 1U,
		cDXT5SelectorBits = 3U, cDXT5SelectorValues = 1U << cDXT5SelectorBits, cDXT5SelectorMask = cDXT5SelectorValues - 1U,
	};

	struct bc1_block
	{
		enum { cTotalEndpointBytes = 2, cTotalSelectorBytes = 4 };

		uint8_t m_low_color[cTotalEndpointBytes];
		uint8_t m_high_color[cTotalEndpointBytes];
		uint8_t m_selectors[cTotalSelectorBytes];

		inline uint32_t get_low_color() const { return m_low_color[0] | (m_low_color[1] << 8U); }
		inline uint32_t get_high_color() const { return m_high_color[0] | (m_high_color[1] << 8U); }
		inline bool is_3color() const { return get_low_color() <= get_high_color(); }
		inline void set_low_color(uint16_t c) { m_low_color[0] = static_cast<uint8_t>(c & 0xFF); m_low_color[1] = static_cast<uint8_t>((c >> 8) & 0xFF); }
		inline void set_high_color(uint16_t c) { m_high_color[0] = static_cast<uint8_t>(c & 0xFF); m_high_color[1] = static_cast<uint8_t>((c >> 8) & 0xFF); }
		inline uint32_t get_selector(uint32_t x, uint32_t y) const { assert((x < 4U) && (y < 4U)); return (m_selectors[y] >> (x * cDXT1SelectorBits)) & cDXT1SelectorMask; }
		inline void set_selector(uint32_t x, uint32_t y, uint32_t val) { assert((x < 4U) && (y < 4U) && (val < 4U)); m_selectors[y] &= (~(cDXT1SelectorMask << (x * cDXT1SelectorBits))); m_selectors[y] |= (val << (x * cDXT1SelectorBits)); }

		inline uint32_t get_endpoint_bits() const { return m_low_color[0] | (m_low_color[1] << 8) | (m_high_color[0] << 16) | (m_high_color[1] << 24); }
		inline void set_endpoint_bits(uint32_t s) { m_low_color[0] = (uint8_t)s; m_low_color[1] = (uint8_t)(s >> 8); m_high_color[0] = (uint8_t)(s >> 16); m_high_color[1] = (uint8_t)(s >> 24); }

		inline uint32_t get_selector_bits() const { return m_selectors[0] | (m_selectors[1] << 8) | (m_selectors[2] << 16) | (m_selectors[3] << 24); }
		inline void set_selector_bits(uint32_t s) { m_selectors[0] = (uint8_t)s; m_selectors[1] = (uint8_t)(s >> 8); m_selectors[2] = (uint8_t)(s >> 16); m_selectors[3] = (uint8_t)(s >> 24); }

		inline bool any_selectors_transparent() const
		{
			uint32_t sel_bits = get_selector_bits();
			for (uint32_t i = 0; i < 16; i++)
			{
				if ((sel_bits & 3) == 3)
					return true;

				sel_bits >>= 2;
			}
			return false;
		}

		static inline uint16_t pack_color(const color32& color, bool scaled, uint32_t bias = 127U)
		{
			uint32_t r = color.r, g = color.g, b = color.b;
			if (scaled)
			{
				r = (r * 31U + bias) / 255U;
				g = (g * 63U + bias) / 255U;
				b = (b * 31U + bias) / 255U;
			}
			return static_cast<uint16_t>(minimum(b, 31U) | (minimum(g, 63U) << 5U) | (minimum(r, 31U) << 11U));
		}

		static inline uint16_t pack_unscaled_color(uint32_t r, uint32_t g, uint32_t b) { return static_cast<uint16_t>(b | (g << 5U) | (r << 11U)); }

		static inline void unpack_color(uint32_t c, uint32_t& r, uint32_t& g, uint32_t& b)
		{
			r = (c >> 11) & 31;
			g = (c >> 5) & 63;
			b = c & 31;

			r = (r << 3) | (r >> 2);
			g = (g << 2) | (g >> 4);
			b = (b << 3) | (b >> 2);
		}

		static inline void unpack_color_unscaled(uint32_t c, uint32_t& r, uint32_t& g, uint32_t& b)
		{
			r = (c >> 11) & 31;
			g = (c >> 5) & 63;
			b = c & 31;
		}
	};

	struct bc4_block
	{
		enum { cBC4SelectorBits = 3, cTotalSelectorBytes = 6, cMaxSelectorValues = 8 };
		uint8_t m_endpoints[2];

		uint8_t m_selectors[cTotalSelectorBytes];

		inline uint32_t get_low_alpha() const { return m_endpoints[0]; }
		inline uint32_t get_high_alpha() const { return m_endpoints[1]; }
		inline bool is_alpha6_block() const { return get_low_alpha() <= get_high_alpha(); }

		inline uint64_t get_selector_bits() const
		{
			return ((uint64_t)((uint32_t)m_selectors[0] | ((uint32_t)m_selectors[1] << 8U) | ((uint32_t)m_selectors[2] << 16U) | ((uint32_t)m_selectors[3] << 24U))) |
				(((uint64_t)m_selectors[4]) << 32U) |
				(((uint64_t)m_selectors[5]) << 40U);
		}

		inline void set_selector_bits(uint64_t v)
		{
			for (uint32_t i = 0; i < 6; i++)
			{
				m_selectors[i] = (uint8_t)v;
				v >>= 8;
			}
		}

		inline uint32_t get_selector(uint32_t x, uint32_t y, uint64_t selector_bits) const
		{
			assert((x < 4U) && (y < 4U));
			return (selector_bits >> (((y * 4) + x) * cBC4SelectorBits)) & (cMaxSelectorValues - 1);
		}

		static inline uint32_t get_block_values6(uint8_t* pDst, uint32_t l, uint32_t h)
		{
			pDst[0] = static_cast<uint8_t>(l);
			pDst[1] = static_cast<uint8_t>(h);
			pDst[2] = static_cast<uint8_t>((l * 4 + h) / 5);
			pDst[3] = static_cast<uint8_t>((l * 3 + h * 2) / 5);
			pDst[4] = static_cast<uint8_t>((l * 2 + h * 3) / 5);
			pDst[5] = static_cast<uint8_t>((l + h * 4) / 5);
			pDst[6] = 0;
			pDst[7] = 255;
			return 6;
		}

		static inline uint32_t get_block_values8(uint8_t* pDst, uint32_t l, uint32_t h)
		{
			pDst[0] = static_cast<uint8_t>(l);
			pDst[1] = static_cast<uint8_t>(h);
			pDst[2] = static_cast<uint8_t>((l * 6 + h) / 7);
			pDst[3] = static_cast<uint8_t>((l * 5 + h * 2) / 7);
			pDst[4] = static_cast<uint8_t>((l * 4 + h * 3) / 7);
			pDst[5] = static_cast<uint8_t>((l * 3 + h * 4) / 7);
			pDst[6] = static_cast<uint8_t>((l * 2 + h * 5) / 7);
			pDst[7] = static_cast<uint8_t>((l + h * 6) / 7);
			return 8;
		}

		static inline uint32_t get_block_values(uint8_t* pDst, uint32_t l, uint32_t h)
		{
			if (l > h)
				return get_block_values8(pDst, l, h);
			else
				return get_block_values6(pDst, l, h);
		}
	};
}

#endif // #ifndef RGBCX_RDO_INCLUDE_H
