#pragma once

#include <mxml.h>

typedef struct {
    char InterfaceName[LSHW_STR_LEN];
    char MacAdress[LSHW_STR_LEN];
    char InterfaceType[LSHW_STR_LEN];
    char Link[LSHW_STR_LEN];
    char Product[LSHW_STR_LEN];
    char Model[LSHW_STR_LEN];
    char Driver[LSHW_STR_LEN];
    char DriverVersion[LSHW_STR_LEN];
    char PciLocation[LSHW_STR_LEN];
    char IPv4[LSHW_STR_LEN];
    char IPv6[LSHW_STR_LEN];
    char Speed[LSHW_STR_LEN];
    char Status[LSHW_STR_LEN];
    char RSS[LSHW_STR_LEN];
} lshw_t;

void get_lshw_stats(lshw_t * net, mxml_node_t * tree);