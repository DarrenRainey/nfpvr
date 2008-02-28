#ifndef _nfpvrsystem_h_
#define _nfpvrsystem_h_

// system specific headers
#if WIN32  // win32
#pragma once
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winsock.h>
#include <tchar.h>

#else
#ifdef _XBOX // xbox
#include "../xbmc/trunk/XBMC/xbmc/stdafx.h"

#else // posix
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>

#endif
#endif

// system specific defines
#if WIN32 || _XBOX

#pragma warning (disable: 4996)

typedef SOCKET   nfpvr_socket_t;
typedef SOCKADDR nfpvr_sockaddr_t;
typedef int      nfpvr_socklen_t;

#define NFPVR_SOCKET_ERROR   SOCKET_ERROR
#define NFPVR_INVALID_SOCKET INVALID_SOCKET

#define snprintf _snprintf
#define vsnprintf _vsnprintf

#else

typedef int             nfpvr_socket_t;
typedef struct sockaddr nfpvr_sockaddr_t;
typedef socklen_t       nfpvr_socklen_t;
#define NFPVR_SOCKET_ERROR   (-1)
#define NFPVR_INVALID_SOCKET (0)

#endif

#endif
