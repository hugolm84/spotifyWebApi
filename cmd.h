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

#ifndef CMD_H__
#define CMD_H__
#include <jansson.h>

extern void cmd_exec_unparsed(char *l);
extern void cmd_dispatch(int argc, char **argv);
extern void cmd_done(void);

extern int cmd_browse(int argc, char **argv);
extern int cmd_search(int argc, char **argv);


/* Shared functions */
extern void cmd_sendresponse(json_t *resp, int code);
void browse_playlist(sp_playlist *pl);
extern json_t *get_track(sp_track *track);
extern json_t *put_error(int code, const char *error);



#endif // CMD_H__
