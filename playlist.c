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

static sp_playlist_callbacks pl_update_callbacks;

/* --------------------  PLAYLIST CONTAINER CALLBACKS  --------------------- */
static void playlist_added(sp_playlistcontainer *pc, sp_playlist *pl,
                           int position, void *userdata)
{
    if(!sp_playlist_is_loaded(pl))
        return;

    printf("New playlist added!\n");

}

static void playlist_removed(sp_playlistcontainer *pc, sp_playlist *pl,
                             int position, void *userdata)
{
    if(sp_playlist_is_loaded(pl))
        return;

     printf("A playlist got removed!\n");
}


static void container_loaded(sp_playlistcontainer *pc, void *userdata)
{
     printf("Rootlist synced!\n");
}


static sp_playlistcontainer_callbacks pc_callbacks = {
    .playlist_added = &playlist_added,
    .playlist_removed = &playlist_removed,
    .container_loaded = &container_loaded,
};

int cmd_load_container(){

    int i;
    sp_playlistcontainer *pc = sp_session_playlistcontainer(g_session);
    sp_playlistcontainer_add_callbacks(pc,&pc_callbacks,NULL);

    if(!sp_playlistcontainer_is_loaded(pc)){
        printf("Container not fully loaded!\n");
        return 0;
    }
    return 1;
}

/* --------------------  PLAYLIST CONTAINER CALLBACKS  --------------------- */

int cmd_create_playlist(int argc, char **argv){
	

    json_t *json = json_object();
    json_t *playlist = json_object();
	char url[255];
	sp_playlistcontainer *pc = sp_session_playlistcontainer(g_session);
	sp_link *link;
 	sp_playlist *pl = sp_playlistcontainer_add_new_playlist(pc, argv[1]);
    sp_playlistcontainer_add_callbacks(pc,&pc_callbacks,NULL);

    if(!sp_playlistcontainer_is_loaded(pc)){
        printf("Container not fully loaded!\n");
        cmd_sendresponse(put_error(400,"Failed to create playlist, container not fully loaded!"), 400);
        cmd_done();
		return 0;
	}

  	if (pl == NULL){ 
        printf("Failed to create playlist!\n");
        cmd_sendresponse(put_error(400,"Failed to create playlist"), 400);
        cmd_done();
        return -1;
	}
    if (!sp_playlist_is_loaded(pl)){
        cmd_sendresponse(put_error(400,"Failed to load new playlist!"), 400);
        cmd_done();
		return -1;
    }

    if(!sp_playlist_is_collaborative(pl))
        sp_playlist_set_collaborative(pl, 1);

    link = sp_link_create_from_playlist(pl);
    sp_link_as_string(link, url, sizeof(url));
    sp_link_release(link);
    json_object_set_new(playlist, "uri", json_string_nocheck(url));
    json_object_set_new(playlist, "name", json_string_nocheck(sp_playlist_name(pl)));
    json_object_set_new(playlist, "collaborative", json_integer(sp_playlist_is_collaborative(pl)));
    json_object_set_new(json, "code", json_integer(200));
    json_object_set_new(json, "create", playlist);

    sp_playlist_release(pl);
    cmd_sendresponse(json, 200);
    cmd_done();

	return 0;
}

/**
 *
 */
struct pl_update_work {
	int position;
	int num_tracks;
    sp_track **tracks;
};

static int apply_changes(sp_playlist *pl, struct pl_update_work *puw)
{
	sp_link *l;
	sp_error err;

	if(!sp_playlist_is_loaded(pl))
		return 1;
			
	if(!sp_playlist_is_collaborative(pl))
		sp_playlist_set_collaborative(pl, 1);


	l = sp_link_create_from_playlist(pl);
	if(l == NULL)
		return 1;
	sp_link_release(l);

    printf("Playlist loaded, applying changes ... ");

    err = sp_playlist_add_tracks(pl, puw->tracks,
    puw->num_tracks, puw->position, g_session);



	switch(err) {
	case SP_ERROR_OK:
        printf("added tracks to pos %d\n", puw->position);
		break;
	case SP_ERROR_INVALID_INDATA:
        printf("Invalid position\n");
		break;

	case SP_ERROR_PERMISSION_DENIED:
        printf("Access denied\n");
		break;
	default:
        printf("Other error (should not happen)\n");
		break;
	}
	return 0;
}

