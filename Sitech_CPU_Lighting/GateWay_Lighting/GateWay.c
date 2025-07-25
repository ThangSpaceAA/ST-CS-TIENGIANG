
// Build: gcc -Wall -o GateWay GateWay.c ../main_proto.pb-c.c ../device_proto.pb-c.c ../user_proto.pb-c.c ../common_proto.pb-c.c -lpthread -lws -lwiringPi -luwsc -lev -lprotobuf-c -ljson-c
#include "GateWay.h"

#pragma region BIEN TOAN CUC
// Bien khai bao toan cuc gw online
const char *url = "wss://api.lighting.sdkd.com.vn/websocket";
// const char *url = "ws://192.168.1.22:8443/websocket";
struct ev_signal signal_watcher;
int ping_interval = 10; /* second */
volatile struct uwsc_client *cl;
int opt;

// Bien xu ly login
char *device_id = NULL;
/**
 * @brief Called when a client connects to the server.
 *
 * @param fd File Descriptor belonging to the client. The @p fd parameter
 * is used in order to send messages and retrieve informations
 * about the client.
 */
#pragma endregion

#pragma region LUONG WEBSOCKET SERVER LOCAL
void onopen(int fd)
{
	char *cli;
	cli = ws_getaddress(fd);
	// #ifndef DISABLE_VERBOSE
	// 	printf("Connection opened, client: %d | addr: %s\n", fd, cli);
	// #endif
	// 	free(cli);

	// Gui ma thiet bi, firmware version, va project ID cho GateWay
}

/**
 * @brief Called when a client disconnects to the server.
 *
 * @param fd File Descriptor belonging to the client. The @p fd parameter
 * is used in order to send messages and retrieve informations
 * about the client.
 */
void onclose(int fd)
{
	char *cli;
	cli = ws_getaddress(fd);
	if (listDevice[0].ID == fd)
	{
		is_flag_register_qtApp = false;
	}
#ifndef DISABLE_VERBOSE
	printf("Connection closed, client: %d | addr: %s\n", fd, cli);
#endif
	free(cli);
}

/**
 * @brief Called when a client connects to the server.
 *
 * @param fd File Descriptor belonging to the client. The
 * @p fd parameter is used in order to send messages and
 * retrieve informations about the client.
 *
 * @param msg Received message, this message can be a text
 * or binary message.
 *
 * @param size Message size (in bytes).
 *
 * @param type Message type.
 */
void onmessage(int fd, const unsigned char *msg, uint64_t size, int type)
{
	char *cli;
	cli = ws_getaddress(fd);
#ifndef DISABLE_VERBOSE
	// printf("I receive a message: %s (size: %" PRId64 ", type: %d), from: %s/%d\n",
	// 	   msg, size, type, cli, fd);
	//	I receive a message: CPU-{"MODE": 2 } (size: 16, type: 1), from: 127.0.0.1/4

#endif

	if (strcmp((char *)msg, "qtApp") == 0)
	{
		is_flag_register_qtApp = true;
		listDevice[0].nameClient = "qtApp";
		listDevice[0].ID = fd;
		// printf("Ten cua o thu nhat la: %s\r\n", listDevice[0].nameClient );
		// printf("Ten cua o thu nhat la: %d\r\n", listDevice[0].ID );
		// ws_sendframe(listDevice[1].ID, "qtApp", size, false, type);
		// char *tmp_firmware;
		char data[1000];
		printf("mac id: %s\r\n", mac_final);
		printf("fw: %s\r\n", firmware_GW_version);
		printf("Project id %s\r\n", "Sitech Co.Ltd");
		asprintf(&tmp_firmware, "sitech-v%s", firmware_GW_version, 3);
		int data_len = sprintf(data, TEMPLATE_HARDWARE_ID, CMD_SERVER_TO_QT_HARDWARE_ID, 11, mac_final, 5, firmware_GW_version, 12, tmp_firmware);
		// printf("reach here\r\n");
		ws_sendframe(qt_packet, (char *)data, data_len, false, 1);
	}
	if (strcmp((char *)msg, "CPU") == 0)
	{
		listDevice[1].nameClient = "CPU";
		listDevice[1].ID = fd;
		// printf("Ten cua o thu hai la: %s\r\n", listDevice[1].nameClient );
		// printf("Ten cua o thu hai la: %d\r\n", listDevice[1].ID);
		// char data[100];
		// int data_len = sprintf(data, TEMPLATE_HARDWARE_ID, CMD_SERVER_TO_QT_HARDWARE_ID, mac_id_gateway, firmware_GW_version, "Sitech Co.Ltd");
		// ws_sendframe(qt_packet, (char *)data, data_len, false, 1);
	}
	if (!strncmp(msg, "CPU-", 4))
	{
		// Chuyen tiep goi tin len cho qt
		ws_sendframe(listDevice[0].ID, (char *)msg, size, false, type);
		// MSG: CPU-{"CMD": 4,                            "mode": 2}
		// printf("MSG: %s", msg);

	}
	if (!strncmp(msg, "qtApp-", 6))
	{
		// // Chuyen tiep goi tin xuong cho cpu
		// printf("Message of CPU\r\n");
		ws_sendframe(listDevice[1].ID, (char *)msg, size, false, type);
	}
	free(cli);
}
#pragma endregion

