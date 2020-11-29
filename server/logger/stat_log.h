#ifndef __STAT_LOG_H__
#define __STAT_LOG_H__

#define LOG_BUFFER 512

int open_log_file(int* logfd);

int write_statistic(int logfd, char* path, char* user);

#endif
