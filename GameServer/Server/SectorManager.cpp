#include "common.h"
#include "SectorManager.h"


SectorManager::SectorManager(int secWidth, int secHeight)
	: mSectorWidth{ secWidth }, mSectorHeight{ secHeight }
{
	mSectorArray.resize(mSectorHeight, 
		std::vector<std::unordered_set<int>>(mSectorWidth));
}

void SectorManager::InsertID(int id, int posx, int posy)
{
	auto sector = GetSectorIndex(posx, posy);

	std::unique_lock<std::mutex> lock{ mSectorMut };
	mSectorArray[sector.first][sector.second].insert(id);
}

void SectorManager::EraseID(int id, int posx, int posy)
{
	auto sector = GetSectorIndex(posx, posy);

	std::unique_lock<std::mutex> lock{ mSectorMut };
	mSectorArray[sector.first][sector.second].erase(id);
}

void SectorManager::MoveID(int id, int prevx, int prevy, int nextx, int nexty)
{
	if (prevx == nextx && prevy == nexty) return;

	std::unique_lock<std::mutex> lock{ mSectorMut };
	auto prevIdx = GetSectorIndex(prevx, prevy);
	auto nextIdx = GetSectorIndex(nextx, nexty);
	mSectorArray[prevIdx.first][prevIdx.second].erase(id);
	mSectorArray[nextIdx.first][nextIdx.second].insert(id);
}

std::pair<int, int> SectorManager::GetSectorIndex(int posx, int posy)
{
	return { posy / mSectorHeight, posx / mSectorWidth };
}

std::vector<std::pair<int,int>> SectorManager::GetNearSectorIndexes(int posx, int posy)
{
	auto currIdx = GetSectorIndex(posx, posy);
	
	// get boundaries of current sector
	int secTop = mSectorHeight * currIdx.first;
	int secBottom = secTop + mSectorHeight - 1;
	int secLeft = mSectorWidth * currIdx.second;
	int secRight = secLeft + mSectorWidth - 1;

	// get boundaries of sight
	int sightTop = posy - RANGE;
	int sightBottom = posy + RANGE;
	int sightLeft = posx - RANGE;
	int sightRight = posx + RANGE;

	std::vector<std::pair<int, int>> sectorIdxes = { currIdx };

	// append side sectors
	if (sightTop >= 0 && sightTop < secTop)
	{
		// top-left corner sector
		if (sightLeft >= 0 && sightLeft < secLeft)
		{
			sectorIdxes.push_back({ currIdx.first - 1, currIdx.second - 1 });
		}
		sectorIdxes.push_back({ currIdx.first - 1, currIdx.second });
	}
	if (sightLeft >= 0 && sightLeft < secLeft)
	{
		// left-bottom corner sector
		if (sightBottom < WORLD_HEIGHT && sightBottom > secBottom)
		{
			sectorIdxes.push_back({ currIdx.first + 1, currIdx.second - 1 });
		}
		sectorIdxes.push_back({ currIdx.first, currIdx.second - 1 });
	}
	if (sightBottom < WORLD_HEIGHT && sightBottom > secBottom)
	{
		// bottom-right corner sector
		if (sightRight < WORLD_WIDTH && sightRight > secRight)
		{
			sectorIdxes.push_back({ currIdx.first + 1, currIdx.second + 1 });
		}
		sectorIdxes.push_back({ currIdx.first + 1, currIdx.second });
	}
	if (sightRight < WORLD_WIDTH && sightRight > secRight)
	{
		// right-top corner sector
		if (sightTop >= 0 && sightTop < secTop)
		{
			sectorIdxes.push_back({ currIdx.first - 1, currIdx.second + 1 });
		}
		sectorIdxes.push_back({ currIdx.first, currIdx.second + 1 });
	}
	return sectorIdxes;
}

std::unordered_set<int> SectorManager::GetIDsInSector(int row, int col)
{
	std::unique_lock<std::mutex> lock{ mSectorMut };
	return mSectorArray[row][col];
}