#include "stdafx.h"
#include "poll.h"

// �迭�� ���� �� �ϳ� �̻��� �̺�Ʈ�� ����ų ������ ��ٸ���.
// ���⼭ timeOut�� ��ٸ��� �ִ� �ð��� ���Ѵ�.
int Poll(PollFD* fdArray, int fdArrayLength, int timeOut)
{
	// PollFD�� mPollfd�� ������ �ٸ� ����� �� ��츦 ��Ƴ���.
	// �̰��� ������ ��� �ٸ� �迭�� �����ϴ� ���� �۾��� �ؾ��Ѵ�.
	static_assert(sizeof(fdArray[0]) == sizeof(fdArray[0].mPollfd), "");

	return WSAPoll((WSAPOLLFD*)fdArray, fdArrayLength, timeOut);
}
