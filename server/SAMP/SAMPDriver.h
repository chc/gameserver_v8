#ifndef _SAMPDRIVER_H
#define _SAMPDRIVER_H
#include <stdint.h>
#include "main.h"
#include "NetDriver.h"
#include "encryption.h"

#include "SAMPRakPeer.h"

#include <vector>
#include <sys/time.h>


#define SAMP_MAGIC 0x504d4153

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
private:

	//samp query stuff
	void handle_server_query(char *data, int len, struct sockaddr_in *address_info);
	void handlePingPacket(int sd, struct sockaddr_in *si_other, SAMPHeader *header, SAMPPing *ping);
	void handleClientsPacket(int sd, struct sockaddr_in *si_other, SAMPHeader *header, bool detailed = false);
	void handleInfoPacket(int sd, struct sockaddr_in *si_other, SAMPHeader *header);
	void handleRulesPacket(int sd, struct sockaddr_in *si_other, SAMPHeader *header);
	
	const char *m_host;
	uint16_t m_port;
	int m_sd;
	std::vector<SAMPRakPeer *> m_connections;
	struct sockaddr_in m_local_addr;

	struct timeval m_server_start;

};
#endif //_SAMPDRIVER_H