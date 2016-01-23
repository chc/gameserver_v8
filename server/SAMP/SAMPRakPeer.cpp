#include "SAMPRakPeer.h"
#include "SAMPDriver.h"
#include "encryption.h"

#include <stdio.h>


SAMPRakPeer::SAMPRakPeer(SAMPDriver *driver) {
	mp_driver = driver;
	m_state = ESAMPConnectionState_ConnectionRequest;
	m_cookie_challenge = 0x1111;
	m_packet_sequence = 0;
}
void SAMPRakPeer::handle_packet(char *data, int len, struct sockaddr_in *address_info) {

	memcpy(&m_address_info,address_info, sizeof(m_address_info));
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
	is.ReadBits((unsigned char *)&hasacks, 1);

	if(hasacks) {
		DataStructures::RangeList<uint16_t> acknowlegements;
		acknowlegements.Deserialize(&is);
		//printf("**** Num acknowlegements: %d\n",acknowlegements.Size());
	}
	while(BITS_TO_BYTES(is.GetNumberOfUnreadBits()) > 1) {
		is.Read(seqid);
		//printf("**** Packet %d - %d\n",seqid, len);
		is.ReadBits(&reliability, 4, true);
		
		//printf("reliability: %d\n",reliability);
		if(reliability == UNRELIABLE_SEQUENCED || reliability == RELIABLE_SEQUENCED || reliability == RELIABLE_ORDERED ) {
			uint8_t orderingChannel = 0;
			uint16_t orderingIndexType;
			is.ReadBits((unsigned char *)&orderingChannel, 5, true);
			is.Read(orderingIndexType);
			//	printf("**** reliability: %d - (chan: %d  index: %d) (%d)\n",reliability,orderingChannel, orderingIndexType, len);


		}

		bool is_split_packet;
		//is.ReadBits((unsigned char *)&is_split_packet, 1);
		is.Read(is_split_packet);

		if(is_split_packet) {
			uint16_t split_packet_id;
			uint32_t split_packet_index, split_packet_count;
			is.Read(split_packet_id);
			is.ReadCompressed(split_packet_index);
			is.ReadCompressed(split_packet_count);
			printf("**** Split: (%d) %d %d\n", split_packet_id, split_packet_index, split_packet_count);
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
	uint8_t msgid;
	stream->Read(msgid);
	printf("Rak MSGID: %d/%02x\n",msgid,msgid);
	switch(msgid) {
		case ID_CONNECTION_REQUEST:
		send_samp_rakauth("38B95A7E7B53507F");
		break;
		case ID_AUTH_KEY:
		uint8_t auth_len;
		uint8_t auth[32];
		memset(&auth,0,sizeof(auth));
		stream->Read(auth_len);
		stream->Read((char *)&auth, auth_len);
		printf("Client Auth key: %s\n",auth);
		send_samp_rakauth(true);
		m_samprak_auth = true;
		break;
	}
}
void SAMPRakPeer::send_samp_rakauth(bool success) {
	RakNet::BitStream bs;
	bs.Write((uint8_t)ID_CONNECTION_REQUEST_ACCEPTED);
	bs.Write(mp_driver->getPort());
	bs.Write(mp_driver->getBindIP());
	uint16_t player_id = 0;
	uint32_t challenge = 0;
	bs.Write(player_id);
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
			printf("Send open conn reply\n");
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
void SAMPRakPeer::send_bitstream(RakNet::BitStream *stream) {
	PacketReliability reliability = UNRELIABLE;
	RakNet::BitStream os;
	os.Write(false); //has no acks
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