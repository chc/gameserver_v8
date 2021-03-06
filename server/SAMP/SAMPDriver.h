#ifndef _SAMPDRIVER_H
#define _SAMPDRIVER_H
#include <stdint.h>
#include "main.h"
#include "NetDriver.h"
#include "encryption.h"


#include <map>
#include <vector>
#ifdef _WINDOWS
#include <time.h>
#else
#include <sys/time.h>
#endif

#include <RakNet/BitStream.h>
#include <RakNet/DS_RangeList.h>
#include <RakNet/StringCompressor.h>
#include <RakNet/GetTime.h>

class SAMPRakPeer;
class SAMPPlayer;

#define SAMP_MAGIC 0x504d4153

#define SAMP_MAX_VEHICLES 2000
#define SAMP_MAX_PLAYERS 1000
#define SAMP_MAX_TEXTDRAWS 2048
#define SAMP_MAX_PLAYER_TEXTDRAW 256
#define SAMP_TEXTDRAW_MAX_STRLEN 1024

#define SAMP_PING_TIME_SEC 5

enum ESAMPRPC {
	ESAMPRPC_SetPlayerPos = 12,	
	ESAMPRPC_SetPlayerHealth = 14,	
	ESAMPRPC_ClientJoin = 25,
	ESAMPRPC_EnterVehicle = 26,
	ESAMPRPC_AddPlayerToWorld = 32,
	ESAMPRPC_Death = 53,
	ESAMPRPC_DeletePlayerFromWorld = 163,
	ESAMPRPC_Create3DTextLabel = 36,
	ESAMPRPC_ClientCommand = 50,
	ESAMPRPC_ClientSpawned = 52,
	ESAMPRPC_Delete3DTextLabel = 58,
	ESAMPRPC_DialogResponse = 62,
	ESAMPRPC_DestroyPickup = 63,
	ESAMPRPC_SetPlayerArmour = 66,
	ESAMPRPC_PutPlayerInVehicle = 70,
	ESAMPRPC_ShowGameText = 73,
	ESAMPRPC_SelectTextDraw = 83,
	ESAMPRPC_CreatePickup = 95,
	ESAMPRPC_SendClientMessage = 93,
	ESAMPRPC_ChatMessage = 101,
	ESAMPRPC_EditTextDraw = 105,
	ESAMPRPC_RequestClass = 128,
	ESAMPRPC_RequestSpawn = 129,
	ESAMPRPC_ShowTextDraw = 134,
	ESAMPRPC_HideTextDraw = 135,
	ESAMPRPC_ServerJoin = 137,
	ESAMPRPC_ServerQuit = 138,
	ESAMPRPC_SetPlayerSkin = 153,
	ESAMPRPC_ExitVehicle = 154,
	ESAMPRPC_VehicleCreate = 164,
	ESAMPRPC_VehicleDelete = 165,
	ESAMPRPC_PlayerDeath = 166,
};


typedef struct {
	uint32_t magic;
	uint32_t ip;
	uint16_t port;
	uint8_t opcode;
} SAMPHeader;

typedef struct {
	uint32_t ping;
} SAMPPing;

enum {
	SAMP_PACKET_INFO = 'i',
	SAMP_PACKET_RULES = 'r',
	SAMP_PACKET_CLIENTS = 'c',
	SAMP_PACKET_CLIENTS_DETAILED = 'd',
	SAMP_PACKET_RCON = 'x',
	SAMP_PACKET_PING = 'p',
};

typedef struct _SAMPPickup
{
	int32_t iModel;
	int32_t iType;
	float fX;
	float fY;
	float fZ;
} SAMPPickup;

#define SAMP_LABEL_MAX_LEN 256
typedef struct _SAMP3DLabel {
	char string[SAMP_LABEL_MAX_LEN];
	float x;
	float y;
	float z;
	int world;
	int stream_index;
	bool test_los;
	float draw_distance;
	uint32_t colour;
	int playerid;
	int vehicleid;
} SAMP3DLabel;

typedef struct {
	uint16_t player_id;
	uint16_t killer_id;
	uint8_t reason;
} SAMPDeathInfo;


typedef struct {
	int 		id; //samp vehicle id
	int 		modelid;
	float 		health;
	float 		roll[3];
	float 		dir[3];
	float 		pos[3];
	float		quat[4];
	float		vel[3];
	float 		zrot;
	uint8_t 	colours[2];
	bool 		respawn_on_death;
	int 		respawn_time;
	uint8_t 	interior;
	uint32_t 	doorDamageStatus;
	uint32_t	panelDamageStatus;
	uint8_t	 	lightDamageStatus;
	uint8_t	  	tireDamageStatus;
	uint8_t	  	addSiren;
	uint8_t     components[14];
	uint8_t	  	paintjob;

	uint8_t siren_on;
	uint8_t landinggear_state;
	uint16_t trailerid_or_thrustangle;
	float train_speed;
} SAMPVehicle;


enum SAMPWeaponState
{
	WS_NO_BULLETS = 0,
	WS_LAST_BULLET,
	WS_MORE_BULLETS,
	WS_RELOADING,
};

typedef struct {
	uint8_t cam_mode;
	float f1[3]; //??
	float pos[3];
	float z;
	uint8_t cam_zoom; //6 bits
	uint8_t weapon_state; //2 bits
	uint8_t unknown;
} SAMPAimSync;

typedef struct {
	uint16_t vehicle_id;
	uint16_t roll[3];
	uint16_t direction[3];
	uint8_t unk[13];
	float pos[3];
	float velocity[3];
	float turning_speed[3];
	float health;
} SAMPUnoccupiedVehData;

