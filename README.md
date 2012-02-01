# spotifyWebApi
This api server will serve the most of the supported api functions that libspotify offers. All responses will be in json format.
To get functions to create and add tracks to playlists, use the addCreate branch.
Note: This is modified and further developed code based on Spotifys spshell example.

## Supported API methods
    GET /help
    GET /search/{query}
    GET /browse/{spotifyid}
    GET /albums/{artistid}
    GET /toplist/{charts} || {artists|albums|tracks / global | region/countryCode | user/userName}

## How to build

1. Make sure you have the required libraries
  * [jansson](http://www.digip.org/jansson/) > 2.0
  * [libspotify](http://developer.spotify.com) > 10

1. Optional deps
  * lmysql

1. Update `spshell.h` with your credentials. A *Spotify premium account is necessary*.

1. Copy `appkey.c` into the directory and run `cmake ..`.

1. Build commands
  * mkdir build && cd build
  * cmake .. 
  * make
  If you have mysql installed but wish not to use it, run
  * cmake -DCMAKE_DISABLE_IND_PACKAGE_MySqlClient=TRUE ..

1. run ./spshell -p PORT or just ./spshell to run on default port, 5112
