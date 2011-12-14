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
#include "spshell.h"
#include "cmd.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef USE_MYSQL
extern MYSQL* g_conn;
extern int _mysql_updateStats(MYSQL *, char*);
#endif
static sp_track *track_browse;
static sp_playlist *playlist_browse;
static sp_playlist_callbacks pl_callbacks;


const char *imagelink(sp_image *image){

    char url[256];
    sp_link *l;
    if(image != NULL ){
        l = sp_link_create_from_image(image);
        sp_link_as_string(l, url, sizeof(url));
        sp_link_release(l);
        sp_image_release(image);
        char *baseurl = "http://o.scdn.co/image/";
        char *cover = strtok(url, ":");
              cover = strtok(NULL, ":");
              cover = strtok(NULL, ":");

        char *coverlink = malloc(strlen(baseurl) + strlen(cover) + 1);

        if ( coverlink != NULL ){
                 strcpy(coverlink, baseurl);
                 strcat(coverlink, cover);

                return (const char*)coverlink;
        }
    }
    return NULL;
}

/**
 * put the track into a json
 *
 * @param sp_track *track
 */

json_t *get_track(sp_track *track)
{

    char url[256];
    sp_link *l;
    l = sp_link_create_from_track(track, 0);
    sp_link_as_string(l, url, sizeof(url));
    sp_link_release(l);
    json_t *metadata = json_object();

    if(sp_track_is_loaded(track)){

            if(sp_track_name(track))
                json_object_set_new_nocheck(metadata, "title", json_string_nocheck(sp_track_name(track)));
            if(sp_album_name(sp_track_album(track)))
                json_object_set_new_nocheck(metadata, "album", json_string_nocheck(sp_album_name(sp_track_album(track))));

            if(sp_artist_name(sp_track_artist(track, 0)))
                json_object_set_new_nocheck(metadata, "artist", json_string_nocheck(sp_artist_name(sp_track_artist(track,0))));


            json_object_set_new_nocheck(metadata, "trackuri", json_string_nocheck(url));


            if(sp_track_duration(track))
                json_object_set_new_nocheck(metadata, "duration", json_integer(sp_track_duration(track)));

                json_object_set_new_nocheck(metadata, "popularity", json_integer(sp_track_popularity(track)));


    }
    return metadata;
}


/**
 * put the artist into a json
 *
 * @param sp_artist *artist
 */

json_t *get_artist(sp_artist *artist)
{

    char url[256];
    sp_link *l;
    l = sp_link_create_from_artist(artist);
    sp_link_as_string(l, url, sizeof(url));
    sp_link_release(l);
    json_t *metadata = json_object();

    if(sp_artist_is_loaded(artist)){

            if(sp_artist_name(artist))
                json_object_set_new_nocheck(metadata, "name", json_string_nocheck(sp_artist_name(artist)));

            json_object_set_new_nocheck(metadata, "artisturi", json_string_nocheck(url));

    }
    return metadata;
}

/**
 * put the track into a json
 *
 * @param sp_track *track
 */

json_t *get_album(sp_album *album)
{

    char url[256];
    sp_link *l;
    l = sp_link_create_from_album(album);
    sp_link_as_string(l, url, sizeof(url));
    sp_link_release(l);
    json_t *metadata = json_object();

    if(sp_album_is_loaded(album)){

             if(sp_album_name(album))
                json_object_set_new_nocheck(metadata, "title", json_string_nocheck(sp_album_name(album)));

             if(sp_artist_name(sp_album_artist(album)))
                json_object_set_new_nocheck(metadata, "artist", json_string_nocheck(sp_artist_name(sp_album_artist(album))));

             json_object_set_new_nocheck(metadata, "albumuri", json_string_nocheck(url));

             const byte *id;
             if((id = sp_album_cover(album)))
                 json_object_set_new_nocheck(metadata, "albumcover", json_string_nocheck(imagelink(sp_image_create(g_session, id))));

              json_object_set_new_nocheck(metadata, "year", json_integer(sp_album_year(album)));


    }
    return metadata;
}



