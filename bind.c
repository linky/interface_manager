#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <dirent.h>

#include "dpdk_misc.h"
#include "lshw.h"

static driver dpdk_drivers[] = { {"igb_uio", 0}, {"vfio-pci", 0}, {"uio_pci_generic", 0} };

static device devices[DEVICES_SIZE];
static size_t devices_size = 0;

static size_t
read_all_file(const char *fname, char *buf, size_t size)
{
	FILE *file = fopen(fname, "r");
	int flen = fread(buf, 1, size, file);
	fclose(file);

	return flen;
}

static int
find_file(const char *name, const char *dir)
{
	DIR *dirp = opendir(dir);
	struct dirent *dp = NULL;
	while ((dp = readdir(dirp)) != NULL) {
		if (!strcmp(dp->d_name, name)) {
			closedir(dirp);
			return 1;
		}
	}
	closedir(dirp);

	return 0;
}

void
reset_dpdk_devices_table(void)
{
	memset(devices, 0, sizeof (devices));
	devices_size = 0;
}

void
device_to_str(const device * dev, char *str)
{
	sprintf(str,
		"Slot:\t%s\n"
		"Class:\t%s\n"
		"Vendor:\t%s\n"
		"Device:\t%s\n"
		"SVendor:\t%s\n"
		"SDevice:\t%s\n"
		"PhySlot:\t%s\n"
		"Rev:\t%s\n"
		"Driver:\t%s\n"
		"DriverStr:\t%s\n"
		"Module:\t%s\n"
		"ModuleStr:\t%s\n"
		"Interface:\t%s\n"
		"ProgIf:\t%s\n"
		"Active:\t%s\n"
		"ssh_if:\t%d\n",
		dev->slot,
		dev->class,
		dev->vendor,
		dev->device,
		dev->svendor,
		dev->sdevice,
		dev->phy_slot,
		dev->rev,
		dev->driver,
		dev->driver_str, dev->module, dev->module_str, dev->interface, dev->progif, dev->active, dev->ssh_if);
}

char *
check_output(const char *cmd)
{
	FILE *f = popen(cmd, "r");
	if (!f) {
		return NULL;
	}

	static char buffer[STR_MAX * 16];
	memset(buffer, 0, sizeof (buffer));
	fread(buffer, 1, sizeof (buffer), f);
	pclose(f);

	return buffer;
}

const char *
find_module(const char *mod)
{
	char *rte_sdk = getenv("RTE_SDK");
	char *rte_target = getenv("RTE_TARGET");
	if (rte_sdk && rte_target) {
		static char path[PATH_MAX];
		sprintf(path, "%s/%s/kmod/%s.ko", rte_sdk, rte_target, mod);
		if (!access(path, R_OK)) {
			return path;
		}
	}

	char cmd[STR_MAX];
	sprintf(cmd, "modinfo -n %s", mod);
	char *depmod_out = check_output(cmd);
	if (!access(depmod_out, R_OK) && strcasestr(depmod_out, "error") == NULL) {
		return depmod_out;
	}

	static char fname[STR_MAX];
	memset(fname, 0, sizeof (fname));
	sprintf(fname, "%s%s", mod, ".ko");
	if (find_file(fname, ".") && !access(fname, R_OK)) {
		return fname;
	}

	return NULL;
}

int
check_dpdk_modules(void)
{
	char modules[4096];
	read_all_file("/proc/modules", modules, sizeof (modules));
	size_t i, j;
	char *module;
	int c = 0;
	char tmp[STR_MAX];

	module = strtok(modules, "\n");
	while (module != NULL) {
		for (i = 0; i < DPDK_SIZE; ++i) {
			if (strstr(module, dpdk_drivers[i].name)) {
				dpdk_drivers[i].found = 1;
				c++;
			} else {
				strcpy(tmp, dpdk_drivers[i].name);

				for (j = 0; j < strlen(tmp); ++j)	// "_" -> "-"
				{
					if (tmp[j] == '_') {
						tmp[j] = '-';
					}
				}

				if (strstr(module, tmp)) {
					dpdk_drivers[i].found = 1;
					c++;
				}
			}
		}
		module = strtok(NULL, "\n");
	}
	return (c);
}

