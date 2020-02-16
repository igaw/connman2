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

struct rtconf {
	struct l_queue *link_list;
	struct l_queue *route_list;
	struct l_queue *ifaddr_list;
};

struct rtconf_route {
	uint8_t family;
	uint8_t table;
	uint32_t index;
	char *dst;
	char *gateway;
	char *src;
};

struct rtconf_ifaddr {
	uint8_t family;
	uint8_t prefix_len;
	uint32_t index;
	char *ip;
	char *broadcast;
};

static struct l_netlink *rtnl;

static void rtconf_route_destroy(void *data)
{
	struct rtconf_route *route = data;

	if (route->dst)
		l_free(route->dst);
	if (route->gateway)
		l_free(route->gateway);
	if (route->src)
		l_free(route->src);

	l_free(route);
}

static void rtconf_ifaddr_destroy(void *data)
{
	struct rtconf_ifaddr *ifaddr = data;

	l_free(ifaddr->ip);
	if (ifaddr->broadcast)
		l_free(ifaddr->broadcast);

	l_free(ifaddr);
}

static bool rtconf_route_match(const void *a, const void *b)
{
	const struct rtconf_route *entry = a;
	const struct rtconf_route *query = b;

	if (entry->family != query->family)
		return false;

	if (entry->table != query->table)
		return false;

	if (entry->index != query->index)
		return false;

	if (entry->dst && query->dst &&
			strcmp(entry->dst, query->dst))
		return false;

	if (entry->gateway && query->gateway &&
			strcmp(entry->gateway, query->gateway))
		return false;

	if (entry->src && query->src &&
			strcmp(entry->src, query->src))
		return false;

	return true;
}

static bool rtconf_ifaddr_match(const void *a, const void *b)
{
	const struct rtconf_ifaddr *entry = a;
	const struct rtconf_ifaddr *query = b;

	if (entry->family != query->family)
		return false;

	if (entry->prefix_len != query->prefix_len)
		return false;

	if (entry->index != query->index)
		return false;

	if (strcmp(entry->ip, query->ip))
		return false;

	if (entry->broadcast && query->broadcast &&
			strcmp(entry->broadcast, query->broadcast))
		return false;

	return true;
}

static void route_add(struct rtconf *rtconf, const struct rtmsg *rtmsg,
			uint32_t table, uint32_t index,
			char *dst, char *gateway, char *src)
{
	struct rtconf_route *route;

	l_info("ADD ROUTE: index %d table %d dst %s gateway %s src %s",
				index, table, dst, gateway, src);

	route = l_new(struct rtconf_route, 1);
	route->family = rtmsg->rtm_family;
	route->table = rtmsg->rtm_table;
	route->index = index;
	route->dst = dst;
	route->gateway = gateway;
	route->src = src;

	l_queue_push_tail(rtconf->route_list, route);
}

static void route_remove(struct rtconf *rtconf, const struct rtmsg *rtmsg,
			uint32_t table, uint32_t index,
			char *dst, char *gateway, char *src)
{
	struct rtconf_route *route;
	struct rtconf_route query;

	l_info("REM ROUTE: index %d table %d dst %s gateway %s src %s",
				index, table, dst, gateway, src);

	query.family = rtmsg->rtm_family;
	query.table = rtmsg->rtm_table;
	query.index = index;
	query.dst = dst;
	query.gateway = gateway;
	query.src = src;

	route = l_queue_remove_if(rtconf->route_list,
				rtconf_route_match, &query);

	if (route)
		rtconf_route_destroy(route);

	l_free(dst);
	l_free(gateway);
	l_free(src);
}

static void ifaddr_add(struct rtconf *rtconf, const struct ifaddrmsg *ifa,
			char *ip, char *broadcast)
{
	struct rtconf_ifaddr *ifaddr;

	l_info("ADD IPv%d:  index %d ip %s broadcast %s",
		ifa->ifa_family == AF_INET? 4: 6, ifa->ifa_index,
		ip, broadcast);

	ifaddr = l_new(struct rtconf_ifaddr, 1);
	ifaddr->family = ifa->ifa_family;
	ifaddr->prefix_len = ifa->ifa_prefixlen;
	ifaddr->index = ifa->ifa_index;
	ifaddr->ip = ip;
	ifaddr->broadcast = broadcast;

	l_queue_push_tail(rtconf->ifaddr_list, ifaddr);
}

