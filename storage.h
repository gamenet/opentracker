#ifndef __STORAGE_H__
#define __STORAGE_H__

#include "trackerlogic.h"

typedef struct {
    int64_t  downloaded;
    int64_t  uploaded;
    int64_t  left;

    char    *userId;
    size_t   userId_len;
    ot_hash *info_hash;

    time_t   timestamp;
    int32_t  connection_id;
    int32_t  status;
} ot_storage;


void storage_init();

void storage_deinit();

void storage_set( const ot_storage *item );


extern char *g_storage_ip;
extern short g_storage_port;
extern int   g_storage_enabled;

#endif //__STORAGE_H__
