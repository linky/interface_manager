#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <dirent.h>

#include "if_private.h"
#include "misc.h"

#define ETHERNET_CLASS "0200"
#define DEVICES_SIZE 20
#define DPDK_SIZE 3

typedef struct
{
    char name[STR_MAX];
    int found;
} driver_t;

static driver_t dpdk_drivers[] = {{"igb_uio", 0}, {"vfio-pci", 0}, {"uio_pci_generic", 0} };

static interface_t devices[DEVICES_SIZE];
static size_t devices_size = 0;

int
parse_file(const char * fname, interface_t ** out)
{
    FILE *file = fopen(fname, "r");
    if (!file)
        return -1;

    fseek(file, 0, SEEK_END);
    size_t flen = ftell(file);
    if (!flen)
        return -1;
    fseek(file, 0, SEEK_SET);

    char *buf = malloc(flen);
    if (!buf)
        return -1;
    flen = fread(buf, 1, flen, file);
    if (!flen) {
        free(buf);
        return -1;
    }

    mxml_node_t *tree = mxmlLoadString(NULL, buf, MXML_OPAQUE_CALLBACK);
    mxml_node_t *it =NULL;
    interface_t *net = NULL;
    int cnt = 0;

    for(it = mxmlFindElement(tree, tree, "node", NULL, NULL, MXML_DESCEND);
        it != NULL; it = mxmlFindElement(it, tree, "node", NULL, NULL, MXML_DESCEND)) {

        net = (interface_t *)realloc(net, (cnt + 1) * sizeof(interface_t));
        get_lshw_stats(net + cnt, it);
        cnt++;
    }

    free(buf);
    fclose(file);
    *out = net;

    return cnt;
}

void
get_lshw_stats(interface_t * net, mxml_node_t * tree)
{
    mxml_node_t *temp;

    int a = 1;
    unsigned i;
    temp = mxmlFindElement(tree, tree, "logicalname", NULL, NULL, MXML_DESCEND);
    if (temp)
        strcpy(net->InterfaceName, mxmlGetOpaque(temp));
    else
        strcpy(net->InterfaceName, "No data");
    printf("%d", a);

    temp = mxmlFindElement(tree, tree, "serial", NULL, NULL, MXML_DESCEND);
    if (temp)
        strcpy(net->MacAdress, mxmlGetOpaque(temp));
    else
        strcpy(net->MacAdress, "No data");

    temp = mxmlFindElement(tree, tree, "capability", "id", "physical", MXML_DESCEND);
    if (temp)
        strcpy(net->InterfaceType, mxmlGetOpaque(temp));
    else
        strcpy(net->InterfaceType, "No data");

    temp = mxmlFindElement(tree, tree, "product", NULL, NULL, MXML_DESCEND);
    if (temp)
        strcpy(net->Product, mxmlGetOpaque(temp));
    else
        strcpy(net->Product, "No data");
    printf("%d", a);

    if (temp) {
        int begin = -1;
        for (i = 0; i < strlen(net->Product); ++i) {
            if (net->Product[i] > 47 && net->Product[i] < 58) {
                begin = i;
                break;
            }
        }

        if (begin > -1)
            for (i = begin + 1; i < strlen(net->Product); ++i) {
                if (net->Product[i] == ' ' || i == strlen(net->Product) - 1) {
                    strncpy(net->Model, net->Product + begin, i - begin);
                    break;
                }
            }
    }

    temp = mxmlFindElement(tree, tree, "setting", "id", "driver", MXML_DESCEND);
    if (temp)
        strcpy(net->Driver, mxmlElementGetAttr(temp, "value"));
    else
        strcpy(net->Driver, "No data");

    temp = mxmlFindElement(tree, tree, "setting", "id", "driverversion", MXML_DESCEND);
    if (temp)
        strcpy(net->DriverVersion, mxmlElementGetAttr(temp, "value"));
    else
        strcpy(net->DriverVersion, "No data");

    temp = mxmlFindElement(tree, tree, "businfo", NULL, NULL, MXML_DESCEND);
    if (temp)
        strcpy(net->PciLocation, mxmlGetOpaque(temp));
    else
        strcpy(net->PciLocation, "No data");

    temp = mxmlFindElement(tree, tree, "setting", "id", "link", MXML_DESCEND);
    if (temp && strcmp("yes", mxmlElementGetAttr(temp, "value")))
        net->Link = 1;
    else if (temp && strcmp("no", mxmlElementGetAttr(temp, "value")))
        net->Link = 0;
    else
        net->Link = -1;

    temp = mxmlFindElement(tree, tree, "setting", "id", "ip", MXML_DESCEND);
    if (temp)
        net->IPv4 = IP_to_Int(mxmlElementGetAttr(temp, "value"));
    else
        net->IPv4 = 0;

    temp = mxmlFindElement(tree, tree, "setting", "id", "ipv6", MXML_DESCEND);
    if (temp)
        net->IPv6 = IP_to_Int(mxmlElementGetAttr(temp, "value"));
    else
        net->IPv6 = 0;

    temp = mxmlFindElement(tree, tree, "capacity", NULL, NULL, MXML_DESCEND);
    if (temp)
        net->Speed = atoi(mxmlGetOpaque(temp));
    else
        net->Speed = 0;

    temp = mxmlFindElement(tree, tree, "setting", "id", "multicast", MXML_DESCEND);
    if (temp && strcmp("yes", mxmlElementGetAttr(temp, "value")))
        net->RSS = 1;
    else if (temp && strcmp("no", mxmlElementGetAttr(temp, "value")))
        net->RSS = 0;
    else
        net->RSS = -1;


    mxmlDelete(temp);
}

