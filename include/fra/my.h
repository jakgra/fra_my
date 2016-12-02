/**
 * Set the default connection
 */
int fra_my_set( char * server, char * user, ... );

/**
 * Set the named connection
 */
int fra_my_con_set( char * con_name, char * server, char * user, ... );

/**
 * Main function. Executes a mysql statement on the specified connection or the default one if fra_my() is used.
 * callback is the callback to call after the statement finished.
 * con_name is the connection name in the hash table.
 * sql is the sql statement passed to my_stmt_prepare(). The ... are the name of the
 * arguments that are looked up in the req and endpoint hashtables.
 * First the input ones and then the output ones.
 * Should there be NULL in beetween and on the end for type safety?
 */
int fra_my( fra_my_cb callback, char * sql, ... );

int fra_my_con( fra_my_cb callback, char * con_name, char * sql, ... );

/**
 * fra_my_multi_cb is called with the fra_req_t and the my_result type...
 * You only have to supply the input params. The output ones you have to set
 * yourself from the my_result type.
 */
int fra_my_multi( fra_my_mutli_cb callback, char * sql, ... );

int fra_my_con_multi( fra_my_multi_cb callback, char * con_name, char * sql, ... );
