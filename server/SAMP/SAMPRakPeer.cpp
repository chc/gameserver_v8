#include "SAMPRakPeer.h"
#include "SAMPDriver.h"
#include "encryption.h"
#include <stdio.h>
#include "../CHCGameServer.h"

#include "SAMPPlayer.h"

#pragma pack(1)
typedef struct _PLAYER_SPAWN_INFO
{
	uint8_t byteTeam;
	int iSkin;
	uint8_t unk;
	float vecPos[3];
	float fRotation;
	int iSpawnWeapons[3];
	int iSpawnWeaponsAmmo[3];
} PLAYER_SPAWN_INFO;

RPCHandler SAMPRakPeer::s_rpc_handler[] = {
	{ESAMPRPC_ClientJoin, &SAMPRakPeer::m_client_join_handler},
	{ESAMPRPC_ClientCommand, &SAMPRakPeer::m_client_command_handler},
	{ESAMPRPC_DialogResponse, &SAMPRakPeer::m_client_dialogresp_handler},
	{ESAMPRPC_ClientSpawned, &SAMPRakPeer::m_client_spawned_handler},
};

SAMPRakPeer::SAMPRakPeer(SAMPDriver *driver, struct sockaddr_in *address_info) {
	mp_driver = driver;
	m_state = ESAMPConnectionState_ConnectionRequest;
	m_cookie_challenge = 0x1111;
	m_packet_sequence = 0;
	m_got_client_join = false;
	memcpy(&m_address_info,address_info, sizeof(m_address_info));
    StringCompressor::AddReference();

    m_vehicle_stream_distance = 1000.0;
    mp_player = new SAMPPlayer(this, mp_driver);
    mp_player->SetPlayerID(mp_driver->GetFreePlayerID());
}
SAMPRakPeer::~SAMPRakPeer() {
	delete mp_player;
    StringCompressor::RemoveReference();
}
void SAMPRakPeer::handle_packet(char *data, int len) {

	sampDecrypt((uint8_t *)data, len, mp_driver->getPort(), 0);

	int sd = mp_driver->getListenerSocket();

	if(m_state != ESAMPConnectionState_SAMPRak) { //special "auth" crap

		if((m_state != ESAMPConnectionState_ConnectionRequest &&  // SAMP sends the connection request twice, 
			m_state != ESAMPConnectionState_WaitConnectCookie) && // once to get cookie, 
			data[0] == ID_OPEN_CONNECTION_REQUEST) return; 			  // and again to reply with the cookie, only then we should get these packets


		if(data[0] == ID_OPEN_CONNECTION_REQUEST) {
			if(m_state == ESAMPConnectionState_ConnectionRequest) {
				set_connection_state(ESAMPConnectionState_WaitConnectCookie);
			} else if(m_state == ESAMPConnectionState_WaitConnectCookie) {
				uint16_t cookie = *((uint16_t*)&data[1]);
				if((m_cookie_challenge ^ SAMP_COOKIE_KEY) != cookie) {
					//cookie mismatch, should fail, kick, etc
				}
				set_connection_state(ESAMPConnectionState_SAMPRak);
			}
		}
	} else {
		handle_raknet_packet(data, len);
	}

}
void SAMPRakPeer::handle_raknet_packet(char *data, int len) {
	RakNet::BitStream is((unsigned char *)data, len, false);
	bool hasacks;
	uint8_t reliability = 0;
	uint16_t seqid = 0;
	is.Read(hasacks);

	if(hasacks) {
		DataStructures::RangeList<uint16_t> incomingAcks;
		incomingAcks.Deserialize(&is);
		uint16_t messageNumber;
		for (int i=0; i<incomingAcks.ranges.Size();i++)
		{
			if (incomingAcks.ranges[i].minIndex>incomingAcks.ranges[i].maxIndex)
			{
				break;
			}

			for (messageNumber=incomingAcks.ranges[i].minIndex; messageNumber >= incomingAcks.ranges[i].minIndex && messageNumber <= incomingAcks.ranges[i].maxIndex; messageNumber++)
			{
				//printf("======= ack msgid: %d\n", messageNumber);
				acknowlegements.Insert(messageNumber);
			}
		}
		//printf("**** Num acknowlegements: %d\n",acknowlegements.Size());
	}
	while(BITS_TO_BYTES(is.GetNumberOfUnreadBits()) > 1) {
		is.Read(seqid);
		if(seqid > m_packet_sequence) {
			m_packet_sequence = seqid+1;
		}
		//printf("**** Packet %d - %d\n",seqid, BITS_TO_BYTES(is.GetNumberOfUnreadBits()));
		is.ReadBits(&reliability, 4, true);
		
		//printf("reliability: %d\n",reliability);
		if(reliability == UNRELIABLE_SEQUENCED || reliability == RELIABLE_SEQUENCED || reliability == RELIABLE_ORDERED ) {
			uint8_t orderingChannel = 0;
			uint16_t orderingIndexType;
			is.ReadBits((unsigned char *)&orderingChannel, 5, true);
			is.Read(orderingIndexType);
			//printf("**** reliability: %d - (chan: %d  index: %d) (%d)\n",reliability,orderingChannel, orderingIndexType, len);


		}
		acknowlegements.Insert(seqid);

		bool is_split_packet;
		//is.ReadBits((unsigned char *)&is_split_packet, 1);
		is.Read(is_split_packet);

		if(is_split_packet) {
			uint16_t split_packet_id;
			uint32_t split_packet_index, split_packet_count;
			is.Read(split_packet_id);
			is.ReadCompressed(split_packet_index);
			is.ReadCompressed(split_packet_count);
			//printf("**** Split: (%d) %d %d\n", split_packet_id, split_packet_index, split_packet_count);
		}
		//loop through all data
		uint16_t data_len;
		is.ReadCompressed(data_len);
		char data[4096];
		is.ReadAlignedBytes((unsigned char *)&data, BITS_TO_BYTES(data_len));

		RakNet::BitStream bs((unsigned char *)&data, BITS_TO_BYTES(data_len), false);
		process_bitstream(&bs);
	}
}
void SAMPRakPeer::process_bitstream(RakNet::BitStream *stream) {
	RakNet::BitStream os(1024);
	uint8_t msgid;
	stream->Read(msgid);
	//printf("Rak MSGID: %d/%02x - %d\n",msgid,msgid, BITS_TO_BYTES(stream->GetNumberOfUnreadBits()));
	uint32_t ping_cookie;
	uint32_t the_time = time(NULL);
	switch(msgid) {
		case ID_CONNECTION_REQUEST:
		send_samp_rakauth("277C2AD934406F33");
		break;
		case ID_TIMESTAMP:
			printf("Got timestamp\n");
			
			os.Write((uint8_t)ID_TIMESTAMP);
			os.Write(the_time);
			send_bitstream(&os);
		break;
		case ID_AUTH_KEY:
			uint8_t auth_len;
			uint8_t auth[32];
			memset(&auth,0,sizeof(auth));
			stream->Read(auth_len);
			stream->Read((char *)&auth, auth_len);
			//printf("Client Auth key: %s\n",auth);
			send_connection_accepted(true);
			m_samprak_auth = true;
		break;
		case ID_PING:
			printf("Got ping\n");
			stream->Read(ping_cookie);

			os.Write((uint8_t)ID_CONNECTED_PONG);
			os.Write(ping_cookie);
			os.Write(RakNet::GetTime());
			send_bitstream(&os);
		break;
		case ID_INTERNAL_PING:
			printf("Got internal ping\n");
			stream->Read(ping_cookie);

			os.Write((uint8_t)ID_INTERNAL_PING);
			os.Write(ping_cookie);
			os.Write((uint32_t)RakNet::GetTime());
			send_bitstream(&os);
		break;
		case ID_CONNECTED_PONG:
		case ID_PONG:
			printf("Got pong\n");
		break;
		case ID_DETECT_LOST_CONNECTIONS:
			printf("Sending lost connections thing\n");
			send_detect_lost_connections();
		break;
		case ID_RPC:
			handle_incoming_rpc(stream);
		break;
		case ID_RECEIVED_STATIC_DATA:
			set_static_data(NULL, 0);
			send_detect_lost_connections();
		break;
		case ID_NEW_INCOMING_CONNECTION:
		//printf("New connection");
		break;
		case ID_PLAYER_SYNC:

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
		stream->Read(leftright_keys);
		stream->Read(updown_keys);
		stream->Read(keys);
		stream->Read(pos[0]);
		stream->Read(pos[1]);
		stream->Read(pos[2]);
		stream->Read(quat[0]);
		stream->Read(quat[1]);
		stream->Read(quat[2]);
		stream->Read(quat[3]);
		stream->Read(health);
		stream->Read(armour);
		stream->Read(weapon);
		stream->Read(specialaction);
		stream->Read(movespeed[0]);
		stream->Read(movespeed[1]);
		stream->Read(movespeed[2]);
		stream->Read(surfoffset[0]);
		stream->Read(surfoffset[1]);
		stream->Read(surfoffset[2]);
		stream->Read(surfinfo);
		stream->Read(anim);

		if(mp_player) {
			mp_player->SetPosition((float *)&pos);
			mp_player->SetQuat((float *)&quat);
			mp_player->SetHealth((float)health);
			mp_player->SetArmour((float)armour);
			mp_player->SetAnim(anim);
			mp_player->SetMoveSpeed((float *)&movespeed);
			mp_player->SetHoldingWeapon(weapon);
			mp_player->SetSurfFlags(surfinfo);
			mp_player->SetSurfOffset((float *)&surfoffset);
			mp_player->SetSpecialAction(specialaction);
			mp_player->SetKeys(leftright_keys, updown_keys, keys);
			mp_driver->SendPlayerUpdate(mp_player);
		}
		
		//printf("Player sync: pos(%f,%f,%f) : %d %d (%f,%f,%f) : %d\n",pos[0],pos[1],pos[2],health,armour,movespeed[0],movespeed[1],movespeed[2], mp_player->GetPlayerID());
		break;
	}
	/*
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
	*/
}
void SAMPRakPeer::find_rpc_handler_by_id(uint8_t id) {

}