int
has_driver(const char *drv)
{
	unsigned i;

	for (i = 0; i < devices_size; ++i) {
		if (!strcmp(devices[i].slot, drv) && devices[i].driver_str) {
			return 1;
		}
	}

	return 0;
}

void
get_pci_device_details(device * dev)
{
	strcpy(dev->active, "");

	char cmd[STR_MAX];
	sprintf(cmd, "lspci -vmmks %s", dev->slot);
	char *extra_info = check_output(cmd);

	char *line;
	line = strtok(extra_info, "\n");
	while (line != NULL) {
		if (line == NULL || strlen(line) == 0) {
			continue;
		}

		const char *name = strsep(&line, "\t");
		const char *value = strsep(&line, "\t");

		char *field = NULL;
		if (!strcmp(name, "Class:"))
			field = dev->class;
		else if (!strcmp(name, "Vendor:"))
			field = dev->vendor;
		else if (!strcmp(name, "Device:"))
			field = dev->device;
		else if (!strcmp(name, "SVendor:"))
			field = dev->svendor;
		else if (!strcmp(name, "SDevice:"))
			field = dev->sdevice;
		else if (!strcmp(name, "Rev:"))
			field = dev->rev;
		else if (!strcmp(name, "Driver:"))
			field = dev->driver;
		else if (!strcmp(name, "Module:"))
			field = dev->module;
		else if (!strcmp(name, "Interface:"))
			field = dev->interface;
		else if (!strcmp(name, "PhySlot:"))
			field = dev->phy_slot;

		if (field) {
			strcpy(field, value);
		}

		line = strtok(NULL, "\n");
	}
}

