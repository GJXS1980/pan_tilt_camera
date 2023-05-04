#include "ros/ros.h"
#include <opencv2/opencv.hpp>
#include <opencv2/core.hpp> 
#include <opencv2/imgproc.hpp>
#include <cv_bridge/cv_bridge.h>
#include <image_transport/image_transport.h>
#include <fstream>
#include <unistd.h>
#include <pthread.h> 
#include <iostream>
#include "HCNetSDK.h"
#include "PlayM4.h"
#include "LinuxPlayM4.h"
#include "CapPicture.h"
#include "playback.h"

#include <termios.h> 
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>

#include <iostream>
#include <std_msgs/Float32MultiArray.h>
#include <std_msgs/Int32MultiArray.h>
#include <std_msgs/String.h>
#include <pthread.h>
#include <sys/select.h>
#include <termios.h>

#define HPR_ERROR       -1
#define HPR_OK           0
#define USECOLOR         0
#define key_ESC 27

using namespace std;

static cv::Mat dst;
cv::Mat image;
cv::Mat dst_img;
cv::Mat imgResize;

string cam_ip;    //用于获取launch传递函数
string user_name;    //用于获取launch传递函数
string password;    //用于获取launch传递函数
string user_log;    //用于获取launch传递函数
string record_video;    //用于获取launch传递函数
string record_dat;    //用于获取launch传递函数
string image_name;
string image_type;

int Init_runing = 1;
int Camera_runing = 1;
int video_recording = 0; 

char key = 0; // 云台控制
int key_runing = 1;
static int num = 0;
static struct termios initial_settings, new_settings;
static int peek_character = -1;         /* 用于测试一个按键是否被按下 */
static int peek_char = -1;         /* 用于测试一个按键是否被按下 */

string dir;
FILE *g_pFile = NULL;
HWND h = NULL;
LONG nPort=-1;
LONG lUserID;
 
pthread_mutex_t mutex_cam;
std::list<cv::Mat> g_frameList;

image_transport::Publisher pub; //  发布图像话题
// 发布图像数据
sensor_msgs::ImagePtr msg;
ros::Subscriber Camera_Control_Sub; 

void close_keyboard();
int readch(); /* 相关函数声明 */
// 读取视频流回调函数
static void CALLBACK g_RealDataCallBack_V30(LONG lRealHandle, DWORD dwDataType, BYTE *pBuffer, DWORD dwBufSize,void* dwUser);
// 预览重连回调函数
static void CALLBACK g_ExceptionCallBack(DWORD dwType, LONG lUserID, LONG lHandle, void *pUser); 

/*
功能：写入头数据
备注：云台、API
*/
void CALLBACK PsDataCallBack(LONG lRealHandle, DWORD dwDataType,BYTE *pPacketBuffer,DWORD nPacketSize, char* pUser)
{  
  char * DAT_data = (char *)record_dat.data();
  if (dwDataType  == NET_DVR_SYSHEAD)
  {
    //写入头数据
    g_pFile = fopen(DAT_data, "wb");
    if (g_pFile == NULL)
    {
      printf("CreateFileHead fail\n");
      return;
    }
    //写入头数据
    fwrite(pPacketBuffer, sizeof(unsigned char), nPacketSize, g_pFile);
    printf("write head len=%d\n", nPacketSize);
  }
  else
  {
    if(g_pFile != NULL)
    {
      fwrite(pPacketBuffer, sizeof(unsigned char), nPacketSize, g_pFile);
      printf("write data len=%d\n", nPacketSize);
    }
  }
}
 
/*
功能：格式转换
备注：云台、API
*/
//void CALLBACK DecCBFun(LONG nPort, char *pBuf, LONG nSize, FRAME_INFO *pFrameInfo, LONG nReserved1, LONG nReserved2)
void CALLBACK DecCBFun(LONG nPort, char *pBuf, LONG nSize, FRAME_INFO *pFrameInfo, void* nReserved1, LONG nReserved2)
{
  long lFrameType = pFrameInfo->nType;
  if (lFrameType == T_YV12)
  {
    dst.create(pFrameInfo->nHeight, pFrameInfo->nWidth,CV_8UC3);
    // printf("hheihg: %d wwidht: %d size: %d\n",pFrameInfo->nHeight,  pFrameInfo->nWidth, nSize );
    cv::Mat src(pFrameInfo->nHeight + pFrameInfo->nHeight/2, pFrameInfo->nWidth, CV_8UC1, (uchar *)pBuf);
	  cv::cvtColor(src, dst, CV_YUV2BGR_YV12);
    pthread_mutex_lock(&mutex_cam);
    g_frameList.push_back(dst);
    pthread_mutex_unlock(&mutex_cam);
  }
  usleep(10);
}
 
