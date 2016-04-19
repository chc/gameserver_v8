#ifndef _CHCGAMESERVER_H
#define _CHCGAMESERVER_H
#include <stdint.h>
#include "NetServer.h"
#include "ScriptInterface.h"
enum CHCGS_Events {
	CHCGS_ClientConnectEvent,
	CHCGS_ClientDisconnectEvent,
	CHCGS_EnterWorld,
	CHCGS_ClientCommand,
	CHCGS_DialogResponse,
	CHCGS_SpawnSelect,
	CHCGS_ChatMessage,
	CHCGS_UIClick,
};
typedef struct {
	int dialog_id;
	char button_id;
	int list_index;
	const char *input;
} DialogEvent;
class SAMPDriver;
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

	IScriptInterface *GetScriptInterface();
	SAMPDriver 		 *getSAMPDriver();
private:
	IScriptInterface *mp_script_interface;
	char m_name[256];
	char m_version[64];
	char m_map[64];
	uint32_t m_max_players;
};
#endif //_CHCGAMESERVER_H