typedef struct {
	uint8_t type;
	uint16_t id;
	float origin[3];
	float target[3];
	float center[3];
	uint8_t weapon;
} SAMPBulletData;

#define SAMP_SPAWN_MAX_WEAPONS 3
typedef struct {
	uint16_t skin;
	uint8_t team;
	uint8_t weapons[SAMP_SPAWN_MAX_WEAPONS];
	uint16_t ammo[SAMP_SPAWN_MAX_WEAPONS];
} SAMPPlayerClass;


enum ETextDrawFlag {
	SAMPTD_IsBox = 0x01,
	SAMPTD_IsLeftAligned = 0x02,
	SAMPTD_IsRightAligned = 0x04,
	SAMPTD_IsCenterAligned = 0x08,
	SAMPTD_IsProportional = 0x10,
};

typedef struct {
	uint16_t id;
	uint8_t flags;
	float font_width;
	float font_height;
	uint32_t font_colour;
	float box_width;
	float box_height;
	uint32_t box_colour;
	uint8_t shadow;
	uint8_t outline;
	uint32_t background_colour;
	uint8_t style;
	uint8_t selectable;
	float x;
	float y;
	uint16_t model;
	uint16_t model_colours[2];
	float rx;
	float ry;
	float rz;
	float zoom;
	char text[SAMP_TEXTDRAW_MAX_STRLEN];
} SAMPTextDraw;




class SAMPDriver : public INetDriver {
public:
	SAMPDriver(INetServer *server, const char *host, uint16_t port);
	~SAMPDriver();
	void tick();
	void think();
	int getListenerSocket();
	uint16_t getPort();
	uint32_t getBindIP();
	uint32_t getDeltaTime();

	SAMPRakPeer *find_client(struct sockaddr_in *address);
	SAMPRakPeer *find_or_create(struct sockaddr_in *address);

	int createPickup(int model, int type, float x, float y, float z);
	void destroyPickup(int pickup_id);
	int create3DTextLabel(const char *string, uint32_t colour, float x, float y, float z, float draw_distance, bool test_los, int world = 0, int stream_index = 0);
	void destroy3DTextLabel(int label);
	void SendRPCToAll(int rpc_id, RakNet::BitStream *bs);
	void SendBitstreamToAll(RakNet::BitStream *bs);
	int CreateVehicle(int modelid, float x, float y, float z, float zrot, uint8_t c1,uint8_t c2, bool respawn_on_death = false, int respawn_time = 3600);

	void StreamUpdate(SAMPRakPeer *peer);

	SAMPPlayer* CreateBot();
	void AddBot(SAMPPlayer *bot);

	void SendScoreboard(SAMPRakPeer *peer);

	void SendRPCToStreamed(SAMPPlayer *player, uint8_t rpc, RakNet::BitStream *stream, bool include_sender = false);
	void SendBitstreamToStreamed(SAMPPlayer *player, RakNet::BitStream *stream);

	void SendRPCToAll(uint8_t rpc, RakNet::BitStream *stream);

	void SendPlayerUpdate(SAMPPlayer *player);

	void SendAddPlayerToScoreboard(SAMPPlayer *player);
	void SendRemovePlayerFromScoreboard(SAMPPlayer *player);

	SAMPPlayer *findPlayerByID(uint16_t id);
	uint16_t GetFreePlayerID();

	SAMPVehicle *findVehicleByID(uint16_t id);
	uint16_t GetFreeVehicleID();

	void SendVehicleUpdate(SAMPPlayer *player, SAMPVehicle *car); 
	void SendPassengerUpdate(SAMPPlayer *player, SAMPVehicle *car);
	void SendAimSync(SAMPPlayer *player, SAMPAimSync *aim);
	void SendBulletData(SAMPPlayer *player, SAMPBulletData *bullet);

	SAMPTextDraw *FindTextDrawByID(uint16_t id);
	SAMPTextDraw *CreateTextDraw();
	uint16_t GetFreeTextDrawID();

	int GetNumConnections(bool include_bots = false, bool include_not_in_scoreboard = false);

	void BroadcastDeath(SAMPPlayer *player, SAMPPlayer* killer, uint8_t reason);

	void StreamOutForAll(SAMPPlayer *player);

private:

	//samp query stuff
	void handle_server_query(char *data, int len, struct sockaddr_in *address_info);
	void handlePingPacket(int sd, struct sockaddr_in *si_other, SAMPHeader *header, SAMPPing *ping);
	void handleClientsPacket(int sd, struct sockaddr_in *si_other, SAMPHeader *header, bool detailed = false);
	void handleInfoPacket(int sd, struct sockaddr_in *si_other, SAMPHeader *header);
	void handleRulesPacket(int sd, struct sockaddr_in *si_other, SAMPHeader *header);

	std::map<int32_t, SAMPPickup *> m_pickups;
	std::map<int32_t, SAMP3DLabel *> m_3dlabels;
	std::vector<SAMPVehicle *> m_vehicles;
	std::vector<SAMPTextDraw *> m_textdraws;
	int m_last_pickup_id;
	int m_last_3dlabel_id;
	
	uint16_t m_port;
	int m_sd;
	std::vector<SAMPRakPeer *> m_connections;
	std::vector<SAMPPlayer *> m_bots;
	struct sockaddr_in m_local_addr;

	struct timeval m_server_start;

};
#endif //_SAMPDRIVER_H