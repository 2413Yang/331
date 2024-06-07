#ifndef TIMER_H
#define TIMER_H

#include <map>
#include <signal.h>
#ifdef _QNX_TARGET_

#include <sys/netmgr.h>
#include <sys/neutrino.h>
#endif

#include "TemplateThread.h"

namespace ZH
{
	namespace BaseLib
	{
		class CTimer
		{
			friend ZH::BaseLib::CTemplateThread<CTimer>;

		public:
			CTimer();
			virtual ~CTimer();
			void SetTimer(int iMilliSecond);
			void KillTimer();
			bool IsRuning() { return !m_bPause; }
			virtual void OnTimer() = 0;

		private:
			void TimerThread();

		private:
			ZH::BaseLib::CTemplateThread<CTimer> m_oRecvPulseThread;
			bool m_bExit;
			bool m_bPause;

			int m_iChannelID;
			struct sigevent m_stEvent;

			timer_t m_iTimeID;
		};
	} // namespace BaseLib
} // namespace ZH

#endif