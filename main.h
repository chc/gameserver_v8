#ifndef _MAIN_H
#define _MAIN_H
	#ifdef _WIN32
	#include <Windows.h>
	#include <WinSock.h>
	typedef int socklen_t;
	#define snprintf sprintf_s
	#define strcasecmp _stricmp
	#define strncasecmp _strnicmp

	int gettimeofday(struct timeval *tp, struct timezone *tzp);
	#else
	#include <unistd.h>
	#include <sys/socket.h>
	#include <sys/types.h>
	#include <arpa/inet.h>
	#include <netinet/in.h>
	#include <signal.h>
	#include <netdb.h>
	#include <pthread.h>
	#include <sys/times.h>

	#endif
	#include <memory.h>
#endif