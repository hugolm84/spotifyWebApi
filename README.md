# spotifyWebApi
This api server will serve the most of the supported api functions that libspotify offers. All responses will be in json format.

Note: This is modified and further developed code based on Spotifys spshell example.

## Create and add tracks to playlists
	To get functions to create and add tracks to playlists, use the addCreate branch.
	
## Supported API methods
    GET /help
    GET /search/{OPTIONAL type}/{query}/{OPTIONAL limit}
    GET /browse/{spotifyid}
    GET /albums/{artistid}
    GET /toplist/{charts} || {artists|albums|tracks / global | region/countryCode | user/userName}
### Add/Create
	GET /searchTopTrack{spType:query} [ Get the top track for a query ]
	GET /create/{playlistName} [ Create a collaborative playlist in server users container ]
	GET /add/{playlistURI}/{Position}/{trackUri} .. <TrackUri> ... [ Adds one or more tracks to specified playlist, if in container or collaborative ]

## How to build

1. Make sure you have the required libraries
  * [jansson](http://www.digip.org/jansson/) > 2.0
  * [libspotify](http://developer.spotify.com) > 11
  

1. Optional deps
  * lmysql ( build with -DBUILD_MYSQL=ON )


1. Update `spshell.h` with your credentials. A *Spotify premium account is necessary*.

1. Copy `appkey.c` into the directory and run `cmake ..`.

1. Build commands
  * mkdir build && cd build
  * cmake .. 
  * make
  If you have mysql installed but wish not to use it, run
  * cmake -DCMAKE_DISABLE_IND_PACKAGE_MySqlClient=TRUE ..

1. run ./spshell -p PORT or just ./spshell to run on default port, 5112
