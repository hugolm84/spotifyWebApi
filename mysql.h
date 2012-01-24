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
#ifndef MYSQL_H__
#define MYSQL_H__
#include <mysql.h>
/// pointer to global connection, initate in thread
MYSQL *g_conn;
MYSQL *_mysql_connect();
int _mysql_updateStats(MYSQL *conn, char *type);
int _mysql_close(MYSQL *conn);
#endif // MYSQL_H__
