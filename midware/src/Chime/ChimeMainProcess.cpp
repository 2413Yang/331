#include "ChimeDataInput.h"
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <termio.h>

using namespace chime;

std::mutex mLogMutex;

void SigExit(int sig)
{
	printf("===== ERROR SIG_TYPE: %d =====\n", sig);
	printf("===== ERROR NO: %d, %s =====\n", errno, strerror(errno));
	signal(SIGINT, SIG_IGN);
	kill(0, SIGINT);
	exit(-1);
}

int main(int /*argc*/, const char** /*argv*/)
{
	#ifdef SIGQUIT
	signal(SIGQUIT, SigExit);
#endif
	signal(SIGINT, SigExit);
	signal(SIGTERM, SigExit);
#ifdef SIGUP
	signal(SIGHUP, SigExit);
#endif
	signal(SIGSEGV, SigExit);
#ifdef SIGBUS
	signal(SIGBUS, SigExit);
#endif
#ifdef SIGKILL
	signal(SIGKILL, SigExit);
#endif
	signal(SIGFPE, SigExit);

	CChimeDataInput chimeIputObj;
	do
	{
		sleep(10);
	} while (true);
	
	
	return 0;
}