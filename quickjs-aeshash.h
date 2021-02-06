#ifndef QUICKJS_AESHASH_H
#define QUICKJS_AESHASH_H

#include <immintrin.h>
#include <stdint.h>

static inline uint32_t
aeshash8(const uint8_t* s, size_t len, uint32_t h) {
	__m128i H, S;

	H = _mm_set_epi32(0, 0, 0, h);

	/* aligned on 8-byte boundary */
	if(((size_t)s & 0x07) == 0) {
		while(len >= 8) {
			/* load 8 bytes into lower half of vector register */
			S = _mm_loadl_epi64((const __m128i*)s);
			/* zero extend unsigned 8-bit integers  to packed 16-bit integers */
			S = _mm_cvtepu8_epi16(S);
			H = _mm_aesenc_si128(S, H);
			s += 8;
			len -= 8;
		}
	} else {
		while(len >= 8) {
			/* unaligned load 8 bytes into lower half of vector register */
			S = _mm_loadu_si64((const __m128i*)s);
			/* zero extend unsigned 8-bit integers  to packed 16-bit integers */
			S = _mm_cvtepu8_epi16(S);
			H = _mm_aesenc_si128(S, H);
			s += 8;
			len -= 8;
		}
	}

	if(len > 0) {
		switch(len) {
			case 1: { S = _mm_set_epi16(0, 0, 0, 0, 0, 0, 0, s[0]); break; }
			case 2: { S = _mm_set_epi16(0, 0, 0, 0, 0, 0, s[1], s[0]); break; }
			case 3: { S = _mm_set_epi16(0, 0, 0, 0, 0, s[2], s[1], s[0]); break; }
			case 4: { S = _mm_set_epi16(0, 0, 0, 0, s[3], s[2], s[1], s[0]); break; }
			case 5: { S = _mm_set_epi16(0, 0, 0, s[4], s[3], s[2], s[1], s[0]); break; }
			case 6: { S = _mm_set_epi16(0, 0, s[5], s[4], s[3], s[2], s[1], s[0]); break; }
			case 7: { S = _mm_set_epi16(0, s[6], s[5], s[4], s[3], s[2], s[1], s[0]); break; }
		}
		H = _mm_aesenclast_si128(S, H);
	}

	// horizontally xor the 4 32-bit parts of the 128-bit register
	return _mm_extract_epi32(H, 0) ^ _mm_extract_epi32(H, 1) ^ _mm_extract_epi32(H, 2) ^ _mm_extract_epi32(H, 3);
}

/*static inline uint32_t
aeshash8_alt(const uint8_t* s, size_t len, uint32_t h) {
	__m128i H = _mm_set_epi32(0, 0, 0, h);

	while(len >= 8) {
		H = _mm_aesenc_si128(_mm_set_epi16(s[7], s[6], s[5], s[4], s[3], s[2], s[1], s[0]), H);
		s += 8;
		len -= 8;
	}

	if(len > 0) {
		switch(len) {
			case 1: { S = _mm_set_epi16(0, 0, 0, 0, 0, 0, 0, s[0]); break; }
			case 2: { S = _mm_set_epi16(0, 0, 0, 0, 0, 0, s[1], s[0]); break; }
			case 3: { S = _mm_set_epi16(0, 0, 0, 0, 0, s[2], s[1], s[0]); break; }
			case 4: { S = _mm_set_epi16(0, 0, 0, 0, s[3], s[2], s[1], s[0]); break; }
			case 5: { S = _mm_set_epi16(0, 0, 0, s[4], s[3], s[2], s[1], s[0]); break; }
			case 6: { S = _mm_set_epi16(0, 0, s[5], s[4], s[3], s[2], s[1], s[0]); break; }
			case 7: { S = _mm_set_epi16(0, s[6], s[5], s[4], s[3], s[2], s[1], s[0]); break; }
		}
		H = _mm_aesenclast_si128(S, H);
	}

	// horizontally xor the 4 32-bit parts of the 128-bit register
	return _mm_extract_epi32(H, 0) ^ _mm_extract_epi32(H, 1) ^ _mm_extract_epi32(H, 2) ^ _mm_extract_epi32(H, 3);
}*/

static inline uint32_t
aeshash16(const uint16_t* s, size_t len, uint32_t h) {
	__m128i H, S;

	H = _mm_set_epi32(0, 0, 0, h);

	/* aligned on 16-byte boundary */
	if(((size_t)s & 0x0f) == 0) {
		while(len >= 8) {
			/* load 8 shorts into vector register */
			S = _mm_load_si128((const __m128i*)s);
			H = _mm_aesenc_si128(S, H);
			s += 8;
			len -= 8;
		}
	} else {
		while(len >= 8) {
			/* load 8 unaligned shorts into vector register */
			S = _mm_lddqu_si128((const __m128i*)s);
			H = _mm_aesenc_si128(S, H);
			s += 8;
			len -= 8;
		}
	}

	if(len > 0) {
		switch(len) {
			case 1: { S = _mm_set_epi16(0, 0, 0, 0, 0, 0, 0, s[0]); break; }
			case 2: { S = _mm_set_epi16(0, 0, 0, 0, 0, 0, s[1], s[0]); break; }
			case 3: { S = _mm_set_epi16(0, 0, 0, 0, 0, s[2], s[1], s[0]); break; }
			case 4: { S = _mm_set_epi16(0, 0, 0, 0, s[3], s[2], s[1], s[0]); break; }
			case 5: { S = _mm_set_epi16(0, 0, 0, s[4], s[3], s[2], s[1], s[0]); break; }
			case 6: { S = _mm_set_epi16(0, 0, s[5], s[4], s[3], s[2], s[1], s[0]); break; }
			case 7: { S = _mm_set_epi16(0, s[6], s[5], s[4], s[3], s[2], s[1], s[0]); break; }
		}
		H = _mm_aesenclast_si128(S, H);
	}

	// horizontally xor the 4 32-bit parts of the 128-bit register
	return _mm_extract_epi32(H, 0) ^ _mm_extract_epi32(H, 1) ^ _mm_extract_epi32(H, 2) ^ _mm_extract_epi32(H, 3);
}

#endif
