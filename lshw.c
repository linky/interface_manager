#include "dpdk_misc.h"
#include "lshw.h"

#include <stdio.h>


mxml_node_t*
parse_file()
{
	FILE* fp; // TODO remove
	mxml_node_t *it;
	lshw_t *net;
	int cnt;

	mxml_node_t *tree = mxmlLoadFile(NULL, fp, MXML_OPAQUE_CALLBACK);
	fclose(fp);
	for(it = mxmlFindElement(tree, tree, "node", NULL, NULL, MXML_DESCEND);
	    it != NULL; it = mxmlFindElement(it, tree, "node", NULL, NULL, MXML_DESCEND)) {
		
		net = (lshw_t *)realloc(net, (cnt + 1) * sizeof(lshw_t));
		get_lshw_stats(net + cnt, it);
		cnt++;
	}
}


void
get_lshw_stats(lshw_t * net, mxml_node_t * tree)
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
	if (temp)
		strcpy(net->Link, mxmlElementGetAttr(temp, "value"));
	else
		strcpy(net->Link, "No data");

	temp = mxmlFindElement(tree, tree, "setting", "id", "ip", MXML_DESCEND);
	if (temp)
		strcpy(net->IPv4, mxmlElementGetAttr(temp, "value"));
	else
		strcpy(net->IPv4, "No data");

	temp = mxmlFindElement(tree, tree, "setting", "id", "ipv6", MXML_DESCEND);
	if (temp)
		strcpy(net->IPv6, mxmlElementGetAttr(temp, "value"));
	else
		strcpy(net->IPv6, "No data");

	temp = mxmlFindElement(tree, tree, "capacity", NULL, NULL, MXML_DESCEND);
	if (temp)
		strcpy(net->Speed, mxmlGetOpaque(temp));
	else
		strcpy(net->Speed, "No data");

	temp = mxmlFindElement(tree, tree, "setting", "id", "multicast", MXML_DESCEND);
	if (temp)
		strcpy(net->RSS, mxmlElementGetAttr(temp, "value"));
	else
		strcpy(net->RSS, "No data");

	mxmlDelete(temp);
}

void dump_lshw(lshw_t * net)
{
	printf("\n");
	printf("Interface name: %s \n", net->InterfaceName);
	printf("MAC adress: %s \n", net->MacAdress);
	printf("Speed: %s \n", net->Speed);
	printf("Link: %s \n", net->Link);
	printf("Status: %s \n", net->Status);
	printf("IPv4: %s \n", net->IPv4);
	printf("IPv6: %s \n", net->IPv6);
	printf("Interface type: %s \n", net->InterfaceType);
	printf("Product: %s \n", net->Product);
	printf("Model: %s \n", net->Model);
	printf("Driver: %s \n", net->Driver);
	printf("Driver version: %s \n", net->DriverVersion);
	printf("PCI location: %s \n", net->PciLocation);
	printf("RSS: %s \n", net->RSS);
}