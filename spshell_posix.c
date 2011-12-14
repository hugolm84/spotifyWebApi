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
#include <unistd.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <signal.h>
#include <wait.h>
#include "spshell.h"
#include <locale.h>
#include "cmd.h"
#ifdef USE_MYSQL
    #include "mysql.h"
#endif
#include <ctype.h>


/// Set when libspotify want to process events
static int notify_events;

/// Synchronization mutex to protect various shared data
static pthread_mutex_t notify_mutex;

/// Synchronization condition variable for the main thread
static pthread_cond_t notify_cond;

/// Synchronization condition variable to disable prompt temporarily
static pthread_cond_t prompt_cond;

/// Command line to execute
static char *request;
static int wait_for_cmd;
extern int is_logged_out;

/// MYSQL related
#ifdef USE_MYSQL
    extern MYSQL *mysql_connect();
    extern int _mysql_close(MYSQL *);
#endif

/**
 * Sighandler
 */

void sigcleaner(int s)
{
    while(wait(NULL) > 0);
}

void sigchld_handler(int s)
{
    sp_session_logout(g_session);
    sp_session_release(g_session);
#ifdef USE_MYSQL
    _mysql_close(g_conn);
#endif
    printf("Exiting...\n,");
    exit(s);
}

/**
 * replaces find with replacement,
 * to be used in the tokenizer later.
 * @todo: replace this function with a proper uri parser
 */

char * replace( char const * const original, char const * const pattern, char const * const replacement)
{

  size_t const replen = strlen(replacement);
  size_t const patlen = strlen(pattern);
  size_t const orilen = strlen(original);

  size_t patcnt = 0;
  const char * oriptr;
  const char * patloc;

  // find how many times the pattern occurs in the original string
  for ( oriptr = original; ( patloc = strstr(oriptr, pattern) ); oriptr = patloc + patlen )
  {
    patcnt++;
  }

  {
    // allocate memory for the new string
    size_t const retlen = orilen + patcnt * (replen - patlen);
    char * const returned = (char *) malloc( sizeof(char) * (retlen + 1) );

    if (returned != NULL)
    {
      // copy the original string,
      // replacing all the instances of the pattern
      char * retptr = returned;
      for (oriptr = original; ( patloc =  strstr(oriptr, pattern) ); oriptr = patloc + patlen )
      {
        size_t const skplen = patloc - oriptr;
        // copy the section until the occurence of the pattern
        strncpy(retptr, oriptr, skplen);
        retptr += skplen;
        // copy the replacement
        strncpy(retptr, replacement, replen);
        retptr += replen;
      }
      // copy the rest of the string.
      strcpy(retptr, oriptr);
    }
    return returned;
  }
}

/* Converts a hex character to its integer value */
char from_hex(char ch) {
  return isdigit(ch) ? ch - '0' : tolower(ch) - 'a' + 10;
}

/* Returns a url-decoded version of str */
/* IMPORTANT: be sure to free() the returned string after use */
char *url_decode(char *str)
{
    char *pstr = str, *buf = malloc(strlen(str) + 1), *pbuf = buf;
    while (*pstr) {
        if (*pstr == '%')
        {
            if (pstr[1] && pstr[2])
            {
                *pbuf++ = from_hex(pstr[1]) << 4 | from_hex(pstr[2]);
                pstr += 2;
            }
        }
        else if (*pstr == '+')
        {
            *pbuf++ = ' ';
        }
        else
        {
            *pbuf++ = *pstr;
        }
        pstr++;
    }
    *pbuf = '\0';
 return buf;
}


/**
 * socketreciver waits for a new request and
 * executes it.
 */
