//#include "main.h"
#include <stdio.h>
#include <wiringPi.h>
#include <wiringSerial.h>
#include <stdlib.h>
#include <string.h>
FILE *fp; // File luu macid Gateway
char mac[18];
char re_mac[100] = "";
char *mac_id_gateway = NULL;
char *mac_final = NULL;
char *substr(char *s, int start, int end);
//char topicraw[50] ="/master/";
int main()
{
    
	system("cat /sys/class/net/eth0/address > mac.txt");
	fp = fopen("mac.txt", "r");
	if (fp == NULL)
	{
		printf("Error openfile\r\n");
		perror("Error opening file");
		// return(-1);
	}
	if (fgets(mac, 18, fp) != NULL)
	{
		/* writing content to stdout */
	}
	/* Closing the file mac.txt */
	fclose(fp);
	printf("Mac CM4: %s\r\n", mac);
	
	// printf("MAC: %s\r\n",mac);
	substr(mac, 6, 16);

	int len = asprintf(&mac_id_gateway, "GW-%s", re_mac, sizeof(re_mac));
	// printf("MAC ID: %s\r\n", mac_id_gateway);
	char *token = NULL;
	const char s[2] = ":";
	token = strtok(mac_id_gateway, s);
	mac_final = token;
	// strcat(mac_final, token);
	token = strtok(NULL, s);
	strcat(mac_final, token);
	token = strtok(NULL, s);
	strcat(mac_final, token);
	token = strtok(NULL, s);
	strcat(mac_final, token);
	printf("Mac ID: %s\r\n", mac_final);
	char url[100] = "http://api.sitechlighting.one/device";
	char message[10000];
	
	snprintf(message, sizeof(message), "%s %s %s \"%s\", %s", "curl -X POST", url, "-H \"Content-Type: application/json\" -d '{\"deviceId\":",mac_final, "\"password\": \"aabbccddeeff\"}'");
	printf("%s\r\n", message);
	system(message);
	
}

char *substr(char *s, int start, int end)
{
	int indext = 0;

	for (int i = start; i <= end; i++)
	{
		re_mac[indext] = s[i];
		indext++;
	}
	re_mac[indext] = '\0';

	return re_mac;
}
