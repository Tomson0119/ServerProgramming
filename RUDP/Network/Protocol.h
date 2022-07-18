#pragma once

using ullong = unsigned long long;

enum pck_type : char {
	ACK,
	DATA
};

#pragma pack(push, 1)
struct packet_header {
	pck_type type;
	ullong packet_id;
};

struct packet_ack : packet_header {
	
};

struct packet_data : packet_header {
	int value;
};
#pragma pack(pop)