void SAMPRakPeer::handle_incoming_rpc(RakNet::BitStream *stream) {
	uint8_t rpcid;
	uint32_t bits = 0;
	stream->Read(rpcid);
	if(!stream->ReadCompressed(bits)) {
		bits = 0;
	}
	uint32_t bytes = BITS_TO_BYTES(bits);
	char rpcdata[1024];
	
	stream->ReadBits((unsigned char *)&rpcdata, bits, false);
	RakNet::BitStream bs((unsigned char *)&rpcdata, bytes, true);

	for(int i=0;i<sizeof(s_rpc_handler)/sizeof(RPCHandler);i++) {
		if(s_rpc_handler[i].id == rpcid) {
			(*this.*s_rpc_handler[i].handler)(&bs);
			return;
		}
	}
	printf("RPC not found: %d\n", rpcid);

}
void SAMPRakPeer::send_connection_accepted(bool success) {
	RakNet::BitStream bs;
	bs.Write((uint8_t)ID_CONNECTION_REQUEST_ACCEPTED);
	bs.Write((uint32_t)(m_address_info.sin_addr.s_addr));
	bs.Write((uint16_t)htons(m_address_info.sin_port));
	uint32_t challenge = 0x62B44F34;
	bs.Write(mp_player->GetPlayerID());
	bs.Write(challenge);
	send_bitstream(&bs);
}
void SAMPRakPeer::set_connection_state(ESAMPConnectionState state) {
	char c[3];
	memset(&c,0,sizeof(c));
	socklen_t slen = sizeof(struct sockaddr_in);
	m_state = state;
	switch(m_state) {
		case ESAMPConnectionState_WaitConnectCookie:
			c[0] = ID_OPEN_CONNECTION_COOKIE;
			*((uint16_t*)&c[1]) = m_cookie_challenge;
			sendto(mp_driver->getListenerSocket(), (char *)&c, 3, 0, (struct sockaddr *)&m_address_info, slen);
		break;
		case ESAMPConnectionState_SAMPRak: 
			c[0] = ID_OPEN_CONNECTION_REPLY;
			sendto(mp_driver->getListenerSocket(), (char *)&c, 2, 0, (struct sockaddr *)&m_address_info, slen);
		break;
	}
}
void SAMPRakPeer::send_samp_rakauth(const char *key) {
	RakNet::BitStream bs;
	bs.Write((uint8_t)ID_AUTH_KEY);
	uint8_t len = strlen(key);
	bs.Write(len);
	bs.Write(key,len);
	bs.Write((char)0);
	send_bitstream(&bs);
}
#define UDP_HEADER_SIZE 28
#define MTUSize 1092
void SAMPRakPeer::send_bitstream(RakNet::BitStream *stream) {
	PacketReliability reliability = RELIABLE;
	RakNet::BitStream os;
	if( acknowlegements.Size() > 0) {
		os.Write(true); //has no acks
		acknowlegements.Serialize(&os, (MTUSize-UDP_HEADER_SIZE)*8-1, true);
		acknowlegements.Clear();

	} else {
		os.Write(false); //has no acks
	}
	os.Write(m_packet_sequence++);
	os.WriteBits((unsigned char *)&reliability, 4, true);
	os.Write(false); //is split packet



	uint16_t num_bits = (stream->GetNumberOfBitsUsed());
	os.WriteCompressed(num_bits);
	os.AlignWriteToByteBoundary();
	os.Write(stream);

	socklen_t slen = sizeof(struct sockaddr_in);
	sendto(mp_driver->getListenerSocket(), (char *)os.GetData(), BITS_TO_BYTES(os.GetNumberOfBitsUsed()), 0, (struct sockaddr *)&m_address_info, slen);
}


