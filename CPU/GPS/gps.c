#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <time.h>

#define SERIAL_PORT "/dev/ttyUSB2"
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

int main() {
    int flag = 0;
    int fd = open_serial_port(SERIAL_PORT);
    if (fd == -1) {
        return 1;
    }
    configure_serial_port(fd);
    char response[1024];
	  char infor[1024];
    char timeSeting_temp[200];
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
    while(1)
    {
      if (strlen(infor) < 50)
      {
          
          sleep(10);
          send_at_command(fd, "AT+CGPSINFO");
          sleep(1);
          read_response(fd, infor, sizeof(infor));
          printf("GPS Info: %s\n", infor);
      }
      else
      {
        char* date = extractDate(infor);
        close(fd);
   	    sprintf(timeSeting_temp, "sudo date --set=\"20%s-%s-%s %s:%s:%s\"", gps.year, gps.month, gps.day, gps.hour, gps.minute, gps.second);
        printf("%s\r\n", timeSeting_temp);
        system(timeSeting_temp);
        break;
      }
    }
}



