#pragma once


class PollFD
{
public:
	WSAPOLLFD mPollfd;
};

int Poll(PollFD* fdArray, int fdArrayLength, int timeOut);