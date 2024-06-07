#include "gstAudioApp.h"
#include <unistd.h>
#include <thread>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <list>
#include "LogServiceApi.h"
#include "mylogCtrl.h"
using namespace chime;

#if 0
static gstWavApp sApp;
int main(int argc,char *argv[])
{
	std::list<std::string> wavFileList;
	if(argc >= 2)
	{
		sApp.playingAudioFile(argv[1]);
		printf("argv1 = %s\n", argv[1]);
		DIR *directory_pointer;
		struct dirent *entry;
		directory_pointer=opendir(argv[1]);
		while((entry=readdir(directory_pointer))!=NULL)
		{
			if(entry->d_type & DT_REG) //查找文件
			{
				std::string fname = entry->d_name;
				//printf("line:%d, name = %s\n", __LINE__, entry->d_name);
				if(fname.find(".wav") != std::string::npos)
				{
					wavFileList.push_back(std::string(std::string(argv[1]) + "/" + entry->d_name));
					printf("line:%d, name = %s\n", __LINE__, entry->d_name);
				}
			}
		}
	}
	else
	{
		printf("line:%d exit program\n", __LINE__);
		return 0;
	}
	sApp.Init(0, 0);
	printf("line:%d,wavFileList.size=%d\n", __LINE__, wavFileList.size());
	for(auto iter : wavFileList)
	{
		sApp.playingAudioFile(iter.c_str());
	}

	printf("argc = %d\n", argc);
	printf("get gstreamer version:%s\n", sApp.getVersion());
	return 0;
}
#endif

GMainLoop* gstWavApp::sLoop = nullptr;
uint32_t gstWavApp::sBusCallMsgType = 0;
gstWavApp::gstWavApp():
	mStrVserion(""),
	mPipeline(nullptr),
	mSource(nullptr),
	mDecoder(nullptr),
	mSink(nullptr),
	mbIsInit(false)
{
}

gstWavApp::~gstWavApp()
{
	Deinit();
}

void gstWavApp::Init(int argc,char *argv[])
{
	if(mbIsInit)
	{
		return;
	}
	LOG_RECORD("%s\n", __func__);
	if(argc <= 1)
	{
		gst_init(nullptr, nullptr);
	}
	else
	{
		gst_init(&argc,&argv);
	}
	#if 1
	guint major, minor, micro, nano;
	gst_version (&major, &minor, &micro, &nano);
	mStrVserion += std::to_string(major) + ".";
	mStrVserion += std::to_string(minor) + ".";
	mStrVserion += std::to_string(micro) + ".";
	mStrVserion += std::to_string(nano) + " ";
	if (nano == 1)
    {
		mStrVserion += " (CVS)";
	}
	else if (nano == 2)
	{
		mStrVserion += " (Prerelease)";
	}
	else
	{
		mStrVserion += "";
	}
	#endif
	sLoop = g_main_loop_new(NULL, false);
	#if 1
	mPipeline = gst_pipeline_new("audio-player");
	#else
	mPipeline = gst_element_factory_make("playbin", NULL);
	g_object_set(mPipeline, "uri", "/opt/data/wav_list/14_normalMOde.wav", NULL);
	#endif
	mSource = gst_element_factory_make("filesrc","file-source");
	mDecoder = gst_element_factory_make("wavparse","mad-decoder");//wav的解码器工程是"wavparse"，mp3的解码器工程是"mda"（目前不支持）
    #if 1
	mSink = gst_element_factory_make("autoaudiosink","audio-output");
	#else
	mSink = gst_element_factory_make("alsasink","alsa-output");
	#endif
	GstBus* bus = gst_pipeline_get_bus(GST_PIPELINE(mPipeline));
	gst_bus_add_watch(bus,bus_call,sLoop);
	gst_object_unref(bus);
    gst_bin_add_many(GST_BIN(mPipeline),mSource,mDecoder,mSink,NULL);
	if(!mSink || !mDecoder || !mSource || !mPipeline || !sLoop)
	{
		this->Deinit();
	}
	else
	{
		mbIsInit = true;
	}
}
void gstWavApp::Deinit()
{
	LOG_RECORD("%s mSink=%p,mDecoder=%p,mSource=%p,mPipeline=%p,sLoop=%p\n", __func__,mSink,mDecoder,mSource,mPipeline,sLoop);
	mbIsInit = false;
	if(mSink)
	{
		LOG_RECORD_DEBUG("%s mSink\n", __func__);
		gst_object_unref(GST_OBJECT(mSink));
		mSink = nullptr;
	}
	if(mDecoder)
	{
		LOG_RECORD_DEBUG("%s mDecoder\n", __func__);
		gst_object_unref(GST_OBJECT(mDecoder));
		mDecoder = nullptr;
	}
	if(mSource)
	{
		LOG_RECORD_DEBUG("%s mSource\n", __func__);
		gst_object_unref(GST_OBJECT(mSource));
		mSource = nullptr;
	}
	if(mPipeline)
	{
		LOG_RECORD_DEBUG("%s mPipeline\n", __func__);
		gst_object_unref(GST_OBJECT(mPipeline));
		mPipeline = nullptr;
	}
	if(sLoop)
	{
		LOG_RECORD_DEBUG("%s sLoop\n", __func__);
		g_main_loop_unref(sLoop);
		sLoop = nullptr;
	}
	
}

