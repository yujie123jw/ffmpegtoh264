/** 
*作者：YIGESHEN更新  from HJL 
*最后更新：2017.8.8
*利用ffmpeg将RTSP传输的h264原始码流保存到文件中 
*未加任何效果，不显示 
*主要代码源于 http://blog.csdn.net/hjl240/article/details/47089755
*更新原因 之前的版本 在 ffmpeg version 2.2.1 Copyright (c) 2000-2014 the FFmpeg developers 无法使新特些更新！

**/  
  
  
#include <stdio.h>  
  
#define __STDC_CONSTANT_MACROS  
#define FF_API_SWS_GETCONTEXT 1
#ifdef _WIN32  
//Windows  
extern "C"  
{  
#include "libavcodec/avcodec.h"  
#include "libavformat/avformat.h"  
#include "libswscale/swscale.h"  
#include "SDL2/SDL.h"  
};  
#else  
//Linux...  
#ifdef __cplusplus  
extern "C"  
{  
#endif  
#include <libavcodec/avcodec.h>  
#include <libavformat/avformat.h>  
#include <libswscale/swscale.h>  
#include <SDL/SDL.h>  
#ifdef __cplusplus  
};  
#endif  
#endif
int main(int argc, char* argv[])  
{  
  
    AVFormatContext *pFormatCtx;  
    int             i, videoindex;  
    AVCodecContext  *pCodecCtx;  
    AVCodec         *pCodec;  
    AVFrame *pFrame,*pFrameYUV;  
    uint8_t *out_buffer;  
    AVPacket *packet;  
    int ret, got_picture;  
	AVDictionary *avdic=NULL;
        char option_key[]="rtsp_transport";
        char option_value[]="tcp";
        av_dict_set(&avdic,option_key,option_value,0);
        char option_key2[]="max_delay";
        char option_value2[]="5000000";
        av_dict_set(&avdic,option_key2,option_value2,0);
  
  
    struct SwsContext *img_convert_ctx;  
    //下面是公共的RTSP测试地址  
    char filepath[]="rtsp://10.3.6.56:5555/test.sdp";  
  
    av_register_all();  
    avformat_network_init();  
    pFormatCtx = avformat_alloc_context();  
  
    if(avformat_open_input(&pFormatCtx,filepath,NULL,&avdic)!=0)////打开网络流或文件流  
    {  
        printf("Couldn't open input stream.\n");  
        return -1;  
    }  
    if(avformat_find_stream_info(pFormatCtx,NULL)<0)  
    {  
        printf("Couldn't find stream information.\n");  
        return -1;  
    }  
    videoindex=-1;  
    for(i=0; i<pFormatCtx->nb_streams; i++)   
        if(pFormatCtx->streams[i]->codec->codec_type==AVMEDIA_TYPE_VIDEO)  
        {  
            videoindex=i;  
            break;  
        }  
        if(videoindex==-1)  
        {  
            printf("Didn't find a video stream.\n");  
            return -1;  
        }  
        pCodecCtx=pFormatCtx->streams[videoindex]->codec;  
        pCodec=avcodec_find_decoder(pCodecCtx->codec_id);  
        if(pCodec==NULL)  
        {  
            printf("Codec not found.\n");  
            return -1;  
        }  
        if(avcodec_open2(pCodecCtx, pCodec,NULL)<0)  
        {  
            printf("Could not open codec.\n");  
            return -1;  
        }  
        pFrame=av_frame_alloc();  
        pFrameYUV=av_frame_alloc();  
        out_buffer=(uint8_t *)av_malloc(avpicture_get_size(PIX_FMT_YUV420P, pCodecCtx->width, pCodecCtx->height));  
        avpicture_fill((AVPicture *)pFrameYUV, out_buffer, PIX_FMT_YUV420P, pCodecCtx->width, pCodecCtx->height);  
  
        //Output Info---输出一些文件（RTSP）信息  
        printf("---------------- File Information ---------------\n");  
        av_dump_format(pFormatCtx,0,filepath,0);  
        printf("-------------------------------------------------\n");  
  
        img_convert_ctx = sws_getContext(pCodecCtx->width, pCodecCtx->height, pCodecCtx->pix_fmt, pCodecCtx->width, pCodecCtx->height, PIX_FMT_YUV420P, SWS_BICUBIC, NULL, NULL, NULL);

        packet=(AVPacket *)av_malloc(sizeof(AVPacket));  
  
        FILE *fpSave;  
        if((fpSave = fopen("test.h264", "ab")) == NULL) //h264保存的文件名  
            return 0;   
        for (;;)   
        {  
            //------------------------------  
            if(av_read_frame(pFormatCtx, packet)>=0)  
            {  
                if(packet->stream_index==videoindex)  
                {  
                    fwrite(packet->data,1,packet->size,fpSave);//写数据到文件中  
                }  
                av_free_packet(packet);  
            }  
        }  
  
  
        //--------------  
        av_frame_free(&pFrameYUV);  
        av_frame_free(&pFrame);  
        avcodec_close(pCodecCtx);  
        avformat_close_input(&pFormatCtx);  
  
        return 0;  
}
