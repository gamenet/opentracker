#include <hiredis/hiredis.h>
#include <inttypes.h>

#include "storage.h"

char *g_storage_ip;
short g_storage_port;
int   g_storage_enabled;
static redisContext *g_ctx;


static const char *status2string( int32_t status ) {
    static char *stats[] = { "none", "completed", "started", "stopped" };

    return stats[status];
}


static size_t hex2string( char *dst_str, const void *hex, size_t hex_size ) {
    char *tmp = dst_str;

    for ( size_t i = 0; i < hex_size; ++i )
        tmp += sprintf( tmp, "%02X", ((uint8_t *) hex )[i] );

    return tmp - dst_str;
}


void storage_init() {
    if ( !g_storage_ip || !g_storage_port )
        exerr( "No remote address for redis storage specified." );

    g_ctx = redisConnect( g_storage_ip, g_storage_port );
    if ( g_ctx != NULL && g_ctx->err ) {
        fprintf( stderr, "Error: %s\n", g_ctx->errstr );
        exerr( "Storage init error" );
    }
}


void storage_deinit() {
    if( g_ctx ) {
      redisFree( g_ctx );
      g_ctx = NULL;
    }
}


void storage_set( const ot_storage *item ) {
    redisReply *reply;

    /* for hex-string representation we need x2 bytes plus terminator */
    char hash_string[OT_HASH_COMPARE_SIZE * 2 + 1] = { };
    char conn_id[sizeof( item->connection_id ) * 2 + 1] = { };

    hex2string( hash_string, item->info_hash, OT_HASH_COMPARE_SIZE );
    hex2string( conn_id, &item->connection_id, sizeof( item->connection_id ) );

    reply = redisCommand( g_ctx,
                          "HMSET %b %s.%s.downloaded %"PRIi64""
                                  " %s.%s.left %"PRIi64""
                                  " %s.%s.uploaded %"PRIi64""
                                  " %s.%s.status %s"
                                  " %s.%s.time %u",
                          item->userId, item->userId_len, hash_string, conn_id, item->downloaded,
                          hash_string, conn_id, item->left,
                          hash_string, conn_id, item->uploaded,
                          hash_string, conn_id, status2string( item->status ),
                          hash_string, conn_id, item->timestamp );

    if ( !reply ) goto global_error;
    if ( reply->type == REDIS_REPLY_ERROR ) goto operation_error;

    reply = redisCommand( g_ctx, "SADD %u %b", item->timestamp, item->userId, item->userId_len );

    if ( !reply ) goto global_error;
    if ( reply->type == REDIS_REPLY_ERROR ) goto operation_error;

    return;

operation_error:
    fprintf( stderr, "Storage operation error: %s\n", reply->str );
    return;

global_error:
    fprintf( stderr, "Storage system error: %s. Reinitializing the storage...\n", g_ctx->errstr );
    /* we need to reinit the storage after global error */
    storage_deinit();
    storage_init();
}