void dump_lshw(interface_t * net)
{
    printf("\n");
    printf("Interface name: %s \n", net->InterfaceName);
    printf("MAC adress: %s \n", net->MacAdress);
    printf("Speed: %lu \n", net->Speed);
    printf("Link: %u \n", net->Link);
    printf("Status: %s \n", net->Status);
    printf("IPv4: %u \n", net->IPv4);
    printf("IPv6: %lu \n", net->IPv6);
    printf("Interface type: %s \n", net->InterfaceType);
    printf("Product: %s \n", net->Product);
    printf("Model: %s \n", net->Model);
    printf("Driver: %s \n", net->Driver);
    printf("Driver version: %s \n", net->DriverVersion);
    printf("PCI location: %s \n", net->PciLocation);
    printf("RSS: %d \n", net->RSS);
}


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
device_to_str(interface_t * dev, char *str)
{
    sprintf(str,
            "Slot:\t%s\n"
            "Class:\t%s\n"
            "Vendor:\t%s\n"
            "Device:\t%s\n"
            "SVendor:\t%s\n"
            "SDevice:\t%s\n"
            "PhySlot:\t%s\n"
            "Rev:\t%lu\n"
            "Driver:\t%s\n"
            "DriverStr:\t%s\n"
            "Module:\t%s\n"
            "ModuleStr:\t%s\n"
            "Interface:\t%s\n"
            "ProgIf:\t%s\n"
            "Active:\t%d\n"
            "ssh_if:\t%d\n",
            dev->Slot,
            dev->Class,
            dev->Vendor,
            dev->Device,
            dev->SVendor,
            dev->SDevice,
            dev->PhySlot,
            dev->Rev,
            dev->Driver,
            dev->DriverStr,
            dev->Module,
            dev->ModuleStr,
            dev->Interface,
            dev->ProgIf,
            dev->Active,
            dev->SSH_If);
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
        if (!strcmp(devices[i].Slot, drv) && devices[i].DriverStr) {
            return 1;
        }
    }

    return 0;
}

