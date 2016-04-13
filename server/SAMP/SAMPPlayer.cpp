#include "SAMPPlayer.h"
#include <stdio.h>
SAMPPlayer::SAMPPlayer(SAMPDriver *driver) {
	mp_driver = driver;
	m_samp_peer = NULL;
	m_spawned = false;
}
SAMPPlayer::SAMPPlayer(SAMPRakPeer *peer, SAMPDriver *driver) {
	m_samp_peer = peer;
	mp_driver = driver;
	m_spawned = false;
}
SAMPPlayer::~SAMPPlayer() {

}

void SAMPPlayer::SetHealth(float h) {
	RakNet::BitStream bsData;
	bsData.Write(h);
	mp_driver->SendRPCToStreamed(this, ESAMPRPC_SetPlayerHealth, &bsData);

	m_health = h;
}
void SAMPPlayer::SetArmour(float a) {
	RakNet::BitStream bsData;
	bsData.Write(a);
	mp_driver->SendRPCToStreamed(this, ESAMPRPC_SetPlayerArmour, &bsData);

	m_armour = a;
}
void SAMPPlayer::SetPosition(float *p) {
	RakNet::BitStream bsData;
	bsData.Write(p[0]);
	bsData.Write(p[1]);
	bsData.Write(p[2]);

	m_pos[0] = p[0];
	m_pos[1] = p[1];
	m_pos[2] = p[2];
	mp_driver->SendRPCToStreamed(this, ESAMPRPC_SetPlayerPos, &bsData);
}
void SAMPPlayer::SetModelID(uint32_t modelid) {
	RakNet::BitStream bsData;
	bsData.Write((uint32_t)m_player_id);
	bsData.Write((uint32_t)modelid);
	mp_driver->SendRPCToStreamed(this, ESAMPRPC_SetPlayerSkin, &bsData);
	m_model_id = modelid;
}
void SAMPPlayer::SetNametagColour(uint32_t c) {
	this->m_nametag_colour = c;
}
void SAMPPlayer::SetFightstyle(uint8_t fs) {
	this->m_fightstyle = fs;
}
void SAMPPlayer::SetName(const char *name) {
	strcpy((char *)&m_name, name);
}

void SAMPPlayer::SetIsNPC(bool npc) {
	m_is_npc = npc;
}

void SAMPPlayer::SetPlayerID(uint16_t id) {
	m_player_id = id;
}

void SAMPPlayer::SetTeam(uint8_t team) {
	this->m_team = team;
}