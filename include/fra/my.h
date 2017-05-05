#ifndef fra_my_h
#define fra_my_h


#include <fra/core.h>
#include <mysql/mysql.h>




/**
 * The init function. It isn't thread safe.
 * Must be called exactly once before calling any other functions from the library.
 */
int fra_my_init();

/**
 * Releases all the resources fra_my_init() and the other functions from the library allocated.
 */
void fra_my_deinit();

/**
 * Set the default connection. For argument description see the mysql_real_connect() function from the
 * version of the libmysqlclient you are linking with.
 */
int fra_my_set( char * host, char * user, char * passwd, char * db, unsigned int port, char * unix_socket, unsigned long clientflag );

/**
 * Callback type for function to be called after mysql server returns the result or fails after a call to fra_my() or fra_my_con()
 */
typedef int (*fra_my_cb)( fra_req_t * );

/**
 * Main function. Executes a mysql statement on the connection previously set by fra_my_set().
 * callback is the callback to call after the statement finished.
 * con is the connection to use for the sql statement.
 * sql is the sql statement passed to my_stmt_prepare(). It must be a static string literal,
 * else use the fra_my_ds() function.
 * The ... are the names of the arguments that are looked up using the fra() macro.
 * They must be static string literals, else use the fra_my_ds() function.
 * First the input ones and then the output ones.
 * Should there be NULL in beetween and on the end for type safety?
 */
int fra_my( fra_req_t * req, fra_my_cb callback, char * sql, ... );

/**
 * Same as fra_my() but the arguments don't have to be string literals,
 * but can be dynamic strings (hence *_ds()), and you have to pass the char * and length.
 * The ... have to be for example: (char *)str1, (int)str1_len, (char *)str2, (int)str2_len, ...
 */
int fra_my_ds( fra_req_t * req, fra_my_cb callback, char * sql, int sql_len, ... );

/**
 * Callback type for function to be called after mysql server returns the result or fails after a call to fra_my_multi() or fra_my_con_multi()
 */
typedef int (*fra_my_multi_cb)( fra_req_t *, MYSQL_RES * );

/**
 * fra_my_multi_cb is called with the fra_req_t and the my_result type...
 * You only have to supply the input params. The output ones you have to set
 * yourself from the my_result type.
 */
int fra_my_multi( fra_req_t * req, fra_my_multi_cb callback, char * sql, ... );

/**
 * Support for multiple different connections, to different mysql servers.
 */
typedef struct fra_my_con fra_my_con_t;

/**
 * Returns a fra_my_con_t object that can be used in all the fra_my_con***() functions
 * to specify this connection. See fra_my_set() for parameters description.
 */
fra_my_con_t * fra_my_con_new( char * host, char * user, char * passwd, char * db, unsigned int port, char * unix_socket, unsigned long clientflag );

/**
 * Call to free memory associated with an fra_my_con_t object,
 * previously created via fra_my_con_new()
 */
void fra_my_con_free( fra_my_con_t * con );

/**
 * Adds a connection to be available in requests with a specific endpoint.
 */
int fra_my_con_add_to_end( fra_my_con_t * con, fra_end_t * e );

/**
 * Adds a connection to be available in all requests.
 */
int fra_my_con_add_to_req( fra_my_con_t * con );

/**
 * Same as fra_my(), but allows to specify the connection to use.
 */
int fra_my_con( fra_my_cb callback, fra_my_con_t * con, char * sql, ... );

/**
 * Same as fra_my_multi(), but allows to specify the connection to use.
 */
int fra_my_con_multi( fra_my_multi_cb callback, fra_my_con_t * con, char * sql, ... );

/**
 * Register additional input or output types for mysql data binding (prepared statements).
 */
int fra_my_reg_input_type(
		char * type_name,
		int (*callback)(
			void * var,
			const char * type_name,
			int type_name_len,
			MYSQL_BIND * input
			)
		);

/**
 * Register additional input or output types for mysql data binding (prepared statements).
 */
int fra_my_reg_output_type(
		char * type_name,
		int (*callback)(
			void * var,
			const char * type_name,
			int type_name_len,
			MYSQL_BIND * output
			)
		);




#endif
