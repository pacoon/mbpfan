/**
 *  Copyright (C) (2012-present) Daniel Graziotin <daniel@ineed.coffee>
 *  Modifications (2017-present) by Robert Musial <rmusial@fastmail.com>
 *  Modifications (2020-present) by Rene Saavedra <pacoon@protonmail.com>
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 */

/**
 * Code formatted with astyle -A3 -s --break-blocks=all --add-brackets *.c *.h
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <syslog.h>
#include <stdbool.h>
#include <sys/types.h>
#include <dirent.h>
#include <errno.h>
#include "mbpfan.h"
#include "daemon.h"
#include "global.h"
#include "main.h"
#include "minunit.h"
#include "util.h"

int daemonize = 1;
int verbose = 0;

const char *PROGRAM_NAME = "mbpfan";
const char *PROGRAM_VERSION = "2.2.1";
const char *PROGRAM_PID = "/var/run/mbpfan.pid";

const char *CORETEMP_PATH = "/sys/devices/platform/coretemp.0";
const char *APPLESMC_PATH = "/sys/devices/platform/applesmc.768";

void print_usage(int argc, char *argv[])
{
    if (argc >=1) {
        printf("Usage: %s OPTION(S) \n", argv[0]);
        printf("Options:\n");
        printf("\t-h Show this help screen\n");
        printf("\t-f Run in foreground\n");
        printf("\t-t Run the tests\n");
        printf("\t-v Be (a lot) verbose\n");
        printf("\n");
    }
}

void check_requirements()
{

    /**
     * Check for root
     */

    uid_t uid=getuid(), euid=geteuid();

    if (uid != 0 || euid != 0) {
        mbp_log(LOG_ERR, "%s needs root privileges. Please run %s as root. Exiting.", PROGRAM_NAME, PROGRAM_NAME);
        exit(EXIT_FAILURE);
    }

    /**
      * Check for coretemp and applesmc modules
      */
    DIR* dir = opendir(CORETEMP_PATH);

    if (ENOENT == errno) {
        mbp_log(LOG_ERR, "%s needs coretemp support. Please either load it or build it into the kernel. Exiting.", PROGRAM_NAME);
        exit(EXIT_FAILURE);
    }

    closedir(dir);


    dir = opendir(APPLESMC_PATH);

    if (ENOENT == errno) {
        mbp_log(LOG_ERR, "%s needs applesmc support. Please either load it or build it into the kernel. Exiting.", PROGRAM_NAME);
        exit(EXIT_FAILURE);
    }

    closedir(dir);


}

int main(int argc, char *argv[])
{

    int c;

    while( (c = getopt(argc, argv, "hftv|help")) != -1) {
        switch(c) {
        case 'h':
            print_usage(argc, argv);
            exit(EXIT_SUCCESS);
            break;

        case 'f':
            daemonize = 0;
            break;

        case 't':
            return tests();

        case 'v':
            verbose = 1;
            break;

        default:
            print_usage(argc, argv);
            exit(EXIT_SUCCESS);
            break;
        }
    }

    check_requirements();

    // pointer to mbpfan() function in mbpfan.c
    void (*fan_control)() = mbpfan;
    go_daemon(fan_control);
    exit(EXIT_SUCCESS);
}