static void ifaddr_remove(struct rtconf *rtconf, const struct ifaddrmsg *ifa,
				char *ip, char *broadcast)
{
	struct rtconf_ifaddr *ifaddr;
	struct rtconf_ifaddr query;

	l_info("REM IPv%d:  index %d ip %s broadcast %s",
		ifa->ifa_family == AF_INET? 4: 6, ifa->ifa_index,
		ip, broadcast);

	query.family = ifa->ifa_family;
	query.prefix_len = ifa->ifa_prefixlen;
	query.index = ifa->ifa_index;
	query.ip = ip;
	query.broadcast = broadcast;

	ifaddr = l_queue_remove_if(rtconf->ifaddr_list,
				rtconf_ifaddr_match, &query);

	if (ifaddr)
		rtconf_ifaddr_destroy(ifaddr);

	l_free(ip);
	l_free(broadcast);
}

static void route_notify(uint16_t type, void const *data,
					uint32_t len, void *user_data)
{
	struct rtconf *rtconf = user_data;
	const struct rtmsg *rtmsg = data;
	char *dst = NULL, *gateway = NULL, *src = NULL;
	uint32_t table, index;

	if (rtmsg->rtm_family == AF_INET)
		l_rtnl_route4_extract(rtmsg, len, &table, &index,
					&dst, &gateway, &src);
	else
		l_rtnl_route6_extract(rtmsg, len, &table, &index,
					&dst, &gateway, &src);

	if (type == RTM_NEWROUTE)
		route_add(rtconf, rtmsg, table, index, dst, gateway, src);
	else if (type == RTM_DELROUTE)
		route_remove(rtconf, rtmsg, table, index, dst, gateway, src);
}

static void route_dump_cb(int error,
				uint16_t type, const void *data,
				uint32_t len, void *user_data)
{
	if (error)
		return;

	route_notify(type, data, len, user_data);
}

static void ifaddr_notify(uint16_t type, void const *data,
					uint32_t len, void *user_data)
{
	struct rtconf *rtconf = user_data;
	const struct ifaddrmsg *ifa = data;
	char *ip = NULL, *broadcast = NULL;

	if (ifa->ifa_family == AF_INET)
		l_rtnl_ifaddr4_extract(ifa, len, NULL, &ip, &broadcast);
	else
		l_rtnl_ifaddr6_extract(ifa, len, &ip);

	if (type == RTM_NEWADDR)
		ifaddr_add(rtconf, ifa, ip, broadcast);
	else if (type == RTM_DELADDR)
		ifaddr_remove(rtconf, ifa, ip, broadcast);
}

static void ifaddr_dump_cb(int error,
			uint16_t type, const void *data,
			uint32_t len, void *user_data)
{
	if (error)
		return;

	ifaddr_notify(type, data, len, user_data);
}

struct rtconf *rtconf_create(void)
{
	struct rtconf *rtconf;
	uint32_t r;

	rtconf = l_new(struct rtconf, 1);

	rtconf->route_list = l_queue_new();
	rtconf->ifaddr_list = l_queue_new();

	rtnl = l_netlink_new(NETLINK_ROUTE);
	if (!rtnl)
		goto err;

	r = l_netlink_register(rtnl, RTNLGRP_IPV4_ROUTE,
				route_notify, rtconf, NULL);
	if (!r) {
		l_error("failed to register to RTNL IPv4 "
				"route notification");
		goto err;
	}
	l_rtnl_route4_dump(rtnl, route_dump_cb, rtconf, NULL);

	r = l_netlink_register(rtnl, RTNLGRP_IPV6_ROUTE,
				route_notify, rtconf, NULL);
	if (!r) {
		l_error("failed to register to RTNL IPv6 "
				"route notification");
		goto err;
	}
	l_rtnl_route6_dump(rtnl, route_dump_cb, rtconf, NULL);

	r = l_netlink_register(rtnl, RTNLGRP_IPV4_IFADDR,
				ifaddr_notify, rtconf, NULL);
	if (!r) {
		l_error("failed to register to RTNL IPv4 "
				"address notification");
		goto err;
	}
	l_rtnl_ifaddr4_dump(rtnl, ifaddr_dump_cb, rtconf, NULL);

	r = l_netlink_register(rtnl, RTNLGRP_IPV6_IFADDR,
				ifaddr_notify, rtconf, NULL);
	if (!r) {
		l_error("failed to register to RTNL IPv6 "
				"address notification");
		goto err;
	}
	l_rtnl_ifaddr6_dump(rtnl, ifaddr_dump_cb, rtconf, NULL);

	return rtconf;

err:
	rtconf_destroy(rtconf);
	return NULL;
}

void rtconf_destroy(struct rtconf *rtconf)
{
	l_netlink_destroy(rtnl);

	l_queue_destroy(rtconf->route_list, rtconf_route_destroy);
	l_queue_destroy(rtconf->ifaddr_list, rtconf_ifaddr_destroy);

	l_free(rtconf);
}