void
get_pci_device_details(interface_t * dev)
{
    dev->Active = -1;

    char cmd[STR_MAX];
    sprintf(cmd, "lspci -vmmks %s", dev->Slot);
    char *extra_info = check_output(cmd);

    char *line;
    line = strtok(extra_info, "\n");
    while (line != NULL) {
        if (line == NULL || strlen(line) == 0) {
            continue;
        }

        const char *name = strsep(&line, "\t");
        const char *value = strsep(&line, "\t");

        if (!strcmp(name, "Class:"))
            strcpy(dev->Class, value);
        else if (!strcmp(name, "Vendor:"))
            strcpy(dev->Vendor, value);
        else if (!strcmp(name, "Device:"))
            strcpy(dev->Device, value);
        else if (!strcmp(name, "SVendor:"))
            strcpy(dev->SVendor, value);
        else if (!strcmp(name, "SDevice:"))
            strcpy(dev->SDevice, value);
        else if (!strcmp(name, "Rev:"))
            dev->Rev = atoll(value);
        else if (!strcmp(name, "Driver:"))
            strcpy(dev->Driver, value);
        else if (!strcmp(name, "Module:"))
            strcpy(dev->Module, value);
        else if (!strcmp(name, "Interface:"))
            strcpy(dev->Interface, value);
        else if (!strcmp(name, "PhySlot:"))
            strcpy(dev->PhySlot, value);

        line = strtok(NULL, "\n");
    }
}

