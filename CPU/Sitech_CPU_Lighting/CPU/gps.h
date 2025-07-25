#ifndef GPS_H
#define GPS_H

#define CMD_CPU_TO_MASTER_SETTING_RTC                                           111
#define CMD_CPU_TO_QT_STATUS_GPS                                                130

// #include <stdio.h>
// #include <string.h>
// #include <unistd.h>
// #include <unistd.h>
// #include <fcntl.h>
// #include <termios.h>
// #include <time.h>

// #include "gps.c"

int modem_get_gps (void);

#endif