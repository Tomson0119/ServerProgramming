#pragma once

const char* const SERVER_IP = "127.0.0.1";
const short SERVER_PORT = 5505;

const int MaxBufferSize = 256;

const int  WORLD_HEIGHT = 8;
const int  WORLD_WIDTH = 8;
const int  MAX_USER = 10;

const char CS_PACKET_LOGIN = 1;
const char CS_PACKET_MOVE = 2;
const char CS_PACKET_QUIT = 3;

const char SC_PACKET_LOGIN_OK = 1;
const char SC_PACKET_MOVE = 2;
const char SC_PACKET_PUT_OBJECT = 3;
const char SC_PACKET_REMOVE_OBJECT = 4;

#pragma pack (push, 1)
struct cs_packet_login 
{
	unsigned char size;
	char	      type;
};

struct cs_packet_move 
{
	unsigned char size;
	char          type;
	char          direction;  // 0 : up,  1: down, 2:left, 3:right
};

struct cs_packet_quit
{
	unsigned char size;
	char          type;
};

struct sc_packet_login_ok 
{
	unsigned char size;
	char		  type;
	int			  id;
	short		  x, y;
};

struct sc_packet_move 
{
	unsigned char size;
	char		  type;
	int			  id;
	short		  x, y;
};

struct sc_packet_put_object
{
	unsigned char size;
	char		  type;
	int			  id;
	short		  x, y;
	char		  object_type;
};

struct sc_packet_remove_object
{
	unsigned char size;
	char		  type;
	int			  id;
};
#pragma pack(pop)


struct PlayerCoord
{
	short Col;
	short Row;
};