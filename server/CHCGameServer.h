#ifndef _CHCGAMESERVER_H
#define _CHCGAMESERVER_H
#include <stdint.h>
#include "NetServer.h"
#include "ScriptInterface.h"
class CHCGameServer : public INetServer {
public:
	CHCGameServer();
	void init();
	void tick();
	void shutdown();

	void setName(const char *name);
	const char *getName();

	void setVersion(const char *version);
	const char *getVersion();

	void setMap(const char *map);
	const char *getMap();

	void setMaxPlayers(uint32_t max);
	uint32_t getMaxPlayers();
private:
	IScriptInterface *mp_script_interface;
	char m_name[256];
	char m_version[64];
	char m_map[64];
	uint32_t m_max_players;
};
#endif //_CHCGAMESERVER_H