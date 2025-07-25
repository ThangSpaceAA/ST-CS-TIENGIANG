
#include "uwsc/log.h"
#include "uwsc/uwsc.h"
#include "uwsc/utils.h"
#include "uwsc/buffer.h"
#include "uwsc/config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <time.h>

#include "Parameter_CPU.h"
#include "gps.h"


extern struct uwsc_client *cl;

#define GPS_PORT "/dev/ttyUSB2"
#define BAUDRATE B115200

typedef struct __attribute__((packed))
{
  char hour[3];
  char minute[3];
  char second[3];
  char year[3];
  char month[3];
  char day[3];
} gps_t; 
gps_t gps;

type_date_time_t type_date_time_update_rtc;

char* extractDate(char* message) {
  char* fields[7];
  char* token;
  int i = 0;
  char formattedDate[9];
  char formattedTime[9];
  token = strtok(message, ",");
  while (token != NULL) {
    fields[i++] = token;
    token = strtok(NULL, ",");
	
  }
  char* dateString = fields[4];
  char* timeString = fields[5];
  
  
  strncpy(gps.day, dateString, 2);
  gps.day[2] = '\0'; 

  strncpy(gps.month, dateString + 2, 2);
  gps.month[2] = '\0';

  strncpy(gps.year, dateString + 4, 2);
  gps.year[2] = '\0';

  sprintf( formattedDate, "%s/%s/%s", gps.day, gps.month, gps.year);
  
  strncpy(gps.hour, timeString, 2);
  gps.hour[2] = '\0';
  int hours = atoi(gps.hour) + 7; 
  hours = (hours % 24 + 24) % 24;
  sprintf(gps.hour, "%d", hours);
  
  strncpy(gps.minute, timeString + 2, 2);
  gps.minute[2] = '\0';

  strncpy(gps.second, timeString + 4, 2);
  gps.second[2] = '\0'; 

  sprintf(formattedTime, "%s:%s:%s", gps.hour, gps.minute, gps.second);
  printf("FormattedDate Info: %s\n", formattedDate);
  printf("FormattedTime Info: %s\n", formattedTime);
  return dateString;
}
int open_serial_port(const char *port_name) {
    int fd = open(port_name, O_RDWR | O_NOCTTY | O_NDELAY);
    if (fd == -1) {
        perror("open_serial_port: Unable to open port");
        return -1;
    }
    fcntl(fd, F_SETFL, 0);
    return fd; 
}

void configure_serial_port(int fd) {
    struct termios options;
    tcgetattr(fd, &options);
    cfsetispeed(&options, BAUDRATE);
    cfsetospeed(&options, BAUDRATE);
    options.c_cflag |= (CLOCAL | CREAD);
    options.c_cflag &= ~PARENB;
    options.c_cflag &= ~CSTOPB;
    options.c_cflag &= ~CSIZE;
    options.c_cflag |= CS8;
    options.c_cflag &= ~CRTSCTS;
    options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
    options.c_iflag &= ~(IXON | IXOFF | IXANY);
    options.c_oflag &= ~OPOST;
    tcsetattr(fd, TCSANOW, &options);
}

void send_at_command(int fd, const char *cmd) {
    write(fd, cmd, strlen(cmd));
    write(fd, "\r\n", 2);
}

void read_response(int fd, char *response, size_t size) {
    memset(response, 0, size);
    read(fd, response, size - 1);
}

char array_tmp[40] = {0};
int array_tmp_len;

