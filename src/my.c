#include <fra/my.h>

#include <fra/core.h>
#include <jak_dbg.h>
#include <bstrlib.h>
#include <mysql/mysql.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <jak_ht.h>
#include <jak_da.h>
#include <jak_hash_bstring.h>




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
	jak_ht_t * stmts;
	jak_da_t * in;
	jak_da_t * out;
};

static fra_my_con_t * default_con = NULL;

static MYSQL_STMT * get_stmt( struct con_my * con, bstring sql ) {

	int rc;

	MYSQL_STMT * stmt;
	bstring key;


	stmt = (MYSQL_STMT *)jak_ht_get( con->stmts, sql );

	if( ! stmt ) {

		stmt = mysql_stmt_init( con->con );
		check( stmt, final_cleanup );

		rc = mysql_stmt_prepare( stmt, bdata( sql ), blength( sql ) );
		check( rc == 0, stmt_cleanup );

		key = malloc( sizeof( struct tagbstring ) );
		check( key, stmt_cleanup );

		key->data = sql->data;
		key->slen = sql->slen;
		key->mlen = -1;

		rc = jak_ht_set( con->stmts, (void *)key, (void *)stmt );
		check( rc == 0, key_cleanup );

	}

	return stmt;

key_cleanup:
	free( key );

stmt_cleanup:
	mysql_stmt_close( stmt );

final_cleanup:
	return NULL;

}

static int add_default_con_to_request( fra_req_t * req ) {

	int rc;
	MYSQL * rc_my;

	MYSQL * c;
	struct con_my * con;


	check_msg(
			default_con,
			final_cleanup,
			"You have to call fra_my_set() before the first request comes in."
		 );

	c = mysql_init( NULL );
	check( c, final_cleanup );

	rc = mysql_options( c, MYSQL_OPT_NONBLOCK, 0 );
	check( rc == 0, c_cleanup );

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

	con = fra( req, "fra_my_con", struct con_my * );
	con = malloc( sizeof( struct con_my ) );
	check( con, c_cleanup );

	con->con = c;
	con->stmts = jak_ht_new( 50, -1.0f, biseq, jak_hash_bstring, stmt_destruct );
	con->in = jak_da_new( 2, -1.0f, sizeof( MYSQL_BIND ) );
	con->out = jak_da_new( 2, -1.0f, sizeof( MYSQL_BIND ) );
	check( con->stmts && con->in && con->out, con_cleanup );

	return 0;

con_cleanup:
	jak_ht_free( con->stmts );
	jak_da_free( con->in );
	jak_da_free( con->out );
	free( con );

c_cleanup:
	mysql_close( c );

final_cleanup:
	return -1;

}

static int remove_default_con_from_request( fra_req_t * req ) {

	struct con_my * con;


	con = fra( req, "fra_my_con", struct con_my * );
	mysql_close( con->con );
	jak_ht_free( con->stmts );
	jak_da_free( con->in );
	jak_da_free( con->out );
	free( con );

	return 0;

}




// public functions

int fra_my_init() {

	int rc;


	rc = mysql_library_init( 0, 0, NULL );
	check( rc == 0, final_cleanup );

	rc = fra_req_reg( "fra_my_con", struct con_my * );
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

int fra_my( fra_req_t * req, fra_my_cb callback, char * sql, ... ) {

	int rc;

	va_list argp;
	unsigned int i;
	unsigned int count;
	const char * var_name;
	const char * type;
	int type_len;
	void * var;
	MYSQL_STMT * stmt;
	struct con_my * con;
	struct tagbstring key;


	count = mysql_stmt_param_count( stmt );

	con = fra( req, "fra_my_con", struct con_my * );
	check( con, final_cleanup );

	key.data = sql;
	key.slen = sizeof( sql ) - 1;
	key.mlen = -1;

	stmt = get_stmt( con, &key );
	check( stmt, final_cleanup );

	rc = jak_da_resize( con->in, count );
	check( rc == 0, final_cleanup );

	jak_da_zero_out( con->in );

	va_start( argp, sql );

	for( i = 0; i < count; i++ ) {

		var_name = va_arg( argp, const char * );
		check_msg(
				var_name,
				va_cleanup,
				"Do you have enough bind params in your fra_my() call?"
			 );

		var = fra_var_get_with_type( req, var_name, sizeof( var_name ) - 1, &type, &type_len );
		check( var && type && type_len > 0, va_cleanup );

		rc = call_bind_function_input(
				var,
				var_name,
				sizeof( var_name ) - 1,
				type,
				type_len,
				(MYSQL_BIND *)jak_da_get( con->in, i )
				);
		check( rc == 0, va_cleanup );

	}

	rc = mysql_stmt_bind_param( stmt, (MYSQL_BIND *)con->in->el );
	check( rc == 0, va_cleanup );

	count = mysql_stmt_field_count( stmt );

	rc = jak_da_resize( con->out, count );
	check( rc == 0, va_cleanup );

	jak_da_zero_out( con->out );

	for( i = 0; i < count; i++ ) {

		var_name = va_arg( argp, const char * );
		check_msg(
				var_name,
				va_cleanup,
				"Do you have enough bind params in your fra_my() call?"
			 );

		var = fra_var_get_with_type( req, var_name, sizeof( var_name ) - 1, &type, &type_len );
		check( var && type && type_len > 0, va_cleanup );

		rc = call_bind_function_output(
				var,
				type,
				type_len,
				(MYSQL_BIND *)jak_da_get( con->out, i )
				);
		check( rc == 0, va_cleanup );

	}

	check_msg(
			va_arg( argp, const char * ) == NULL,
			va_cleanup,
			"Do you have to many bind params in your fra_my() call?"
		 );

	rc = mysql_stmt_bind_result( stmt, (MYSQL_BIND *)con->out->el );
	check( rc == 0, va_cleanup );

	va_end( argp );

	return 0;

va_cleanup:
	va_end( argp );

final_cleanup:
	return -1;

}