static void pl_state_change(sp_playlist *pl, void *userdata)
{
	struct pl_update_work *puw = userdata;
	if(apply_changes(pl, puw))
		return;

	sp_playlist_remove_callbacks(pl, &pl_update_callbacks, puw);
	sp_playlist_release(pl);
    cmd_done();
}

static sp_playlist_callbacks pl_update_callbacks = {
	NULL,
	NULL,
	NULL,
	NULL,
	pl_state_change,
};


/**
 *
 */
int cmd_playlist_add_track(int argc, char **argv)
{


    json_t *json = json_object();
    json_t *tracks = json_array();
    json_object_set_new(json, "tracks", tracks);


	sp_link *plink, *tlink, *l;
	sp_track *t;
	sp_playlist *pl;
    int i, k, j, c;
	char url[256];	
	struct pl_update_work *puw;

	if(argc < 4) {
        printf("\n");
        cmd_sendresponse(put_error(400,"add/[playlist uri]/[position]/[track uri] <[track uri]>..."), 400);
        cmd_done();
		return 1;
	}

	plink = sp_link_create_from_string(argv[1]);
	if (!plink) {
        cmd_sendresponse(put_error(400,"Playlist link is not a spotify link"), 400);
        cmd_done();
		return -1;
	}

	if(sp_link_type(plink) != SP_LINKTYPE_PLAYLIST) {
        cmd_sendresponse(put_error(400,"Playlist link is not a valid spotify link"), 400);
        cmd_done();
		sp_link_release(plink);
		return -1;
	}

	puw = malloc(sizeof(struct pl_update_work));
	puw->position = atoi(argv[2]);
	puw->tracks = malloc(sizeof(sp_track *) * argc - 3);
	puw->num_tracks = 0;

    c = argc - 3;

    pl = sp_playlist_create(g_session, plink);
    puw->position = sp_playlist_num_tracks(pl);

    for(i = 0; i < argc - 3; i++)
    {
        for(k = 0; k < sp_playlist_num_tracks(pl); k++)
        {
		
			l = sp_link_create_from_track(sp_playlist_track(pl, k), 0);
			sp_link_as_string(l, url, sizeof(url));
			sp_link_release(l);

            for(j = 0; j < argc - 3; j++)
            {
                if(!strcasecmp(url,argv[j + 3]))
                {
					
                    printf("Skipping, duplicate...\n");
					argv[j+3] = "\0";
                    c--;
				}	
			}
			
		}

        if( c == 0 )
        {
            cmd_sendresponse(put_error(200,"All tracks where dupes, will not add those again!"), 200);
            cmd_done();
            return -1;

        }

        if( argv[i + 3] ){
            if(strcasecmp("\0",argv[i + 3]))
            {
                tlink = sp_link_create_from_string(argv[i + 3]);

                if(tlink == NULL)
                {
                    printf("%s is not a spotify link, skipping\n", argv[i + 3]);
                    continue;
                }
                if(sp_link_type(tlink) != SP_LINKTYPE_TRACK)
                {
                    printf("%s is not a track link, skipping\n", argv[i + 3]);
                    continue;
                }
                t = sp_link_as_track(tlink);
                sp_track_add_ref(t);

                json_array_append_new(tracks, json_string_nocheck(argv[i + 3]));

                puw->tracks[puw->num_tracks++] = t;
                sp_link_release(tlink);
            }
        }
	}


    if(puw->num_tracks != 0)
    {
        if(!apply_changes(pl, puw))
        {
            // Changes applied directly, we're done
            if(!sp_playlist_is_collaborative(pl))
                sp_playlist_set_collaborative(pl, 1);

            sp_playlist_release(pl);
            sp_link_release(plink);

            if(puw->num_tracks == 0)
                cmd_sendresponse(put_error(400,"Added 0 tracks, probably beacause they are allready in the list!"), 400);
            else
            {
                json_object_set_new(json, "code", json_integer(200));
                json_object_set_new(json, "addcount", json_integer(c));
                cmd_sendresponse(json, 200);
                cmd_done();
                free(puw);
            }
            return 1;
        }
	
        printf("Playlist not yet loaded, waiting...\n");
		sp_playlist_add_callbacks(pl, &pl_update_callbacks, puw);
		sp_link_release(plink);
		return 0;
	}
    // No more tracks to add, we're done
	return 1;
}

