#include "CHCGameServer.h"
#include "ScriptInterface.h"
#include "PythonInterface.h"
CHCGameServer::CHCGameServer() {
	mp_script_interface = NULL;
}
void CHCGameServer::init() {
	mp_script_interface = new PythonScriptInterface(this);
}
void CHCGameServer::tick() {
	NetworkTick();
}
void CHCGameServer::shutdown() {

}
void CHCGameServer::setName(const char *name) {
	strcpy(m_name, name);
}
const char *CHCGameServer::getName() {
	return m_name;
}

void CHCGameServer::setMap(const char *map) {
	strcpy(m_map, map);
}
const char *CHCGameServer::getMap() {
	return m_map;
}

void CHCGameServer::setVersion(const char *version) {
	strcpy(m_version, version);
}
const char *CHCGameServer::getVersion() {
	return m_version;
}

void CHCGameServer::setMaxPlayers(uint32_t max) {
	m_max_players = max;
}
uint32_t CHCGameServer::getMaxPlayers() {
	return m_max_players;
}
IScriptInterface *CHCGameServer::GetScriptInterface() {
	return mp_script_interface;
}

#include "SAMP/SAMPDriver.h"
SAMPDriver *CHCGameServer::getSAMPDriver() {
	std::vector<INetDriver *>::iterator it = m_net_drivers.begin();
	while(it != m_net_drivers.end()) {
		SAMPDriver *driver = dynamic_cast<SAMPDriver *>(*it);
		if(driver) {
			return driver;
		}
		it++;
	}
	return NULL;
}