#ifndef __USERAUTH_H__
#define __USERAUTH_H__


typedef struct {
    char *userId;
    size_t userId_len;

    char *user_hash;
    size_t user_hash_len;
} ot_auth;


int auth_user( const ot_auth *creds );

void auth_init();

extern char *g_auth_salt;

#endif //__USERAUTH_H__
