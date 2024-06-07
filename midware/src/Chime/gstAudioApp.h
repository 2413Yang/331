#ifndef _GSTAUDIOAPP_H_
#define _GSTAUDIOAPP_H_
#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <string>
#include <glib-2.0/glib.h>
#include <gst/gst.h>

namespace chime
{

class gstWavApp
{
private:
	std::string		mStrVserion;
	GstBus*			mBus;
	GstElement*		mPipeline;
	GstElement*		mSource;
	GstElement*		mDecoder;
	GstElement*		mSink;
	bool			mbIsInit;
private:
	static GMainLoop*		sLoop;
    static uint32_t sBusCallMsgType;
private:
	static gboolean bus_call(GstBus * bus, GstMessage * message, gpointer user_data);
public:
	gstWavApp();
	~gstWavApp();
	void Init(int argc,char *argv[]);
	void Deinit();
	const char* getVersion();
	void playingAudioFile(const char* audioFileName);
    void terminatePlayback();
};


}

#endif //!_GSTAUDIOAPP_H_