/**
 * Callback for libspotify
 *
 * @param browse    The browse result object that is now done
 * @param userdata  The opaque pointer given to sp_albumbrowse_create()
 */
static void browse_album_callback(sp_albumbrowse *browse, void *userdata)
{
    int i;
    json_t *json = json_object();

        if (sp_albumbrowse_error(browse) == SP_ERROR_OK){

            json_t *album = json_object();
            json_object_set_new(json, "type", json_string_nocheck("album"));
            json_object_set_new(json, "album", album);

            const byte *id;
            if((id = sp_album_cover(sp_albumbrowse_album(browse))))
                json_object_set_new_nocheck(album, "albumcover", json_string_nocheck(imagelink(sp_image_create(g_session, id))));



            json_object_set_new(album, "artist",
                                json_string_nocheck(sp_artist_name(sp_album_artist(sp_albumbrowse_album(browse)))));
            json_object_set_new(album, "name",
                                json_string_nocheck(sp_album_name(sp_albumbrowse_album(browse))));


            json_t *result = json_array();
            json_object_set_new(album, "result", result);

            for (i = 0; i < sp_albumbrowse_num_tracks(browse); ++i)
                json_array_append_new(result, get_track(sp_albumbrowse_track(browse, i)));




             cmd_sendresponse(json, 200);

        }else cmd_sendresponse(put_error(400,sp_error_message(sp_albumbrowse_error(browse))), 400);

	sp_albumbrowse_release(browse);

	cmd_done();
}

int comp(const void* p1, const void* p2) {

  int* arr1 = (int*)p2;
  int* arr2 = (int*)p1;
  int diff1 = arr1[1] - arr2[1];

  if (diff1) return diff1;
    return arr1[0] - arr2[0];
}
/**
 * Callback for libspotify
 *
 * @param browse    The browse result object that is now done
 * @param userdata  The opaque pointer given to sp_artistbrowse_create()
 */
static void browse_artist_callback(sp_artistbrowse *browse, void *userdata)
{
    int i;
    int *limit = (int*)&userdata;


    json_t *json = json_object();

        if (sp_artistbrowse_error(browse) == SP_ERROR_OK){

            json_t *artist = json_object();
            json_object_set_new(json, "type", json_string_nocheck("artist"));
            json_object_set_new(json, "artist", artist);

            const byte *id;
            if((id = sp_artistbrowse_portrait(browse, 0)))
                json_object_set_new_nocheck(artist, "portrait", json_string_nocheck(imagelink(sp_image_create(g_session, id))));



            json_object_set_new(artist, "name",
                                json_string_nocheck(sp_artist_name(sp_artistbrowse_artist(browse))));

            json_object_set_new(artist, "totalTracks",
                                json_integer(sp_artistbrowse_num_tracks(browse)));
            json_t *result = json_array();
            json_object_set_new(artist, "result", result);

            /**
            * So maybe we dont want all tracks, just the top
            * or a limit. So lets sort the response for that
            **/
            int trackArray[sp_artistbrowse_num_tracks(browse)][2];
            int k = 0;
            int j = 0;

            /**
            * @note: High number of tracks seems to lead to timeouts or unreasonable response times.
            * @todo: Do some more checking here, and validate offical tracks only?
            **/
            char *excludelist[4] = {"Remix", "Remaster", "Cover", "Ringtone"};

            for (i = 0; i < sp_artistbrowse_num_tracks(browse)-1; ++i){
                int skip;
                if( strlen(sp_artist_name(sp_artistbrowse_artist(browse))) == strlen(sp_artist_name(sp_track_artist(sp_artistbrowse_track(browse,i),0))) ){
                    if( strcmp(sp_artist_name(sp_artistbrowse_artist(browse)), sp_artist_name(sp_track_artist(sp_artistbrowse_track(browse,i),0))) == 0){
                        for(j = 0; j < 4; j++)
                            if(strstr(sp_track_name(sp_artistbrowse_track(browse,i)), excludelist[j]) != NULL)
                                skip = 1;

                        if(skip != 1){
                            trackArray[k][0] = i;
                            trackArray[k][1] = sp_track_popularity(sp_artistbrowse_track(browse,i));
                            k++;
                        }
                    }
                }
            }


            /**
             * Sort
             **/
            qsort(trackArray, k, 2*sizeof(int), comp);
            j = 0;
            for(i = 0; i < k; i++)
                if(*limit == 0){
                    json_array_append_new(result, get_track(sp_artistbrowse_track(browse, trackArray[i][0])));
                    j++;
                }
                else if(i < *limit){
                    json_array_append_new(result, get_track(sp_artistbrowse_track(browse, trackArray[i][0])));
                    j++;
                }

            json_object_set_new(artist, "resultCount",
                                json_integer(j));

            cmd_sendresponse(json, 200);

        }else cmd_sendresponse(put_error(400, sp_error_message(sp_artistbrowse_error(browse))), 400);

	sp_artistbrowse_release(browse);

	cmd_done();
}

