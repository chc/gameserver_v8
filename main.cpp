#include <stdio.h>
#include "NetServer.h"
#include "server/CHCGameServer.h"
#include "server/SAMP/SAMPDriver.h"
INetServer *g_gameserver = NULL;
int main() {
	g_gameserver = new CHCGameServer();
	g_gameserver->addNetworkDriver(new SAMPDriver(g_gameserver, "10.1.1.100",7777));
	g_gameserver->init();
	while(true) {
		g_gameserver->tick();
	}
}
