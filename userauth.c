#include <openssl/sha.h>
#include <string.h>
#include <malloc.h>

#include "userauth.h"
#include "trackerlogic.h"

char *g_auth_salt;


static void string2hex( void *dst_hex, size_t hex_size, const char *str ) {
  size_t i;
  for ( i = 0; i < hex_size; ++i, str += 2 ) {
    sscanf( str, "%02hhx", &((char *) dst_hex )[i] );
  }
}

/* returns true if user has passed an authorization */
int auth_user( const ot_auth *creds ) {
  /* bad auth if user has weird hash-string length */
  if ( creds->user_hash_len != SHA_DIGEST_LENGTH * 2 )
    return 0;

  int ret = 0;
  unsigned char legit_hash[SHA_DIGEST_LENGTH];
  char user_hash[SHA_DIGEST_LENGTH];

  size_t length = creds->userId_len + strlen( g_auth_salt );

  unsigned char *buffer = malloc( length + 1 );
  if ( !buffer )
    exerr( "Out of memory" );
  buffer[0] = '\0';

  strncat( (char *) buffer, creds->userId, creds->userId_len );
  strcat( (char *) buffer, g_auth_salt );
  SHA1( buffer, length, legit_hash );
  free( buffer );

  string2hex( user_hash, SHA_DIGEST_LENGTH, creds->user_hash );

  if ( !memcmp( user_hash, legit_hash, SHA_DIGEST_LENGTH )) ret = 1;

  return ret;
}


void auth_init() {
  if ( !g_auth_salt )
    exerr( "No hash salt for user auth specified." );
}