/**
 * Callback for libspotify
 *
 * @param browse    The browse result object that is now done
 * @param userdata  The opaque pointer given to sp_artistbrowse_create()
 */
static void browse_artistalbums_callback(sp_artistbrowse *browse, void *userdata)
{
    int i;
    json_t *json = json_object();

        if (sp_artistbrowse_error(browse) == SP_ERROR_OK){

            json_t *artist = json_object();
            json_object_set_new(json, "type", json_string_nocheck("albums"));
            json_object_set_new(json, "albums", artist);

            const byte *id;
            if((id = sp_artistbrowse_portrait(browse, 0)))
                json_object_set_new_nocheck(artist, "portrait", json_string_nocheck(imagelink(sp_image_create(g_session, id))));

            json_object_set_new(artist, "name",
                                json_string_nocheck(sp_artist_name(sp_artistbrowse_artist(browse))));

            json_t *result = json_array();
            json_object_set_new(artist, "result", result);

            for (i = 0; i < sp_artistbrowse_num_albums(browse); ++i)
                json_array_append_new(result, get_album(sp_artistbrowse_album(browse, i)));



            cmd_sendresponse(json, 200);


        }else cmd_sendresponse(put_error(400, sp_error_message(sp_artistbrowse_error(browse))), 400);

        sp_artistbrowse_release(browse);

        cmd_done();
}


/**
 *
 */
static void track_browse_try(void)
{
        json_t *json = json_object();
        int ok;
        switch (sp_track_error(track_browse)) {
	case SP_ERROR_OK:
                ok = 1;
		break;

	case SP_ERROR_IS_LOADING:
		return; // Still pending

	default:
                cmd_sendresponse(put_error(400,sp_error_message(sp_track_error(track_browse))), 400);
		break;
	}
        if(ok){

            json_t *track = json_object();
            json_object_set_new(json, "type", json_string_nocheck("track"));
            json_object_set_new(json, "track", track);
            json_t *result = json_array();
            json_object_set_new(track, "result", result);
            json_array_append_new(result, get_track(track_browse));

            cmd_sendresponse(json, 200);
        }
	metadata_updated_fn = NULL;
        sp_track_release(track_browse);
	cmd_done();

}



/**
 *
 */
static void playlist_browse_try(void)
{
        int i, tracks;

        metadata_updated_fn = playlist_browse_try;
        if(!sp_playlist_is_loaded(playlist_browse)) {
                return;
        }

        tracks = sp_playlist_num_tracks(playlist_browse);
        for(i = 0; i < tracks; i++) {
                sp_track *t = sp_playlist_track(playlist_browse, i);
                if (!sp_track_is_loaded(t))
                        return;
        }

        json_t *json = json_object();
        json_t *playlist = json_object();
        json_object_set_new(json, "type", json_string_nocheck("playlist"));
        json_object_set_new(json, "playlist", playlist);

        json_t *result = json_array();
        json_object_set_new(playlist, "result", result);


        for(i = 0; i < tracks; i++)
            json_array_append_new(result, get_track(sp_playlist_track(playlist_browse, i)));


        json_object_set_new(playlist, "name",
                            json_string_nocheck(sp_playlist_name(playlist_browse)));

        sp_user *owner = sp_playlist_owner(playlist_browse);
        json_object_set_new_nocheck(playlist, "creator",
                                    json_string_nocheck(sp_user_display_name(owner)));
        sp_user_release(owner);

        json_object_set_new(playlist, "subscribers",
                            json_integer(sp_playlist_num_subscribers(playlist_browse)));

        sp_playlist_remove_callbacks(playlist_browse, &pl_callbacks, NULL);
        sp_playlist_release(playlist_browse);
        playlist_browse = NULL;
        metadata_updated_fn = NULL;

        cmd_sendresponse(json, 200);
        cmd_done();

}



