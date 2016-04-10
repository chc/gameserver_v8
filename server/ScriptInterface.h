#ifndef _SCRIPTINTERFACE_H
#define _SCRIPTINTERFACE_H
class CHCGameServer;
class IScriptInterface {
public:
	IScriptInterface(CHCGameServer *gameserver) { mp_gameserver = gameserver; };
	virtual void run() = 0;
	CHCGameServer *getGameServer() { return mp_gameserver; }
	virtual void HandleEvent(int event_id, void *user, void *extra) = 0;
private:
	CHCGameServer *mp_gameserver;
};
#endif //_SCRIPTINTERFACE_H