/*
功能：读取视频流回调
备注：云台、API
*/
void CALLBACK g_RealDataCallBack_V30(LONG lRealHandle, DWORD dwDataType, BYTE *pBuffer, DWORD dwBufSize,void* dwUser)
{
  DWORD dRet;
  switch (dwDataType)
  {
    case NET_DVR_SYSHEAD:           //系统头
    if (!PlayM4_GetPort(&nPort))  //获取播放库未使用的通道号
    {
      break;
    }
    //第一次回调的是系统头，将获取的播放库port号赋值给全局port，下次回调数据时即使用此port号播放
    if (dwBufSize > 0) 
    {
      //设置实时流播放模式
      if (!PlayM4_SetStreamOpenMode(nPort, STREAME_REALTIME)) 
      {
        dRet = PlayM4_GetLastError(nPort);
        break;
      }
      //打开流接口
      if (!PlayM4_OpenStream(nPort, pBuffer, dwBufSize, 1024 * 1024))
      {
        dRet = PlayM4_GetLastError(nPort);
        break;
      }
      //设置解码回调函数 解码且显示
      if (!PlayM4_SetDecCallBackEx(nPort, DecCBFun, NULL, NULL))
      {
        dRet = PlayM4_GetLastError(nPort);
        break;
      }
      //打开视频解码，播放开始
      if (!PlayM4_Play(nPort, h))
      {
        dRet = PlayM4_GetLastError(nPort);
        break;
      }
      //打开音频解码, 需要码流是复合流
      if (!PlayM4_PlaySound(nPort)) 
      {
        dRet = PlayM4_GetLastError(nPort);
        break;
      }
    }
    break;
    //usleep(500);
    case NET_DVR_STREAMDATA:  //码流数据
       if (dwBufSize > 0 && nPort != -1) 
       {
         BOOL inData = PlayM4_InputData(nPort, pBuffer, dwBufSize);
         while (!inData) 
         {
           sleep(10);
           inData = PlayM4_InputData(nPort, pBuffer, dwBufSize);
           std::cerr << "PlayM4_InputData failed \n" << std::endl;
         }
       }
       break;
   }
}
 
/*
功能：预览时重连回调
备注：无
*/ 
void CALLBACK g_ExceptionCallBack(DWORD dwType, LONG lUserID, LONG lHandle, void *pUser)
{
  char tempbuf[256] = {0};
  std::cout << "EXCEPTION_RECONNECT = " << EXCEPTION_RECONNECT << std::endl;
  switch(dwType)
  {
    case EXCEPTION_RECONNECT:	//预览时重连
                              printf("pyd reconnect %d\n", time(NULL));
                              break;
    default:
                              break;
  }
}

