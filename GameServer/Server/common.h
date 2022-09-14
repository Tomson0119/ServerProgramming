#pragma once

#include <stdafx.h>
#include <Socket.h>
#include <EndPoint.h>
#include <NetException.h>
#include <BufferQueue.h>
#include <Protocol.h>
#include <IOCP.h>
#include <Timer.h>
#include <MemoryPoolManager.h>

namespace Helper
{
	inline bool IsNPC(int id)
	{
		return (id >= NPC_ID_START && id <= NPC_ID_END);
	}

	inline bool IsNear(const PlayerInfo& a_info, const PlayerInfo& b_info)
	{
		if (ABS(a_info.x - b_info.x) > RANGE) return false;
		if (ABS(a_info.y - b_info.y) > RANGE) return false;
		return true;
	}

	inline void MovePosition(std::atomic<short>& x, std::atomic<short>& y, char direction)
	{
		switch (direction)
		{
		case 0: if (y > 0) y--; break;
		case 1: if (y < WORLD_HEIGHT - 1) y++; break;
		case 2: if (x > 0) x--; break;
		case 3: if (x < WORLD_WIDTH - 1) x++; break;
		}
	}

	inline int GetDirectionToTarget(const PlayerInfo& my_info, const PlayerInfo& target_info)
	{
		int abs_x = ABS(target_info.x - my_info.x);
		int abs_y = ABS(target_info.y - my_info.y);
		
		if (abs_x == 0 && abs_y == 0)
			return -1;

		int direction = -1;
		if (abs_x < abs_y)
		{
			direction = (target_info.y < my_info.y) ? 0 : 1;
		}
		else
		{
			direction = (target_info.x < my_info.x) ? 2 : 3;
		}
		return direction;
	}
}