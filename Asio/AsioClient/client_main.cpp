#include "netCommon.h"

using namespace std;

enum class CustomMsgTypes : uint32_t
{
	FireBullet,
	MovePlayer
};

class CustomClient : public net::c

int main()
{
	net::message<CustomMsgTypes> msg;
	msg.header.id = CustomMsgTypes::FireBullet;

	int a = 1;
	bool b = true;
	int c = 10;

	msg << a << b << c;
	
	msg >> c >> b >> a;

	cout << a << b << c;
}