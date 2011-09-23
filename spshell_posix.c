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
#include "cmd.h"

/// Socket identifiers
extern int sockfd, new_fd, newfd, numbytes;

struct sockaddr_in server;

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

/**
 * Sighandler
 */
void sigchld_handler(int s){
        while(wait(NULL) > 0);
}

/**
 * replaces / and + with a space,
 * to be used in the tokenizer later.
 * @todo: replace this function with a proper uri parser
 */
char *replace(char *st)
{

    char newstring[BUF_SIZE];
    char *str;
    strncpy(newstring, st, BUF_SIZE);
    newstring[BUF_SIZE - 1] = 0;
    unsigned int i = 0;
    for(i = 0; i < BUF_SIZE; ++i){
        if (newstring[i] == '/')
            newstring[i] = ' ';
        if (newstring[i] == '+')
            newstring[i] = ' ';
    }
    str = (char *) newstring;
    return str;

}


/**
 * socketreciver waits for a new request and
 * executes it.
 */
static void *socketreciver(void *aux)
{
        pthread_mutex_lock(&notify_mutex);

        while(1) {

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
                if ((newfd = accept(sockfd, (struct sockaddr *)&client, &len)) < 0) {
                        perror("accept");
                        exit(EXIT_FAILURE);
                }


                if (recv(newfd, buf, sizeof(buf), 0) < 0) {
                        perror("recv");
                        exit(EXIT_FAILURE);
                }
                sscanf(buf, "%s %s %s", method, url, protocol);

                /// request body
                do {
                        if (strstr(buf, "\r\n\r\n")) {
                                break;
                        }
                        if (strlen(buf) >= sizeof(buf)) {
                                memset(&buf, 0, sizeof(buf));
                        }
                } while (recv(newfd, buf+strlen(buf), sizeof(buf) - strlen(buf), 0) > 0);


            pthread_mutex_lock(&notify_mutex);
            wait_for_cmd = 0;
            request = replace(url);
            pthread_cond_signal(&notify_cond);
        }
        return NULL;
}


/**
 *
 */
void start_recv(void)
{
        static pthread_t id;
        if (id)
                return;
        wait_for_cmd = 1;
        pthread_create(&id, NULL, socketreciver, NULL);
}

/**
 *
 */
int main(int argc, char **argv)
{

    int port = (argc == 2) ? atoi(argv[1]) : PORT;
    printf("Listening on port %d\n", port);
    struct sigaction sa;
    if ((sockfd = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
            perror("socket");
            exit(EXIT_FAILURE);
    }

    /// Setting server ip
    memset(&server, 0, sizeof(server));
    server.sin_family = PF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(port);

    /// Set the socket
    char opt = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, sizeof(opt));

    /// ! IMPORTANT!! no ioctl =  socket blocks on large buffers!
    /// @todo Does it really? Check that
    /*int dontblock;
    int rc;
    int server_sock;
    dontblock = 1;

    rc = ioctl(server_sock, FIONBIO, (char *) &dontblock);*/


    /// Bind the port
    if (bind(sockfd, (struct sockaddr *) &server, sizeof(server)) < 0) {
            perror("bind");
            exit(EXIT_FAILURE);
    }

    /// And also listen
    if (listen(sockfd, SOMAXCONN) < 0) {
            perror("listen");
            exit(EXIT_FAILURE);
    }

    /// clean all the dead processes
    sa.sa_handler = sigchld_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;

    if(sigaction(SIGCHLD, &sa, NULL) == -1){
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

        while(!is_logged_out) {
                /// Release prompt



		if (next_timeout == 0) {
                        while(!notify_events && !request)
				pthread_cond_wait(&notify_cond, &notify_mutex);
		} else {
			struct timespec ts;

#if _POSIX_TIMERS > 0
			clock_gettime(CLOCK_REALTIME, &ts);
#else
			struct timeval tv;
			gettimeofday(&tv, NULL);
			TIMEVAL_TO_TIMESPEC(&tv, &ts);
#endif

			ts.tv_sec += next_timeout / 1000;
			ts.tv_nsec += (next_timeout % 1000) * 1000000;

                        while(!notify_events && !request) {
				if(pthread_cond_timedwait(&notify_cond, &notify_mutex, &ts))
					break;
			}
		}

                /// Process input from request
                if(request) {
                        char *l = request;
                        request = NULL;
		
			pthread_mutex_unlock(&notify_mutex);
                        cmd_exec_unparsed(l);
                        pthread_mutex_lock(&notify_mutex);
		}

                /// Process libspotify events
		notify_events = 0;
		pthread_mutex_unlock(&notify_mutex);

		do {
			sp_session_process_events(g_session, &next_timeout);
		} while (next_timeout == 0);

		pthread_mutex_lock(&notify_mutex);
	}
	printf("Logged out\n");
	sp_session_release(g_session);
	printf("Exiting...\n");
	return 0;
}


void cmd_sendresponse(json_t *json, int code){


    char *response = json_dumps(json, JSON_COMPACT);
    json_decref(json);

    /// Not that nice way of sending response header
    /// @todo fix a better way
    char *header;
    switch(code){
        case 200: header = STATUS_OK;
        break;
        case 400: header = STATUS_ERR;
        break;
        case 503: header = STATUS_CONNERR;
        break;
        default: header = STATUS_NIMPL;
    }
        /// this is the child process
        if(!fork()){

                /// child doesn’t need the listener
                close(sockfd);
                send(newfd, header, strlen(header), 0);
                char *jsonheader = JSON;
                 send(newfd,jsonheader, strlen(jsonheader), 0);
                int bytes_sent;
                bytes_sent = sendto(newfd, response, strlen(response), 0,(struct sockaddr*)&server, sizeof server);
                if (bytes_sent < 0) {
                   printf("Error sending packet: %s\n", strerror(errno));

               };

                close(newfd);
                exit(0);
        }else
             close(newfd);

}

/**
 *
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
