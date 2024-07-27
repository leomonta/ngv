/**
 * Generic, potentially used anywhere functions
 */
#pragma once

#include <stdint.h>

// just a standard
typedef uint32_t bitfield;
typedef uint64_t longBitfield;

/**
 * Retrives the values at index `i` from the bitfield `bf`
 *
 * @param[in]
 * @param[in]
 *
 * @return the value of the bit at the index `i` of `false` if out of bounds
 */
bool at_bit(const bitfield bf, const unsigned char i);

/**
 * Retrives the values at index `i` from the longBitfield `lbf`
 *
 * @param[in]
 * @param[in]
 *
 * @return the value of the bit at the index `i` of `false` if out of bounds
 */
bool at_bit_long(const longBitfield lbf, const unsigned char i);

/**
 * Set the values at index `i` of the bitfield `bf`
 *
 * @param[in]
 * @param[in]
 *
 */
void set_bit(bitfield bf, const unsigned char i);

/**
 * Set the values at index `i` of the longBitfield `bf`
 *
 * @param[in]
 * @param[in]
 *
 */
void set_bit_long(longBitfield lbf, const unsigned char i);

/**
 * Return the amout of bits set in the bitfield (if on x86 it will try to use the assembly instruction)
 *
 * @param[in] the `bitfield` from where to count the bits
 *
 * @return the number if bits set in bitfield
 */
uint32_t popcnt(bitfield bf);
