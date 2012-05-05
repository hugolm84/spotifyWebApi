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
#include <ctype.h>
#include "spshell.h"
#include "cmd.h"


/**
 * Callback for libspotify
 *
 * @param result    The toplist result object that is now done
 * @param userdata  The opaque pointer given to sp_toplistbrowse_create()
 */
static void got_toplist(sp_toplistbrowse *result, void *userdata)
{
	int i;
        json_t *json = json_object();
        json_t *toplist = json_object();

        json_object_set_new(json, "toplist", toplist);
        json_t *results = json_array();
        json_object_set_new(toplist, "result", results);
        // We print from all types. Only one of the loops will acually yield anything.

        for(i = 0; i < sp_toplistbrowse_num_artists(result); i++)
            json_array_append_new(results, get_artist(sp_toplistbrowse_artist(result, i)));

        for(i = 0; i < sp_toplistbrowse_num_albums(result); i++)
            json_array_append_new(results, get_album(sp_toplistbrowse_album(result, i)));

        for(i = 0; i < sp_toplistbrowse_num_tracks(result); i++)
            json_array_append_new(results, get_track(sp_toplistbrowse_track(result, i)));

        cmd_sendresponse(json, 200);
        sp_toplistbrowse_release(result);
        cmd_done();
}

/**
 * toplist_charts
 *
 * returnes json result for possible charts
 * the types are taken from spotify client, there may be more types
 * available for other users in other countries then Sweden.
 */
static void toplist_charts(void)
{


    json_t *obj = json_pack("{s:[{s:[{s:s,s:s},{s:s,s:s},{s:s,s:s}]},{s:[{s:s,s:s},{s:s,s:s},{s:s,s:s},{s:s,s:s},{s:s,s:s},{s:s,s:s},{s:s,s:s},{s:s,s:s},{s:s,s:s},{s:s,s:s},{s:s,s:s},]}]}",
                            "Charts",
                                "types",
                                    "name", "Top Artists",
                                    "id", "artists",
                                    "name", "Top Albums",
                                    "id", "albums",
                                    "name", "Top Tracks",
                                    "id", "tracks",
                                "geo",
                                    "name", "Everywhere",
                                    "id", "global",
                                    "name", "For me",
                                    "id", "user/<username>",
                                    "name", "DK",
                                    "id", "region/DK",
                                    "name", "ES",
                                    "id", "region/ES",
                                    "name", "FI",
                                    "id", "region/FI",
                                    "name", "FR",
                                    "id", "region/FR",
                                    "name", "GB",
                                    "id", "region/GB",
                                    "name", "NO",
                                    "id", "region/NO",
                                    "name", "US",
                                    "id", "region/US",
                                    "name", "SE",
                                    "id", "region/SE",
                                    "name", "NL",
                                    "id", "region/NL"
                            );

    cmd_sendresponse(obj, 200);
}
/**
 *
 */
static void toplist_usage(void)
{
    cmd_sendresponse(put_error(400,"Usage: toplist / (charts) | ( (tracks | albums | artists) / (global | region / <countrycode> | user / <username>) )"), 400);
}


/**
 *
 */
int cmd_toplist(int argc, char **argv)
{
	sp_toplisttype type;
	sp_toplistregion region;
        char username[252];
        if(argc == 2) {
            if(!strcasecmp(argv[1], "charts"))
                toplist_charts();
                return -1;
        }
	if(argc < 3) {
		toplist_usage();
		return -1;
        }

        if(!strcasecmp(argv[1], "artists"))
		type = SP_TOPLIST_TYPE_ARTISTS;
        else if(!strcasecmp(argv[1], "albums"))
		type = SP_TOPLIST_TYPE_ALBUMS;     
        else if(!strcasecmp(argv[1], "tracks"))
		type = SP_TOPLIST_TYPE_TRACKS;    
        else {
		toplist_usage();
		return -1;
	}


        if(!strcasecmp(argv[2], "global"))
                region = SP_TOPLIST_REGION_EVERYWHERE;
        else if(!strcasecmp(argv[2], "user")){
                region = SP_TOPLIST_REGION_USER;

                if(argc != 4 || strlen(argv[3]) < 0) {
                        toplist_usage();
                        return -1;
                }
                strcpy(username, argv[3]);

        }else if(!strcasecmp(argv[2], "region")) {

		if(argc != 4 || strlen(argv[3]) != 2) {
			toplist_usage();
			return -1;
		}

                int i;
                for (i = 0; argv[3][i]; i++)
                    argv[3][i] = toupper(argv[3][i]);

		region = SP_TOPLIST_REGION(argv[3][0], argv[3][1]);
	} else {
		toplist_usage();
		return -1;
        }
        
#ifdef USE_MYSQL
                _mysql_updateStats(g_conn, "toplist");
#endif        

        sp_toplistbrowse_create(g_session, type, region, username, got_toplist, NULL);
	return 0;
}
