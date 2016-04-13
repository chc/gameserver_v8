#ifndef _SAMPPLAYER_H
#define _SAMPPLAYER_H
#include "SAMPDriver.h"
#include "SAMPRakPeer.h"

#define MAX_SAMP_NAME 24

class SAMPPlayer {
public:
	SAMPPlayer(SAMPDriver *driver);
	SAMPPlayer(SAMPRakPeer *peer, SAMPDriver *driver);
	~SAMPPlayer();

	float GetHealth() { return m_health; };
	void SetHealth(float h);
	float GetArmour() { return m_armour; };
	void SetArmour(float a);
	float *GetPosition() { return (float *)&m_pos; };
	void SetPosition(float *p);
	uint32_t GetModelID() { return m_model_id; };
	void SetModelID(uint32_t modelid);
	uint32_t GetNametagColour() { return m_nametag_colour; };
	void SetNametagColour(uint32_t c);
	uint8_t GetFightstyle() { return m_fightstyle; };
	void SetFightstyle(uint8_t fs);
	SAMPRakPeer* GetSAMPPeer() { return m_samp_peer; }
	const char *GetName() { return (char *)&m_name; };
	void SetName(const char *name);

	bool GetIsNPC() { return m_is_npc; };
	void SetIsNPC(bool npc);

	uint16_t GetPlayerID() { return m_player_id; };
	void SetPlayerID(uint16_t id);

	uint8_t GetTeam() { return m_team; };
	void SetTeam(uint8_t team);
	bool GetSpawned() { return m_spawned; };
	void SetSpawned(bool spawned) { m_spawned = spawned; };
private:
	float m_health;
	float m_armour;
	uint16_t m_player_id;
	float m_pos[3];
	float m_rot_z; //probs should be the full quat
	uint32_t m_model_id;
	uint32_t m_nametag_colour;
	uint8_t m_fightstyle;
	SAMPRakPeer *m_samp_peer;
	SAMPDriver *mp_driver;
	bool m_is_npc;

	uint8_t m_team;

	uint8_t m_name[MAX_SAMP_NAME];

	bool m_spawned;
};
#endif