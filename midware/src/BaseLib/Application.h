#ifndef APPLICATION_H
#define APPLICATION_H
#include <stdint.h>
#include <time.h>
namespace ZH
{
    namespace BaseLib
    {
        class CApplication
        {
        public:
            CApplication();
            ~CApplication();

            void Start(bool bBlock);

        private:
            virtual void Init() {}
            virtual void DoWork() {}
            virtual void UInit() {}
        };

        extern uint32_t getWorldTimeMS();
		enum simpleTimerStatus
		{
			timer_stop,
			timer_running,
			timer_pause,
			timer_expired,
		};
    }
}

#endif