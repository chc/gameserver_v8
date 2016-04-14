#include "SAMPPlayer.h"
#include <stdio.h>
SAMPPlayer::SAMPPlayer(SAMPDriver *driver) {
	mp_driver = driver;
	mp_samp_peer = NULL;
	m_spawned = false;
	m_is_npc = false;
	m_fightstyle = 0;
	m_nametag_colour = 0;
}
SAMPPlayer::SAMPPlayer(SAMPRakPeer *peer, SAMPDriver *driver) {
	mp_samp_peer = peer;
	mp_driver = driver;
	m_spawned = false;
	m_is_npc = false;
	m_fightstyle = 0;
	m_nametag_colour = 0;
}
SAMPPlayer::~SAMPPlayer() {

}

void SAMPPlayer::SetHealth(float h) {
	RakNet::BitStream bsData;
	bsData.Write(h);
	if(mp_samp_peer)
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
void SAMPPlayer::SetQuat(float *q) {
	m_quat[0] = q[0];
	m_quat[1] = q[1];
	m_quat[2] = q[2];
	m_quat[3] = q[3];
}

float *SAMPPlayer::GetMoveSpeed() {
	return (float *)m_move_speed;
}
void SAMPPlayer::SetMoveSpeed(float *s) {
	m_move_speed[0] = s[0];
	m_move_speed[1] = s[1];
	m_move_speed[2] = s[2];
}

uint32_t SAMPPlayer::GetAnim() {
	return m_anim;
}
void SAMPPlayer::SetAnim(uint32_t id) {
	m_anim = id;
}

float *SAMPPlayer::GetSurfOffset() {
	return (float *)&m_surf_offset;
}
void SAMPPlayer::SetSurfOffset(float *o) {
	m_surf_offset[0] = o[0];
	m_surf_offset[1] = o[1];
	m_surf_offset[2] = o[2];
}

uint8_t SAMPPlayer::GetSpecialAction() {
	return m_special_action;
}
void SAMPPlayer::SetSpecialAction(uint8_t action) {
	m_special_action = action;
}

uint16_t SAMPPlayer::GetSurfFlags() {
	return m_surf_flags;
}
void SAMPPlayer::SetSurfFlags(uint16_t f) {
	m_surf_flags = f;
}

uint8_t SAMPPlayer::GetHoldingWeapon() {
	return m_holding_weapon;
}
void SAMPPlayer::SetHoldingWeapon(uint8_t weapon) {
	m_holding_weapon = weapon;
}

void SAMPPlayer::GetKeys(uint16_t &left_right, uint16_t &up_down, uint16_t &keys) {
	left_right = m_left_right_keys;
	up_down = m_up_down_keys;
	keys = m_keys;
}
void SAMPPlayer::SetKeys(uint16_t left_right, uint16_t up_down, uint16_t keys) {
	m_left_right_keys = left_right;
	m_up_down_keys = up_down;
	m_keys = keys;

}