# spotifyWebApi
Modified and further developed code based on Spotifys spshell example.
I would love to get any thoughts on makeing this more threaded, but consider that libspotify only supports one session per process...

## Supported API methods

### Playlists

    GET /playlist/{id}
    GET /search/{query}
    GET /browse/{id}

## How to build

1. Make sure you have the required libraries
  * [jansson](http://www.digip.org/jansson/) > 2.0

1. Update `spshell.h` with your credentials. A *Spotify premium account is necessary*.

1. Copy `appkey.c` into the directory and run `make`.
