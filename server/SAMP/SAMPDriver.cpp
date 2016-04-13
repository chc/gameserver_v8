#include "SAMPDriver.h"
#include <stdio.h>

#include <server/CHCGameServer.h>
#include <buffwriter.h>

#include "SAMPRakPeer.h"
#include "SAMPPlayer.h"

#define SAMP_MAX_PLAYERS 5

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
	std::vector<SAMPVehicle *>::iterator itv = m_vehicles.begin();
	while(itv != m_vehicles.end()) {
		SAMPVehicle *veh = *itv;
		peer->VehicleStreamCheck(veh);
		itv++;
	}
	std::vector<SAMPRakPeer *>::iterator it = m_connections.begin();
	while(it != m_connections.end()) {
		SAMPRakPeer *user = *it;
		if(user != peer) {
			peer->PlayerStreamCheck(user->GetPlayer());
		}
		it++;
	}
	std::vector<SAMPPlayer *>::iterator itb = m_bots.begin();
	while(itb != m_bots.end()) {
		SAMPPlayer *user = *itb;
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

	veh->id = 1999;
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
	player->SetPlayerID(666);
	return player;
}
void SAMPDriver::AddBot(SAMPPlayer *bot) {
	printf("Add bot\n");
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
void SAMPDriver::SendRPCToStreamed(SAMPPlayer *player, uint8_t rpc, RakNet::BitStream *stream) {
	std::vector<SAMPRakPeer *>::iterator it = m_connections.begin();
	while(it != m_connections.end()) {
		SAMPRakPeer *peer = *it;
		if(peer->IsPlayerStreamed(player)) {
			peer->send_rpc(rpc, stream);
		}
		it++;
	}
}