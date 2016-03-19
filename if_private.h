#pragma once

#include <stdlib.h>
#include <mxml.h>

#define STR_MAX 256
#define LSHW_STR_LEN STR_MAX
#define ETHERNET_CLASS "0200"
#define DEVICES_SIZE 20
#define DPDK_SIZE 3

typedef struct driver
{
    char name[STR_MAX];
    int found;
} driver;

typedef struct
{
    char slot[STR_MAX];
    char class[STR_MAX];
    char vendor[STR_MAX];
    char device[STR_MAX];
    char svendor[STR_MAX];
    char sdevice[STR_MAX];
    char phy_slot[STR_MAX];
    char rev[STR_MAX];
    char driver[STR_MAX];
    char driver_str[STR_MAX];
    char module[STR_MAX];
    char module_str[STR_MAX];
    char interface[STR_MAX];
    char progif[STR_MAX];
    char active[STR_MAX];
    int ssh_if;
} device;

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


int parse_file(const char * fname, lshw_t ** out);

void get_lshw_stats(lshw_t * net, mxml_node_t * tree);


char* check_output(const char* cmd);

const char* find_module(const char* mod);

void check_modules();

int has_driver(const char* drv);

void get_pci_device_details(device* dev);

void get_nic_details();

const char* dev_id_from_dev_name(const char* dev_name);

int unbind_one(const char* dev_id, int force);

int unbind_all(const char* dev_list[], size_t size, int force);

int bind_one(const char* dev_id, const char* driver, int force);

int bind_all(const char* dev_list[], size_t size, const char* driver, int force);

void show_status(device* kernel_drv, size_t* kernel_drv_size, device* dpdk_drv, size_t* dpdk_drv_size, device* no_drv, size_t* no_drv_size);
