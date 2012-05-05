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
#include <jansson.h>
#include "spshell.h"
#include "cmd.h"

#ifdef USE_MYSQL
extern MYSQL* g_conn;
extern int _mysql_updateStats(MYSQL *, char*);
#endif

extern json_t *get_track(sp_track *track);
extern json_t *get_artist(sp_artist *artist);
extern json_t *get_album(sp_album *album);
extern char* replace(const char *const original, const char *const pattern, const char *const replacement);

struct searchData {
    char *type;
    char query[1024];
    int limit;
};

/**
    @param  search   The search result
    @TODO: use limit and utilize popularity
 */
static void get_search(sp_search *search,void* userdata)
{
        int i;
        struct searchData *data = userdata;
        json_t *json = json_object();

        // Default search is tracks
        if( !strcmp(data->type, "") || !strcmp(data->type, "track" ) ){
            json_t *tracks = json_array();
            json_object_set_new(json, "tracks", tracks);
            for (i = 0; i < sp_search_num_tracks(search); ++i){
                json_array_append_new(tracks, get_track(sp_search_track(search, i)));
            }
        }

        if( !strcmp(data->type, "artist" ) ){
            json_t *artists = json_array();
            json_object_set_new(json, "artists", artists);
            for (i = 0; i < sp_search_num_artists(search); ++i){
                json_array_append_new(artists, get_artist(sp_search_artist(search, i)));
            }
        }

        if( !strcmp(data->type, "album" ) ){
            json_t *albums = json_array();
            json_object_set_new(json, "albums", albums);
            for (i = 0; i < sp_search_num_albums(search); ++i){
                json_array_append_new(albums, get_album(sp_search_album(search, i)));
            }
        }

        if( !strcmp(data->type, "playlist" ) ){
            json_t *playlists = json_array();
            json_object_set_new(json, "playlists", playlists);
            for (i = 0; i < sp_search_num_playlists(search); ++i){
                json_t *metadata = json_object();
                json_object_set_new_nocheck(metadata, "name", json_string_nocheck( sp_search_playlist_name(search, i)));
                json_object_set_new_nocheck(metadata, "playlisturi", json_string_nocheck(sp_search_playlist_uri(search, i)));
                json_array_append_new(playlists, metadata);
            }
        }

        json_object_set_new_nocheck(json, "query", json_string_nocheck(sp_search_query(search)));
        cmd_sendresponse(json, 200);
        free(data);
}

/**
 *
 */
static void search_complete(sp_search *search, void *userdata)
{

    if (sp_search_error(search) == SP_ERROR_OK){
         get_search(search, userdata);
    }
    else cmd_sendresponse(put_error(400, sp_error_message(sp_search_error(search))), 400);

    sp_search_release(search);
    cmd_done();
}



/**
 *
 */
static void search_usage(void)
{
    cmd_sendresponse(put_error(400, "Usage: search/<string-optional type>(track as default)/<string-query>/<int-limit> Example: /search/track/artist:Madonna track:Like a prayer."), 400);
}


/**
 *
 */
int cmd_search(int argc, char **argv)
{

        int i;
        if (argc < 2 ) {
                search_usage();
                return -1;
        }

        // Keeping struct of searchdata, to pass on
        // remember to free it
        struct searchData *data = malloc(sizeof(struct searchData));
        data->limit = 0;
        data->type = NULL;
        data->query[0] = 0;
        int limitIsZero = 0;

        // Searchtypes are:
        char *searchTypes[] = { "playlist","track","album","artist" };
        // Find the type
        for(i = 0; i < sizeof(searchTypes) / sizeof(searchTypes[0]); i++)
            if(!strcmp(searchTypes[i], argv[1]))
                data->type = searchTypes[i];

        // Limit is last arg
        // Guard against 0 as limit in param
        if(!strcmp("0", argv[argc-1]))
            limitIsZero = 1;
         data->limit = atoi(argv[argc-1]);

        // Concatenate the query tokens, leave out the type and limit
        for(i = ( data->type != NULL ? 2 : 1); i < argc-( data->limit > 0 ? 1 : 0+limitIsZero ); i++)
        {
            /// @note: artist:/track:/album: prefixing wont work on playlists
            if( data->type != NULL && !strcmp( data->type, "playlist"))
            {
                argv[i] = replace(argv[i], "artist:", "");
                argv[i] = replace(argv[i], "track:", "");
                argv[i] = replace(argv[i], "album:", "");
                argv[i] = replace(argv[i], "playlist:", "");
            }
            snprintf(data->query + strlen(data->query), sizeof(data->query) - strlen(data->query), "%s%s", i == 1 ? "" : " ", argv[i]);

        }

        // Just for safety when passing on
        if(data->type == NULL)
            data->type = "";

        // Null or 0 limit is 100 as default
        if( data->limit < 0)
            data->limit = 100;

        //printf("Query type:%s, limit:%d, query:%s\n", data->type, data->limit, data->query);

#ifdef USE_MYSQL
       _mysql_updateStats(g_conn, "search");
#endif
        sp_search_create(g_session,data->query, 0, data->limit, 0, data->limit, 0, data->limit, 0, data->limit, SP_SEARCH_STANDARD, &search_complete, data);


   return 0;
}