/****************************************************************
函数功能：初始化SDK，用户注册设备
****************************************************************/
void CameraInitOnce() 
{
  char * LOG = (char *)user_log.data(); // 离线语法识别资源路径
  char * RECORD = (char *)record_video.data(); // 离线语法识别资源路径
  char *IP = (char *)cam_ip.data(); // 海康威视网络摄像头的ip
  char *UName = (char *)user_name.data(); // 海康威视网络摄像头的用户名
  char *PSW = (char *)password.data(); // 海康威视网络摄像头的密码

  NET_DVR_Init(); // 初始化SDK
  // 设置连接时间与重连时间
  NET_DVR_SetConnectTime(2000, 1);
  NET_DVR_SetReconnect(1000, true);
  // 设置日志保存路径
  NET_DVR_SetLogToFile(3, LOG);

  NET_DVR_DEVICEINFO_V30 struDeviceInfo = {0};
  NET_DVR_SetRecvTimeOut(5000);
  lUserID = NET_DVR_Login_V30(IP, 8000, UName, PSW, &struDeviceInfo); // 用户注册设备
  if (lUserID < 0) {
    printf("###########<<Login error>>########## erro code: %d\n", NET_DVR_GetLastError());
    NET_DVR_Cleanup();
    exit(-1); // 初始化失败，直接退出程序
  }

  NET_DVR_SetExceptionCallBack_V30(0, NULL, g_ExceptionCallBack, NULL);

  long lRealPlayHandle;
  NET_DVR_CLIENTINFO ClientInfo = {0};

  ClientInfo.lChannel = 1; // 预览通道号
  ClientInfo.lLinkMode = 0; // 0-TCP方式，1-UDP方式，2-多播方式，3-RTP方式，4-RTP/RTSP，5-RSTP/HTTP
  ClientInfo.hPlayWnd = 0; // 需要SDK解码时句柄设为有效值，仅取流不解码时可设为空
  ClientInfo.sMultiCastIP = NULL;

  // 启动预览
  lRealPlayHandle = NET_DVR_RealPlay_V30(lUserID, &ClientInfo, g_RealDataCallBack_V30, NULL, 0);
  if (video_recording) {
    NET_DVR_SaveRealData(lRealPlayHandle, RECORD);
  }

  if (lRealPlayHandle < 0) {
    printf("pyd1---NET_DVR_RealPlay_V30 error\n");
  }
}


/*
功能：相机图像发布线程
备注：相机图像
*/ 
void *RunIPCameraInfo(void*)
{
  // 将自己设置为分离状态
  pthread_detach(pthread_self());

  cv::Mat image;
  cv::Mat dst_img;

  while(Camera_runing)
  {
    pthread_mutex_lock(&mutex_cam);
    if(!g_frameList.empty())
    {
      // 使用 std::list::back() 方法获取最后一个元素
      // 使用 cv::Mat::clone()从 image 复制到 dst_img 
      image = g_frameList.back().clone();
      // 清空所有帧
      g_frameList.clear();

      if (!image.empty())
      {
        //  修改图片尺寸大小（像素为640*480）
        cv::resize(image, dst_img,  cv::Size(640, 480), 0, 0, cv::INTER_LINEAR);
        imshow("image", dst_img);  

        auto msg = cv_bridge::CvImage(std_msgs::Header(), "bgr8", dst_img).toImageMsg();
        if(msg)
        {
          pub.publish(msg); 
        }
      }
    }
    pthread_mutex_unlock(&mutex_cam);
    cv::waitKey(10);
  }
  pthread_exit(NULL);
}

/*
功能：设置终端为非阻塞模式,让程序在没有键盘输入时进入休眠状态，降低 CPU 占用率
备注：键盘
*/ 
void set_non_blocking()
{
  struct termios term;
  tcgetattr(STDIN_FILENO, &term);
  term.c_lflag &= ~(ICANON | ECHO);
  term.c_cc[VMIN] = 0;
  term.c_cc[VTIME] = 0;
  tcsetattr(STDIN_FILENO, TCSANOW, &term);
}

/*
功能：捕获键盘输入的,当用户按下Ctrl + C时,它会将终端恢复为阻塞模式,并停止相机采集以及线程循环。
备注：键盘
*/
void *capture_keyvalue(void*)
{
  // 将自己设置为分离状态
  pthread_detach(pthread_self());  
  set_non_blocking(); // 设置终端为非阻塞模式
  while(key_runing)
  {
    fd_set rfds;
    FD_ZERO(&rfds);
    FD_SET(STDIN_FILENO, &rfds);

    struct timeval tv = { 0, 100000 }; // 设置 select 的超时时间为 100ms(线程在没有输入的情况下暂停 100ms,避免不必要的 CPU 占用)
    select(STDIN_FILENO + 1, &rfds, NULL, NULL, &tv);

    if (FD_ISSET(STDIN_FILENO, &rfds))
    {
      key = readch(); 
      if(key == 3)
      {
        close_keyboard();
        key_runing = 0;
        Camera_runing = 0;  
      }
    }
  }	
  close_keyboard();	
  pthread_exit(NULL);
}


