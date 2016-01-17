#include "SAMPRakPeer.h"
#include "SAMPDriver.h"
#include "encryption.h"
#include <RakNetTime.h>
SAMPRakPeer::SAMPRakPeer(SAMPDriver *driver) {
	mp_driver = driver;
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
	RakNet::BitStream inBitStream( (unsigned char *) data, len, false );
	RakNet::BitStream outBitStream;
/*
	char c[2];
	c[0] = ID_CONNECTION_BANNED;
	c[1] = 0; // Pad, some routers apparently block 1 byte packets

	int sd = mp_driver->getListenerSocket();

	socklen_t slen = sizeof(struct sockaddr_in);
	sendto(sd,(char *)&c,2,0,(struct sockaddr *)address_info, slen);

		// These are all messages from unconnected systems.  Messages here can be any size, but are never processed from connected systems.
		if ( ( (unsigned char) data[ 0 ] == ID_PING_OPEN_CONNECTIONS
			|| (unsigned char)(data)[0] == ID_PING)	&& length == sizeof(unsigned char)+sizeof(RakNetTime) )
		{
			if ( (unsigned char)(data)[0] == ID_PING ||
				rakPeer->AllowIncomingConnections() ) // Open connections with players
			{
#if !defined(_COMPATIBILITY_1)
				RakNet::BitStream inBitStream( (unsigned char *) data, length, false );
				inBitStream.IgnoreBits(8);
				RakNetTime sendPingTime;
				inBitStream.Read(sendPingTime);

				RakNet::BitStream outBitStream;
				outBitStream.Write((unsigned char)ID_PONG); // Should be named ID_UNCONNECTED_PONG eventually
				outBitStream.Write(sendPingTime);
				//tempBitStream.Write( data, UnconnectedPingStruct_Size );
				rakPeer->rakPeerMutexes[ RakPeer::offlinePingResponse_Mutex ].Lock();
				// They are connected, so append offline ping data
				outBitStream.Write( (char*)rakPeer->offlinePingResponse.GetData(), rakPeer->offlinePingResponse.GetNumberOfBytesUsed() );
				rakPeer->rakPeerMutexes[ RakPeer::offlinePingResponse_Mutex ].Unlock();


	*/
	switch(data[0]) {
		case ID_PING:
		case ID_PING_OPEN_CONNECTIONS:
				inBitStream.IgnoreBits(8); //skip type
				uint32_t sendPingTime;
				inBitStream.Read(sendPingTime);
				printf("Ping time is: %d\n", sendPingTime);
				//if(sendPingTime == 0)
					sendPingTime = time(NULL);

				outBitStream.Write((unsigned char)ID_PONG); // Should be named ID_UNCONNECTED_PONG eventually
				outBitStream.Write((uint16_t)sendPingTime);

				rakPeerMutexes[ RakPeer::offlinePingResponse_Mutex ].Lock();
				// They are connected, so append offline ping data
				//outBitStream.Write( (char*)offlinePingResponse.GetData(), offlinePingResponse.GetNumberOfBytesUsed() );
				rakPeerMutexes[ RakPeer::offlinePingResponse_Mutex ].Unlock();

				sendto(sd,outBitStream.GetData(), outBitStream.GetNumberOfBytesUsed(), 0,(struct sockaddr *)address_info, slen);
				
				printf("Send ping resp %d\n",outBitStream.GetNumberOfBytesUsed());
		break;
		case ID_TIMESTAMP:
		
		break;
		default:
		printf("Got unknown thing: %02x %d\n",data[0],data[0]);
		break;
	}
}