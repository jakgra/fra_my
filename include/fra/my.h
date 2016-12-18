/**
 * Set the default connection
 */
int fra_my_set( char * server, char * user, ... );

/**
 * Main function. Executes a mysql statement on the specified connection or the default one if fra_my() is used.
 * callback is the callback to call after the statement finished.
 * con is the connection to use for the sql statement.
 * sql is the sql statement passed to my_stmt_prepare(). The ... are the name of the
 * arguments that are looked up in the req and endpoint hashtables.
 * First the input ones and then the output ones.
 * Should there be NULL in beetween and on the end for type safety?
 */
int fra_my( fra_my_cb callback, char * sql, ... );

/**
 * fra_my_multi_cb is called with the fra_req_t and the my_result type...
 * You only have to supply the input params. The output ones you have to set
 * yourself from the my_result type.
 */
int fra_my_multi( fra_my_mutli_cb callback, char * sql, ... );

/**
 * Support for multiple different connections, to different mysql servers.
 */
typedef fra_my_con_t struct fra_my_con;

/**
 * Returns a fra_my_con_t object that can be used in all the fra_my_con***() functions
 * to specify this connection. See fra_my_set() for parameters description.
 */
fra_my_con_t * fra_my_con_new( char * server, char * user, .. );

/**
 * Call to free memory associated with an fra_my_con_t object,
 * previously created via fra_my_con_new()
 */
void fra_my_con_free( fra_my_con_t * con );

/**
 * Same as fra_my(), but allows to specify the connection to use.
 */
int fra_my_con( fra_my_cb callback, fra_my_con_t * con, char * sql, ... );

/**
 * Same as fra_my_multi(), but allows to specify the connection to use.
 */
int fra_my_con_multi( fra_my_multi_cb callback, fra_my_con_t * con, char * sql, ... );
