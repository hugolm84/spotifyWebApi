/**  This file is part of SpotifyWebApi - <hugolm84@gmail.com> ===
 *
 *   Copyright 2011,Hugo Lindström <hugolm84@gmail.com>
 *
 *   Permission is hereby granted, free of charge, to any person obtaining a copy
 *   of this software and associated documentation files (the "Software"), to deal
 *   in the Software without restriction, including without limitation the rights
 *   to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 *   copies of the Software, and to permit persons to whom the Software is
 *   furnished to do so, subject to the following conditions:
 *
 *   The above copyright notice and this permission notice shall be included in
 *   all copies or substantial portions of the Software.
 */
#include <mysql/mysql.h>
#include <stdio.h>
#include "spshell.h"
#include <stdlib.h>
#include <string.h>


MYSQL *_mysql_connect(){


    MYSQL *conn; /* pointer to connection handler */
    if (( conn = malloc( sizeof( MYSQL )) ) != NULL ){

        conn = mysql_init ( NULL );

        if (!mysql_real_connect(conn, _mHOST,_mUSER, _mPASS, _mDATABASE, 0, NULL, 0)){
              fprintf(stderr, "%s\n", mysql_error(conn));
              printf("Failed to init mysql!\n");
              exit(1);
        }
        else
            printf("Successfully initated mysql connection\n");
    }

    return conn;

}

int _mysql_updateStats(MYSQL *conn, char *type){

    if(!conn)
        fprintf(stderr, "%s\n", mysql_error(conn));

    char stmt_buf[1024], buf[1024];
    {
        (void) mysql_real_escape_string (conn, buf, type, strlen (type));
        sprintf (stmt_buf, "INSERT IGNORE spotifyparser SET statskey='%s', hits = 0;", buf);
        if(stmt_buf != NULL){
            printf("Doing query %s\n", stmt_buf);
            mysql_query(conn, stmt_buf);
        }
    }
    {
        (void) mysql_real_escape_string (conn, buf, type, strlen (type));
        sprintf (stmt_buf, "UPDATE spotifyparser SET hits = hits + 1 WHERE statskey='%s'", buf);
        if(stmt_buf != NULL){
            printf("Doing query %s\n", stmt_buf);
            mysql_query(conn, stmt_buf);
        }
    }

    return 1;

}

int _mysql_close(MYSQL *conn){

    if(conn)
        mysql_close ( conn );
    return 0;
}
