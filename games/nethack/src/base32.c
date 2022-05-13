/* base32 helper functions */
/* Copyright (c) Patric Mueller. */
/* NetHack may be freely redistributed.  See license for details. */

#include <string.h>
#include <stdio.h>

/**
 * base32 mapping.
 *
 * The mapping is based on the Crockford base32 encoding,
 * with the visually similar characters mapped to each other.
 */
static int base32_decoding[] = {
    10, /* A */
    11, /* B */
    12, /* C */
    13, /* D */
    14, /* E */
    15, /* F */
    16, /* G */
    17, /* H */
    1,  /* I */
    18, /* J */
    19, /* K */
    1,  /* L */
    20, /* M */
    21, /* N */
    0,  /* O */
    22, /* P */
    23, /* Q */
    24, /* R */
    25, /* S */
    26, /* T */
    0,  /* U */
    27, /* V */
    28, /* W */
    29, /* X */
    30, /* Y */
    31, /* Z */
};

/** Returns unicode codepoint of character according to selected graphics mode. */
long
decode_base32(const char* str)
{
    size_t len = strlen(str);
    unsigned long ret = 0;
    int i;
    for (i=0; i<len; i++) {
        if (str[i] == 'o' || str[i] == 'u') {
            /* ignore */
        } else if (str[i] >= '0' && str[i] <= '9') {
            ret = ret * 32 + str[i] - '0';
        } else if (str[i] >= 'a' && str[i] <= 'z') {
            int index = str[i] - 'a';
            ret = ret * 32 + base32_decoding[index];
        } else if (str[i] >= 'A' && str[i] <= 'Z') {
            int index = str[i] - 'A';
            ret = ret * 32 + base32_decoding[index];
        }
    }
    return ret;
}

static const char* base32_encoding[] = {
    "o", /*  0 */
    "i", /*  1 */
    "2", /*  2 */
    "3", /*  3 */
    "4", /*  4 */
    "5", /*  5 */
    "6", /*  6 */
    "7", /*  7 */
    "8", /*  8 */
    "9", /*  9 */
    "a", /* 10 */
    "b", /* 11 */
    "c", /* 12 */
    "d", /* 13 */
    "e", /* 14 */
    "f", /* 15 */
    "g", /* 16 */
    "h", /* 17 */
    "j", /* 18 */
    "k", /* 19 */
    "m", /* 20 */
    "n", /* 21 */
    "p", /* 22 */
    "q", /* 23 */
    "r", /* 24 */
    "s", /* 25 */
    "t", /* 26 */
    "v", /* 27 */
    "w", /* 28 */
    "x", /* 29 */
    "y", /* 30 */
    "z", /* 31 */
};

static char base32_output[100];
const char *
encode_base32(unsigned long input)
{
    base32_output[0] = '\0';
    if (input == 0) {
        return "0";
    }
    while (input > 0) {
        int index = input % 32;
        strcat(base32_output, base32_encoding[index]);
        input /= 32;
    }
    int len = strlen(base32_output);
    int i;
    for (i=0; i<len/2; i++) {
        char tmp = base32_output[i];
        base32_output[i] = base32_output[len-1-i];
        base32_output[len-1-i] = tmp;
    }
    return base32_output;
}

/* base32.c */
