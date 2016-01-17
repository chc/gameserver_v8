#ifndef _SCRIPTINTERFACE_H
#define _SCRIPTINTERFACE_H
class CHCGameServer;
class IScriptInterface {
public:
	IScriptInterface(CHCGameServer *gameserver) { mp_gameserver = gameserver; };
	virtual void run() = 0;
	CHCGameServer *getGameServer() { return mp_gameserver; }
private:
	CHCGameServer *mp_gameserver;
};
#endif //_SCRIPTINTERFACE_H