void SAMPRakPeer::m_client_command_handler(RakNet::BitStream *stream) {
	uint32_t cmdlen;
	uint8_t cmd[256];
	stream->Read(cmdlen);
	memset(&cmd,0,sizeof(cmd));
	stream->Read((char *)&cmd, cmdlen);
	printf("Cmd: %s\n",cmd);
	CHCGameServer *server = (CHCGameServer *)mp_driver->getServer();
	server->GetScriptInterface()->HandleEvent(CHCGS_ClientCommand, this, cmd);
}
void SAMPRakPeer::m_client_dialogresp_handler(RakNet::BitStream *stream) {
	uint16_t dialogid;
	uint8_t buttonid;
	uint16_t list_idx;
	char resp[128+1];
	uint8_t resplen;
	memset(&resp,0,sizeof(resp));

	
	stream->Read(dialogid);
	stream->Read(buttonid);
	stream->Read(list_idx);
	stream->Read(resplen);
	stream->Read(resp,resplen);

	DialogEvent event;
	event.input = (const char *)&resp;
	event.dialog_id = dialogid;
	event.button_id = buttonid;
	event.list_index = list_idx;

	CHCGameServer *server = (CHCGameServer *)mp_driver->getServer();
	server->GetScriptInterface()->HandleEvent(CHCGS_DialogResponse, this, (void *)&event);
}
void SAMPRakPeer::m_client_spawned_handler(RakNet::BitStream *stream) {
	if(mp_player) {
		mp_player->SetSpawned(true);
	}
}
void SAMPRakPeer::m_client_join_handler(RakNet::BitStream *stream) {
	uint32_t netver;
	char name[24];
	uint32_t challenge;
	char version[24];
	uint8_t mod;
	char auth[4*16];
	uint8_t auth_len;
	memset(&auth,0,sizeof(auth));
	memset(&version,0,sizeof(version));
	memset(&name,0,sizeof(name));
	uint8_t namelen;
	uint8_t verlen;

	uint32_t client_chal;

	stream->Read(netver);
	stream->Read(mod);
	stream->Read(namelen);
	stream->Read(name, namelen);
	stream->Read(client_chal);
	stream->Read(auth_len);
	stream->Read(auth, auth_len);
	stream->Read(verlen);
	stream->Read(version, verlen);

	printf("Name: %s\nGPCI: %s\nVersion: %s\n",name,auth,version);

	mp_player->SetName(name);
	mp_driver->SendScoreboard(this);
	mp_driver->SendAddPlayerToScoreboard(this->GetPlayer());
	if(!m_got_client_join) {

		send_player_update();
		send_game_init();
		//send_fake_players();
	}
	m_got_client_join = true;
}
void SAMPRakPeer::send_detect_lost_connections() {
	RakNet::BitStream bs;
	bs.Write((uint8_t)ID_DETECT_LOST_CONNECTIONS);
	send_bitstream(&bs);
}
void SAMPRakPeer::set_static_data(const char *data, int len) {
	RakNet::BitStream bs;
	//this should be an unsequenced, normal priority msg
	bs.Write((uint8_t)ID_RECEIVED_STATIC_DATA);
	if(len > 0)
		bs.Write(data, len);
	send_bitstream(&bs);
}
void SAMPRakPeer::think() {
	//if(m_got_client_join)
		//send_ping();
	if(mp_player && mp_player->GetSpawned())
		mp_driver->StreamUpdate(this);
}
void SAMPRakPeer::send_ping() {
	return;
	static int last_ping = 0;

	if(time(NULL)-last_ping > 2) {
		int atime = time(NULL);
		
		RakNet::BitStream bs;
		bs.Write((uint8_t)ID_PING);
		bs.Write(atime);
		send_bitstream(&bs);
		last_ping = atime;
	}	
}
void SAMPRakPeer::SendClientMessage(uint32_t colour, const char *msg) {
	RakNet::BitStream bsData;
	bsData.Reset();
	bsData.Write(colour);
	uint32_t len = strlen(msg);
	bsData.Write(len);
	bsData.Write(msg,len);
	send_rpc(93, &bsData);
	
}
void SAMPRakPeer::send_fake_players() {
	RakNet::BitStream bsData;

	PLAYER_SPAWN_INFO psInfo;
	memset(&psInfo, 0, sizeof(psInfo));
	psInfo.byteTeam = 0xFF;
	psInfo.iSkin = 33;
	//1529.6,-1691.2,13.3
	psInfo.vecPos[0] = 1529.6f;
	psInfo.vecPos[1] = -1691.2f;
	psInfo.vecPos[2] = 13.3f;
	psInfo.fRotation = 90.0f;
	psInfo.iSpawnWeapons[0] = 38;
	psInfo.iSpawnWeaponsAmmo[0] = 69;

	bsData.Write((uint8_t)1);
	bsData.Write((char *)&psInfo, sizeof(psInfo));
	send_rpc(ESAMPRPC_RequestClass, &bsData);

	bsData.Reset();
	
	bsData.Write((uint32_t)2);
	send_rpc(ESAMPRPC_RequestSpawn, &bsData);
	
	
	char name[24];
	for(uint16_t i=0;i<1000;i++) {
		RakNet::BitStream bs;
		sprintf(name,"user_%d",i);
		bs.Write(i);
		bs.Write((uint32_t)0); //colour, 0 = client chooses
		bs.Write((uint8_t)0); //isNPC
		bs.Write((uint8_t)strlen(name));
		bs.Write(name, strlen(name));
		send_rpc(ESAMPRPC_ServerJoin, &bs);
	}
	
}
void SAMPRakPeer::SpawnPlayer(float x, float y, float z, int skin, int team) {
	RakNet::BitStream bsData;

	PLAYER_SPAWN_INFO psInfo;
	memset(&psInfo, 0, sizeof(psInfo));
	psInfo.byteTeam = team;
	psInfo.iSkin = skin;
	psInfo.vecPos[0] = x;
	psInfo.vecPos[1] = y;
	psInfo.vecPos[2] = z;

	mp_player->SetModelID(skin);
	mp_player->SetTeam(team);
	mp_player->SetSpawned(true);

	bsData.Write((uint8_t)1); //maybe the id or something
	bsData.Write((char *)&psInfo, sizeof(psInfo));
	send_rpc(ESAMPRPC_RequestClass, &bsData);

	bsData.Reset();
		
	//send spawn request
	bsData.Write((uint32_t)2);
	send_rpc(ESAMPRPC_RequestSpawn, &bsData);

	CHCGameServer *server = (CHCGameServer *)mp_driver->getServer();
	server->GetScriptInterface()->HandleEvent(CHCGS_EnterWorld, this, NULL);

}
void SAMPRakPeer::ShowPlayerDialog(int dialogid, int type, const char *title, const char *msg, const char *b1, const char *b2) {
	RakNet::BitStream bs;

	const char *button_default = "";
	if(b1 == NULL) b1 = button_default;
	if(b2 == NULL) b2 = button_default;
	int b1len = strlen(b1);
	int b2len = strlen(b2);

	bs.Write((uint16_t)dialogid);
	bs.Write((uint8_t)type);
	bs.Write((uint8_t)strlen(title));
	bs.Write(title, strlen(title));

	bs.Write((uint8_t)b1len);
	bs.Write(b1, b1len);

	bs.Write((uint8_t)b2len);
	bs.Write(b2, b2len);
	StringCompressor::Instance()->EncodeString(msg, strlen(msg)+1, &bs);
	send_rpc(61, &bs);
}
void SAMPRakPeer::send_rpc(uint8_t rpc, RakNet::BitStream *stream) {
	RakNet::BitStream bs;
	bs.Write((uint8_t)ID_RPC);
	bs.Write(rpc);
	bs.WriteCompressed((uint32_t)stream->GetNumberOfBitsUsed());
	bs.Write(stream);
	send_bitstream(&bs);
}
void SAMPRakPeer::send_player_update() {
	RakNet::BitStream os;
	os.Write(mp_driver->getDeltaTime());
	send_rpc(60, &os);
}
void SAMPRakPeer::send_game_init() {
	RakNet::BitStream os;

	os.WriteCompressed(false); //zone names
	os.WriteCompressed(false); //CJ walk
	os.WriteCompressed(true); //allow weapons
	os.WriteCompressed(false); //limit chat radius
	os.Write(0.0f); //chat radius
	os.WriteCompressed(false); //stunt bonus
	os.Write(100.0f); //nametag distance
	os.WriteCompressed(true); //disable enter exists
	os.WriteCompressed(true); //nametag LOS
	os.WriteCompressed(false); //manual vehicle lighting
	os.Write((uint32_t)1); //"spawns available??"
	os.Write(GetPlayer()->GetPlayerID());
	os.WriteCompressed(true); //show nametags
	os.Write((uint32_t)0); //show player markers
	os.Write((uint8_t)12); //server hour
	os.Write((uint8_t)1); //server weather
	os.Write(0.008f); //gravity
	os.WriteCompressed(false); //lan mode
	os.Write((uint32_t)0); //drop money on death
	os.WriteCompressed(false); //unknown

	os.Write((uint32_t)40); //on foot send rate
	os.Write((uint32_t)40); //in car send rate
	os.Write((uint32_t)40); //firing send rate
	os.Write((uint32_t)10); //send multiplier
	os.Write((uint8_t)true); //lagcomp

	uint8_t unk = 0;
	os.Write(unk);
	os.Write(unk);
	os.Write(unk);

	char name[24];
	memset(&name,0,sizeof(name));
	strcpy(name,"1132222222abcdef");
	uint8_t servlen = strlen(name);
	os.Write(servlen);
	os.Write((const char *)&name,servlen);

	for(int i=0;i<=212;i++) {
		os.Write((uint8_t)1);
	}
	send_rpc(139, &os);
}
void SAMPRakPeer::StreamInCar(SAMPVehicle *car) {
	RakNet::BitStream bsData;
	bsData.Write((uint16_t)car->id);
	bsData.Write((uint32_t)car->modelid);
	bsData.Write((float)car->pos[0]);
	bsData.Write((float)car->pos[1]);
	bsData.Write((float)car->pos[2]);
	bsData.Write((float)car->zrot);
	bsData.Write((uint8_t)car->colours[0]);
	bsData.Write((uint8_t)car->colours[1]);
	bsData.Write((float)car->health);
	bsData.Write((uint8_t)car->interior);
	bsData.Write((uint32_t)car->doorDamageStatus);
	bsData.Write((uint32_t)car->panelDamageStatus);
	bsData.Write((uint8_t)car->lightDamageStatus);
	bsData.Write((uint8_t)car->tireDamageStatus);
	bsData.Write((uint8_t)car->addSiren);
	bsData.Write((char *)&car->components,sizeof(car->components));
	bsData.Write((uint8_t)car->paintjob);
	bsData.Write((uint32_t)0); 
	bsData.Write((uint32_t)0);
	bsData.Write((uint8_t)0);

	send_rpc(ESAMPRPC_VehicleCreate, &bsData);	
	SAMPStreamRecord rec;
	rec.data = (void *)car;
	m_streamed_vehicles.push_back(rec);
}
void SAMPRakPeer::StreamOutCar(SAMPVehicle *car) {
	RakNet::BitStream bsData;
	std::vector<SAMPStreamRecord>::iterator it = m_streamed_vehicles.begin();
	while(it != m_streamed_vehicles.end()) {
		SAMPStreamRecord rec = *it;
		if(rec.data == (void *)car) {
			m_streamed_vehicles.erase(it);
		}
		it++;
	}
	bsData.Write(car->id);
	send_rpc(ESAMPRPC_VehicleDelete, &bsData);	
}
bool VecInRadius(float r, float x, float y, float z) {
	return ((x) + (y) + (z)) < (r);
}
bool SAMPRakPeer::VehicleInStreamRange(SAMPVehicle *car) {
	return VecInRadius(m_vehicle_stream_distance, car->pos[0], car->pos[1], car->pos[2]);
}
bool SAMPRakPeer::IsVehicleStreamed(SAMPVehicle *car) {
	std::vector<SAMPStreamRecord>::iterator it = m_streamed_vehicles.begin();
	while(it != m_streamed_vehicles.end()) {
		SAMPStreamRecord rec = *it;

		if(rec.data == car) {
			return true;
		}
		it++;
	}
	return false;
}
bool SAMPRakPeer::IsPlayerStreamed(SAMPPlayer *car) {
	std::vector<SAMPStreamRecord>::iterator it = m_streamed_players.begin();
	while(it != m_streamed_players.end()) {
		SAMPStreamRecord rec = *it;

		if(rec.data == car) {
			return true;
		}
		it++;
	}
	return false;
}
void SAMPRakPeer::VehicleStreamCheck(SAMPVehicle *car) {
	bool is_streamed = IsVehicleStreamed(car);

	if(!is_streamed) {
		if(VehicleInStreamRange(car)) {
			 StreamInCar(car);
		}
	} else {
		if(!VehicleInStreamRange(car)) {
			StreamOutCar(car);
		}
	}
}
void SAMPRakPeer::PlayerStreamCheck(SAMPPlayer *car) {
	if(!mp_player || !mp_player->GetSpawned() || !car->GetSpawned())
		return;
	bool is_streamed = IsPlayerStreamed(car);
	if(!is_streamed) {
		if(PlayerInStreamRange(car)) {
			 StreamInPlayer(car);
		}
	} else {
		if(!PlayerInStreamRange(car)) {
			StreamOutPlayer(car);
		}
	}
}
bool SAMPRakPeer::PlayerInStreamRange(SAMPPlayer *car) {
	float *pos = car->GetPosition();
	return VecInRadius(m_vehicle_stream_distance, pos[0], pos[1], pos[2]);
}