/*
功能：捕获键盘输入
备注：键盘
*/
/* 用来接收按下的按键,并peek_character = -1恢复状态 */
int readch()
{
  char ch = NULL;
  if ( peek_character != -1 )
  {
    ch = peek_character;
    peek_character = -1;
    return(ch);
  }
  read( 0, &ch, 1 );
  return(ch);
}

/*
功能：关闭键盘
备注：键盘
*/
void close_keyboard()
{
    struct termios initial_settings;
    tcgetattr(0, &initial_settings);
    struct termios term = initial_settings;
    term.c_lflag |= (ICANON | ECHO);
    tcsetattr(0, TCSANOW, &term);
}

/*
功能：键盘回调函数
备注：话题控制
*/
void Camera_Control_Callback(const std_msgs::String::ConstPtr& cmd)
{
  string temp = cmd -> data;
  key = temp[0];
}


int main(int argc,char **argv)
{
  ros::init(argc, argv, "Pan_Tilt_Camera_NODE");
  ros::NodeHandle n;		//创建句柄	
  ros::NodeHandle nh("~");    //用于launch文件传递参数

  ros::Rate loop_rate(10); 

	nh.param("cam_ip", cam_ip, std::string("192.168.5.64"));    //从launch文件获取appid参数
	nh.param("user_name", user_name, std::string("admin"));    //从launch文件获取参数
	nh.param("password", password, std::string("abcd1234"));    //从launch文件获取参数
	nh.param("user_log", user_log, std::string("./record/sdkLog"));    //从launch文件获取参数
	nh.param("record_video", record_video, std::string("./yuntai.mp4"));    //从launch文件获取参数
	nh.param("record_dat", record_dat, std::string("./record/ps.dat"));    //从launch文件获取参数

	nh.param("image_name", image_name, std::string("./img/image_"));    //从launch文件获取参数
	nh.param("image_type", image_type, std::string(".jpeg"));    //从launch文件获取参数

  Camera_Control_Sub = n.subscribe("/Camera_Control_TOPIC", 1, &Camera_Control_Callback);	 
  image_transport::ImageTransport it(n);
  pub = it.advertise("camera/image", 10);
  pthread_t camerainit;
  pthread_t input_key;
	
  int ret;
  pthread_mutex_init(&mutex_cam, NULL); 
  CameraInitOnce(); //  初始化相机
  ret = pthread_create(&camerainit, NULL, RunIPCameraInfo, NULL); //  创建图像显示与发布线程
  if(ret!=0)
  {
    printf("Create pthread error!\n");
  }

  ret = pthread_create(&input_key, NULL, capture_keyvalue, NULL); //  创建键盘获取线程
  if(ret!=0)
  {
    printf("Create pthread error!\n");
  }  

  while(ros::ok())
  {
    loop_rate.sleep();
    switch(key)
    {
      // 焦距变大(倍率变大)
      case '1': 	NET_DVR_PTZControl_Other(lUserID, 1, ZOOM_IN, 0);	
                  usleep(10);
                  key = 117;
                  break;
      case 'u': 	NET_DVR_PTZControl_Other(lUserID, 1, ZOOM_IN, 1);	
                  break;

      // 焦距变小(倍率变小)
      case '2':   NET_DVR_PTZControl_Other(lUserID, 1, ZOOM_OUT, 0); 
                  usleep(10);
                  key = 105; 	
			            break;  
      case 'i':   NET_DVR_PTZControl_Other(lUserID, 1, ZOOM_OUT, 1);  	
			            break;  

      // 接通灯光电源
      case '3':   
                  NET_DVR_PTZControl_Other(lUserID, 1, LIGHT_PWRON, 0);
			            break;
      //  焦点前调
      case '4':   
                  NET_DVR_PTZControl_Other(lUserID, 1, FOCUS_NEAR, 0); 
                  usleep(10);
                  key = 111;  
		              break;
      case 'o':   
                  NET_DVR_PTZControl_Other(lUserID, 1, FOCUS_NEAR, 1);  
		              break;
      //  焦点后调
      case '5':   
                  NET_DVR_PTZControl_Other(lUserID, 1, FOCUS_FAR, 0);
                  usleep(10);
                  key = 112;  
		              break;
      case 'p':   
                  NET_DVR_PTZControl_Other(lUserID, 1, FOCUS_FAR, 1);  
		              break;

      //  光圈扩大
      case '6':   
                  NET_DVR_PTZControl_Other(lUserID, 1, IRIS_OPEN, 0); 
                  usleep(10);
                  key = 106;  
		              break;
      case 'j':   
                  NET_DVR_PTZControl_Other(lUserID, 1, IRIS_OPEN, 1);  
		              break;

      //  光圈缩小
      case '7':   
                  NET_DVR_PTZControl_Other(lUserID, 1, IRIS_CLOSE, 0);
                  usleep(10); 
                  key = 108; 
		              break;
      case 'l':   
                  NET_DVR_PTZControl_Other(lUserID, 1, IRIS_CLOSE, 1); 
		              break;

      //接通雨刷开关
      case '8':   
                  NET_DVR_PTZControl_Other(lUserID, 1, WIPER_PWRON, 0); 
		              break;

      //关闭雨刷开关
      case '9':   
                  NET_DVR_PTZControl_Other(lUserID, 1, WIPER_PWRON, 1); 
		              break;

      //云台上仰
      case 'w':   
                  NET_DVR_PTZControlWithSpeed_Other(lUserID, 1, TILT_UP, 0, 7); 
                  usleep(10);
                  key = 107; 
			            break;
      //云台下俯
      case 's':   
                  NET_DVR_PTZControlWithSpeed_Other(lUserID, 1, TILT_DOWN, 0, 7);
                  usleep(10);
                  key = 98; 
			            break;

      // 云台左转
      case 'a':   
                  NET_DVR_PTZControlWithSpeed_Other(lUserID, 1, PAN_LEFT, 0, 7); 
                  usleep(10);
                  key = 110; 
			            break;
      // 云台右转
      case 'd':   
                  NET_DVR_PTZControlWithSpeed_Other(lUserID, 1, PAN_RIGHT, 0, 7);
                  usleep(10);
                  key = 109; 
			            break;

      case 'f':  
                  if(!image.empty())
                  {
                    num += 1;
                    dir = image_name + std::to_string(num) + image_type;
                    cv::imwrite(dir, image);
                    printf("save current image!");
                  }
                  key = 0;
			            break;

      case 'g':  Demo_PlayBack(lUserID);
			          break;
	  
	    case 'v':   video_recording = !video_recording;		//录制视频
			            if(video_recording)	printf(" start video recording\n");
			            else 			printf(" stop video recording\n");
			            break;

      // 停止云台上仰
      case 'k':   
                  NET_DVR_PTZControlWithSpeed_Other(lUserID, 1, TILT_UP, 1, 3); 
			            break;  

      // 停止云台下俯
      case 'b':   
                  NET_DVR_PTZControlWithSpeed_Other(lUserID, 1, TILT_DOWN, 1, 3);
			            break; 
      // 停止云台左转
      case 'n':   
                  NET_DVR_PTZControlWithSpeed_Other(lUserID, 1, PAN_LEFT, 1, 3);
			            break; 
      // 停止云台右转
      case 'm':   
                  NET_DVR_PTZControlWithSpeed_Other(lUserID, 1, PAN_RIGHT, 1, 3);
			            break; 
	    case  3:    goto exit;break;
	    default : 	
                  // printf(" wait\n");
                  // key = !key;
                  break;

	  }	
	  ros::spinOnce();//循环等待回调函数
  }

  close_keyboard();	
  Init_runing = 0;
  key_runing = 0;
  Camera_runing = 0; 
  pthread_exit(NULL);
  NET_DVR_Cleanup();  // 释放资源,防止内存泄漏
  return 0;
 
exit:
  close_keyboard();	
  Init_runing = 0;
  key_runing = 0;
  Camera_runing = 0; 
  pthread_exit(NULL);
  NET_DVR_Cleanup();  // 释放资源,防止内存泄漏
  return 0;
}
