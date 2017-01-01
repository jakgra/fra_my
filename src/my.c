#include <fra/my.h>

#include <fra/core.h>
#include <mysql/mysql.h>
#include <stdio.h>




int fra_my_set( char * host, char * user, char * passwd, char * db, unsigned int port, char * unix_socket, unsigned long clientflag ) {

	fra_end_t * e;
	int res;


	printf( "Mysql client version is: %s\n", mysql_get_client_info() );

	e = fra_end_new( 20 );
	res = ( e != NULL );

	fra_end_free( e );

	return res + 187;

}
