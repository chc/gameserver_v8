#ifndef _SAMPRAKPEER_H
#define _SAMPRAKPEER_H
#include <main.h>
#include "SAMPDriver.h"
#include <RakNet/BitStream.h>
#include <RakNet/DS_RangeList.h>
#include <RakNet/StringCompressor.h>
#include <RakNet/GetTime.h>
class SAMPDriver;
class SAMPPlayer;

enum PacketEnumeration
{
	ID_INTERNAL_PING = 6,
	ID_PING,
	ID_PING_OPEN_CONNECTIONS,
	ID_CONNECTED_PONG,
	ID_REQUEST_STATIC_DATA,
	ID_CONNECTION_REQUEST,
	ID_AUTH_KEY,
	ID_BROADCAST_PINGS = 14,
	ID_SECURED_CONNECTION_RESPONSE,
	ID_SECURED_CONNECTION_CONFIRMATION,
	ID_RPC_MAPPING,
	ID_SET_RANDOM_NUMBER_SEED = 19,
	ID_RPC,
	ID_RPC_REPLY,
	ID_DETECT_LOST_CONNECTIONS = 23,
	ID_OPEN_CONNECTION_REQUEST,
	ID_OPEN_CONNECTION_REPLY,
	ID_OPEN_CONNECTION_COOKIE,
	ID_RSA_PUBLIC_KEY_MISMATCH = 28,
	ID_CONNECTION_ATTEMPT_FAILED,
	ID_NEW_INCOMING_CONNECTION = 30,
	ID_NO_FREE_INCOMING_CONNECTIONS = 31,
	ID_DISCONNECTION_NOTIFICATION,	
	ID_CONNECTION_LOST,
	ID_CONNECTION_REQUEST_ACCEPTED,
	ID_CONNECTION_BANNED = 36,
	ID_INVALID_PASSWORD,
	ID_MODIFIED_PACKET,
	ID_PONG,
	ID_TIMESTAMP,
	ID_RECEIVED_STATIC_DATA,
	ID_REMOTE_DISCONNECTION_NOTIFICATION,
	ID_REMOTE_CONNECTION_LOST,
	ID_REMOTE_NEW_INCOMING_CONNECTION,
	ID_REMOTE_EXISTING_CONNECTION,
	ID_REMOTE_STATIC_DATA,
	ID_ADVERTISE_SYSTEM = 55,

	ID_PLAYER_SYNC = 207,
	ID_MARKERS_SYNC = 208,
	ID_UNOCCUPIED_SYNC = 209,
	ID_TRAILER_SYNC = 210,
	ID_PASSENGER_SYNC = 211,
	ID_SPECTATOR_SYNC = 212,
	ID_AIM_SYNC = 203,
	ID_VEHICLE_SYNC = 200,
	ID_RCON_COMMAND = 201,
	ID_RCON_RESPONCE = 202,
	ID_WEAPONS_UPDATE = 204,
	ID_STATS_UPDATE = 205,
	ID_BULLET_SYNC = 206,
};

/// These enumerations are used to describe when packets are delivered.
enum PacketPriority
{
	SYSTEM_PRIORITY,   /// \internal Used by RakNet to send above-high priority messages.
	HIGH_PRIORITY,   /// High priority messages are send before medium priority messages.
	MEDIUM_PRIORITY,   /// Medium priority messages are send before low priority messages.
	LOW_PRIORITY,   /// Low priority messages are only sent when no other messages are waiting.
	NUMBER_OF_PRIORITIES
};

/// These enumerations are used to describe how packets are delivered.
/// \note  Note to self: I write this with 3 bits in the stream.  If I add more remember to change that
enum PacketReliability
{
	UNRELIABLE = 6,   /// Same as regular UDP, except that it will also discard duplicate datagrams.  RakNet adds (6 to 17) + 21 bits of overhead, 16 of which is used to detect duplicate packets and 6 to 17 of which is used for message length.
	UNRELIABLE_SEQUENCED,  /// Regular UDP with a sequence counter.  Out of order messages will be discarded.  This adds an additional 13 bits on top what is used for UNRELIABLE.
	RELIABLE,   /// The message is sent reliably, but not necessarily in any order.  Same overhead as UNRELIABLE.
	RELIABLE_ORDERED,   /// This message is reliable and will arrive in the order you sent it.  Messages will be delayed while waiting for out of order messages.  Same overhead as UNRELIABLE_SEQUENCED.
	RELIABLE_SEQUENCED /// This message is reliable and will arrive in the sequence you sent it.  Out or order messages will be dropped.  Same overhead as UNRELIABLE_SEQUENCED.
};

enum ESAMPConnectionState {
	ESAMPConnectionState_ConnectionRequest, //client has initially connected, sending response(banned, full, or pre-auth cookie)
	ESAMPConnectionState_WaitConnectCookie,
	ESAMPConnectionState_SAMPRak, //"SAMPRak" mode
};

class SAMPRakPeer;