void
get_dpdk_nic_details(void)
{
	device dev;
	size_t j, i, o;
	char *dev_line;

	memset(devices, 0, sizeof (devices));
	memset((char *)&dev, 0, sizeof(dev));

	char *dev_lines = check_output("lspci -Dvmmn");
	dev_line = strtok(dev_lines, "\n");
	i = 0;
	while (dev_line != NULL) {
		if (strstr(dev_line, "Slot")) {
			i = 0;
			if (!strcmp(dev.class, ETHERNET_CLASS)) {
				devices[devices_size] = dev;
				++devices_size;
			}
			if (i == 0)
				memset((char *)&dev, 0, sizeof(dev));
		}

		const char *name = strsep(&dev_line, "\t");
		const char *value = strsep(&dev_line, "\t");
		char *field = NULL;
		if (!strcmp(name, "Slot:"))
			field = dev.slot;
		else if (!strcmp(name, "Class:"))
			field = dev.class;
		else if (!strcmp(name, "Vendor:"))
			field = dev.vendor;
		else if (!strcmp(name, "Device:"))
			field = dev.device;
		else if (!strcmp(name, "SVendor:"))
			field = dev.svendor;
		else if (!strcmp(name, "SDevice:"))
			field = dev.sdevice;
		else if (!strcmp(name, "Rev:"))
			field = dev.rev;
		else if (!strcmp(name, "Driver:"))
			field = dev.driver;
		else if (!strcmp(name, "Module:"))
			field = dev.module;
		else if (!strcmp(name, "ProgIf:"))
			field = dev.progif;
		else if (!strcmp(name, "Interface:"))
			field = dev.interface;
		else if (!strcmp(name, "PhySlot:"))
			field = dev.phy_slot;

		if (field) {
			strcpy(field, value);
		}

		dev_line = strtok(NULL, "\n");
		++i;
	}

	if ((dev_line == NULL) && !strcmp(dev.class, ETHERNET_CLASS)) {
		devices[devices_size] = dev;
		++devices_size;
	}

	char ssh_if[DEVICES_SIZE][STR_MAX] = { {0} };
	size_t ssh_if_size = 0;
	char *route = check_output("ip -o route");
	char *new_route = calloc(strlen(route), 1);
	char *line;
	line = strtok(route, "\n");
	while (line != NULL) {
		if (strstr(line, "169.254") == NULL) {
			strcat(new_route, line);
		}
		line = strtok(NULL, "\n");
	}

	line = strtok(new_route, " ");
	while (line != NULL) {
		if (!strcmp(line, "dev")) {
			line = strtok(NULL, " ");
			if (line == NULL) {
				break;
			}
			strcat(ssh_if[ssh_if_size], line);
			++ssh_if_size;
			continue;
		}
		line = strtok(NULL, " ");
	}

	for (i = 0; i < devices_size; ++i) {
		get_pci_device_details(devices + i);

		for (j = 0; j < ssh_if_size; ++j) {
			if (strstr(devices[i].driver, ssh_if[j])) {
				devices[i].ssh_if = 1;
				strcpy(devices[i].active, "*Active*");
				break;
			}
		}

		if (devices[i].module_str[0]) {
			for (j = 0; j < DPDK_SIZE; ++j) {
				if (dpdk_drivers[j].found && strstr(devices[i].module_str, dpdk_drivers[j].name) == NULL) {
					char buf[STR_MAX];
					sprintf(buf, "%s,%s", devices[i].module_str, dpdk_drivers[j].name);
					strcpy(devices[i].module_str, buf);
				}
			}
		} else {
			char buf[STR_MAX] = { 0 };
			for (j = 0; j < DPDK_SIZE; ++j) {
				if (dpdk_drivers[j].found) {
					strcat(buf, dpdk_drivers[j].name);
					strcat(buf, ",");
				}
			}
			strcat(devices[i].module_str, buf);
		}

		if (has_driver(devices[i].slot)) {
			char *driver_str = NULL;
			for (o = 0; o < devices_size; ++o) {
				if (devices[o].driver_str[0]) {
					driver_str = devices[o].driver_str;
					break;
				}
			}
			if (driver_str == NULL) 
				continue;

			char modules[STR_MAX * 2];
			strcpy(modules, driver_str);
			line = strtok(modules, ",");
			while (line != NULL) {
				if (strcmp(line, driver_str)) {
					strcpy(driver_str, modules);
				}
				line = strtok(NULL, "\n");
			}
		}
	}

	free(new_route);
}

const char *
dev_id_from_dev_name(const char *dev_name)
{
	size_t i;
	for (i = 0; i < devices_size; ++i) {
		if (!strcmp(dev_name, devices[i].device)) {
			return dev_name;
		}

		static char buf[STR_MAX];
		memset(buf, 0, sizeof (buf));
		sprintf(buf, "0000:%s", dev_name);
		if (!strcmp(buf, devices[i].device)) {
			return buf;
		}

		if (strstr(devices[i].interface, dev_name)) {
			return devices[i].slot;
		}
	}

	exit(1);
	return NULL;
}

int
unbind_one(const char *dev_id, int force)
{

	size_t i;

	if (!has_driver(dev_id)) {
		return (1);
	}

	device *dev = NULL;
	for (i = 0; i < devices_size; ++i) {
		if (!strcmp(dev_id, devices[i].slot)) {
			dev = &devices[i];
			break;
		}
	}

	if (dev == NULL || (dev->ssh_if && !force)) {
		return (1);
	}

	char path[STR_MAX];
	sprintf(path, "/sys/bus/pci/drivers/%s/unbind", dev->driver);
	FILE *f = fopen(path, "a");
	if (f == NULL) {
		return (1);
	}

	memcpy(dev->driver_str, dev->driver, strlen(dev->driver));
	fwrite(dev_id, 1, strlen(dev_id), f);
	fclose(f);
	return (0);

}

int
unbind_all(const char *dev_list[], size_t size, int force)
{
	size_t i;
	int err;

	for (i = 0; i < size; ++i) {
		err = unbind_one(dev_list[i], force);
		if (err)
			return (1);
	}
	return (0);
}