int modem_get_gps (void)
{
    char infor[1024];
    char timeSeting_temp[200];
    int fd = open_serial_port(GPS_PORT);
    if (fd == -1) {
        return 1;
    }
    configure_serial_port(fd);
    char response[1024];

    // AT kiem tra ket noi 
    send_at_command(fd, "AT");
    sleep(1);
    read_response(fd, response, sizeof(response));
    printf("Response: %s\n", response);
    
    send_at_command(fd, "AT+CUSBPIDSWITCH=9011,1,1");
    sleep(1);
    read_response(fd, response, sizeof(response));
    printf("Response: %s\n", response);

    // cau hinh bang tan 1
    send_at_command(fd, "AT+CNBP=0x0002000000400183,0x000001E000000000,0x0000000000000021");
    sleep(1);
    read_response(fd, response, sizeof(response));
    printf("Response: %s\n", response);
    
    // cau hinh bang tan 2
    send_at_command(fd, "AT+CNBP=0x0002000000400180,0x480000000000000000000000000000000000000000000042000001E200000095,0x0000000000000021");
    sleep(1);
    read_response(fd, response, sizeof(response));
    printf("Response: %s\n", response);
    
    // Turn on the GPIO ANTEN
    send_at_command(fd, "AT+CVAUXS=1");
    sleep(1);
    read_response(fd, response, sizeof(response));
    printf("Response: %s\n", response);
	
    // Turn on the GPS
    send_at_command(fd, "AT+CGPS=1");
    sleep(1);
    read_response(fd, response, sizeof(response));
    printf("Response: %s\n", response);

    // Get GPS information
    send_at_command(fd, "AT+CGPSINFO");
    sleep(1);
    read_response(fd, infor, sizeof(infor));
    printf("GPS Info: %s\n", infor);
    // sleep(5);

    while(1)
    {         
        if (strlen(infor) < 50)
        {
            send_at_command(fd, "AT+CGPSINFO");
            sleep(1);
            read_response(fd, infor, sizeof(infor));
            printf("GPS Info: %s\n", infor);
            array_tmp_len = sprintf(array_tmp, TEMPLATE_STATUS_MANUAL_GPS,
                                        CMD_CPU_TO_QT_STATUS_GPS, 0);
            
            cl->send_ex(cl, UWSC_OP_TEXT, 1, array_tmp_len, array_tmp);
            memset(array_tmp, 0, 40); 
        }
        else
        {
            char* date = extractDate(infor);
            // close(fd);
            sprintf(timeSeting_temp, "sudo hwclock --set --date \"20%s/%s/%s %s:%s:%s +07\"", gps.year, gps.month, gps.day, gps.hour, gps.minute, gps.second);
            printf("%s\r\n", timeSeting_temp);
            system(timeSeting_temp);
            memset(timeSeting_temp, 0, sizeof(timeSeting_temp));
            type_date_time_update_rtc.year = (uint16_t)atoi(gps.year) + 2000;
            type_date_time_update_rtc.month = (uint8_t)atoi(gps.month);
            type_date_time_update_rtc.date = (uint8_t)atoi(gps.day);
            type_date_time_update_rtc.hour = (uint8_t)atoi(gps.hour);
            type_date_time_update_rtc.minute = (uint8_t)atoi(gps.minute);
            type_date_time_update_rtc.seconds = (uint8_t)atoi(gps.second);
            // printf("timestamp: %d/%d/%d-%d:%d:%d\r\n", RTC_set_master.years, RTC_set_master.month, RTC_set_master.day, 
            // RTC_set_master.hh, RTC_set_master.mm, RTC_set_master.ss);
            sprintf(timeSeting_temp, "sudo date --set=\"20%s/%s/%s %s:%s:%s\"", gps.year, gps.month, gps.day, gps.hour, gps.minute, gps.second);
            printf("%s\r\n", timeSeting_temp);
            system(timeSeting_temp);

            send_struct(CMD_CPU_TO_MASTER_SETTING_RTC, (uint8_t *)&type_date_time_update_rtc, sizeof(type_date_time_t));

            memset(timeSeting_temp, 0, sizeof(timeSeting_temp));

            array_tmp_len = sprintf(array_tmp, TEMPLATE_STATUS_MANUAL_GPS,
                                    CMD_CPU_TO_QT_STATUS_GPS, 1);
                                    
            cl->send_ex(cl, UWSC_OP_TEXT, 1, array_tmp_len, array_tmp);
            memset(array_tmp, 0, 40);
            break;
        }  
        // sleep(5);
    }
    return 0;
}