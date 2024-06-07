#ifndef TEMPLATE_TIMER_H
#define TEMPLATE_TIMER_H

#include "Timer.h"
#include "Mutex.h"
#include <vector>
#include <functional>

namespace ZH
{
	namespace BaseLib
	{
		template <typename T>
		class CTemplateTimer : public CTimer
		{
		public:
			typedef void (T::*ProcessTimeOut)();

		public:
			CTemplateTimer(T *pTimerTaker, ProcessTimeOut pFunc) : m_pTimerTaker(pTimerTaker), m_pFunc(pFunc) {}

			virtual void SetTimer(int iMilliSecond)
			{
				CTimer::SetTimer(iMilliSecond);
			};
			virtual void KillTimer() { CTimer::KillTimer(); };

			bool IsRunning() { return CTimer::IsRuning(); };

		private:
			virtual void OnTimer()
			{
				if (NULL != m_pTimerTaker && NULL != m_pFunc)
				{
					(m_pTimerTaker->*m_pFunc)();
				}
			};

		private:
			T *m_pTimerTaker;
			ProcessTimeOut m_pFunc;
		};

		class CMultitaskTemplateTimer
			: public CTemplateTimer<CMultitaskTemplateTimer>
		{
			struct StuTimeProgress
			{
				unsigned long long timer;
				unsigned long cycle, onlyTag;
			};

		public:
			CMultitaskTemplateTimer() : CTemplateTimer(NULL, NULL) {}
			~CMultitaskTemplateTimer() { this->KillTimer(); }

			void addTask(unsigned long time, std::function<void(void)> task, unsigned long tag)
			{
				CAutoLock lock(mMtx);
				mMultitask.push_back({{0, time, tag}, task});
			}

			void delTask(unsigned long tag)
			{
				CAutoLock lock(mMtx);
				for (auto it = mMultitask.begin(); it != mMultitask.end(); it++)
				{
					if (it->first.onlyTag == tag)
					{
						mMultitask.erase(it);
						return;
					}
				}
			}

			void retTask(unsigned long tag)
			{
				CAutoLock lock(mMtx);
				for (auto it = mMultitask.begin(); it != mMultitask.end(); it++)
				{
					if (it->first.onlyTag == tag)
						it->first.timer = 0;
				}
			}

			virtual void SetTimer(int iMilliSecond)
			{
				mMilliSecond = iMilliSecond;
				CTemplateTimer<CMultitaskTemplateTimer>::SetTimer(mMilliSecond);
			};

		private:
			virtual void OnTimer()
			{
				CAutoLock lock(mMtx);
				if (!mMultitask.empty())
				{
					for (auto it = mMultitask.begin(); it != mMultitask.end(); it++)
					{
						if (((0 == it->first.timer % (it->first.cycle / mMilliSecond))) && (0 != it->first.cycle))
							(it->second)();
						it->first.timer++;
					}
				}
			};

		private:
			CMutex mMtx;
			unsigned int mMilliSecond;
			// unsigned long long mTime;
			std::vector<std::pair<StuTimeProgress, std::function<void(void)>>> mMultitask;
		};
	} // namespace BaseLib
} // namespace ZH

#endif
