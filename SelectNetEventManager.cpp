#include "SelectNetEventManager.h"
#include <stdio.h>
SelectNetEventManager::SelectNetEventManager() {

}
SelectNetEventManager::~SelectNetEventManager() {

}
void SelectNetEventManager::run() {
	
	struct timeval timeout;

    memset(&timeout,0,sizeof(struct timeval));
    
    //timeout.tv_sec = 1;
    timeout.tv_usec = 16000;

    int hsock = setup_fdset();
    if(select(hsock, &m_fdset, NULL, NULL, &timeout) < 0)
    	return;

	std::vector<INetDriver *>::iterator it = m_net_drivers.begin();
	while(it != m_net_drivers.end()) {
		INetDriver *driver = *it;
		if(FD_ISSET(driver->getListenerSocket(), &m_fdset)) {
			driver->tick();
		} else {
			driver->think();
		}
		it++;
	}
}
int SelectNetEventManager::setup_fdset() {
	FD_ZERO(&m_fdset);
	int hsock = -1;
	std::vector<INetDriver *>::iterator it = m_net_drivers.begin();
	while(it != m_net_drivers.end()) {
		INetDriver *driver = *it;
		int sd = driver->getListenerSocket();
		if(sd > hsock)
			hsock = sd;
		FD_SET(sd, &m_fdset);
		it++;
	}
	return ++hsock;
}