const char* gstWavApp::getVersion()
{
	//GstElement *element = gst_element_factory_make ("fakesrc", "source");
	return mStrVserion.c_str();
}

void gstWavApp::terminatePlayback()
{
    LOG_RECORD("%s MsgType:%x", __func__, sBusCallMsgType);
    if(gstWavApp::sLoop)
    {
        g_main_loop_quit (gstWavApp::sLoop);
    }
}
gboolean gstWavApp::bus_call(GstBus * bus, GstMessage * message, gpointer data)
{
	//LOG_RECORD_DEBUG("(%s,%d), msgName:%s, msgType:0x%08x\n",__func__,__LINE__, GST_MESSAGE_TYPE_NAME (message), GST_MESSAGE_TYPE (message));
    sBusCallMsgType = GST_MESSAGE_TYPE (message);
	switch (sBusCallMsgType)
	{
	case GST_MESSAGE_EOS:
		g_main_loop_quit (gstWavApp::sLoop);
		break;
	case GST_MESSAGE_ERROR:
		g_main_loop_quit (gstWavApp::sLoop);
		break;
	case GST_MESSAGE_STATE_CHANGED:
	{
		// GstState old_state, new_state, pending_state;
		// gst_message_parse_state_changed (message, &old_state, &new_state, &pending_state);
	}
	default:
		break;
	}
	return true;
}

void gstWavApp::playingAudioFile(const char* audioFileName)
{
	#if 1
	if(access(audioFileName, F_OK) == -1)
	{
		LOG_RECORD("(%s,%d) audioFileName = %s no find file\n", __func__, __LINE__, audioFileName);
		return;
	}
	#endif
	LOG_RECORD_DEBUG("(%s,%d) audioFileName = %s, sLoop:%p\n", __func__, __LINE__, audioFileName, sLoop);
	if(!sLoop)
	{
		return;
	}
	LOG_RECORD_DEBUG("(%s,%d) audioFileName = %s\n", __func__, __LINE__, audioFileName);
	g_object_set(G_OBJECT(mSource),"location",audioFileName,NULL);
	//gst_bin_add_many(GST_BIN(mPipeline),mSource,mDecoder,mSink,NULL);
	gst_element_link_many(mSource,mDecoder,mSink,NULL);
	//
	gst_element_set_state(mPipeline,GST_STATE_PLAYING);
	g_main_loop_run(sLoop);//阻塞式播放
	gst_element_set_state(mPipeline,GST_STATE_NULL);
}