typedef struct {
	uint8_t id;
	void (SAMPRakPeer::*handler)(RakNet::BitStream *stream);
} RPCHandler;

#define SAMP_COOKIE_KEY 0x6969

typedef struct {
	uint32_t stream_in_time;
	void *data;
} SAMPStreamRecord;

class SAMPRakPeer {
public:
	SAMPRakPeer(SAMPDriver *driver, struct sockaddr_in *address_info);
	~SAMPRakPeer();
	void handle_packet(char *data, int len);
	void send_ping();
	void think(); //called when no data is recieved
	const struct sockaddr_in *getAddress() { return &m_address_info; }
	void SendClientMessage(uint32_t colour, const char *msg);
	void ShowPlayerDialog(int dialogid, int type, const char *title, const char *msg, const char *b1 = NULL, const char *b2 = NULL);
	void SpawnPlayer(float x, float y, float z, int skin = 0, int team = -1, uint32_t *weapons = NULL, uint32_t *ammo = NULL);
	void send_rpc(uint8_t rpc, RakNet::BitStream *stream);
	void send_bitstream(RakNet::BitStream *stream);



	void StreamInCar(SAMPVehicle *car);
	void StreamOutCar(SAMPVehicle *car);

	void VehicleStreamCheck(SAMPVehicle *car);
	bool VehicleInStreamRange(SAMPVehicle *car);
	bool IsVehicleStreamed(SAMPVehicle *car);


	void StreamInPlayer(SAMPPlayer *player);
	void StreamOutPlayer(SAMPPlayer *player);
	
	bool IsPlayerStreamed(SAMPPlayer *car);
	void PlayerStreamCheck(SAMPPlayer *car);
	bool PlayerInStreamRange(SAMPPlayer *car);

	void AddToScoreboard(SAMPPlayer *bot);
	void RemoveFromScoreboard(SAMPPlayer *bot);
	SAMPPlayer *GetPlayer() { return mp_player; };

	void PutInCar(SAMPVehicle *car, uint8_t seat = 0);
	void SendEnterCar(SAMPVehicle *car, uint8_t seat);
	void SendExitCar(SAMPVehicle *car);

	int  GetNumSpawnClasses() { return m_num_spawn_classes; };
	void SetNumSpawnClasses(int num_classes) { m_num_spawn_classes = num_classes; };

	void SendGameText(const char *msg, uint32_t time_ms, uint32_t style = 6);
	
	void ShowTextDraw(SAMPTextDraw *td);
	void HideTextDraw(SAMPTextDraw *td);

	void SelectTextDraw(uint32_t hover_colour, bool cancel = false);

	bool ShouldDelete() { return m_delete_flag; };
	bool IsTimeout() { return m_timeout_flag; }

	int GetPing();
private:
	void handle_raknet_packet(char *data, int len);
	void process_bitstream(RakNet::BitStream *stream);
	void set_connection_state(ESAMPConnectionState state);

	void set_static_data(const char *data, int len);

	void send_samp_rakauth(const char *key);
	void send_connection_accepted(bool success);

	void send_detect_lost_connections();
	void handle_incoming_rpc(RakNet::BitStream *stream);

	uint16_t m_cookie_challenge;
	SAMPDriver *mp_driver;
	ESAMPConnectionState m_state;

	struct timeval m_last_ping;

	DataStructures::RangeList<uint16_t> acknowlegements;

	bool m_samprak_auth; //exchanged auth keys
	bool m_got_client_join;

	struct sockaddr_in m_address_info;

	uint16_t m_packet_sequence;

	float m_vehicle_stream_distance;

	std::vector<SAMPStreamRecord> m_streamed_vehicles;
	std::vector<SAMPStreamRecord> m_streamed_players;
	std::vector<SAMPStreamRecord> m_streamed_textlabels;
	std::vector<SAMPStreamRecord> m_streamed_pickups;

	//RPC Handlers
	static RPCHandler s_rpc_handler[];
	void m_client_join_handler(RakNet::BitStream *stream);
	void m_client_command_handler(RakNet::BitStream *stream);
	void m_client_dialogresp_handler(RakNet::BitStream *stream);
	void m_client_spawned_handler(RakNet::BitStream *stream);
	void m_client_enter_vehicle_handler(RakNet::BitStream *stream);
	void m_client_exit_vehicle_handler(RakNet::BitStream *stream);
	void m_client_request_spawn(RakNet::BitStream *stream);
	void m_client_request_class(RakNet::BitStream *stream);
	void m_client_chat_message_handler(RakNet::BitStream *stream);
	void m_textdraw_clicked_handler(RakNet::BitStream *stream);

	//Misc RPC stuff
	void send_game_init();
	void send_player_update(); //seems to just send timestamp?? unsure of importance

	void send_fake_players();

	SAMPPlayer *mp_player;

	int m_num_spawn_classes;

	bool m_delete_flag;
	bool m_timeout_flag;
	
};
#endif //_SAMPRAKPEER_H