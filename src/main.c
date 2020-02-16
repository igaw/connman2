// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 *  Copyright (C) 2020  Daniel Wagner <dwagner@suse.de>
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <asm/types.h>
#include <sys/socket.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>
#include <net/if.h>

#include <ell/ell.h>

#include "rtconf.h"

static void signal_handler(uint32_t signo, void *user_data)
{
	switch (signo) {
	case SIGINT:
	case SIGTERM:
		l_info("Terminate");
		l_main_quit();
		break;
	}
}

int main(int argc, char *argv[])
{
	struct rtconf *rtconf;
	int exit_status = EXIT_SUCCESS;

	l_log_set_stderr();

	if (!l_main_init())
		return EXIT_FAILURE;

	l_info("ConnMan %s", VERSION);

	l_debug_enable("*");

	rtconf = rtconf_create();
	if (!rtconf) {
		exit_status = EXIT_FAILURE;
		goto done;
	}

	exit_status = l_main_run_with_signal(signal_handler, NULL);

	rtconf_destroy(rtconf);

done:
	l_main_exit();

	return exit_status;
}
