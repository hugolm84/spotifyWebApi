# spotifyWebApi
Modified and further developed code based on Spotifys spshell example.
I would love to get any thoughts on makeing this more threaded, but consider that libspotify only supports one session per process...

## Supported API methods
    GET /help [Shows Api methods and examples ]
    GET /search/{query} [ Search for a query, allows artist: track: album: single or togheter ]
    GET /browse/{spotifyid} [ Browse tracks for a spotifyuri: playlist, track, artist, album ]
    GET /albums/{artistid} [ Get albums from artisturi ]
    GET /toplist/{charts} || {artists|albums|tracks / global | region/countryCode | user/userName} [ Get charts capablities and more ] 
    GET /pong [ Check if server is online ]
    GET /searchTopTrack{spType:query} [ Get the top track for a query ]
    GET /create/{playlistName} [ Create a collaborative playlist in server users container ]
    GET /add/{playlistURI}/{Position}/{trackUri} .. <TrackUri> ... [ Adds one or more tracks to specified playlist, if in container or collaborative ]

## How to build

1. Make sure you have the required libraries
  * [jansson](http://www.digip.org/jansson/) > 2.0
  * [libspotify](http://developer.spotify.com) > 10

1. Optional deps
  * lmysql

1. Update `spshell.h` with your credentials. A *Spotify premium account is necessary*.

1. Copy `appkey.c` into the directory and run `make`.

1. run ./spshell -p PORT or just ./spshell to run on default port, 5112
