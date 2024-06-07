#include "videoPlayer.h"
#include <unistd.h>
#include "videoPlayer_api.h"
#include <pthread.h>
#include "mylogCtrl.h"
#include <syslog.h>

#define STARTUP_ANIMATION_TOTAL_COUNT   (60)
#define STARTUP_ANIMATION_TOTAL_TIMEMS   (3*1000)
#define STARTUP_ANIMATION_FRAME_INTERVAL    (50)
//#define ANIMATION_FILE_NAME     "/usr/sbin/zhapp/midware/res/maiyaBootAnimation.h264"
#define ANIMATION_FILE_NAME     "/usr/sbin/zhapp/midware/res/a301_welcome.h264"

using namespace std;


class CVideoCtrl 
{ 
public: //用户接口
    CVideoCtrl():
	cMinPlayInterval(8),
	cMaxPlayInterval(200),
	mVecStepSpd({0,30,60,90,120, 161}),
	mVecStepTick({0.6f, 0.2f, 0.1f, 0.05f, 0.05f})
    {
		mLaneSpeed = 0;
		mLaneColor = EnLaneColor::purple;
        pthread_spin_init(&spinlock,0);
        playStartupAnimationFlag = ANIMATION_FILE_NAME;
        castScreenFlag = false;
		mVecSteptime.push_back(cMaxPlayInterval);
		float lastValue = cMaxPlayInterval;
		for(int i = 0; i < mVecStepTick.size(); i++)
		{
			float val = (cMaxPlayInterval - cMinPlayInterval)* mVecStepTick[i];
			val = lastValue - val;
			mVecSteptime.push_back(val);
			lastValue = val;
		}
		// for(int i = 0; i < mVecSteptime.size(); i++)
		// {
		// 	printf("mVecSteptime[%d]:%f\n", i, mVecSteptime[i]);
		// }
    } 
    void StartupAnimation_Start( string fileName = ANIMATION_FILE_NAME) 
    { 
        LOG_SINFO ("%s, %s\n", __func__, fileName.c_str()); 
        pthread_spin_lock(&spinlock);
        if(0 == access(fileName.c_str(), F_OK))
        {
            AnimationFileName = fileName;
        }
        playStartupAnimationFlag = true;
        pthread_spin_unlock(&spinlock);
    }
    void StartupAnimation_Stop( int a) 
    { 
        LOG_SINFO ("%s, %d, curTick = %d\n", __func__, a, videoPlayer::getSysTimeMS()); 
        pthread_spin_lock(&spinlock);
        playStartupAnimationFlag = false;
        pthread_spin_unlock(&spinlock);
    }
    void CastScreen_Ctrl(bool flag)
    {
        pthread_spin_lock(&spinlock);
        castScreenFlag = flag;
        pthread_spin_unlock(&spinlock);
    }
	void LaneAnimation_Speed(int spd)
	{
		LOG_SDEBUG ("%s, spd = %d\n", __func__, spd);
		mLaneSpeed = spd;
	}
	void LaneAnimation_SetColor(EnLaneColor color)
	{
		LOG_SDEBUG ("%s, color = %d\n", __func__, int(color));
		if(uint32_t(color) < (sizeof(sLaneAnimationList)/ sizeof(sLaneAnimationList[0])))
		{
			mLaneColor = color;
		}
	}
	void playStartupAnimation();
	void playLaneAnimation();
private:
    bool playStartupAnimationFlag = false; 
    bool castScreenFlag;
    string AnimationFileName;
    pthread_spinlock_t spinlock;
	EnLaneColor	mLaneColor;
	int 	mLaneSpeed;
	std::vector<int> mVecStepSpd;
	std::vector<float> mVecSteptime;
	std::vector<float> mVecStepTick;
private:
	const static char* sLaneAnimationList[4];
	const uint32_t cMinPlayInterval;
	const uint32_t cMaxPlayInterval;
} oVideoCtrl;

const char* CVideoCtrl::sLaneAnimationList[4] ={
	"/usr/sbin/zhapp/midware/res/lane_green_tcc.h264",
	"/usr/sbin/zhapp/midware/res/lane_purple_tcc.h264",
	"/usr/sbin/zhapp/midware/res/lane_orange_tcc.h264",
	"/usr/sbin/zhapp/midware/res/lane_red_tcc.h264",
};

