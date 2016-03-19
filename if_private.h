#pragma once

#include <stdlib.h>
#include <mxml.h>

#include "if_manager.h"

int parse_file(const char * fname, interface_t ** out);

void get_lshw_stats(interface_t * net, mxml_node_t * tree);


char* check_output(const char* cmd);

const char* find_module(const char* mod);

int check_dpdk_modules(void);

int has_driver(const char* drv);

void get_pci_device_details(interface_t* dev);

void get_dpdk_nic_details(void);

const char* dev_id_from_dev_name(const char* dev_name);

int unbind_one(const char* dev_id, int force);

int unbind_all(const char* dev_list[], size_t size, int force);

int bind_one(const char* dev_id, const char* driver, int force);

int bind_all(const char* dev_list[], size_t size, const char* driver, int force);

void show_status(interface_t* kernel_drv, size_t* kernel_drv_size, interface_t* dpdk_drv, size_t* dpdk_drv_size, interface_t* no_drv, size_t* no_drv_size);
