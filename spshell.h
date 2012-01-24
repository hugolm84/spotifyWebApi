/**  This file is part of SpotifyWebApi - <hugolm84@gmail.com> ===
 *
 *   Copyright 2011,Hugo Lindström <hugolm84@gmail.com>
 *
 *
 *   Inspiration and reused functions has come from Spotifys spshell example
 *   and that code is:
 *   Copyright (c) 2006-2010 Spotify Ltd
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
#ifndef SPSHELL_H__
#define SPSHELL_H__

#include <stdlib.h>
#include <stdio.h>
#include <libspotify/api.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>


/**
 * BUF_SIZE
 * IMPORTANT!
 *  If you are getting truncated responses, increase this
 */

#define BUF_SIZE 4096
#define PORT 5112
#define USERNAME ""
#define PASSWORD ""
#define STATUS_OK "HTTP/1.0 200 OK\n"
#define STATUS_ERR "HTTP/1.0 400 BAD REQUEST\n"
#define STATUS_CONTERR "HTTP/1.0 404 NOT FOUND\n"
#define STATUS_NIMPL "HTTP/1.0 501 NOT IMPLEMENTED\n"
#define STATUS_CONNERR "HTTP/1.0 503 SERVICE UNAVAILABLE\n"
#define JSON "Content-type: application/json; charset=UTF-8\n\n";

/**
 * MYSQL
 */
/// Comment out USE_MYSQL if you do not wish to use it
//#define USE_MYSQL

#ifdef USE_MYSQL
    #include "sp_mysql.h"
    #include <mysql.h>
    extern MYSQL *g_conn;
#endif

#define _mHOST ""
#define _mUSER ""
#define _mPASS ""
#define _mDATABASE ""
#define _mPORT ""

/// Socket identifiers
int newfd, numbytes, DPORT;
//extern void verbose(char *msg);
extern sp_session *g_session;
extern void (*metadata_updated_fn)(void);
extern int spshell_init(const char *username, const char *password);
extern void notify_main_thread(sp_session *session);
extern void start_recv(void);

#endif // SPSHELL_H__
