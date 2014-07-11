/*
 * (c) Copyright 2012 EVRYTHNG Ltd London / Zurich
 * www.evrythng.com
 *
 * --- DISCLAIMER ---
 *
 * EVRYTHNG provides this source code "as is" and without warranty of any kind,
 * and hereby disclaims all express or implied warranties, including without
 * limitation warranties of merchantability, fitness for a particular purpose,
 * performance, accuracy, reliability, and non-infringement.
 *
 * Author: Michel Yerly
 *
 */
#include "util.h"

Serial dbg(USBTX, USBRX);


void sprinti64(char* dest, int64_t v, char** end)
{
    int len;
    if (v != 0x8000000000000000LL) {
        char str[20];
        int p = sizeof(str);
        str[--p] = '\0';
        str[p-1] = '0';
        bool neg = false;
        if (v < 0) {
            v = -v;
            neg = true;
        }
        while (v > 0) {
            str[--p] = '0' + (v % 10);
            v /= 10;
        }
        if (neg) {
            str[--p] = '-';
        }
        len = sizeof(str) - p;
        strncpy(dest, str + p, len);
    } else {
        len = 20;
        strncpy(dest, "-9223372036854775808", len);
    }    *end = dest + len;
}