void CVideoCtrl::playStartupAnimation()
{
    uint32_t pos = (4 << 16) | 0;
    uint32_t vSize = ((1920 - 4) << 16) | 720;
    videoPlayer* startupAnimationPlayer = new videoPlayer(AnimationFileName.c_str(), pos, vSize);
    uint32_t startTick = videoPlayer::getSysTimeMS();
    uint32_t framePlayCount = 0;
    do
    {
        uint32_t preTick = videoPlayer::getSysTimeMS();
        startupAnimationPlayer->videoPlayerNextFrame();
        uint32_t nowTick = videoPlayer::getSysTimeMS();
        uint32_t deltaTick = nowTick - preTick;
        if(deltaTick > 15)
        {//保险
            deltaTick = 15;
        }
        if((deltaTick) < STARTUP_ANIMATION_FRAME_INTERVAL)
        {
            usleep((STARTUP_ANIMATION_FRAME_INTERVAL - (deltaTick)) * 1000);
        }
        //LOG_SDEBUG("%s deltaTick=%d\n", __func__, deltaTick);
        if(0== framePlayCount)
        {
            StartupAnimation_PlaySts(tstAnimationSts::PLAYING_FIRSTFRAME);
        }
        LOG_SDEBUG("######### deltaTick = %u, playtime = %u\n", videoPlayer::getSysTimeMS() - preTick, (nowTick - preTick));
        const int CorrectionTick = 200;
        if((nowTick - startTick) > (STARTUP_ANIMATION_TOTAL_TIMEMS + CorrectionTick))
        {//保险
            break;
        }
        if(++framePlayCount >= STARTUP_ANIMATION_TOTAL_COUNT)
        {
            break;
        }
        if(false == playStartupAnimationFlag)
        {
            break;
        }
    } while (true);
    LOG_SINFO("useTick = %u, framePlayCount = %d, curTick = %d\n", videoPlayer::getSysTimeMS() - startTick, framePlayCount, videoPlayer::getSysTimeMS());
    bool sendflag = false;
    uint32_t curTick = videoPlayer::getSysTimeMS();
    while (playStartupAnimationFlag)
    {
        if(!sendflag)
        {
            sendflag = true;
            StartupAnimation_PlaySts(tstAnimationSts::PLAYING_LASTFRAME);
        }
        usleep(10*1000);
        if((videoPlayer::getSysTimeMS() - curTick) > 10*1000)
        {//超时也推出开机动画
            break;
        }
    }
    StartupAnimation_PlaySts(tstAnimationSts::PLAYING_NONE);
    LOG_SDEBUG("StartupAnimation_PlaySts(tstAnimationSts::PLAYING_NONE) curTick = %d\n", videoPlayer::getSysTimeMS());
    delete startupAnimationPlayer;
    LOG_SINFO("StartupAnimation_PlaySts End curTick = %d\n", videoPlayer::getSysTimeMS());
}

void CVideoCtrl::playLaneAnimation()
{
	EnLaneColor curColor = mLaneColor;
	const int pos = (212 << 16) | (255);
	const int size = (1496 << 16) | 430;
	videoPlayer* player = new videoPlayer(sLaneAnimationList[int(mLaneColor)], pos, size, 0);
	LOG_SDEBUG("sLaneAnimationList[%d] = %s\n",int(mLaneColor),sLaneAnimationList[int(mLaneColor)]);
	player->videoPlayerNextFrame();
	usleep(10*1000);
	do
	{
		uint32_t preTick = videoPlayer::getSysTimeMS();
		int ret = player->videoPlayerNextFrame();
		if(ret != 0)
		{
			LOG_SDEBUG(".....ret:%d\n", ret);
			player->videoPlayerResetFrame();
			player->videoPlayerNextFrame();
		}
		if(curColor != mLaneColor)
		{
			curColor = mLaneColor;
			player->setFileName(sLaneAnimationList[int(mLaneColor)]);
			LOG_SDEBUG("sLaneAnimationList[%d] = %s\n",int(mLaneColor),sLaneAnimationList[int(mLaneColor)]);
		}
		for(;;)
		{
			usleep(5*1000);
			uint32_t now = videoPlayer::getSysTimeMS();
			uint32_t deltaTick = now - preTick;
			if(mLaneSpeed == 0)
			{
				usleep(5*1000);
				if(curColor != mLaneColor)
				{
					curColor = mLaneColor;
					player->setFileName(sLaneAnimationList[int(mLaneColor)]);
					player->videoPlayerNextFrame();
					usleep(5*1000);
					player->videoPlayerNextFrame();
					LOG_SDEBUG("%d,sLaneAnimationList[%d] = %s\n",__LINE__,int(mLaneColor),sLaneAnimationList[int(mLaneColor)]);
				}
				continue;
			}
			else
			{
				float interval;
				#if 0
				interval = cMaxPlayInterval - (float(mLaneSpeed*(cMaxPlayInterval - cMinPlayInterval)) / 160.f);
				#else
				int step = 0;
				for(int i = 1; i < mVecStepSpd.size(); i++)
				{
					if(mLaneSpeed < mVecStepSpd[i])
					{
						break;
					}
					else
					{
						step = i;
					}
				}
				if(step >= (mVecSteptime.size() - 1))
				{
					step = mVecSteptime.size() - 2;
				}
				float stepMaxInterval = mVecSteptime[step];
				float stepMinInterval = mVecSteptime[step + 1];
				int stepMaxSpd = mVecStepSpd[step + 1];
				int stepMinSpd = mVecStepSpd[step];
				interval = stepMinInterval + (float((stepMaxSpd - mLaneSpeed)*(stepMaxInterval - stepMinInterval)) / (stepMaxSpd - stepMinSpd));
				LOG_SDEBUG("stepMaxInterval:%f, stepMinInterval:%f, stepMaxSpd:%d, stepMinSpd:%d, interval:%f\n", stepMaxInterval, stepMinInterval, stepMaxSpd, stepMinSpd, interval);
				#endif
				if(deltaTick > uint32_t(interval))
				{
//					printf("preTick:%d, now:%d, deltaTick:%d, interval:%f\n", preTick, now, deltaTick, interval);
					break;
				}
			}
		}
	} while (true);
	delete player;
}

int main(int argc, char* argv[])
{
    LOG_SINFO("#################### %s start tick = %u ####################\n", argv[0], videoPlayer::getSysTimeMS());
    //
    oVideoCtrl.StartupAnimation_Start();
    CIPCConnector opt(argv[0]);
    subscriber::StartupAnimation_Start(opt, oVideoCtrl); // 当数据发布时该接口回调
    subscriber::StartupAnimation_Stop(opt, oVideoCtrl); // 
    subscriber::LaneAnimation_Speed(opt, oVideoCtrl); // 
	subscriber::LaneAnimation_SetColor(opt, oVideoCtrl); 
    publisher::StartupAnimation_PlaySts (opt); // 初始化
    publisher::CastScreen_Status(opt);

    opt.start();
    oVideoCtrl.playStartupAnimation();
	oVideoCtrl.playLaneAnimation();
    while (true)
    {
        usleep(20*1000);
    }
    return 0;
}