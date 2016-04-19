#include "SAMPDriver.h"
#include <stdio.h>
#include <stdlib.h>

#include <server/CHCGameServer.h>
#include <buffwriter.h>

#include "SAMPRakPeer.h"
#include "SAMPPlayer.h"

SAMPDriver::SAMPDriver(INetServer *server, const char *host, uint16_t port) : INetDriver(server) {
    #ifdef _WIN32
    WSADATA wsdata;
    WSAStartup(MAKEWORD(1,0),&wsdata);
	#endif

	uint32_t bind_ip = INADDR_ANY;

	if((m_sd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0){
		//signal error
    }
    
	m_port = port;

    m_local_addr.sin_port = htons(port);
    m_local_addr.sin_addr.s_addr = htonl(bind_ip);
    m_local_addr.sin_family = AF_INET;
	int n = bind(m_sd, (struct sockaddr *)&m_local_addr, sizeof m_local_addr);
    if(n < 0) {

        //signal error
    }
    gettimeofday(&m_server_start, NULL);

    m_last_pickup_id = 0;
    m_last_3dlabel_id = 0;
}
SAMPDriver::~SAMPDriver() {

}
void SAMPDriver::think() {
	std::vector<SAMPVehicle *>::iterator itv = m_vehicles.begin();
	while(itv != m_vehicles.end()) {
		SAMPVehicle *veh = *itv;
		itv++;
	}


	std::vector<SAMPRakPeer *>::iterator it = m_connections.begin();
	while(it != m_connections.end()) {
		SAMPRakPeer *peer = *it;
		peer->think();
		it++;
	}
}
void SAMPDriver::StreamUpdate(SAMPRakPeer *peer) {
	if(!peer->GetPlayer() || !peer->GetPlayer()->GetSpawned())
		return;
	std::vector<SAMPVehicle *>::iterator itv = m_vehicles.begin();
	while(itv != m_vehicles.end()) {
		SAMPVehicle *veh = *itv;
		peer->VehicleStreamCheck(veh);
		itv++;
	}
	std::vector<SAMPRakPeer *>::iterator it = m_connections.begin();
	while(it != m_connections.end()) {
		SAMPRakPeer *user = *it;
		if(user != peer && user->GetPlayer() && user->GetPlayer()->GetSpawned()) {
			peer->PlayerStreamCheck(user->GetPlayer());
		}
		it++;
	}
	std::vector<SAMPPlayer *>::iterator itb = m_bots.begin();
	while(itb != m_bots.end()) {
		SAMPPlayer *user = *itb;
		if(user->GetSpawned())
			peer->PlayerStreamCheck(user);
		itb++;
	}
}
SAMPRakPeer *SAMPDriver::find_client(struct sockaddr_in *address) {
	std::vector<SAMPRakPeer *>::iterator it = m_connections.begin();
	while(it != m_connections.end()) {
		SAMPRakPeer *peer = *it;
		const struct sockaddr_in *peer_address = peer->getAddress();
		if(address->sin_port == peer_address->sin_port && address->sin_addr.s_addr == peer_address->sin_addr.s_addr) {
			return peer;
		} 
		it++;
	}
	return NULL;
}
SAMPRakPeer *SAMPDriver::find_or_create(struct sockaddr_in *address) {
	std::vector<SAMPRakPeer *>::iterator it = m_connections.begin();
	while(it != m_connections.end()) {
		SAMPRakPeer *peer = *it;
		const struct sockaddr_in *peer_address = peer->getAddress();
		if(address->sin_port == peer_address->sin_port && address->sin_addr.s_addr == peer_address->sin_addr.s_addr) {
			return peer;
		} 
		it++;
	}
	SAMPRakPeer *ret = new SAMPRakPeer(this, address);
	((CHCGameServer *)getServer())->GetScriptInterface()->HandleEvent(CHCGS_ClientConnectEvent, ret, NULL);	
	m_connections.push_back(ret);
	return ret;
}
void SAMPDriver::tick() {

	char recvbuf[1024];

    struct sockaddr_in si_other;
    socklen_t slen = sizeof(struct sockaddr_in);

	int len = recvfrom(m_sd,(char *)&recvbuf,sizeof(recvbuf),0,(struct sockaddr *)&si_other,&slen);


	SAMPHeader *header = (SAMPHeader *)&recvbuf;
	

	if(header->magic != SAMP_MAGIC) {
		SAMPRakPeer *mp_samprak = find_or_create(&si_other);
		mp_samprak->handle_packet((char *)&recvbuf, len);
	} else {
		handle_server_query((char *)&recvbuf, len, &si_other);
	}
}
void SAMPDriver::handle_server_query(char *data, int len, struct sockaddr_in *address_info) {
	SAMPHeader *header = (SAMPHeader *)data;
	

	SAMPPing *ping = (SAMPPing *)(data+sizeof(SAMPHeader)-1);
	switch(header->opcode) {
		case SAMP_PACKET_INFO: {
			handleInfoPacket(m_sd, address_info, header);
			break;
		}
		case SAMP_PACKET_PING:		
			handlePingPacket(m_sd, address_info, header, ping);
			break;
		case SAMP_PACKET_CLIENTS:
			handleClientsPacket(m_sd, address_info, header);
			break;
		case SAMP_PACKET_CLIENTS_DETAILED:
			handleClientsPacket(m_sd, address_info, header, true);
			break;
		case SAMP_PACKET_RULES:
			handleRulesPacket(m_sd, address_info, header);
			break;
	}
}

void SAMPDriver::handleClientsPacket(int sd, struct sockaddr_in *si_other, SAMPHeader *header, bool detailed) {
	char writebuff[10000];
	uint32_t len = 0;

	uint8_t *p = (uint8_t *)&writebuff;
	BufferWriteData(&p, &len, (uint8_t *)header, sizeof(SAMPHeader)-1);


	int num_players = 0;

	BufferWriteShort(&p, &len, num_players);

	
	/*
	std::vector<_PlayerInfo>::iterator it = players.begin();
	int i = 0;

	
	while(it != players.end()) {

		player = *it;

		if(detailed)
		BufferWriteByte(&p, &len, i++); //write player id

		BufferWriteByte(&p, &len, strlen(player.name));

		BufferWriteData(&p, &len, (uint8_t *)&player.name, strlen(player.name));

		BufferWriteInt(&p, &len, player.score);

		if(detailed) {
			int ping = player.ping + rand() % 100;
			BufferWriteInt(&p, &len, ping);
		}
		it++;

	}
	*/


	socklen_t slen = sizeof(struct sockaddr_in);
	sendto(sd,(char *)&writebuff,len,0,(struct sockaddr *)si_other, slen);
}

void SAMPDriver::handleInfoPacket(int sd, struct sockaddr_in *si_other, SAMPHeader *header) {
	char writebuff[256];

	uint32_t len = 0;

	uint8_t *p = (uint8_t *)&writebuff;

	BufferWriteData(&p, &len, (uint8_t *)header, sizeof(SAMPHeader)-1);

	BufferWriteByte(&p,&len, 0); //password

	CHCGameServer *server = ((CHCGameServer *)m_server);

	int num_players = 0;
	int max_players = server->getMaxPlayers();

	BufferWriteShort(&p,&len, num_players); //num players

	BufferWriteShort(&p,&len, max_players); //max players

	const char *server_name = server->getName();
	const char *server_gamemode = server->getVersion();
	const char *server_mapname = server->getMap();

	
	BufferWriteInt(&p, &len, strlen(server_name));

	BufferWriteData(&p, &len, (uint8_t *)server_name, strlen(server_name));

	BufferWriteInt(&p, &len, strlen(server_gamemode));

	BufferWriteData(&p, &len, (uint8_t *)server_gamemode, strlen(server_gamemode));

	BufferWriteInt(&p, &len, strlen(server_mapname));

	BufferWriteData(&p, &len, (uint8_t *)server_mapname, strlen(server_mapname));

	socklen_t slen = sizeof(struct sockaddr_in);
	sendto(sd,(char *)&writebuff,len,0,(struct sockaddr *)si_other, slen);

}

void SAMPDriver::handleRulesPacket(int sd, struct sockaddr_in *si_other, SAMPHeader *header) {
	char writebuff[256];

	uint32_t len = 0;

	uint8_t *p = (uint8_t *)&writebuff;

	BufferWriteData(&p, &len, (uint8_t *)header, sizeof(SAMPHeader)-1);
	
	BufferWriteShort(&p,&len, 0);//rule_list.size());

/*
	std::vector<struct RuleInfo>::iterator it = rule_list.begin();
	while(it != rule_list.end()) {
		struct RuleInfo rule = *it;
		BufferWriteByte(&p,&len, strlen(rule.name));
		BufferWriteData(&p, &len, (uint8_t*)rule.name, strlen(rule.name));

		BufferWriteByte(&p,&len, strlen(rule.value));
		BufferWriteData(&p, &len, (uint8_t*)rule.value, strlen(rule.value));
		it++;
	}
*/

	socklen_t slen = sizeof(struct sockaddr_in);
	sendto(sd,(char *)&writebuff,len,0,(struct sockaddr *)si_other, slen);

}

void SAMPDriver::handlePingPacket(int sd, struct sockaddr_in *si_other, SAMPHeader *header, SAMPPing *ping) {
	char writebuff[256];
	uint32_t len = 0;

	uint8_t *p = (uint8_t *)&writebuff;

	BufferWriteData(&p, &len, (uint8_t *)header, sizeof(SAMPHeader)-1);
	BufferWriteInt(&p, &len, ping->ping);
	
	socklen_t slen = sizeof(struct sockaddr_in);

	sendto(sd,(char *)&writebuff,len,0,(struct sockaddr *)si_other, slen);
}



int SAMPDriver::getListenerSocket() {
	return m_sd;
}

uint16_t SAMPDriver::getPort() {
	return m_port;
}
uint32_t SAMPDriver::getBindIP() {
	return htonl(m_local_addr.sin_addr.s_addr);
}
uint32_t SAMPDriver::getDeltaTime() {
	struct timeval now;
	gettimeofday(&now, NULL);
	uint32_t t = (now.tv_usec/1000.0) - (m_server_start.tv_usec/1000.0);
	return t;
}
int SAMPDriver::createPickup(int model, int type, float x, float y, float z) {
	RakNet::BitStream bs;
	
	SAMPPickup *pickup = (SAMPPickup *)malloc(sizeof(SAMPPickup));
	memset(pickup,0,sizeof(SAMPPickup));

	pickup->iModel = model;
	pickup->iType = type;
	pickup->fX = x;
	pickup->fY = y;
	pickup->fZ = z;
	m_last_pickup_id++;

	m_pickups[m_last_pickup_id] = pickup;

	bs.Write((int32_t)m_last_pickup_id);
	bs.Write((char *)pickup, sizeof(SAMPPickup));

	//SendBitstreamToAll(bs);
	SendRPCToAll(ESAMPRPC_CreatePickup, &bs);

	return m_last_pickup_id;
}
void SAMPDriver::destroyPickup(int id) {
	
	free((void *)m_pickups[id]);

	m_pickups.erase(id);

	RakNet::BitStream bs;
	bs.Write((int32_t)id);

	SendRPCToAll(ESAMPRPC_DestroyPickup, &bs);
	
}


int SAMPDriver::create3DTextLabel(const char *string, uint32_t colour, float x, float y, float z, float draw_distance, bool test_los, int world, int stream_index) {
	m_last_3dlabel_id++;

	SAMP3DLabel *label = (SAMP3DLabel *)malloc(sizeof(SAMP3DLabel));
	memset(label,0,sizeof(SAMP3DLabel));
	label->x = x;
	label->y = y;
	label->z = z;
	label->colour = colour;
	strcpy(label->string, string); //eeee
	label->draw_distance = draw_distance;
	label->test_los = test_los;

	m_3dlabels[m_last_3dlabel_id] = label;

	RakNet::BitStream bs;
	bs.Write((uint16_t)m_last_3dlabel_id);
	bs.Write(colour);
	bs.Write(x);
	bs.Write(y);
	bs.Write(z);
	bs.Write(draw_distance);
	bs.Write((uint8_t)test_los);
	bs.Write((uint16_t)-1); //player id
	bs.Write((uint16_t)-1); //vehicle id

	StringCompressor::Instance()->EncodeString(string, strlen(string)+1, &bs);

	SendRPCToAll(ESAMPRPC_Create3DTextLabel, &bs);
	return m_last_3dlabel_id;
}

int SAMPDriver::CreateVehicle(int modelid, float x, float y, float z, float zrot, uint8_t c1,uint8_t c2, bool respawn_on_death, int respawn_time) {
	SAMPVehicle *veh = (SAMPVehicle *)malloc(sizeof(SAMPVehicle));
	memset(veh, 0, sizeof(SAMPVehicle));

	veh->id = GetFreeVehicleID();
	veh->modelid = modelid;
	veh->health = 1000.0;
	veh->pos[0] = x;
	veh->pos[1] = y;
	veh->pos[2] = z;

	veh->colours[0] = c1;
	veh->colours[1] = c2;

	m_vehicles.push_back(veh);

	printf("Made new car: %d\n", veh->id);

	return veh->id;

}

void SAMPDriver::destroy3DTextLabel(int label) {
	RakNet::BitStream bs;

	bs.Write((uint16_t)m_last_3dlabel_id);
	SendRPCToAll(ESAMPRPC_Delete3DTextLabel, &bs);

	free((void *)m_3dlabels[label]);
	m_3dlabels.erase(label);	
}

void SAMPDriver::SendBitstreamToAll(RakNet::BitStream *bs) {
	std::vector<SAMPRakPeer *>::iterator it = m_connections.begin();
	while(it != m_connections.end()) {
		SAMPRakPeer *peer = *it;
		peer->send_bitstream(bs);
		it++;
	}
}
void SAMPDriver::SendRPCToAll(int rpc_id, RakNet::BitStream *bs) {
	std::vector<SAMPRakPeer *>::iterator it = m_connections.begin();
	while(it != m_connections.end()) {
		SAMPRakPeer *peer = *it;
		peer->send_rpc(rpc_id, bs);
		it++;
	}
}
SAMPPlayer* SAMPDriver::CreateBot() {
	SAMPPlayer *player = new SAMPPlayer(this); 
	player->SetPlayerID(GetFreePlayerID());
	return player;
}
void SAMPDriver::AddBot(SAMPPlayer *bot) {
	m_bots.push_back(bot);
}
void SAMPDriver::SendScoreboard(SAMPRakPeer *peer) {
	std::vector<SAMPRakPeer *>::iterator it = m_connections.begin();
	while(it != m_connections.end()) {
		SAMPRakPeer *user = *it;
		if(user != peer) {
			peer->AddToScoreboard(user->GetPlayer());
		}
		it++;
	}

	std::vector<SAMPPlayer *>::iterator itb = m_bots.begin();
	while(itb != m_bots.end()) {
		SAMPPlayer *user = *itb;
		peer->AddToScoreboard(user);
		itb++;
	}
}
void SAMPDriver::SendRPCToStreamed(SAMPPlayer *player, uint8_t rpc, RakNet::BitStream *stream, bool include_sender) {
	std::vector<SAMPRakPeer *>::iterator it = m_connections.begin();
	while(it != m_connections.end()) {
		SAMPRakPeer *peer = *it;
		if((peer->IsPlayerStreamed(player) || peer->GetPlayer() == player) && (peer->GetPlayer() != player || include_sender)) {
			peer->send_rpc(rpc, stream);
			stream->ResetReadPointer();	
		}
		it++;
	}
}
void SAMPDriver::SendBitstreamToStreamed(SAMPPlayer *player, RakNet::BitStream *stream) {
	std::vector<SAMPRakPeer *>::iterator it = m_connections.begin();
	while(it != m_connections.end()) {
		SAMPRakPeer *peer = *it;
		if(peer->IsPlayerStreamed(player) && peer->GetPlayer() != player) {
			peer->send_bitstream(stream);
			stream->ResetReadPointer();			
		}
		it++;
	}	
}
void SAMPDriver::SendAddPlayerToScoreboard(SAMPPlayer *player) {
	std::vector<SAMPRakPeer *>::iterator it = m_connections.begin();
	while(it != m_connections.end()) {
		SAMPRakPeer *peer = *it;
		if(peer->GetPlayer() != player)
			peer->AddToScoreboard(player);
		it++;
	}	
}
SAMPPlayer *SAMPDriver::findPlayerByID(uint16_t id) {
	std::vector<SAMPRakPeer *>::iterator it = m_connections.begin();
	while(it != m_connections.end()) {
		SAMPRakPeer *peer = *it;
		SAMPPlayer *player = peer->GetPlayer();
		if(player && player->GetPlayerID() == id)
			return player;
		it++;
	}

	std::vector<SAMPPlayer *>::iterator itb = m_bots.begin();
	while(itb != m_bots.end()) {
		SAMPPlayer *player = *itb;
		if(player->GetPlayerID() == id)
			return player;
		itb++;
	}	

	return NULL;
}

SAMPVehicle *SAMPDriver::findVehicleByID(uint16_t id) {
	std::vector<SAMPVehicle *>::iterator it = m_vehicles.begin();
	while(it != m_vehicles.end()) {
		SAMPVehicle *veh = *it;
		if(veh && veh->id == id)
			return veh;
		it++;
	}
	return NULL;
}
SAMPTextDraw *SAMPDriver::FindTextDrawByID(uint16_t id) {
	std::vector<SAMPTextDraw *>::iterator it = m_textdraws.begin();
	while(it != m_textdraws.end()) {
		SAMPTextDraw *td = *it;
		if(td->id == id) {
			return td;
		}
		it++;
	}
	return NULL;
}
uint16_t SAMPDriver::GetFreePlayerID() {
	for(int i=0;i<SAMP_MAX_PLAYERS;i++) {
		if(findPlayerByID(i) == NULL)
			return i;
	}
	return -1;
}

SAMPTextDraw *SAMPDriver::CreateTextDraw() {
	SAMPTextDraw *text = (SAMPTextDraw *)malloc(sizeof(SAMPTextDraw));
	memset(text,0,sizeof(SAMPTextDraw));
	text->id = GetFreeTextDrawID();
	text->text[0] = '_';
	text->x = 0.0f;
	text->y = 0.0f;
	text->font_width = 0.480000f;
	text->font_height = 1.12f;
	text->box_width = 1280.0f;
	text->box_height = 1280.6f;
	text->box_colour = 0x80808080;
	text->font_colour = 0xFF000000;
	text->background_colour = 0xFFFFFFFF;
	//text->flags |= SAMPTD_IsProportional;
	text->style = 1;
	text->zoom = 1.0;
	text->model_colours[0] = 0x0000FFFF;
	text->model_colours[1] = 0x0000FFFF;
	m_textdraws.push_back(text);
	return text;
}
uint16_t SAMPDriver::GetFreeTextDrawID() {
	for(int i=1;i<SAMP_MAX_TEXTDRAWS;i++) {
		if(FindTextDrawByID(i) == NULL) {
			return i;
		}
	}
	return -1;
}
uint16_t SAMPDriver::GetFreeVehicleID() {
	for(int i=1;i<SAMP_MAX_VEHICLES;i++) {
		if(findVehicleByID(i) == NULL)
			return i;
	}
	return -1;
}
void SAMPDriver::SendPlayerUpdate(SAMPPlayer *player) {
	RakNet::BitStream bs;
	bs.Write((uint8_t)ID_PLAYER_SYNC);
	bs.Write(player->GetPlayerID());

	float *pos = player->GetPosition();
	float *quat = player->GetQuat();
	float *speed = player->GetMoveSpeed();
	uint8_t health = (uint8_t)player->GetHealth(), armour = (uint8_t)player->GetArmour();
	uint16_t left_right, up_down, keys;

	uint16_t surf_info = player->GetSurfFlags();
	float *surf_offset = player->GetSurfOffset();

	uint32_t anim = player->GetAnim();

	player->GetKeys(left_right, up_down, keys);

	if(left_right) {
		bs.Write(true);
		bs.Write(left_right);
	} else {
		bs.Write(false);
	}

	if(up_down) {
		bs.Write(true);
		bs.Write(up_down);
	} else {
		bs.Write(false);
	}
	bs.Write(keys);

	bs.Write(pos[0]);
	bs.Write(pos[1]);
	bs.Write(pos[2]);

	bs.WriteNormQuat(quat[0], quat[1], quat[2], quat[3]);

	uint8_t health_armour = 0;
	if(health > 0 && health < 100) {
		health_armour = ((uint8_t)(health / 7)) << 4;
	} else if(health >= 100) {
		health_armour = 0xF << 4;
	}
	if(armour > 0 && armour < 100) {
		health_armour |=  (uint8_t)(armour / 7);
	} else if(armour >= 100) {
		health_armour |= 0xF;
	}

	bs.Write(health_armour);
	bs.Write(player->GetHoldingWeapon());

	bs.Write(player->GetSpecialAction());

	bs.WriteVector(speed[0], speed[1], speed[2]);
	if(surf_info) {
		bs.Write(true);
		bs.Write(surf_info);
		bs.Write(surf_offset[0]);
		bs.Write(surf_offset[1]);
		bs.Write(surf_offset[2]);
	} else {
		bs.Write(false);
	}

	if(anim) {
		bs.Write(true);
		bs.Write(anim);
	} else {
		bs.Write(false);
	}
	SendBitstreamToStreamed(player, &bs);
}
void SAMPDriver::SendVehicleUpdate(SAMPPlayer *player, SAMPVehicle *car) {
	RakNet::BitStream bs;
	uint16_t left_right, up_down, keys;
	uint8_t health = (uint8_t)player->GetHealth(), armour = (uint8_t)player->GetArmour();
	player->GetKeys(left_right, up_down, keys);

	bs.Write((uint8_t)ID_VEHICLE_SYNC);
	bs.Write(player->GetPlayerID());
	bs.Write((uint16_t)car->id);

	bs.Write(left_right);
	bs.Write(up_down);
	bs.Write(keys);

	bs.WriteNormQuat(car->quat[0], car->quat[1], car->quat[2], car->quat[3]);

	bs.Write(car->pos[0]);
	bs.Write(car->pos[1]);
	bs.Write(car->pos[2]);

	bs.WriteVector(car->vel[0], car->vel[1], car->vel[2]);

	bs.Write((uint16_t)car->health);

	uint8_t health_armour = 0;
	if(health > 0 && health < 100) {
		health_armour = ((uint8_t)(health / 7)) << 4;
	} else if(health >= 100) {
		health_armour = 0xF << 4;
	}
	if(armour > 0 && armour < 100) {
		health_armour |=  (uint8_t)(armour / 7);
	} else if(armour >= 100) {
		health_armour |= 0xF;
	}
	bs.Write(health_armour);

	bs.Write(player->GetHoldingWeapon());

	if(car->siren_on)
		bs.Write(true);
	else 
		bs.Write(false);

	if(car->landinggear_state)
		bs.Write(true);
	else
		bs.Write(false);

	//trust angles or trailer ID
	bs.Write(false);
	bs.Write(false);

	bs.Write((uint32_t)0);

	//train stuff
	bs.Write(false);
	SendBitstreamToStreamed(player, &bs);
}
void SAMPDriver::SendPassengerUpdate(SAMPPlayer *player, SAMPVehicle *car) {
	RakNet::BitStream bs;
	/*
		BYTE byteSeatFlags : 7;
	BYTE byteDriveBy : 1;
	BYTE byteCurrentWeapon;
	BYTE bytePlayerHealth;
	BYTE bytePlayerArmour;
	WORD lrAnalog;
	WORD udAnalog;
	WORD wKeys;
	float vecPos[3];
	*/
	uint16_t left_right, up_down, keys;
	float *pos = player->GetPosition();
	bs.Write((uint8_t)ID_PASSENGER_SYNC);
	bs.Write(player->GetPlayerID());
	bs.Write((uint16_t)car->id);
	bs.Write(player->GetSeatFlags());
	bs.Write(player->GetHoldingWeapon());
	bs.Write((uint8_t)player->GetHealth());
	bs.Write((uint8_t)player->GetArmour());
	player->GetKeys(left_right, up_down, keys);
	bs.Write(left_right);
	bs.Write(up_down);
	bs.Write(keys);

	bs.Write(pos[0]);
	bs.Write(pos[1]);
	bs.Write(pos[2]);


	SendBitstreamToStreamed(player, &bs);
}
void SAMPDriver::SendAimSync(SAMPPlayer *player, SAMPAimSync *aim) {
	RakNet::BitStream bs;
	bs.Write((uint8_t)ID_AIM_SYNC);
	bs.Write(player->GetPlayerID());
	bs.Write(aim->cam_mode);
	bs.Write(aim->f1[0]);
	bs.Write(aim->f1[1]);
	bs.Write(aim->f1[2]);
	bs.Write(aim->pos[0]);
	bs.Write(aim->pos[1]);
	bs.Write(aim->pos[2]);
	bs.Write(aim->z);
	bs.WriteBits(&aim->cam_zoom, 6);
	bs.WriteBits(&aim->weapon_state, 2);
	bs.Write(aim->unknown);
	SendBitstreamToStreamed(player, &bs);
}
void SAMPDriver::SendBulletData(SAMPPlayer *player, SAMPBulletData *bullet) {
	RakNet::BitStream bs;
	bs.Write((uint8_t)ID_BULLET_SYNC);
	bs.Write(player->GetPlayerID());
	bs.Write(bullet->type);
	bs.Write(bullet->id);
	bs.Write(bullet->origin[0]);
	bs.Write(bullet->origin[1]);
	bs.Write(bullet->origin[2]);
	bs.Write(bullet->target[0]);
	bs.Write(bullet->target[1]);
	bs.Write(bullet->target[2]);
	bs.Write(bullet->center[0]);
	bs.Write(bullet->center[1]);
	bs.Write(bullet->center[2]);
	bs.Write(bullet->weapon);
	SendBitstreamToStreamed(player, &bs);
}
