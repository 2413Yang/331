
#ifndef TEMPLATECACHE__H__
#define TEMPLATECACHE__H__
#include <queue>
#include <condition_variable>

namespace ZH
{
    namespace BaseLib
    {
        template <class T>
        class CTemplateCache
        {
        public:
            CTemplateCache() : bReady(false) {}
            ~CTemplateCache() {}
            void push(const T &it)
            {
                std::unique_lock<std::mutex> guard(mCacheLock);
                mCache.push(it);
                bReady = true;
                cv.notify_all();
            }

            void wait(void)
            {
                std::unique_lock<std::mutex> guard(mCacheLock);
                while (!bReady)
                    cv.wait(guard);
            }

            bool wait(unsigned int time)
            {
                bool ret = false;
                std::unique_lock<std::mutex> guard(mCacheLock);
                if (!mCache.empty())
                    ret = true;
                else
                    cv.wait_for(guard, std::chrono::milliseconds(time));

                if (!mCache.empty())
                    ret = true;
                return ret;
            }

            bool pull(T &it)
            {
                bool ret = false;
                std::unique_lock<std::mutex> guard(mCacheLock);
                if (!mCache.empty())
                {
                    it = mCache.front();
                    mCache.pop();
                    ret = true;
                }
                else
                    bReady = false;

                return ret;
            }

            void lock()
            {
                mCacheLock.lock();
            }

            void unlock()
            {
                mCacheLock.unlock();
            }

            int getSize()
            {
                std::unique_lock<std::mutex> guard(mCacheLock);
                return mCache.size();
            }

        protected:
            bool bReady;
            std::queue<T> mCache;
            std::mutex mCacheLock;
            std::condition_variable cv;
        };
    };
};
#endif /*TEMPLATECACHE__H__*/