#pragma region LUONG MO SOCKET ONLINE

void uwsc_onopen(struct uwsc_client *cl)
{
	is_flag_opend_weboscket_online = true;
}

void uwsc_onmessage(struct uwsc_client *cl,	void *type_mtfc_working_message, size_t len, bool binary)
{
	if (binary)
	{
		size_t i;
		uint8_t *p = type_mtfc_working_message;

		for (i = 0; i < len; i++)
		{
			// printf("%02hhX ", p[i]);
			if (i % 16 == 0 && i > 0)
				puts("");
		}
		puts("");
	}
	else
	{
#ifdef DEBUG_OPEN_WEBSOCKET_ONLINE
		printf("[%.*s]\n", (int)len, (char *)type_mtfc_working_message);
#endif
	}

	

}

void uwsc_onerror(struct uwsc_client *cl, int err, const char *msg)
{
#ifdef DEBUG_OPEN_WEBSOCKET_ONLINE
	uwsc_log_err("onerror:%d: %s\n", err, msg);
#endif
	is_flag_opend_weboscket_online = false;
	is_register_device_to_server_online_success = false;
	is_login_success = false;
	ev_break(cl->loop, EVBREAK_ALL);
}

void signal_cb(struct ev_loop *loop, ev_signal *w, int revents)
{
	if (w->signum == SIGINT)
	{
#ifdef DEBUG_OPEN_WEBSOCKET_ONLINE
		uwsc_log_info("Normal quit\n");
#endif
	}
	ev_break(loop, EVBREAK_ALL);
}

void uwsc_onclose(struct uwsc_client *cl, int code, const char *reason)
{

	// printf("Recreate socket to login\r\n");
#ifdef DEBUG_OPEN_WEBSOCKET_ONLINE
	uwsc_log_err("onclose:%d: %s\n", code, reason);
#endif
	is_flag_opend_weboscket_online = false;
	is_register_device_to_server_online_success = false;
	is_login_success = false;
	ev_break(cl->loop, EVBREAK_ALL);
}

void *open_Websocket_Online(void *threadArgs)
{
	struct ev_loop *loop = EV_DEFAULT;
	while (1)
	{
		// delay(1000);
#ifdef DEBUG_OPEN_WEBSOCKET_ONLINE
		printf("thread for open websocket\r\n");
		uwsc_log_info("Libuwsc: %s\n", UWSC_VERSION_STRING);
#endif

		cl = uwsc_new(loop, url, ping_interval, NULL);
		if (!cl)
		{
#ifdef DEBUG_OPEN_WEBSOCKET_ONLINE
			printf("Create new socket fail\r\n");
#endif
			continue;
		}
		else
		{
#ifdef DEBUG_OPEN_WEBSOCKET_ONLINE
			printf("Create new socket success\r\n");
#endif
		}
#ifdef DEBUG_OPEN_WEBSOCKET_ONLINE
		uwsc_log_info("Start connect...\n");
#endif
		cl->onopen = uwsc_onopen;
		cl->onmessage = uwsc_onmessage;
		cl->onerror = uwsc_onerror;
		cl->onclose = uwsc_onclose;

		device_id = NULL;
		ev_signal_init(&signal_watcher, signal_cb, SIGINT);
		ev_signal_start(loop, &signal_watcher);
		ev_run(loop, 0);
		free(cl);
	}
	// free(device_id);
	free(mac_id_gateway);
	return 0;
}
#pragma endregion

#pragma region LUONG MAIN
/**
 * @brief Main routine.
 *
 * @note After invoking @ref ws_socket, this routine never returns,
 * unless if invoked from a different thread.
 */
int main(void)
{
	struct ws_events evs;
#pragma region LAY THONG TIN MACID THIET BI LUU FILE
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
	fw_version_GW = fopen("/home/pi/Version.txt", "r");
	if (fw_version_GW == NULL)
	{
		printf("Error openfile\r\n");
		perror("Error opening file");
		// return(-1);
	}
	if (fgets(firmware_GW_version, 18, fp) != NULL)
	{
		/* writing content to stdout */
		// exit(1);
	}
	fclose(fp);
	printf("Firmware version is: %s", firmware_GW_version);

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
#pragma endregion

#pragma region TAO LUONG WEBSOCKET ONLINE
	if (pthread_create(&thread_Open_Websocket_Online, NULL, open_Websocket_Online, NULL) != 0)
		printf("thread_create() failed\n");
#pragma endregion

#pragma region TAO LUONG DANG KY THIET BI ONLINE
	if (pthread_create(&thread_send_Websocket_Online, NULL, sender_Websocket_Online, NULL) != 0)
		printf("thread_create() failed\n");
#pragma endregion

	// if (pthread_create(&thread_Test_Send, NULL, Send_Message_Test, NULL) != 0)
	// 	printf("thread_create() failed\n");
	evs.onopen = &onopen;
	evs.onclose = &onclose;
	evs.onmessage = &onmessage;
	ws_socket(&evs, 8080, 0); /* Never returns. */
	/*
	 * If you want to execute code past ws_socket, invoke it like:
	 *   ws_socket(&evs, 8080, 1)
	 */

	return (0);
}
#pragma endregion

