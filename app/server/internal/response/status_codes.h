#ifndef __STATUS_CODES_H__
#define __STATUS_CODES_H__

/* Success */
#define STATUS_OK 0

/* Cliens errors */
#define STATUS_METHOD_NOT_ALLOWED 10
#define STATUS_PID_INCORRECT 11
#define STATUS_PROTOCOL_NOT_ALLOWED 12
#define STATUS_BAD_REQUEST 13

/* Server errors */
#define STATUS_SERVER_INTERNAL_ERROR 20
#define STATUS_DRIVER_ERROR 21
#define STATUS_IOCTL_ERROR 22

#endif
