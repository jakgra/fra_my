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

struct con_my {
	MYSQL * con;
	/*statements hashtable...*/
};

static fra_my_con_t * default_con = NULL;

static int add_default_con_to_request( fra_req_t * req ) {

	MYSQL * rc_my;

	MYSQL * c;


	check_msg( default_con, final_cleanup, "You have to call fra_my_set() before the first request comes in." );

	c = mysql_init( NULL );
	check( c, final_cleanup );

	//TODO connect async
	rc_my = mysql_real_connect(
			c,
			bdata( default_con->host ),
			bdata( default_con->user ),
			bdata( default_con->passwd ),
			bdata( default_con->db ),
			default_con->port,
			bdata( default_con->unix_socket ),
			default_con->clientflag
			);
	check_msg( rc_my, c_cleanup, "mysql_real_connect() failed, are your params ok?" );

	fra( req, "fra_my_con", struct con_my ).con = c;

	return 0;

c_cleanup:
	mysql_close( c );

final_cleanup:
	return -1;

}

static int remove_default_con_from_request( fra_req_t * req ) {

	mysql_close( fra( req, "fra_my_con", struct con_my ).con );

	return 0;

}




// public functions

int fra_my_init() {

	int rc;


	rc = mysql_library_init( 0, 0, NULL );
	check( rc == 0, final_cleanup );

	rc = fra_req_reg( "fra_my_con", struct con_my );
	check( rc == 0, final_cleanup );

	rc = fra_req_hook_reg( FRA_REQ_CREATED, add_default_con_to_request, 9.0f );
	check( rc == 0, final_cleanup );

	rc = fra_req_hook_reg( FRA_REQ_FREE, remove_default_con_from_request, 9.0f );
	check( rc == 0, final_cleanup );

	return 0;

final_cleanup:
	return -1;

}

void fra_my_deinit() {

	fra_my_con_free( default_con );

	mysql_library_end();

}

fra_my_con_t * fra_my_con_new( char * host, char * user, char * passwd, char * db, unsigned int port, char * unix_socket, unsigned long clientflag ) {

	fra_my_con_t * c;


	c = malloc( sizeof( fra_my_con_t ) );
	check( c, final_cleanup );

	c->port = port;
	c->clientflag = clientflag;
	c->host = bfromcstr( host );
	c->user = bfromcstr( user );
	c->passwd = bfromcstr( passwd );
	c->db = bfromcstr( db );
	c->unix_socket = bfromcstr( unix_socket );
	check(
			( c->host != NULL || host == NULL )
			&& ( c->user != NULL || user == NULL )
			&& ( c->passwd != NULL || passwd == NULL )
			&& ( c->db != NULL || db == NULL )
			&& ( c->unix_socket != NULL || unix_socket == NULL ),
			c_cleanup
	     );

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
