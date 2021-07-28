#include "stdafx.h"
#include "poll.h"

// 배열의 소켓 중 하나 이상이 이벤트를 일으킬 떄까지 기다린다.
// 여기서 timeOut은 기다리는 최대 시간을 말한다.
int Poll(PollFD* fdArray, int fdArrayLength, int timeOut)
{
	// PollFD에 mPollfd를 제외한 다른 멤버가 들어간 경우를 잡아낸다.
	// 이것이 실패할 경우 다른 배열로 복사하는 등의 작업을 해야한다.
	static_assert(sizeof(fdArray[0]) == sizeof(fdArray[0].mPollfd), "");

	return WSAPoll((WSAPOLLFD*)fdArray, fdArrayLength, timeOut);
}
