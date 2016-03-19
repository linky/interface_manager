#pragma once

#define STR_MAX 128

typedef struct
{
    char Slot[STR_MAX];
    char Class[STR_MAX];
    char Vendor[STR_MAX];
    char Device[STR_MAX];
    char SVendor[STR_MAX];
    char SDevice[STR_MAX];
    char PhySlot[STR_MAX];
    char Rev[STR_MAX];
    char Driver[STR_MAX];
    char DriverStr[STR_MAX];
    char Module[STR_MAX];
    char ModuleStr[STR_MAX];
    char Interface[STR_MAX];
    char ProgIf[STR_MAX];
    char Active[STR_MAX];
    int SSH_If;
    char InterfaceName[STR_MAX];
    char MacAdress[STR_MAX];
    char InterfaceType[STR_MAX];
    char Link[STR_MAX];
    char Product[STR_MAX];
    char Model[STR_MAX];
    //char Driver[STR_MAX];
    char DriverVersion[STR_MAX];
    char PciLocation[STR_MAX];
    char IPv4[STR_MAX];
    char IPv6[STR_MAX];
    char Speed[STR_MAX];
    char Status[STR_MAX];
    char RSS[STR_MAX];
} interface_t; // TODO rename and check fields

int im_Init();

int im_RefreshAll();

int im_Refresh_System();

int im_Refresh_Bind();

int im_Refresh_PCIDevice(const char *id);

int im_StatusAllMulti();

int im_StatusAll();

int im_GetDrivers();

int im_GetInterfaceByIP(const char *ip);

int im_GetInterfaceByDPDKPort(int port);

int im_GetInterfaceByPCI(const char *id);

int im_GetInterfaceByName(const char *name);

int im_bindInterface();

int im_unbindInterface();

int im_UpdatePort();

int im_UpdateIP();

int im_UpdateLinkState();

int im_Replace();