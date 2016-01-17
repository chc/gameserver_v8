#include "SAMPDriver.h"
#include <stdio.h>

#include <server/CHCGameServer.h>
#include <buffwriter.h>

#define SAMP_MAX_PLAYERS 5

SAMPDriver::SAMPDriver(INetServer *server, const char *host, uint16_t port) : INetDriver(server) {
    struct sockaddr_in local_addr;
	

	#ifdef _WIN32
    WSADATA wsdata;
    WSAStartup(MAKEWORD(1,0),&wsdata);
	#endif

	uint32_t bind_ip = INADDR_ANY;

	if((m_sd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0){
		//signal error
    }
	mp_samprak = new SAMPRakPeer(this);

	m_port = port;

    local_addr.sin_port = htons(port);
    local_addr.sin_addr.s_addr = htonl(bind_ip);
    local_addr.sin_family = AF_INET;

	int n = bind(m_sd, (struct sockaddr *)&local_addr, sizeof local_addr);
    if(n < 0) {
        //signal error
    }
}
SAMPDriver::~SAMPDriver() {

}

void SAMPDriver::tick() {

	char recvbuf[1024];

    struct sockaddr_in si_other;
    socklen_t slen = sizeof(struct sockaddr_in);
	int len = recvfrom(m_sd,(char *)&recvbuf,sizeof(recvbuf),0,(struct sockaddr *)&si_other,&slen);


	SAMPHeader *header = (SAMPHeader *)&recvbuf;
	
	if(header->magic != SAMP_MAGIC) {
		printf("Got raknet stuff\n");
		mp_samprak->handle_packet((char *)&recvbuf, len, &si_other);
		
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