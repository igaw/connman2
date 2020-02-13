// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 *  Copyright (C) 2020  Daniel Wagner <dwagner@suse.de>
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <signal.h>
#include <ell/ell.h>

static struct l_timeout *timeout;
static bool terminating;

static void main_loop_quit(struct l_timeout *timeout, void *user_data)
{
	l_main_quit();
}

static void connman_shutdown(void)
{
	if (terminating)
		return;

	terminating = true;

	timeout = l_timeout_create(1, main_loop_quit, NULL, NULL);
}

static void signal_handler(uint32_t signo, void *user_data)
{
	switch (signo) {
	case SIGINT:
	case SIGTERM:
		l_info("Terminate");
		connman_shutdown();
		break;
	}
}

int main(int argc, char *argv[])
{
	int exit_status;

	l_log_set_stderr();

	if (!l_main_init())
		return EXIT_FAILURE;

	l_info("ConnMan %s", VERSION);

	l_debug_enable("*");

	exit_status = l_main_run_with_signal(signal_handler, NULL);

	l_timeout_remove(timeout);

	l_main_exit();

	return exit_status;
}
