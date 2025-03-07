/**
 * Generic, potentially used anywhere functions
 */
#pragma once

#include <stddef.h>
#include <stdint.h>

// just a standard
typedef uint32_t bitfield;
typedef uint64_t longBitfield;

#define TEST_MALLOC(ptr)                                                 \
	if (ptr == NULL) {                                                   \
		llog(LOG_FATAL, "[MEM] 'malloc' failed: %s\n", strerror(errno)); \
		return false;                                                    \
	}

#define TEST_MALLOC_RET(ptr, ret)                                        \
	if (ptr == NULL) {                                                   \
		llog(LOG_FATAL, "[MEM] 'malloc' failed: %s\n", strerror(errno)); \
		return ret;                                                      \
	}

/**
 * Retrives the values at index `i` from the bitfield `bf`
 *
 * @param[in] `bf` the `bitfield` to retrive the bit from
 * @param[in] `i` the index of the desired bit
 *
 * @return the value of the bit at the index `i` of `false` if out of bounds
 */
bool at_bit(const bitfield bf, const unsigned char i);

/**
 * Retrives the values at index `i` from the longBitfield `lbf`
 *
 * @param[in] `lbf` the long `bitfield` to retrive the bit from
 * @param[in] `i` the index of the desired bit
 *
 * @return the value of the bit at the index `i` of `false` if out of bounds
 */
bool at_bit_long(const longBitfield lbf, const unsigned char i);

/**
 * Set the value `true` at index `i` of the bitfield `bf`
 *
 * @param[in] `bf` the `bitfield` to set the bit at
 * @param[in] `i` the infex of the bit to get
 *
 */
void set_bit(bitfield *bf, const unsigned char i);

/**
 * Set the value `true` at index `i` of the longBitfield `bf`
 *
 * @param[in] `lbf` the long `bitfield` to set the bit at
 * @param[in] `i` the infex of the bit to get
 *
 */
void set_bit_long(longBitfield *lbf, const unsigned char i);

/*
 * Set the value `false` at index `i` of the bitfield `bf`
 *
 * @param[in] `bf` the `bitfield` to set the bit at
 * @param[in] `i` the infex of the bit to get
 *
 */
void unset_bit(bitfield *bf, const unsigned char i);

/**
 * Set the value `false` at index `i` of the longBitfield `bf`
 *
 * @param[in] `lbf` the long `bitfield` to set the bit at
 * @param[in] `i` the infex of the bit to get
 *
 */
void unset_bit_long(longBitfield *lbf, const unsigned char i);

/**
 * Return the amout of bits set in the bitfield (if on x86 it will try to use the assembly instruction)
 *
 * @param[in] `bf` the `bitfield` from where to count the bits
 *
 * @return the number of set bits in `bf`
 */
uint32_t popcnt(bitfield bf);

/**
 * clamps `val` between `min` and `max`
 *
 * @param[in] `val` the value to clamp
 * @param[in] `min` the lower bound
 * @param[in] `max` the upper bound
 *
 * @return the clamped value
 */
uint32_t clamp(uint32_t val, uint32_t min, uint32_t max);