int
bind_one(const char *dev_id, const char *driver, int force)
{
	device *dev = NULL;
	device tmp;
	unsigned i;
	int j, err, ret;
	char path[STR_MAX];
	const char *saved_driver = NULL;

	for (i = 0; i < devices_size; ++i) {
		if (!strcmp(dev_id, devices[i].slot)) {
			dev = devices + i;
		}
	}


	if (dev == NULL || (dev->ssh_if && !force)) 
		return (1);
	
	if (has_driver(dev_id)) {
		if (!strcmp(dev->driver_str, driver)) {
			return (1);
		} else {
			saved_driver = dev->driver_str;
			err = unbind_one(dev_id, force);
			strcpy(dev->device, "");
			if (err)
				return (1);
		}
	}

	for (j = 0; j < DPDK_SIZE; ++j) {
		if (dpdk_drivers[j].found && strstr(dpdk_drivers[j].name, driver)) {
			memset(path, 0, sizeof(path));
			sprintf(path, "/sys/bus/pci/drivers/%s/new_id", driver);
			FILE *f = fopen(path, "w");
			fprintf(f, "%04x %04x", atoi(dev->vendor), atoi(dev->device));
			fclose(f);
		}
	}

	memset(path, 0, sizeof(path));
	sprintf(path, "/sys/bus/pci/drivers/%s/bind", driver);
	FILE *f = fopen(path, "a");
	if (f == NULL) {
		if (saved_driver) {
			err = bind_one(dev_id, saved_driver, force);
			return (err);
		}
	}
	ret = fwrite(dev_id, 1, strlen(dev_id), f);
	fclose(f);
	if (ret <= 0) {
		memset((char *)&tmp, 0, sizeof(tmp));
		strcpy(tmp.slot, dev_id);
		get_pci_device_details(&tmp);
		if (!tmp.driver_str[0]) {
			return (1);
		}

		if (saved_driver == NULL) {
			err = bind_one(dev_id, saved_driver, force);
			if (err)
				return (err);
		}
	}
	return (0);
}

int
bind_all(const char *dev_list[], size_t size, const char *driver, int force)
{
	size_t i, j;
	int err;

	for (i = 0; i < size; ++i) {
		err = bind_one(dev_list[i], driver, force);
		if (err)
			return (1);
	}

	for (j = 0; j < devices_size; ++j) {
		int cont = 0;
		if (devices[j].driver_str[0]) {
			for (i = 0; i < size; ++i) {
				if (strstr(dev_list[i], devices[i].slot)) {
					cont = 1;
				}
			}
		}
		if (cont) {
			continue;
		}

		char *d = devices[j].slot;
		get_pci_device_details(&devices[j]);

		if (devices->driver_str[0]) {
			err = unbind_one(d, force);
			if (err)
				return (1);
		}
	}
	return (0);
}

void
show_status(device * kernel_drv, size_t * kernel_drv_size, device * dpdk_drv, size_t * dpdk_drv_size, device * no_drv,
	    size_t * no_drv_size)
{
	unsigned i;
	int j;

	size_t ksize = 0, dpdksize = 0, nosize = 0;
	for (i = 0; i < devices_size; ++i) {
		if (!has_driver(devices[i].slot) && ksize < *kernel_drv_size) {
			kernel_drv[ksize] = devices[i];
			++ksize;
			continue;
		}

		int found = 0;
		for (j = 0; j < DPDK_SIZE; ++j) {
			if (dpdk_drivers[j].found && strstr(devices[i].driver_str, dpdk_drivers[j].name)
			    && dpdksize < *dpdk_drv_size) {
				dpdk_drv[dpdksize] = devices[i];
				++dpdksize;
				found = 1;
			}
		}

		if (!found && nosize < *no_drv_size) {
			no_drv[nosize] = devices[i];
			++nosize;
		}
	}

	*kernel_drv_size = ksize;
	*dpdk_drv_size = dpdksize;
	*no_drv_size = nosize;
}

#ifndef NOMAIN
int
main(int argc, char *argv[])
{
	lshw_t *net; // need free
	size_t size = parse_file("/root/interface_manager/out.xml", &net);
}
#endif