void SAMPRakPeer::StreamInPlayer(SAMPPlayer *car) {
	if(car == mp_player || !car->GetSpawned() || !mp_player->GetSpawned()) 
		return;
	RakNet::BitStream bsData;
	float *pos = car->GetPosition();
	bsData.Write((uint16_t)car->GetPlayerID());
	bsData.Write((uint8_t)car->GetTeam());
	bsData.Write((uint32_t)car->GetModelID());

	printf("Stream in: %p - %d - %d\n", car, car->GetPlayerID(), car->GetModelID());
	bsData.Write((float)pos[0]);
	bsData.Write((float)pos[1]);
	bsData.Write((float)pos[2]);
	bsData.Write((float)0.0f);
	bsData.Write((uint32_t)car->GetNametagColour());
	bsData.Write((uint8_t)car->GetFightstyle());

	send_rpc(ESAMPRPC_AddPlayerToWorld, &bsData);
	SAMPStreamRecord rec;
	rec.data = (void *)car;
	m_streamed_players.push_back(rec);
}
void SAMPRakPeer::StreamOutPlayer(SAMPPlayer *peer) {
	RakNet::BitStream bsData;
	bsData.Write(peer->GetPlayerID());
	send_rpc(ESAMPRPC_DeletePlayerFromWorld, &bsData);
	std::vector<SAMPStreamRecord>::iterator it = m_streamed_players.begin();
	while(it != m_streamed_players.end()) {
		SAMPStreamRecord rec = *it;
		if(rec.data == (void *)peer) {
			m_streamed_players.erase(it);
		}
		it++;
	}
}

void SAMPRakPeer::AddToScoreboard(SAMPPlayer *peer) {
	RakNet::BitStream bs;
	const char *name = peer->GetName();
	int len = strlen(name);
	bs.Write(peer->GetPlayerID());
	bs.Write((uint32_t)peer->GetNametagColour()); //colour, 0 = client chooses
	bs.Write((uint8_t)peer->GetIsNPC()); //isNPC
	bs.Write((uint8_t)len);
	bs.Write(name, len);
	send_rpc(ESAMPRPC_ServerJoin, &bs);
}