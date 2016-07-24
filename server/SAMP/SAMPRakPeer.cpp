#include "SAMPRakPeer.h"
#include "SAMPDriver.h"
#include "encryption.h"
#include <stdio.h>
#include "../CHCGameServer.h"

#include "SAMPPlayer.h"

#include <sys/time.h>

RPCHandler SAMPRakPeer::s_rpc_handler[] = {
	{ESAMPRPC_ClientJoin, &SAMPRakPeer::m_client_join_handler},
	{ESAMPRPC_ClientCommand, &SAMPRakPeer::m_client_command_handler},
	{ESAMPRPC_DialogResponse, &SAMPRakPeer::m_client_dialogresp_handler},
	{ESAMPRPC_ClientSpawned, &SAMPRakPeer::m_client_spawned_handler},
	{ESAMPRPC_EnterVehicle, &SAMPRakPeer::m_client_enter_vehicle_handler},
	{ESAMPRPC_ExitVehicle, &SAMPRakPeer::m_client_exit_vehicle_handler},
	{ESAMPRPC_RequestClass, &SAMPRakPeer::m_client_request_class},
	{ESAMPRPC_RequestSpawn, &SAMPRakPeer::m_client_request_spawn},
	{ESAMPRPC_ChatMessage, &SAMPRakPeer::m_client_chat_message_handler},
	{ESAMPRPC_SelectTextDraw, &SAMPRakPeer::m_textdraw_clicked_handler},
	{ESAMPRPC_Death, &SAMPRakPeer::m_client_death_handler}
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

    m_num_spawn_classes = 0;

    m_delete_flag = false;
    m_timeout_flag = false;
}
SAMPRakPeer::~SAMPRakPeer() {
	printf("Rak peer delete\n");
	if(mp_player) {
		mp_driver->SendRemovePlayerFromScoreboard(this->GetPlayer());
		delete mp_player;
	}
    StringCompressor::RemoveReference();
}
void SAMPRakPeer::handle_packet(char *data, int len) {
	gettimeofday(&m_last_ping, NULL);
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
		if(reliability == RELIABLE || reliability == RELIABLE_SEQUENCED || reliability == RELIABLE_ORDERED)
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


	uint16_t leftright_keys;
	uint16_t updown_keys;
	uint16_t keys;
	float pos[3];
	float quat[4];


	uint8_t health;
	uint8_t armour;
	uint8_t weapon;

	SAMPVehicle *car;

	float movespeed[3];

	uint16_t vehicleid;
	uint8_t seat_flags;

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
		case ID_VEHICLE_SYNC:
			stream->Read(vehicleid);
			car = mp_driver->findVehicleByID(vehicleid);
			if(!car)
				return;
			stream->Read(leftright_keys);
			stream->Read(updown_keys);
			stream->Read(keys);
			stream->Read(car->quat[0]);
			stream->Read(car->quat[1]);
			stream->Read(car->quat[2]);
			stream->Read(car->quat[3]);
			stream->Read(car->pos[0]);
			stream->Read(car->pos[1]);
			stream->Read(car->pos[2]);
			stream->Read(car->vel[0]);
			stream->Read(car->vel[1]);
			stream->Read(car->vel[2]);
			stream->Read(car->health);
			stream->Read(health);
			stream->Read(armour);
			stream->Read(weapon);
			stream->Read(car->siren_on);
			stream->Read(car->landinggear_state);
			stream->Read(car->trailerid_or_thrustangle);
			stream->Read(car->train_speed);

			printf("Car pos: %f %f %f\nHealth: %d %d\n",car->pos[0],car->pos[1],car->pos[2],health,armour);

			mp_player->SetPosition((float *)&car->pos);
			mp_player->SetQuat((float *)&car->quat);
			mp_player->SetHealth((float)health);
			mp_player->SetArmour((float)armour);
			mp_player->SetHoldingWeapon(weapon);
			mp_player->SetKeys(leftright_keys, updown_keys, keys);
			mp_driver->SendVehicleUpdate(mp_player, car);
		break;
		case ID_PASSENGER_SYNC:
			stream->Read(vehicleid);
			car = mp_driver->findVehicleByID(vehicleid);
			if(!car)
				return;
			stream->Read(seat_flags);
			stream->Read(weapon);
			stream->Read(health);
			stream->Read(armour);
			stream->Read(leftright_keys);
			stream->Read(updown_keys);
			stream->Read(keys);
			stream->Read(pos[0]);
			stream->Read(pos[1]);
			stream->Read(pos[2]);

			mp_player->SetPosition((float *)&car->pos);
			mp_player->SetKeys(leftright_keys, updown_keys, keys);
			mp_player->SetHealth((float)health);
			mp_player->SetArmour((float)armour);
			mp_player->SetHoldingWeapon(weapon);
			mp_player->SetSeatFlags(seat_flags);

			mp_driver->SendPassengerUpdate(mp_player, car);
		break;
		case ID_AIM_SYNC:
			SAMPAimSync aimsync;
			stream->Read(aimsync.cam_mode);
			stream->Read(aimsync.f1[0]);
			stream->Read(aimsync.f1[1]);
			stream->Read(aimsync.f1[2]);
			stream->Read(aimsync.pos[0]);
			stream->Read(aimsync.pos[1]);
			stream->Read(aimsync.pos[2]);
			stream->Read(aimsync.z);
			stream->ReadBits(&aimsync.cam_zoom, 6);
			stream->ReadBits(&aimsync.weapon_state, 2);
			stream->Read(aimsync.unknown);
			printf("Aim sync: %d %f %f %f - %f %f %f - %d | %d | %d\n", aimsync.cam_mode,aimsync.pos[0],aimsync.pos[1],aimsync.pos[2], aimsync.f1[0], aimsync.f1[1], aimsync.f1[2], aimsync.cam_zoom, aimsync.weapon_state, aimsync.unknown);
			mp_driver->SendAimSync(mp_player, &aimsync);
		break;
		case ID_BULLET_SYNC:
			SAMPBulletData bulletsync;
			stream->Read(bulletsync.type);
			stream->Read(bulletsync.id);
			stream->Read(bulletsync.origin[0]);
			stream->Read(bulletsync.origin[1]);
			stream->Read(bulletsync.origin[2]);
			stream->Read(bulletsync.target[0]);
			stream->Read(bulletsync.target[1]);
			stream->Read(bulletsync.target[2]);
			stream->Read(bulletsync.center[0]);
			stream->Read(bulletsync.center[1]);
			stream->Read(bulletsync.center[2]);
			stream->Read(bulletsync.weapon);
			printf("Bullet sync: %f %f %f (%d) | %d\n",bulletsync.target[0],bulletsync.target[1],bulletsync.target[2], bulletsync.weapon);
			mp_driver->SendBulletData(mp_player, &bulletsync);
		break;
		case ID_UNOCCUPIED_SYNC:
		break;
		case ID_PLAYER_SYNC:
			uint8_t specialaction;
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
		break;
		case ID_WEAPONS_UPDATE:
		printf("Weapons update - %d\n", BITS_TO_BYTES(stream->GetNumberOfUnreadBits()));
		break;
		case ID_STATS_UPDATE:
		printf("Stats update - %d\n", BITS_TO_BYTES(stream->GetNumberOfUnreadBits()));
		break;
		case ID_DISCONNECTION_NOTIFICATION:
			m_delete_flag = true;
		break;
	}
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
	printf("RPC not found: %d %d\n", rpcid, BITS_TO_BYTES(stream->GetNumberOfBitsUsed()));

}
void SAMPRakPeer::m_textdraw_clicked_handler(RakNet::BitStream *stream) {
	void *val = (void *)-1;
	uint16_t td_id;
	stream->Read(td_id);
	if(td_id != (uint16_t)-1) {
		val = (void *)td_id;
	}

	CHCGameServer *server = (CHCGameServer *)mp_driver->getServer();
	server->GetScriptInterface()->HandleEvent(CHCGS_UIClick, this, val);

}
void SAMPRakPeer::m_client_death_handler(RakNet::BitStream *stream) {
	uint8_t reason;
	uint16_t killer_id;

	SAMPDeathInfo info;

	stream->Read(info.reason);
	stream->Read(info.killer_id);

	printf("Got death: %d - %d\n", reason, killer_id);

	mp_player->SetSpawned(false);


	CHCGameServer *server = (CHCGameServer *)mp_driver->getServer();
	server->GetScriptInterface()->HandleEvent(CHCGS_PlayerDeath, GetPlayer(), (void *)&info);
}
void SAMPRakPeer::m_client_enter_vehicle_handler(RakNet::BitStream *stream) {
	uint16_t carid;
	uint8_t seat;
	stream->Read(carid);
	stream->Read(seat);
	SAMPVehicle *car = mp_driver->findVehicleByID(carid);
	if(car) {
		SendEnterCar(car, seat);
	}
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
		mp_driver->StreamOutForAll(mp_player);
		mp_player->SetSpawned(true);
	}
}
void SAMPRakPeer::m_client_chat_message_handler(RakNet::BitStream *stream) {
	uint32_t len;
	char message[256];
	memset(&message,0, sizeof(message));
	stream->Read((char *)&message, BITS_TO_BYTES(stream->GetNumberOfBitsUsed()));
	CHCGameServer *server = (CHCGameServer *)mp_driver->getServer();
	server->GetScriptInterface()->HandleEvent(CHCGS_ChatMessage, this, (void *)&message);
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
void SAMPRakPeer::m_client_exit_vehicle_handler(RakNet::BitStream *stream) {
	uint16_t carid;
	uint8_t seat;
	stream->Read(carid);
	SAMPVehicle *car = mp_driver->findVehicleByID(carid);
	if(car) {
		SendExitCar(car);
	}
}
void SAMPRakPeer::m_client_request_spawn(RakNet::BitStream *stream) {
	//printf("Rak MSGID: %d/%02x - %d\n",msgid,msgid, BITS_TO_BYTES(stream->GetNumberOfUnreadBits()));
	printf("spawn request Remaining amount: %d\n", BITS_TO_BYTES(stream->GetNumberOfUnreadBits()));
}
void SAMPRakPeer::m_client_request_class(RakNet::BitStream *stream) {
	uint32_t class_id;
	stream->Read(class_id);
	CHCGameServer *server = (CHCGameServer *)mp_driver->getServer();
	server->GetScriptInterface()->HandleEvent(CHCGS_SpawnSelect, this, (void *)class_id);
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
void SAMPRakPeer::ShowTextDraw(SAMPTextDraw *td) {
	RakNet::BitStream bs;
	bs.Write(td->id);
	bs.Write(td->flags);
	bs.Write(td->font_width);
	bs.Write(td->font_height);
	bs.Write(td->font_colour);
	bs.Write(td->box_width);
	bs.Write(td->box_height);
	bs.Write(td->box_colour);
	bs.Write(td->shadow);
	bs.Write(td->outline);
	bs.Write(td->background_colour);
	bs.Write(td->style);
	bs.Write(td->selectable);
	bs.Write(td->x);
	bs.Write(td->y);
	bs.Write(td->model);
	bs.Write(td->rx);
	bs.Write(td->ry);
	bs.Write(td->rz);
	bs.Write(td->zoom);
	bs.Write(td->model_colours[0]);
	bs.Write(td->model_colours[1]);
	int len = strlen(td->text);
	bs.Write((uint16_t)len);
	bs.Write((char *)&td->text, len);

	printf("Sending mdl: %d : %d [%d - %d]\n", td->model, td->style, td->model_colours[0], td->model_colours[1]);

	send_rpc(ESAMPRPC_ShowTextDraw, &bs);
}
void SAMPRakPeer::HideTextDraw(SAMPTextDraw *td) {
	RakNet::BitStream bs;
	bs.Write((uint16_t)td->id);
	send_rpc(ESAMPRPC_HideTextDraw, &bs);
}
void SAMPRakPeer::SelectTextDraw(uint32_t hover_colour, bool cancel) {
	RakNet::BitStream bs;
	if(cancel) {
		hover_colour = 0;
	}
	bs.Write(hover_colour);		
	if(cancel) {
		bs.Write((uint8_t)0x00);
	} else {
		bs.Write((uint8_t)0x80); //flags??
	}
	send_rpc(ESAMPRPC_SelectTextDraw, &bs);
}
void SAMPRakPeer::think() {
	//if(m_got_client_join)
		//send_ping();

	//check for timeout
	struct timeval current_time;
	gettimeofday(&current_time, NULL);
	if(current_time.tv_sec - m_last_ping.tv_sec > SAMP_PING_TIME_SEC) {
		m_delete_flag = true;
		m_timeout_flag = true;
	}

	if(mp_player)
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
	send_rpc(ESAMPRPC_SendClientMessage, &bsData);
	
}
void SAMPRakPeer::SpawnPlayer(float x, float y, float z, int skin, int team, uint32_t *weapons, uint32_t *ammo) {
	RakNet::BitStream bsData;


	bsData.Write((uint8_t)1);
	bsData.Write((uint8_t)team);
	bsData.Write((uint32_t)skin);
	bsData.Write((uint8_t)0);
	bsData.Write(x);
	bsData.Write(y);
	bsData.Write(z);
	bsData.Write(0.0f);


	if(weapons && ammo) {
		//weapons
		bsData.Write((uint32_t)weapons[0]);
		bsData.Write((uint32_t)weapons[1]);
		bsData.Write((uint32_t)weapons[2]);

		//ammo
		bsData.Write((uint32_t)ammo[0]);
		bsData.Write((uint32_t)ammo[1]);
		bsData.Write((uint32_t)ammo[2]);
	}

	else {
		//weapons
		bsData.Write((uint32_t)0);
		bsData.Write((uint32_t)0);
		bsData.Write((uint32_t)0);

		//ammo
		bsData.Write((uint32_t)0);
		bsData.Write((uint32_t)0);
		bsData.Write((uint32_t)0);
	}
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
	os.Write((uint32_t)m_num_spawn_classes); //number of classes
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
	bsData.Write((uint32_t)car->colours[0]); 
	bsData.Write((uint32_t)car->colours[1]);
	bsData.Write((uint8_t)0);

	send_rpc(ESAMPRPC_VehicleCreate, &bsData);	
	SAMPStreamRecord rec;
	rec.data = (void *)car;
	m_streamed_vehicles.push_back(rec);
}
void SAMPRakPeer::SendGameText(const char *msg, uint32_t time_ms, uint32_t style) {
	RakNet::BitStream bs;
	int len = strlen(msg);
	bs.Write(style);
	bs.Write(time_ms);
	bs.Write((uint32_t)len);
	bs.Write(msg, len);
	printf("Sending gamemsg: %s\n", msg);
	send_rpc(ESAMPRPC_ShowGameText ,&bs);
}
void SAMPRakPeer::StreamOutCar(SAMPVehicle *car) {
	RakNet::BitStream bsData;
	std::vector<SAMPStreamRecord>::iterator it = m_streamed_vehicles.begin();
	while(it != m_streamed_vehicles.end()) {
		SAMPStreamRecord rec = *it;
		if(rec.data == (void *)car) {
			m_streamed_vehicles.erase(it);
			break;
		}
		it++;
	}
	bsData.Write(car->id);
	send_rpc(ESAMPRPC_VehicleDelete, &bsData);	
}
bool VecInRadius(float r, float x, float y, float z) {
	return sqrtf(pow(x, 2) + pow(y, 2) + pow(z, 2)) < r;
}
bool SAMPRakPeer::VehicleInStreamRange(SAMPVehicle *car) {
	float dist[3];
	float *pos = GetPlayer()->GetPosition();
	dist[0] = car->pos[0] - pos[0];
	dist[1] = car->pos[1] - pos[1];
	dist[2] = car->pos[2] - pos[2];
	return VecInRadius(m_vehicle_stream_distance, dist[0], dist[1], dist[2]);
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
	if(!mp_player)
		return;
	bool is_streamed = IsPlayerStreamed(car);
	if(!is_streamed) {
		if(PlayerInStreamRange(car)) {
			 StreamInPlayer(car);
		}
	} else {
		if(!PlayerInStreamRange(car)) {
			printf("Streaming out\n");
			StreamOutPlayer(car);
		}
	}
}

bool SAMPRakPeer::PlayerInStreamRange(SAMPPlayer *car) {
	if(!car->GetSpawned() || !GetPlayer()->GetSpawned()) {
		return false;
	}
	float dist[3];
	float *pos = GetPlayer()->GetPosition();
	float *other_pos = car->GetPosition();
	dist[0] = other_pos[0] - pos[0];
	dist[1] = other_pos[1] - pos[1];
	dist[2] = other_pos[2] - pos[2];
	return VecInRadius(m_vehicle_stream_distance, dist[0], dist[1], dist[2]);
}
void SAMPRakPeer::PutInCar(SAMPVehicle *car, uint8_t seat) {
	RakNet::BitStream bs;
	bs.Write(car->id);
	bs.Write(seat);
	send_rpc(ESAMPRPC_PutPlayerInVehicle, &bs);
}
void SAMPRakPeer::SendEnterCar(SAMPVehicle *car, uint8_t seat) {
	RakNet::BitStream bs;
	bs.Write(car->id);
	bs.Write(seat);
	mp_driver->SendRPCToStreamed(mp_player, ESAMPRPC_EnterVehicle, &bs, true);
}
void SAMPRakPeer::SendExitCar(SAMPVehicle *car) {
	RakNet::BitStream bs;
	bs.Write(car->id);
	mp_driver->SendRPCToStreamed(mp_player, ESAMPRPC_ExitVehicle, &bs, true);	
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
			break;
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
void SAMPRakPeer::RemoveFromScoreboard(SAMPPlayer *peer) {
	RakNet::BitStream bs;
	const char *name = peer->GetName();
	int len = strlen(name);
	bs.Write(peer->GetPlayerID());
	bs.Write((uint8_t)peer->GetSAMPPeer()->IsTimeout());
	send_rpc(ESAMPRPC_ServerQuit, &bs);
}
int SAMPRakPeer::GetPing() {
	return 0;
}