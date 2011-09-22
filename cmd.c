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
#include <string.h>

#include "spshell.h"
#include "cmd.h"
#include <jansson.h>

static int cmd_help(int argc, char **argv);

/**
 *
 */
struct {
	const char *name;
	int (*fn)(int argc, char **argv);
	const char *help;
        const char *usage;
        const char *notes;
} commands[] = {

{ "browse",cmd_browse,"Browse a Spotify URI","/browse/<spotify-uri>", ""},
{ "search",cmd_search,"Search","/search/<query-string>", "Replace spaces with +"},
{ "help",  cmd_help,  "help", "/help", ""},

};


json_t *put_error(int code, const char *error){


        json_t *obj = json_pack("{s:i,s:{s:s}}",
                                "code", code,
                                "error",
                                    "msg", error);

        return obj;
}
/**
 *
 */
static int tokenize(char *buf, char **vec, int vsize)
{
        int n = 0;
        while(1) {
                while(*buf > 0 && *buf < 33)
                        buf++;
                if(!*buf)
                        break;
                vec[n++] = buf;
                if(n == vsize)
                        break;
                while(*buf > 32)
                        buf++;
                if(*buf == 0)
                        break;
                *buf = 0;
                buf++;
        }
        return n;
}


/**
 *
 */
void cmd_exec_unparsed(char *l)
{
	char *vec[32];
	int c = tokenize(l, vec, 32);
	cmd_dispatch(c, vec);
}


/**
 *
 */
void cmd_dispatch(int argc, char **argv)
{
	int i;

	if(argc < 1) {
                cmd_sendresponse(put_error(501,"Not implemented"), 501);
		cmd_done();
		return;
	}

	for(i = 0; i < sizeof(commands) / sizeof(commands[0]); i++) {
		if(!strcmp(commands[i].name, argv[0])) {
			if(commands[i].fn(argc, argv))
				cmd_done();
			return;
		}
	}
        cmd_sendresponse(put_error(501,"Not implemented"), 501);
	cmd_done();
}

/**
 *
 */
static int cmd_help(int argc, char **argv)
{
	int i;
        json_t *json = json_object();
        json_t *help = json_object();
        json_object_set_new(json, "help", help);
        json_t *result = json_array();
        json_object_set_new(help, "commands", result);

        for(i = 0; i < sizeof(commands) / sizeof(commands[0]); i++){
               json_t *obj = json_pack("{s:s, s:s, s:s}",
                                       commands[i].name,commands[i].help,
                                       "usage", commands[i].usage,
                                       "notes", commands[i].notes);
               json_array_append(result, obj);

        }
        cmd_sendresponse(json, 200);
	return -1;
}
