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


static sp_track *track_browse;
static sp_playlist *playlist_browse;
static sp_playlist_callbacks pl_callbacks;

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

            json_object_set_new(json, "album", album);
            json_t *result = json_array();
            json_object_set_new(album, "result", result);

            for (i = 0; i < sp_albumbrowse_num_tracks(browse); ++i)
                json_array_append(result, get_track(sp_albumbrowse_track(browse, i)));

            json_object_set_new(album, "artist",
                                json_string_nocheck(sp_artist_name(sp_album_artist(sp_albumbrowse_album(browse)))));
            json_object_set_new(album, "name",
                                json_string_nocheck(sp_album_name(sp_albumbrowse_album(browse))));

             cmd_sendresponse(json, 200);

        }else cmd_sendresponse(put_error(400,sp_error_message(sp_albumbrowse_error(browse))), 400);

	sp_albumbrowse_release(browse);

	cmd_done();
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
    json_t *json = json_object();

        if (sp_artistbrowse_error(browse) == SP_ERROR_OK){

            json_t *artist = json_object();

            json_object_set_new(json, "artist", artist);
            json_t *result = json_array();
            json_object_set_new(artist, "result", result);

            for (i = 0; i < sp_artistbrowse_num_tracks(browse); ++i)
                json_array_append(result, get_track(sp_artistbrowse_track(browse, i)));

            json_object_set_new(artist, "name",
                                json_string_nocheck(sp_artist_name(sp_artistbrowse_artist(browse))));

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

            json_object_set_new(json, "track", track);
            json_t *result = json_array();
            json_object_set_new(track, "result", result);
            json_array_append(result, get_track(track_browse));

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

        json_object_set_new(json, "playlist", playlist);
        json_t *result = json_array();
        json_object_set_new(playlist, "result", result);


        for(i = 0; i < tracks; i++)
            json_array_append(result, get_track(sp_playlist_track(playlist_browse, i)));


        json_object_set_new(playlist, "name",
                            json_string_nocheck(sp_playlist_name(playlist_browse)));

        sp_user *owner = sp_playlist_owner(playlist_browse);
        json_object_set_new_nocheck(playlist, "creator",
                                    json_string_nocheck(sp_user_display_name(owner)));
        sp_user_release(owner);

        json_object_set_new(playlist, "subscribers",
                            json_integer(sp_playlist_num_subscribers(playlist_browse)));


        json_object_set_new(playlist, "trackCount",
                            json_integer(sp_playlist_num_tracks(playlist_browse)));

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
static void browse_usage(void)
{
        cmd_sendresponse(put_error(400,"Usage: browse <spotify-uri>"), 400);
}


/**
 *
 */
int cmd_browse(int argc, char **argv)
{
	sp_link *link;

	if (argc != 2) {
		browse_usage();
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

	case SP_LINKTYPE_ALBUM:
		sp_albumbrowse_create(g_session, sp_link_as_album(link), browse_album_callback, NULL);
		break;

	case SP_LINKTYPE_ARTIST:
		sp_artistbrowse_create(g_session, sp_link_as_artist(link), browse_artist_callback, NULL);
		break;

	case SP_LINKTYPE_LOCALTRACK:
	case SP_LINKTYPE_TRACK:
                track_browse = sp_link_as_track(link);
                metadata_updated_fn = track_browse_try;
                sp_track_add_ref(track_browse);
                track_browse_try();
		break;

	case SP_LINKTYPE_PLAYLIST:
		browse_playlist(sp_playlist_create(g_session, link));
		break;
	}

	sp_link_release(link);
	return 0;
}