static void *socketreciver(void *sock)
{
        pthread_mutex_lock(&notify_mutex);
        int *sockfd = (int*)&sock;

        while(1)
        {

            while(wait_for_cmd == 0)
                    pthread_cond_wait(&prompt_cond, &notify_mutex);


            pthread_mutex_unlock(&notify_mutex);

            struct sockaddr_in client;
            char buf[BUF_SIZE] = "";
            char method[BUF_SIZE] = "";
            char url[BUF_SIZE] = "";
            char protocol[BUF_SIZE] = "";


            socklen_t len;
            memset(&client, 0, sizeof(client));
            len = sizeof client;

            if ((newfd = accept(*sockfd, (struct sockaddr *)&client, &len)) < 0)
            {
                    perror("accept");
                    exit(EXIT_FAILURE);
            }


            if (recv(newfd, buf, sizeof(buf), 0) < 0)
            {
                    perror("recv");
                    exit(EXIT_FAILURE);
            }

            if( buf != NULL )
            {
                if ( sscanf ( buf, "%s %s %s", method, url, protocol ) != 1 )
                {

                    if( ( strcmp( method, "GET" ) == 0 ) && ( strcmp( url, "/favicon.ico" ) != 0 ) )
                    {
                        //printf("%s %s %s\n",protocol, method, url);
#ifdef USE_MYSQL
                        _mysql_updateStats(g_conn, "globalHit");
#endif
                        pthread_mutex_lock(&notify_mutex);
                        wait_for_cmd = 0;
                        request = replace(url_decode( url ), "/", " ");
                        //printf("request: %s\n",request);
                        pthread_cond_signal(&notify_cond);
                    }
                }
            }

        }
        return NULL;
}

/**
 *
 */