#pragma region LUONG XU LY GUI GOI TIN SOCKET ONLINE
void *sender_Websocket_Online(void *threadArgs)
{
	while (1)
	{
		delay(1);
		while (!is_register_device_to_server_online_success)
		{
			if (!is_flag_opend_weboscket_online)
			{
				printf("Cho mo ket noi thanh cong vs server online\r\n");
				delay(2000);
				continue;
			}
			printf("ket noi thanh cong vs server online thanh cong\r\n");
			MainMessage mainMessage = MAIN_MESSAGE__INIT;
			DeviceMessage deviceMessage = DEVICE_MESSAGE__INIT;
			DeviceAuthMessage deviceAuthMessage = DEVICE_AUTH_MESSAGE__INIT;
			DeviceRegisterRequest deviceRegisterRequest = DEVICE_REGISTER_REQUEST__INIT;

			printf("Mac ID: %s\r\n", mac_final);
			printf("firmware version is: %s", firmware_GW_version);
			printf("project ID is: %s", "Sitech Co.Ltd\r\n");
			// deviceRegisterRequest.hardwareid = mac
			deviceRegisterRequest.hardwareid = mac_final;
			deviceRegisterRequest.firmwareversion = firmware_GW_version;
			deviceRegisterRequest.mantoken = "112233445566";
			deviceRegisterRequest.cpuimei = "112233445566";
			deviceRegisterRequest.numphase = 2;

			deviceAuthMessage.deviceregisterrequest = &deviceRegisterRequest;
			deviceMessage.deviceauthmessage = &deviceAuthMessage;
			mainMessage.devicemessage = &deviceMessage;

			int size = main_message__pack(&mainMessage, tx_buffer);
			cl->send_ex(cl, UWSC_OP_BINARY, 1, size, tx_buffer);
			printf("Send Register device GateWay to server \r\n");
			delay(2000);
		}

		while ((is_register_device_to_server_online_success == true) && (!is_login_success)) // Login
		{
			printf("Send Register device GateWay to server success tien hanh login\r\n");
			if (device_id == NULL)
			{
				printf("Device did not register\n");
				// return;
				continue;
			}
			if (!is_register_device_to_server_online_success)
			{
				delay(2000);
				continue;
			}
			if (strlen(device_id) <= 0)
			{
				delay(1000);
				exit(1);
			}
			printf("Device registered, to login\n");
			printf("Device ID: %s", device_id);
			MainMessage mainMessage = MAIN_MESSAGE__INIT;
			DeviceMessage deviceMessage = DEVICE_MESSAGE__INIT;
			DeviceAuthMessage deviceAuthMessage = DEVICE_AUTH_MESSAGE__INIT;
			DeviceLoginRequest deviceLoginRequest = DEVICE_LOGIN_REQUEST__INIT;

			deviceLoginRequest.deviceid = device_id;
			deviceLoginRequest.cpuimage = "112233445566";
			deviceAuthMessage.deviceloginrequest = &deviceLoginRequest;
			deviceMessage.deviceauthmessage = &deviceAuthMessage;
			mainMessage.devicemessage = &deviceMessage;

			int size = main_message__pack(&mainMessage, tx_buffer);
			cl->send_ex(cl, UWSC_OP_BINARY, 1, size, tx_buffer);
			printf("Send Login to server\r\n");
			delay(2000);
		}

		// Update mode
	}
}
#pragma endregion

#pragma region HAM XU LY CHUOI
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
#pragma endregion

// void *Send_Message_Test(void *threadArgs)
// {
// 	while(1)
// 	{
// 		delay(1);
// 		if (is_flag_register_qtApp)
// 		{
// 			ws_sendframe(listDevice[0].ID, (char *)"hello qtApp", 12, true, 1);
// 			delay(1000);
// 		}
// 	}
// }

// char data[1000];
// // printf("mac id: %s\r\n", mac_final);
// // printf("fw: %s\r\n",firmware_GW_version);
// // printf("Project id %s\r\n","Sitech Co.Ltd");
// int data_len = sprintf(data, TEMPLATE_HARDWARE_ID, CMD_SERVER_TO_QT_HARDWARE_ID, mac_final, firmware_GW_version, "Sitech Co.Ltd");
// // printf("reach here\r\n");
// ws_sendframe(qt_packet, (char *)data, data_len, false, 1);