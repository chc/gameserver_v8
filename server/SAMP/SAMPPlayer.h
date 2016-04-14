#ifndef _SAMPPLAYER_H
#define _SAMPPLAYER_H
#include "SAMPDriver.h"
#include "SAMPRakPeer.h"

#define MAX_SAMP_NAME 24


/*
		uint16_t leftright_keys;
		uint16_t updown_keys;
		uint16_t keys;
		float pos[3];
		float quat[4];
		uint8_t health;
		uint8_t armour;
		uint8_t weapon;
		uint8_t specialaction;
		float movespeed[3];
		float surfoffset[3];
		uint16_t surfinfo;
		uint32_t anim;
*/

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
	SAMPRakPeer* GetSAMPPeer() { return mp_samp_peer; }
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

	float *GetQuat() { return (float *)&m_quat; };
	void SetQuat(float *q);

	float *GetMoveSpeed();
	void SetMoveSpeed(float *s);

	uint32_t GetAnim();
	void SetAnim(uint32_t id);

	float *GetSurfOffset();
	void SetSurfOffset(float *o);

	uint8_t GetSpecialAction();
	void SetSpecialAction(uint8_t action);

	uint16_t GetSurfFlags(); //????
	void SetSurfFlags(uint16_t f);

	uint8_t GetHoldingWeapon();
	void SetHoldingWeapon(uint8_t weapon);

	void GetKeys(uint16_t &left_right, uint16_t &up_down, uint16_t &keys);
	void SetKeys(uint16_t left_right, uint16_t up_down, uint16_t keys);

private:
	float m_health;
	float m_armour;
	uint16_t m_player_id;
	float m_pos[3];
	float m_rot_z; //probs should be the full quat
	float m_quat[4];
	uint32_t m_model_id;
	uint32_t m_nametag_colour;
	uint8_t m_fightstyle;
	SAMPRakPeer *mp_samp_peer;
	SAMPDriver *mp_driver;
	bool m_is_npc;

	uint8_t m_team;

	uint8_t m_name[MAX_SAMP_NAME];

	bool m_spawned;

	uint8_t m_holding_weapon;
	uint8_t m_special_action;

	uint16_t m_left_right_keys;
	uint16_t m_up_down_keys;
	uint16_t m_keys;

	uint16_t m_surf_flags;

	float m_move_speed[3];
	float m_surf_offset[3];

	uint32_t m_anim;

};
#endif