long startListner()
{

    int port;
    if(!DPORT)
        port = PORT;
    else
        port = DPORT;

    long socketfd;

    /// clean all the dead processes
    struct sigaction sa;
    sa.sa_handler = sigcleaner;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;

    if(sigaction(SIGCHLD, &sa, NULL) == -1){
        perror("Server-sigaction() error");
        exit(1);
    }

    printf("Listening on port %d\n", port);

    if ((socketfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
            perror("socket");
            exit(EXIT_FAILURE);
    }

    /// Set the socket and reuse
    int opt = 1;
    if (setsockopt(socketfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
    {
        perror("setsockopt failed");
        exit(EXIT_FAILURE);
    }

    /// Setting server ip
    struct sockaddr_in server;
    memset(&server, 0, sizeof(server));
    server.sin_family = AF_INET; 
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(port);

    /// Bind the port
    if (bind(socketfd, (struct sockaddr *) &server, sizeof(server)) < 0)
    {
            perror("bind");
            exit(EXIT_FAILURE);
    }

    /// And also listen
    if (listen(socketfd, SOMAXCONN) < 0)
    {
            perror("listen");
            exit(EXIT_FAILURE);
    }


    return socketfd;

}

/**
 *
 */
void start_recv(void)
{
        static pthread_t id;
        long socketfd;
        if (id)
                return;

        /// Start the listner
        socketfd = startListner();

        wait_for_cmd = 1;
        pthread_create(&id, NULL, socketreciver, (void*)socketfd);
}



/**
 *
 */
int main(int argc, char **argv)
{


    /// @todo, Implement Verbose and Debug options
    /// Currently supports setting custom port, -p PORT
    opterr = 0;
    int c;

    while ((c = getopt (argc, argv, "p:")) != -1)
    {
        switch (c)
        {
            case 'p':
                DPORT = atoi(optarg);
                break;
            case '?':
                if (optopt == 'p')
                    fprintf (stderr, "Option -%c requires a port as argument.\n", optopt);
                else if (isprint (optopt))
                    fprintf (stderr, "Unknown option `-%c'.\n", optopt);
                else
                    fprintf (stderr, "Unknown option character `\\x%x'.\n", optopt);
                return 1;
            default:
                abort ();
        }
    }

    /// clean all the dead processes
    struct sigaction sigIntHandler;

    sigIntHandler.sa_handler = sigchld_handler;
    sigemptyset(&sigIntHandler.sa_mask);
    sigIntHandler.sa_flags = 0;

    if(sigaction(SIGINT, &sigIntHandler, NULL) == -1)
    {
            perror("Server-sigaction() error");
            exit(1);
    }

    int r;
    int next_timeout = 0;

    pthread_mutex_init(&notify_mutex, NULL);
    pthread_cond_init(&notify_cond, NULL);
    pthread_cond_init(&prompt_cond, NULL);

    if ((r = spshell_init(USERNAME, PASSWORD)) != 0)
        exit(r);

    pthread_mutex_lock(&notify_mutex);

    /// Mysql connection initatition
#ifdef USE_MYSQL
    g_conn = _mysql_connect();

    if(!g_conn)
    {
        printf("Failed to set mysql connection!\n");
        exit(1);
    }
#endif
    while(!is_logged_out)
    {
            /// Release prompt

            if (next_timeout == 0)
            {
                    while(!notify_events && !request)
                        pthread_cond_wait(&notify_cond, &notify_mutex);
            }
            else
            {
                struct timespec ts;
                clock_gettime(CLOCK_REALTIME, &ts);

                ts.tv_sec += next_timeout / 1000;
                ts.tv_nsec += (next_timeout % 1000) * 1000000;

                while(!notify_events && !request)
                {
                        if(pthread_cond_timedwait(&notify_cond, &notify_mutex, &ts))
                            break;
                }
            }

            /// Process input from request
            if(request)
            {
                    char *l= request;
                    request = NULL;
                    pthread_mutex_unlock(&notify_mutex);
                    cmd_exec_unparsed(l);
                    pthread_mutex_lock(&notify_mutex);
                    free(l);
            }

            /// Process libspotify events
            notify_events = 0;
            pthread_mutex_unlock(&notify_mutex);

            do
            {
                sp_session_process_events(g_session, &next_timeout);

            } while (next_timeout == 0);

            pthread_mutex_lock(&notify_mutex);
    }
    printf("Logged out\n");
    sp_session_release(g_session);
#ifdef USE_MYSQL
    if(g_conn)
        _mysql_close(g_conn);
#endif
    printf("Exiting...\n");
    return 0;
}

/**
 * setHeader(int code)
 * set the header to respective err code
 */
char *setHeader(int code)
{

    char *header;
    switch(code)
    {
        case 200: header = STATUS_OK;
        break;
        case 400: header = STATUS_ERR;
        break;
        case 404: header = STATUS_CONTERR;
        break;
        case 503: header = STATUS_CONNERR;
        break;
        default: header = STATUS_NIMPL;
    }
    return header;

}

/**
 * cmd_sendresponse(json_t *resp, int code);
 * we need to send the response back to the httpd
 *
 */

void cmd_sendresponse(json_t *json, int code)
{

    char *response = json_dumps(json, JSON_COMPACT);
    json_decref(json);

    /// this is the child process
    if(!fork())
    {

        int rc;
        // Send HTTP Status header
        char *header = setHeader(code);
        send(newfd, header, strlen(header), 0);

        // Send JSON Header
        char *jsonheader = JSON;
        send(newfd,jsonheader, strlen(jsonheader), 0);

        // Send the Response
        do
        {
            if(response != NULL)
            {
                rc = send(newfd, response, strlen(response),0);
                    if (rc == -1)
                    {
                        printf("Fail to send to server %s\n", strerror(errno));
                        break;
                    }
            }

        } while (rc <= 0);

        close(newfd);
        exit(0);
    }
    else
    {
        close(newfd);
    }
}

/**
 * Cmd done
 * notifys that the commands is finnished, back to wait_for_cmd
 */
void cmd_done(void)
{

        pthread_mutex_lock(&notify_mutex);
        wait_for_cmd = 1;
        pthread_cond_signal(&prompt_cond);
        pthread_mutex_unlock(&notify_mutex);
}


/**
 *
 */
void notify_main_thread(sp_session *session)
{
        pthread_mutex_lock(&notify_mutex);
        notify_events = 1;
        pthread_cond_signal(&notify_cond);
        pthread_mutex_unlock(&notify_mutex);
}
