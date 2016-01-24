#ifndef _NETDRIVER_H
#define _NETDRIVER_H
class INetServer;
class INetDriver {
public:
	INetDriver(INetServer *server);
	~INetDriver();
	/*
		Check for incoming data, etc
	*/
	virtual void tick() = 0;
	virtual void think() = 0;
	virtual int getListenerSocket() = 0;
protected:
	INetServer *m_server;
};
#endif //_NETDRIVER_H