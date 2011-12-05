# spotifyWebApi
Modified and further developed code based on Spotifys spshell example.
I would love to get any thoughts on makeing this more threaded, but consider that libspotify only supports one session per process...

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

1. Copy `appkey.c` into the directory and run `make`.

1. run ./spshell -p PORT or just ./spshell to run on default port, 5112
