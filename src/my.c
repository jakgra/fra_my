#include <fra/my.h>

#include <fra/core.h>
#include <jak_dbg.h>
#include <bstrlib.h>
#include <mysql/mysql.h>
#include <stdio.h>
#include <stdlib.h>




struct fra_my_con {
	bstring host;
	bstring user;
	bstring passwd;
	bstring db;
	unsigned int port;
	bstring unix_socket;
	unsigned long clientflag;
};

static fra_my_con_t * default_con = NULL;

static int assign_str( char * s, bstring bs ) {

	if( s ) {

		bs = bfromcstr( s );
		check( bs, final_cleanup );

	} else {

		bs = NULL;

	}

	return 0;

final_cleanup:
	return -1;

}




// public functions

int fra_my_init() {

	return 0;

}

void fra_my_deinit() {

	fra_my_con_free( default_con );

}

fra_my_con_t * fra_my_con_new( char * host, char * user, char * passwd, char * db, unsigned int port, char * unix_socket, unsigned long clientflag ) {

	int rc;

	fra_my_con_t * c;


	c = malloc( sizeof( fra_my_con_t ) );
	check( c, final_cleanup );

	c->port = port;
	c->clientflag = clientflag;

	rc = assign_str( host, c->host );
	rc += assign_str( user, c->user );
	rc += assign_str( passwd, c->passwd );
	rc += assign_str( db, c->db );
	rc += assign_str( unix_socket, c->unix_socket );
	check( rc == 0, c_cleanup );

	return c;

c_cleanup:
	fra_my_con_free( c );

final_cleanup:
	return NULL;

}

void fra_my_con_free( fra_my_con_t * c ) {

	if( c ) {

		bdestroy( c->host );
		bdestroy( c->user );
		bdestroy( c->passwd );
		bdestroy( c->db );
		bdestroy( c->unix_socket );

		free( c );

	}

}

int fra_my_set( char * host, char * user, char * passwd, char * db, unsigned int port, char * unix_socket, unsigned long clientflag ) {

	fra_my_con_free( default_con );

	default_con = fra_my_con_new( host, user, passwd, db, port, unix_socket, clientflag );
	check( default_con, final_cleanup );

	return 0;

final_cleanup:
	return -1;

}
