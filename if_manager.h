#pragma once

int Init();

int RefreshAll();

int Refresh_System();

int Refresh_Bind();

int Refresh_PCIDevice(const char * id);

int StatusAllMulti();

int StatusAll();

int GetDrivers();

int GetInterfaceByIP(const char * ip);

int GetInterfaceByDPDKPort(int port);

int GetInterfaceByPCI(const char * id);

int GetInterfaceByName(const char * name);

int bindInterface();

int unbindInterface();

int UpdatePort();

int UpdateIP();

int UpdateLinkState();

int Replace();