/**
 *
 */
static void pl_state_change(sp_playlist *pl, void *userdata)
{
        playlist_browse_try();
}

static sp_playlist_callbacks pl_callbacks = {
        NULL,
        NULL,
        NULL,
        NULL,
	pl_state_change,
};


void browse_playlist(sp_playlist *pl)
{
        playlist_browse = pl;
        sp_playlist_add_callbacks(playlist_browse, &pl_callbacks, NULL);
        playlist_browse_try();
}


/**
 *
 */
static void albums_usage(void)
{
        cmd_sendresponse(put_error(400,"Usage: albums <spotify-uri>"), 400);
}

/**
 *
 */
static void browse_usage(void)
{
        cmd_sendresponse(put_error(400,"Usage: browse <spotify-uri> <int-limit>"), 400);
}




/**
 *
 */
int cmd_albums(int argc, char **argv)
{
        sp_link *link;

        if (argc != 2) {
                albums_usage();
                return -1;
        }


        link = sp_link_create_from_string(argv[1]);

        if (!link) {
                cmd_sendresponse(put_error(400,"Not a spotify link"), 400);
                return -1;
        }

        switch(sp_link_type(link)) {
        default:
                cmd_sendresponse(put_error(400,"Can not handle link"), 400);
                sp_link_release(link);
                return -1;

        case SP_LINKTYPE_ARTIST:
                sp_artistbrowse_create(g_session, sp_link_as_artist(link), SP_ARTISTBROWSE_NO_TRACKS, browse_artistalbums_callback, NULL);
                break;
        }

        sp_link_release(link);
        return 0;
}

/**
 *
 */
int cmd_browse(int argc, char **argv)
{

	sp_link *link;

        if (argc < 2) {
		browse_usage();
		return -1;
	}

	
	link = sp_link_create_from_string(argv[1]);

        long limit = (argc == 3) ? atoi(argv[2]) : 0;

	if (!link) {
                cmd_sendresponse(put_error(400,"Not a spotify link"), 400);
		return -1;
	}

	switch(sp_link_type(link)) {
	default:
                cmd_sendresponse(put_error(400,"Can not handle link"), 400);
		sp_link_release(link);
		return -1;

        case SP_LINKTYPE_ALBUM:
#ifdef USE_MYSQL
                _mysql_updateStats(g_conn, "album");
#endif
		sp_albumbrowse_create(g_session, sp_link_as_album(link), browse_album_callback, NULL);
		break;

	case SP_LINKTYPE_ARTIST:
#ifdef USE_MYSQL
                _mysql_updateStats(g_conn, "artist");
#endif
                sp_artistbrowse_create(g_session, sp_link_as_artist(link), SP_ARTISTBROWSE_FULL, browse_artist_callback, (void*)limit);
                break;

	case SP_LINKTYPE_LOCALTRACK:
	case SP_LINKTYPE_TRACK:
#ifdef USE_MYSQL
                _mysql_updateStats(g_conn, "track");
#endif
                track_browse = sp_link_as_track(link);
                metadata_updated_fn = track_browse_try;
                sp_track_add_ref(track_browse);
                track_browse_try();
		break;

	case SP_LINKTYPE_PLAYLIST:
#ifdef USE_MYSQL
                _mysql_updateStats(g_conn, "playlist");
#endif
		browse_playlist(sp_playlist_create(g_session, link));
		break;
	}

	sp_link_release(link);
	return 0;
}
