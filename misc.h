#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

static uint64_t IP_to_Int(const char * str)
{
    union {
        uint8_t data[8];
        uint64_t ip;
    } res;
    res.ip = 0;
    char buf[STR_MAX];
    strcpy(buf, str);

    char *dig = strtok(buf, ".:");
    int i;
    for (i = 0; dig != NULL; ++i) {
        res.data[i] = atoi(dig);
        dig = strtok(NULL, ".:");
    }

    return res.ip;
}

static uint64_t Speed_to_Int(const char * str)
{
    int speed = 0;
    char unit = 0;
    sscanf(str, "%d%c", &speed, &unit);

    if (unit == 'M')
        return speed *= 1000000;
    else if (unit == 'k' || unit == 'K')
        return speed*1000;

    return speed;
}