#include "SAMPRakPeer.h"
#include "SAMPDriver.h"
#include "encryption.h"
#include <RakNetTime.h>
SAMPRakPeer::SAMPRakPeer(SAMPDriver *driver) {
	mp_driver = driver;
	m_state = ESAMPConnectionState_PreInit;
}
void SAMPRakPeer::handle_packet(char *data, int len, struct sockaddr_in *address_info) {


	sampDecrypt((uint8_t *)data, len, mp_driver->getPort(), 0);


	static RakNet::RakPeer *peer = NULL;
	if(!peer) {

	}
	static FILE *fd = NULL;
	if(!fd) {
		fd = fopen("dump.bin", "wb");
	}
	if(fd) {
		fwrite(data, len, 1, fd);
		fflush(fd);
	}
	printf("Handling packet: %02x %d (%d)\n", data[0], data[0], len);
	int sd = mp_driver->getListenerSocket();
	socklen_t slen = sizeof(struct sockaddr_in);

	if(m_state == ESAMPConnectionState_WaitInitChallenge) {
		m_state = ESAMPConnectionState_PreInit;

	uint8_t peer1_2[] = {
	0xe3, 0x00, 0x00 };
	uint8_t peer1_3[] = {
	0x00, 0x00, 0x42, 0x88, 0x0c, 0x0f, 0x31, 0x42, 
	0x37, 0x30, 0x38, 0x33, 0x37, 0x32, 0x46, 0x34, 
	0x35, 0x32, 0x30, 0x35, 0x00 };
		sendto(sd,(char *)&peer1_2,sizeof(peer1_2),0,(struct sockaddr *)address_info, slen);
		sendto(sd,(char *)&peer1_3,sizeof(peer1_3),0,(struct sockaddr *)address_info, slen);
		return;
	}

	switch(data[0]) {
		case ID_OPEN_CONNECTION_REQUEST:
			static int count = 0;

			char c[3];
			if(count++ < 1) {
				c[0] = ID_OPEN_CONNECTION_COOKIE;
				*((uint16_t*)&c[1]) = *((uint16_t *)&data[1]) ^ 0x6969;
				sendto(sd,(char *)&c,3,0,(struct sockaddr *)address_info, slen);
			} else {
				c[0] = ID_OPEN_CONNECTION_REPLY;
				c[1] = 0;
				sendto(sd,(char *)&c,2,0,(struct sockaddr *)address_info, slen);
				m_state = ESAMPConnectionState_WaitInitChallenge;
			}
			

			
			break;
	}


}