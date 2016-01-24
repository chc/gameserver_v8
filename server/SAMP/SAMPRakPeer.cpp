#include "SAMPRakPeer.h"
#include "SAMPDriver.h"
#include "encryption.h"
#include <stdio.h>

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
	{ESAMPRPC_ClientJoin, &SAMPRakPeer::m_client_join_handler}
};

SAMPRakPeer::SAMPRakPeer(SAMPDriver *driver) {
	mp_driver = driver;
	m_state = ESAMPConnectionState_ConnectionRequest;
	m_cookie_challenge = 0x1111;
	m_packet_sequence = 0;
	m_player_id = 0;
	m_got_client_join = false;
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
		printf("**** Packet %d - %d\n",seqid, BITS_TO_BYTES(is.GetNumberOfUnreadBits()));
		is.ReadBits(&reliability, 4, true);
		
		//printf("reliability: %d\n",reliability);
		if(reliability == UNRELIABLE_SEQUENCED || reliability == RELIABLE_SEQUENCED || reliability == RELIABLE_ORDERED ) {
			uint8_t orderingChannel = 0;
			uint16_t orderingIndexType;
			is.ReadBits((unsigned char *)&orderingChannel, 5, true);
			is.Read(orderingIndexType);
			printf("**** reliability: %d - (chan: %d  index: %d) (%d)\n",reliability,orderingChannel, orderingIndexType, len);


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
	RakNet::BitStream os(1024);
	uint8_t msgid;
	stream->Read(msgid);
	printf("Rak MSGID: %d/%02x - %d\n",msgid,msgid, BITS_TO_BYTES(stream->GetNumberOfUnreadBits()));
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

			os.Write((uint8_t)ID_CONNECTED_PONG);
			os.Write(ping_cookie);
			os.Write((uint32_t)time(NULL));
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
		case ID_NEW_INCOMING_CONNECTION:
		//printf("New connection");
		break;
	}
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
			(this->*s_rpc_handler[i].handler)(&bs);
		}
	}
	

}
void SAMPRakPeer::send_connection_accepted(bool success) {
	RakNet::BitStream bs;
	bs.Write((uint8_t)ID_CONNECTION_REQUEST_ACCEPTED);
	bs.Write((uint32_t)(m_address_info.sin_addr.s_addr));
	bs.Write((uint16_t)htons(m_address_info.sin_port));
	uint32_t challenge = 0x62B44F34;
	bs.Write(m_player_id);
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
void SAMPRakPeer::send_bitstream(RakNet::BitStream *stream) {
	PacketReliability reliability = RELIABLE;
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




void SAMPRakPeer::m_client_join_handler(RakNet::BitStream *stream) {
	uint32_t netver;
	char name[24];
	uint32_t challenge;
	char version[24];
	uint8_t mod;
	char auth[4*16];
	uint8_t auth_len;
	memset(&auth,0,sizeof(auth));
	memset(&version,0,sizeof(auth));
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

	//printf("Name: %s\nGPCI: %s\nVersion: %s\n",name,auth,version);

	m_got_client_join = true;
	char b = 0;
	//set_static_data(NULL, 0);
	send_player_update();
	send_game_init();
	
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
}
void SAMPRakPeer::send_ping() {
	
	static int last_ping = 0;

	if(time(NULL)-last_ping > 2) {
		int atime = time(NULL);
		
		RakNet::BitStream bs;
		bs.Write((uint8_t)ID_INTERNAL_PING);
		bs.Write(atime);
		send_bitstream(&bs);
		printf("Sending ping : %d\n", atime);
		last_ping = time(NULL);
	}	
}
void SAMPRakPeer::send_fake_players() {
	printf("Send fake players\n");

	RakNet::BitStream bsData;
	PLAYER_SPAWN_INFO psInfo;
	memset(&psInfo, 0, sizeof(psInfo));
	psInfo.byteTeam = 0xFF;
	psInfo.iSkin = 33;
	psInfo.vecPos[0] = 389.8672f;
	psInfo.vecPos[1] = 2543.0046f;
	psInfo.vecPos[2] = 16.5391f;
	psInfo.fRotation = 90.0f;
	psInfo.iSpawnWeapons[0] = 38;
	psInfo.iSpawnWeaponsAmmo[0] = 69;

	bsData.Write((uint8_t)1);
	bsData.Write((char *)&psInfo, sizeof(psInfo));
	send_rpc(128, &bsData);

	bsData.Reset();
	
	bsData.Write((uint32_t)2);
	send_rpc(129, &bsData);
	
	bsData.Reset();
	bsData.Write(0xFF00FFFF);
	const char *str = "Hello there";
	uint32_t len = strlen(str);
	bsData.Write(len);
	bsData.Write(str,len);
	send_rpc(93, &bsData);
	
	/*
	char name[24];
	for(uint16_t i=0;i<1000;i++) {
		RakNet::BitStream bs;
		sprintf(name,"Player_%d",i);
		bs.Write(i);
		bs.Write((uint32_t)0); //colour, 0 = client chooses
		bs.Write((uint8_t)0); //isNPC
		bs.Write((uint8_t)strlen(name));
		bs.Write(name, strlen(name));
		send_rpc(137, &bs);
	}
	*/
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
	uint32_t t = time(NULL);
	os.Write((uint32_t)6990);
	os.Write(t);
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
	os.Write(m_player_id);
	os.WriteCompressed(true); //show nametags
	os.Write((uint32_t)0); //show player markers
	os.Write((uint8_t)12); //server hour
	os.Write((uint8_t)1); //server weather
	os.Write(0.08f); //gravity
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

	for(int i=0;i<212;i++) {
		os.Write((uint8_t)1);
	}
	send_rpc(139, &os);
}