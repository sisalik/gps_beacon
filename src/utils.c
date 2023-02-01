#include <stdlib.h>
#include <string.h>

#include "utils.h"

long coord_to_fixed_point(char *coord) {
    char *token;
    char *saveptr;
    long fixed_point = 0;
    long fractional_part = 0;
    int i = 0;

    token = strtok_r(coord, ".", &saveptr);
    while (token != NULL) {
        if (i == 0) {
            // Parse the first part of the coordinate as degrees
            fixed_point = strtol(token, NULL, 10) * 10000000;
        } else if (i == 1) {
            // Parse the second part of the coordinate as fractional degrees
            fractional_part = strtol(token, NULL, 10);
            fractional_part *= pow_10(7 - strlen(token));
            // Correct for negative coordinates
            if (fixed_point < 0) {
                fractional_part *= -1;
            }
            fixed_point += fractional_part;
        } else {
            // There should only be two parts to the coordinate
            return ERROR_INVALID_COORDINATE;
        }
        token = strtok_r(NULL, ".", &saveptr);
        i++;
    }

    return fixed_point;
}

long pow_10(int n) {
    long result = 1;
    for (int i = 0; i < n; i++) {
        result *= 10;
    }
    return result;
}
