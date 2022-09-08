#pragma once

class SectorManager
{
public:
	SectorManager(int secWidth, int secHeight);
	virtual ~SectorManager() = default;

public:
	void InsertID(int id, int posx, int posy);
	void EraseID(int id, int posx, int posy);
	void MoveID(int id, int prevx, int prevy, int nextx, int nexty);

	std::pair<int, int> GetSectorIndex(int posx, int posy);
	std::vector<std::pair<int,int>> GetNearSectorIndexes(int posx, int posy);

	std::unordered_set<int> GetIDsInSector(int row, int col);

private:
	int mSectorWidth;
	int mSectorHeight;

	std::mutex mSectorMut;
	std::vector<std::vector<std::unordered_set<int>>> mSectorArray;
};