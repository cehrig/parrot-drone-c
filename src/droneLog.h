//
// Created by cehrig on 6/4/16.
//

#ifndef DRONE_CONNECT_DRONELOG_H
#define DRONE_CONNECT_DRONELOG_H

#define LOGFILE "/var/log/drone-connect.log"

typedef enum
{
    LOG_DEFAULT,
    LOG_DEBUG,
    LOG_ERROR,
    LOG_CRITICAL
} LOG_LEVEL;

FILE * openlog(int);
void writelog(LOG_LEVEL level, const char *, ...);

#endif //DRONE_CONNECT_DRONELOG_H
