#include "quickjs-aeshash.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <uchar.h>
#include <time.h>

static const int times = 100;

size_t
strlen16(const char16_t* str) {
	size_t i = 0;
	while(*str) {
		str++;
		i++;
	}
	return i;
}

char16_t*
strcpy16(char16_t* out, const char16_t* in) {
	char16_t* s = out;
	const char16_t* t = in;
	for(;;) {
		if(!(*s = *t))
			break;
		++s;
		++t;
	}
	return out;
}

char*
misaligned_strdup(const char* str, unsigned int add) {
	char* ret = aligned_alloc(0x10, strlen(str) + 0x10) + (add & 0x0f);
	strcpy(ret, str);
	return ret;
}

char16_t*
misaligned_strdup16(const char16_t* str, unsigned int add) {
	char16_t* ret = aligned_alloc(0x10, 2 * strlen16(str) + 0x10) + (add & 0x0f);
	strcpy16(ret, str);
	return ret;
}

void
misaligned_free(void* ptr) {
	free((void*)((size_t)ptr & (~0xf)));
}

typedef uint32_t hash8(const uint8_t*, size_t, uint32_t);
typedef uint32_t hash16(const uint16_t*, size_t, uint32_t);

static inline uint32_t
hash_string8(const uint8_t* str, size_t len, uint32_t h) {
	size_t i;

	for(i = 0; i < len; i++) h = h * 263 + str[i];
	return h;
}

static inline uint32_t
hash_string16(const uint16_t* str, size_t len, uint32_t h) {
	size_t i;

	for(i = 0; i < len; i++) h = h * 263 + str[i];
	return h;
}

double
timespec_to_double(const struct timespec* t) {
	return (double)t->tv_sec * 1e09 + (double)t->tv_nsec;
}

void
dump_m128i(const char* str, __m128i m) {
	uint8_t* p = (uint8_t*)&m;
	printf("%s %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x\n",
		str,
		p[0],
		p[1],
		p[2],
		p[3],
		p[4],
		p[5],
		p[6],
		p[7],
		p[8],
		p[9],
		p[10],
		p[11],
		p[12],
		p[13],
		p[14],
		p[15]);
}

void
dump_ptr(const char* str, const void* ptr) {
	printf("%s %p\n", str, ptr);
}

void
hash_str8(const char* str, uint32_t h, const char* name, hash8* fn) {
	size_t len = strlen(str);
	uint32_t hash;
	struct timespec t1, t2;

	clock_gettime(CLOCK_MONOTONIC_RAW, &t1);

	for(int i = 0; i < times; i++) hash = fn((const uint8_t*)str, len, h);

	clock_gettime(CLOCK_MONOTONIC_RAW, &t2);

	printf("%-13s [%zu] (0x%1lx) %08x (%6.2lfns) '%s'\n",
		name,
		len,
		((size_t)str) & 0xf,
		hash,
		(timespec_to_double(&t2) - timespec_to_double(&t1)) / times,
		str);
}

void
hash_str16(const char16_t* str, uint32_t h, const char* name, hash16* fn) {
	size_t len = strlen16(str);
	uint32_t hash;
	struct timespec t1, t2;

	clock_gettime(CLOCK_MONOTONIC_RAW, &t1);

	for(int i = 0; i < times; i++) hash = fn(str, len, h);

	clock_gettime(CLOCK_MONOTONIC_RAW, &t2);

	printf("%-13s [%zu] (0x%1lx) %08x (%6.2lfns) '%c%c%c%c%c%c%c%c...'\n",
		name,
		len,
		((size_t)str) & 0xf,
		hash,
		(timespec_to_double(&t2) - timespec_to_double(&t1)) / times,
		str[0],
		str[1],
		str[2],
		str[3],
		str[4],
		str[5],
		str[6],
		str[7]);
}

int
test(unsigned int misalignment) {
	char* strings[] = {
		misaligned_strdup("quickjs - quickjs - quickjs - quickjs - quickjs - quickjs - quickjs", misalignment),
		misaligned_strdup("getOwnPropertyDescriptors", misalignment),
		misaligned_strdup("BigUint64Array", misalignment)};

	char16_t* strings16[] = {
		misaligned_strdup16(u"quickjs - quickjs - quickjs - quickjs - quickjs - quickjs - quickjs", misalignment),
		misaligned_strdup16(u"getOwnPropertyDescriptors", misalignment),
		misaligned_strdup16(u"BigUint64Array", misalignment)};

	hash_str8(strings[0], 0, "hash_string8", &hash_string8);
	hash_str8(strings[0], 0, "aeshash8", &aeshash8);
	hash_str8(strings[1], 0, "hash_string8", &hash_string8);
	hash_str8(strings[1], 0, "aeshash8", &aeshash8);
	hash_str8(strings[2], 0, "hash_string8", &hash_string8);
	hash_str8(strings[2], 0, "aeshash8", &aeshash8);
	hash_str16(strings16[0], 0, "hash_string16", &hash_string16);
	hash_str16(strings16[0], 0, "aeshash16", &aeshash16);
	hash_str16(strings16[1], 0, "hash_string16", &hash_string16);
	hash_str16(strings16[1], 0, "aeshash16", &aeshash16);
	hash_str16(strings16[2], 0, "hash_string16", &hash_string16);
	hash_str16(strings16[2], 0, "aeshash16", &aeshash16);

	for(int i = 0; i < 3; i++) {
		misaligned_free(strings[i]);
		misaligned_free(strings16[i]);
	}
	return 0;
}

int
main() {
	test(3);
	test(4);
	test(7);
	test(8);
	test(0);
}
