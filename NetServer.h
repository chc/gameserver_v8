#ifndef _INETSERVER_H
#define _INETSERVER_H
#include <vector>
#include "NetDriver.h"
#include "NetEventManager.h"
class INetServer {
public:
	INetServer();
	~INetServer();
	virtual void init() = 0;
	virtual void tick();
	virtual void shutdown() = 0;
	/*
		Currently the driver is aware of what type of NetServer its connected to.
	*/
	void addNetworkDriver(INetDriver *driver);
protected:
	void NetworkTick(); //fires the INetEventMgr
//private:
	INetEventManager *mp_net_event_mgr;
	std::vector<INetDriver *> m_net_drivers;
};
#endif //_IGAMESERVER_H