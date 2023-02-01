#ifndef GPS_BEACON_SRC_UTILS_H_
#define GPS_BEACON_SRC_UTILS_H_

// Custom error codes
/** Operation completed successfully */
#define SUCCESS 0
/** Unrecognised command */
#define ERROR_UNRECOGNISED_CMD 1
/** Missing arguments for command */
#define ERROR_MISSING_ARGUMENTS 2
/** Invalid arguments for command */
#define ERROR_INVALID_ARGUMENTS 3
/** Invalid coordinate format */
#define ERROR_INVALID_COORDINATE 0xFFFFFFFF

/** Command syntax: separator between command and arguments */
#define CMD_SEPARATOR " "

/** Extract a byte at offset n from numeric variable x */
#define GET_BYTE(n, x) ((x >> (8 * n)) & 0xFF)

/**
 * @brief Convert a coordinate string to a fixed point number
 *
 * For example, "60.1234567" will be converted to 60.1234567 * 10^7 = 601234567
 *
 * @param coord Coordinate string (e.g. "60.1234567")
 * @retval Fixed point number (e.g. 601234567) on success
 * @retval ERROR_INVALID_COORDINATE on failure
 */
long coord_to_fixed_point(char *coord);

/**
 * @brief Calculate the nth power of 10
 *
 * @param n Power
 *
 */
long pow_10(int n);

#endif  // GPS_BEACON_SRC_UTILS_H_
