/* nslookup.c - query Internet name servers
 *
 * Copyright 2013 Ashwini Kumar <ak.ashwini@gmail.com>
 * Copyright 2015 Rajni Kant <rajnikant12345@gmail.com>
 *

USE_NSLOOKUP(NEWTOY(nslookup, "<1?", TOYFLAG_USR|TOYFLAG_BIN))

config NSLOOKUP
  bool "nslookup"
  default y
  help
    usage: nslookup [HOST] [SERVER]

    Query the nameserver for the IP address of the given HOST
    optionally using a specified DNS server.

    Note:- Only non-interactive mode is supported.
*/

#define FOR_nslookup
#include "toys.h"
#include <resolv.h>

static char *address_to_name(struct sockaddr *sock)
{
  //man page of getnameinfo.
  char hbuf[NI_MAXHOST] = {0,}, sbuf[NI_MAXSERV] = {0,};
  int status = 0;
  if (sock->sa_family == AF_INET) {
    socklen_t len = sizeof(struct sockaddr_in);
    if ((status = getnameinfo(sock, len, hbuf, sizeof(hbuf), sbuf, sizeof(sbuf),
            NI_NUMERICHOST | NI_NUMERICSERV)) == 0)
      return xmprintf("%s:%s", hbuf, sbuf);
    else {
      fprintf(stderr, "getnameinfo: %s\n", gai_strerror(status));
      return NULL;
    }
  }
  else if (sock->sa_family == AF_INET6) {
    socklen_t len = sizeof(struct sockaddr_in6);
    if ((status = getnameinfo(sock, len, hbuf, sizeof(hbuf), sbuf,
            sizeof(sbuf), NI_NUMERICHOST | NI_NUMERICSERV)) == 0) {
      //verification for resolved hostname.
      if (strchr(hbuf, ':')) return xmprintf("[%s]:%s", hbuf, sbuf);
      else return xmprintf("%s:%s", hbuf, sbuf);
    }
    else {
      fprintf(stderr, "getnameinfo: %s\n", gai_strerror(status));
      return NULL;
    }
  }
  else if (sock->sa_family == AF_UNIX) {
    struct sockaddr_un *sockun = (void*)sock;
    return xmprintf("local:%.*s", (int) sizeof(sockun->sun_path), sockun->sun_path);
  }
  return NULL;
}

static void print_addrs(char *hostname, char *msg)
{
  struct addrinfo hints, *res = NULL, *cur = NULL;
  int ret_ga;
  char *n = xstrdup(hostname), *p, *tmp;
  tmp = n;
  if ((*n == '[') && (p = strrchr(n, ']')) != NULL ) {
    n++;
    *p = '\0';
  }

  memset(&hints, 0, sizeof(struct addrinfo));
  hints.ai_socktype = SOCK_STREAM;

  ret_ga = getaddrinfo(n, NULL, &hints, &res);                                                                                   
  if (ret_ga) perror_exit("Hostname %s", gai_strerror(ret_ga));

  cur = res;
  while(cur) {
    char *colon = NULL;
    char *name = address_to_name(cur->ai_addr);
    if (name) {
      colon = strrchr(name, ':');
      if (colon) *colon = '\0';
      xprintf("%-8s %s\n",msg, hostname);
      xprintf("Address: %s\n", name);
      free(name);
    }
    cur = cur->ai_next;
  }
  if (!res->ai_addr) error_exit("getaddrinfo failed");

  freeaddrinfo(res);
  free(tmp);
}

static void resolve_addr(char *host, void *addr, char* port)
{
  struct addrinfo *info, hint;
  int ret = atolx(port);
 
  if(ret <0 || ret > 65535 ) error_exit("bad port:  %s", port);
  if (strncmp(host, "local:", 6) == 0) {
    struct sockaddr *sockun = (struct sockaddr *)addr;
    sockun->sa_family = AF_UNIX;
    strncpy(((struct sockaddr_un *)sockun)->sun_path, host + 6,
            sizeof(((struct sockaddr_un *)sockun)->sun_path));
    return ;
  }
  memset(&hint, 0, sizeof(hint));

  ret = getaddrinfo(host, port , &hint, &info);

  if (ret || !info) error_exit("bad address:  %s", host);

  memcpy(addr, info->ai_addr, info->ai_addrlen);
  freeaddrinfo(info);
}

void nslookup_main(void)
{
  struct sockaddr* sock;
  char *args[2] = {0,0}, *colon = NULL, *name = NULL;

  res_init(); //initialize the _res struct, for DNS name.

  for (;*toys.optargs; toys.optargs++) {
    if (**toys.optargs == '-') {
      if (!strncmp(&toys.optargs[0][1], "retry=", 6)) {
        _res.retry = atolx(toys.optargs[0]+7);
      } else if (!strncmp(&toys.optargs[0][1], "timeout=", 8)) {
        _res.retrans = atolx(toys.optargs[0]+9);
      } else error_msg("invalid option '%s'", *toys.optargs);
    } else if (!args[0]) args[0] = *toys.optargs;
    else if (!args[1]) args[1] = *toys.optargs;
    else error_exit("bad arg '%s'",*toys.optargs);
  }

  if ( !*args) {
    toys.exithelp++;
    error_exit("Needs 1 args minimum");
  }
  if (args[1]) { //set the default DNS
    struct sockaddr_storage addr;
    char* port = NULL ;

    if (args[1][0] == '[') {
      int len = strchr(args[1],']') - &args[1][0];
      if (len > 45|| len <= 0 ) error_exit("bad address:  %s", args[1]);
      strncpy(toybuf,&args[1][1], len-1);
      args[1] += len;
    }
    if ((port = strchr(args[1],':'))) {
      *port = '\0';
      port++;
    }
    else port = "53";

    resolve_addr((toybuf[0])?toybuf:args[1], &addr, port );

    if (addr.ss_family == AF_INET) {
      _res.nscount = 1;
      _res.nsaddr_list[0] = *((struct sockaddr_in*)&addr);
    }
    else if (addr.ss_family == AF_INET6) {
      _res._u._ext.nscount = 1;
      _res._u._ext.nsaddrs[0] = ((struct sockaddr_in6*)&addr);
    }
  }

  sock = (struct sockaddr*)_res._u._ext.nsaddrs[0];
  if (!sock) sock = (struct sockaddr*)&_res.nsaddr_list[0];
  if ((name = address_to_name(sock))) {
    colon = strrchr(name, ':');
    if (colon) *colon = '\0';
    print_addrs(name, "Server:");
    free(name);
  }
  puts("");

  print_addrs(args[0], "Name:");
}
