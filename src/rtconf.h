// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 *  Copyright (C) 2020  Daniel Wagner <dwagner@suse.de>
 */

#ifndef __CONNMAN_RTCONF_H
#define __CONNMAN_RTCONF_H

struct rtconf;

struct rtconf *rtconf_create(void);
void rtconf_destroy(struct rtconf *rtconf);

#endif /* __CONNMAN_RTCONF_H */
