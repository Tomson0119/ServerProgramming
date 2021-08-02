#include <netCommon.h>
#include <iostream>

using namespace std;

constexpr int ServerPort = 5505;

enum class CustomMsgTypes : uint32_t
{
	FireBullet,
	MovePlayer
};

class CustomClient : public net::client_side<CustomMsgTypes>
{
public:
	bool FireBullet(float x, float y)
	{
		net::message<CustomMsgTypes> msg;
		msg.header.id = CustomMsgTypes::FireBullet;
		msg << x << y;
		Send(msg);
	}
};

int main()
{
	CustomClient client;
	client.Connect("127.0.0.1", 5505);
	client.FireBullet(10.0f, 20.0f);
}