void
get_dpdk_nic_details(void)
{
    interface_t dev;
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
            if (!strcmp(dev.Class, ETHERNET_CLASS)) {
                devices[devices_size] = dev;
                ++devices_size;
            }
            if (i == 0)
                memset((char *)&dev, 0, sizeof(dev));
        }

        const char *name = strsep(&dev_line, "\t");
        const char *value = strsep(&dev_line, "\t");

        if (!strcmp(name, "Class:"))
            strcpy(dev.Class, value);
        else if (!strcmp(name, "Vendor:"))
            strcpy(dev.Vendor, value);
        else if (!strcmp(name, "Device:"))
            strcpy(dev.Device, value);
        else if (!strcmp(name, "SVendor:"))
            strcpy(dev.SVendor, value);
        else if (!strcmp(name, "SDevice:"))
            strcpy(dev.SDevice, value);
        else if (!strcmp(name, "Rev:"))
            dev.Rev = atoll(value);
        else if (!strcmp(name, "Driver:"))
            strcpy(dev.Driver, value);
        else if (!strcmp(name, "Module:"))
            strcpy(dev.Module, value);
        else if (!strcmp(name, "Interface:"))
            strcpy(dev.Interface, value);
        else if (!strcmp(name, "PhySlot:"))
            strcpy(dev.PhySlot, value);

        dev_line = strtok(NULL, "\n");
        ++i;
    }

    if ((dev_line == NULL) && !strcmp(dev.Class, ETHERNET_CLASS)) {
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
            if (strstr(devices[i].Driver, ssh_if[j])) {
                devices[i].SSH_If = 1;
                devices[i].Active = 1;
                break;
            }
        }

        if (devices[i].ModuleStr[0]) {
            for (j = 0; j < DPDK_SIZE; ++j) {
                if (dpdk_drivers[j].found && strstr(devices[i].ModuleStr, dpdk_drivers[j].name) == NULL) {
                    char buf[STR_MAX];
                    sprintf(buf, "%s,%s", devices[i].ModuleStr, dpdk_drivers[j].name);
                    strcpy(devices[i].ModuleStr, buf);
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
            strcat(devices[i].ModuleStr, buf);
        }

        if (has_driver(devices[i].Slot)) {
            char *driver_str = NULL;
            for (o = 0; o < devices_size; ++o) {
                if (devices[o].DriverStr[0]) {
                    driver_str = devices[o].DriverStr;
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
        if (!strcmp(dev_name, devices[i].Device)) {
            return dev_name;
        }

        static char buf[STR_MAX];
        memset(buf, 0, sizeof (buf));
        sprintf(buf, "0000:%s", dev_name);
        if (!strcmp(buf, devices[i].Device)) {
            return buf;
        }

        if (strstr(devices[i].Interface, dev_name)) {
            return devices[i].Slot;
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

    interface_t *dev = NULL;
    for (i = 0; i < devices_size; ++i) {
        if (!strcmp(dev_id, devices[i].Slot)) {
            dev = &devices[i];
            break;
        }
    }

    if (dev == NULL || (dev->SSH_If && !force)) {
        return (1);
    }

    char path[STR_MAX];
    sprintf(path, "/sys/bus/pci/drivers/%s/unbind", dev->Driver);
    FILE *f = fopen(path, "a");
    if (f == NULL) {
        return (1);
    }

    memcpy(dev->DriverStr, dev->Driver, strlen(dev->Driver));
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
    interface_t *dev = NULL;
    interface_t tmp;
    unsigned i;
    int j, err, ret;
    char path[STR_MAX];
    const char *saved_driver = NULL;

    for (i = 0; i < devices_size; ++i) {
        if (!strcmp(dev_id, devices[i].Slot)) {
            dev = devices + i;
        }
    }


    if (dev == NULL || (dev->SSH_If && !force))
        return (1);

    if (has_driver(dev_id)) {
        if (!strcmp(dev->DriverStr, driver)) {
            return (1);
        } else {
            saved_driver = dev->DriverStr;
            err = unbind_one(dev_id, force);
            strcpy(dev->Device, "");
            if (err)
                return (1);
        }
    }

    for (j = 0; j < DPDK_SIZE; ++j) {
        if (dpdk_drivers[j].found && strstr(dpdk_drivers[j].name, driver)) {
            memset(path, 0, sizeof(path));
            sprintf(path, "/sys/bus/pci/drivers/%s/new_id", driver);
            FILE *f = fopen(path, "w");
            fprintf(f, "%04x %04x", atoi(dev->Vendor), atoi(dev->Device));
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
        strcpy(tmp.Slot, dev_id);
        get_pci_device_details(&tmp);
        if (!tmp.DriverStr[0]) {
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
        if (devices[j].DriverStr[0]) {
            for (i = 0; i < size; ++i) {
                if (strstr(dev_list[i], devices[i].Slot)) {
                    cont = 1;
                }
            }
        }
        if (cont) {
            continue;
        }

        char *d = devices[j].Slot;
        get_pci_device_details(&devices[j]);

        if (devices->DriverStr[0]) {
            err = unbind_one(d, force);
            if (err)
                return (1);
        }
    }
    return (0);
}

void
show_status(interface_t * kernel_drv, size_t * kernel_drv_size, interface_t * dpdk_drv, size_t * dpdk_drv_size, interface_t * no_drv,
        size_t * no_drv_size)
{
    unsigned i;
    int j;

    size_t ksize = 0, dpdksize = 0, nosize = 0;
    for (i = 0; i < devices_size; ++i) {
        if (!has_driver(devices[i].Slot) && ksize < *kernel_drv_size) {
            kernel_drv[ksize] = devices[i];
            ++ksize;
            continue;
        }

        int found = 0;
        for (j = 0; j < DPDK_SIZE; ++j) {
            if (dpdk_drivers[j].found && strstr(devices[i].DriverStr, dpdk_drivers[j].name)
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
    check_dpdk_modules();
    get_dpdk_nic_details();

    enum { DSIZE = 10 };
    interface_t a[DSIZE], b[DSIZE], c[DSIZE];
    size_t asize = DSIZE, bsize = DSIZE, csize = DSIZE;
    show_status(a, &asize, b, &bsize, c, &csize);
    static char buf[STR_MAX*20];
    device_to_str(c, buf);

    interface_t *net; // need free
    int size = parse_file("/root/interface_manager/out.xml", &net);
}
#endif