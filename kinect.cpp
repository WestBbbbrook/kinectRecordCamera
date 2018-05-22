#include <windows.h>
#include <iostream> 
#include <NuiApi.h>
#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
using namespace std;
using namespace cv;
//Kinect录制并同时存储深度和彩色图像
//kinect 
int main()
{
	cout << "input 1 to  store video , input 0 is not"<<endl;
	int store = 0;
	cin >> store;

 string colorImgPath="F:\\kinect_vedio\\color\\";
 string depthImgPath="F:\\kinect_vedio\\depth\\";
 Mat image_rgb, image_depth;
 image_rgb.create(480, 640, CV_8UC3); //初始化3通道的彩色图像
 image_depth.create(480, 640, CV_8UC1);//初始化单通道的深度图像

 //1、初始化NUI 使用彩色数据和不带游戏者ID的深度数据
 HRESULT hr = NuiInitialize(NUI_INITIALIZE_FLAG_USES_COLOR | NUI_INITIALIZE_FLAG_USES_DEPTH);
 if (FAILED(hr))
 {
  cout<<"NuiInitialize failed"<<endl;
  return hr;
 }

 //2、定义事件句柄
 //创建读取下一帧的信号事件句柄，控制KINECT是否可以开始读取下一帧数据
 HANDLE nextColorFrameEvent = CreateEvent( NULL, TRUE, FALSE, NULL );
 HANDLE colorStreamHandle = NULL; //保存彩色图像数据流的句柄，用以提取数据
 HANDLE nextDepthFrameEvent = CreateEvent( NULL, TRUE, FALSE, NULL );
 HANDLE depthStreamHandle = NULL; //保存深度图像数据流的句柄，用以提取数据

 //3、打开KINECT设备的彩色图信息通道，并用colorStreamHandle保存该流的句柄，以便于以后读取
 hr = NuiImageStreamOpen(NUI_IMAGE_TYPE_COLOR, NUI_IMAGE_RESOLUTION_640x480,
  0, 2, nextColorFrameEvent, &colorStreamHandle);
 if( FAILED( hr ) )//判断是否提取正确
 {
  cout<<"Could not open color image stream video"<<endl;
  NuiShutdown();
  return hr;
 }
 hr = NuiImageStreamOpen(NUI_IMAGE_TYPE_DEPTH, NUI_IMAGE_RESOLUTION_640x480,
  0, 2, nextDepthFrameEvent, &depthStreamHandle);
 if( FAILED( hr ) )//判断是否提取正确
 {
  cout<<"Could not open depth image stream video"<<endl;
  NuiShutdown();
  return hr;
 }
 namedWindow("colorImage", CV_WINDOW_AUTOSIZE);
 namedWindow("depthImage", CV_WINDOW_AUTOSIZE);
 //4、开始读取彩色图数据和深度图数据
 int num_colorImg=0;
 int num_depthImg=0;
 while(1)
 {
  const NUI_IMAGE_FRAME * pImageFrame_rgb = NULL;
  const NUI_IMAGE_FRAME * pImageFrame_depth = NULL;

  //4.1、无限等待新的彩色数据，等到后返回
  if (WaitForSingleObject(nextColorFrameEvent, INFINITE)==0)
  {
   //4.2、从刚才打开数据流的流句柄中得到该帧数据，读取到的数据地址存于pImageFrame
   hr = NuiImageStreamGetNextFrame(colorStreamHandle, 0, &pImageFrame_rgb);
   if (FAILED(hr))
   {
    cout<<"Could not get color image"<<endl;
    NuiShutdown();
    return -1;
   }

   INuiFrameTexture * pTexture = pImageFrame_rgb->pFrameTexture;
   NUI_LOCKED_RECT LockedRect;

   //4.3、提取数据帧到LockedRect，它包括两个数据对象：pitch每行字节数，pBits第一个字节地址
   //并锁定数据，这样当我们读数据的时候，kinect就不会去修改它
   pTexture->LockRect(0, &LockedRect, NULL, 0);
   //4.4、确认获得的数据是否有效
   if( LockedRect.Pitch != 0 )
   {   

    //4.5、将数据转换为OpenCV的Mat格式
    for (int i=0; i<image_rgb.rows; i++) 
    {
     uchar *ptr = image_rgb.ptr(i);  //第i行的指针

     //每个字节代表一个颜色信息，直接使用uchar
     uchar *pBuffer = (uchar*)(LockedRect.pBits) + i * LockedRect.Pitch;
    for (int j=0; j<image_rgb.cols; j++) 
     {
      ptr[3*j] = pBuffer[4*j];  //内部数据是4个字节，0-1-2是BGR，第4个现在未使用
      ptr[3*j+1] = pBuffer[4*j+1];
      ptr[3*j+2] = pBuffer[4*j+2];
     }
    }
    imshow("colorImage", image_rgb); //显示图像
	
	if (store == 1){
		imwrite(colorImgPath + to_string(num_colorImg) + ".jpg", image_rgb);
	}
	num_colorImg++;
   }
   else
   {
    cout<<"Buffer length of received texture is bogus\r\n"<<endl;
   }
   //5、这帧已经处理完了，所以将其解锁
   pTexture->UnlockRect(0);
   //6、释放本帧数据，准备迎接下一帧
   NuiImageStreamReleaseFrame(colorStreamHandle, pImageFrame_rgb );
  }
  //7.1、无限等待新的深度数据，等到后返回
  if (WaitForSingleObject(nextDepthFrameEvent, INFINITE)==0)
  {
   //7.2、从刚才打开数据流的流句柄中得到该帧数据，读取到的数据地址存于pImageFrame
   hr = NuiImageStreamGetNextFrame(depthStreamHandle, 0, &pImageFrame_depth);
   if (FAILED(hr))
   {
    cout<<"Could not get color image"<<endl;
    NuiShutdown();
    return -1;
   }

   INuiFrameTexture * pTexture = pImageFrame_depth->pFrameTexture;
   NUI_LOCKED_RECT LockedRect;

   //7.3、提取数据帧到LockedRect，它包括两个数据对象：pitch每行字节数，pBits第一个字节地址
   //并锁定数据，这样当我们读数据的时候，kinect就不会去修改它
   pTexture->LockRect(0, &LockedRect, NULL, 0);
   //7.4、确认获得的数据是否有效
   if( LockedRect.Pitch != 0 )
   {

    //7.5、将数据转换为OpenCV的Mat格式
    for (int i=0; i<image_depth.rows; i++) 
    {
     uchar *ptr = image_depth.ptr(i);  //第i行的指针

     //每个字节代表一个颜色信息，直接使用uchar
     uchar *pBuffer = (uchar*)(LockedRect.pBits) + i * LockedRect.Pitch;
     USHORT *pBufferRun = (USHORT *)pBuffer;//这里需要转换，因为每个深度数据是2个字节，应将BYTE转成USHORT
    for (int j=0; j<image_depth.cols; j++) 
     {
      ptr[j] = 255-(BYTE)(256*pBufferRun[j]/0x1fff); //将数据归一化处理*
     }
    }
    imshow("depthImage", image_depth); //显示图像
	if (store == 1){
		imwrite(depthImgPath + to_string(num_depthImg) + ".jpg", image_depth);
	}
	num_depthImg++;
   }
   else
   {
    cout<<"Buffer length of received texture is bogus\r\n"<<endl;
   }

   //8、这帧已经处理完了，所以将其解锁
   pTexture->UnlockRect(0);
   //9、释放本帧数据，准备迎接下一帧
   NuiImageStreamReleaseFrame(depthStreamHandle, pImageFrame_depth );
  }

  if (cvWaitKey(20) == 27)
   break;
 }
 //10、关闭NUI链接
 NuiShutdown();
 return 0;
}
//int main(int argc, char *argv[])
//{
//	Mat colorImage;
//	Mat depthImage;
//	colorImage.create(480, 640, CV_8UC3);
//	depthImage.create(480,640,CV_8UC1);
// 
//    //1、初始化NUI 
//    HRESULT hr = NuiInitialize(NUI_INITIALIZE_FLAG_USES_COLOR|NUI_INITIALIZE_FLAG_USES_DEPTH); 
//    if (FAILED(hr)) 
//    { 
//        cout<<"NuiInitialize failed"<<endl; 
//        return hr; 
//    } 
//
//    //2、定义事件句柄 
//	//创建读取下一帧的信号事件句柄，控制KINECT是否可以开始读取下一帧数据
//    HANDLE nextColorFrameEvent = CreateEvent( NULL, TRUE, FALSE, NULL );
//    HANDLE colorStreamHandle = NULL; //保存彩色图像数据流的句柄，用以提取数据 
// 
//	HANDLE nextDepthFrameEvent = CreateEvent( NULL, TRUE, FALSE, NULL );
//    HANDLE depthStreamHandle = NULL; //保存深度图像数据流的句柄，用以提取数据 
//    //3、打开KINECT设备的彩色图信息通道，并用colorStreamHandle保存该流的句柄，以便于以后读取
//	hr = NuiImageStreamOpen(NUI_IMAGE_TYPE_COLOR, NUI_IMAGE_RESOLUTION_640x480, 
//							0, 2, nextColorFrameEvent, &colorStreamHandle); 
//    if( FAILED( hr ) )//判断是否提取正确 
//    { 
//        cout<<"Could not open color image stream video"<<endl; 
//        NuiShutdown(); 
//        return hr; 
//    }
//	namedWindow("colorImage", CV_WINDOW_AUTOSIZE);
//
//	hr = NuiImageStreamOpen(NUI_IMAGE_TYPE_COLOR, NUI_IMAGE_RESOLUTION_640x480, 
//							0, 2, nextDepthFrameEvent, &depthStreamHandle); 
//   if( FAILED( hr ) )//判断是否提取正确 
//    { 
//        cout<<"Could not open color image stream video"<<endl; 
//        NuiShutdown(); 
//        return hr; 
//    }
//    namedWindow("depthImage", CV_WINDOW_AUTOSIZE);
//	//4、开始读取彩色图数据 
//    while(1) 
//    { 
//        const NUI_IMAGE_FRAME * pColorImageFrame = NULL;
//		const NUI_IMAGE_FRAME * pDepthImageFrame = NULL;
//
//		//////4.1、无限等待新的数据，等到后返回
//  //      if (WaitForSingleObject(nextColorFrameEvent, INFINITE)==0) 
//  //      { 
//		//	//4.2、从刚才打开数据流的流句柄中得到该帧数据，读取到的数据地址存于pImageFrame
//  //          hr = NuiImageStreamGetNextFrame(colorStreamHandle, 0, &pColorImageFrame); 
//		//	if (FAILED(hr))
//		//	{
//		//		cout<<"Could not get color image"<<endl; 
//		//		NuiShutdown();
//		//		return -1;
//		//	}
//
//		//	INuiFrameTexture * pTexture = pColorImageFrame->pFrameTexture;
//		//	NUI_LOCKED_RECT LockedRect;
//
//		//	//4.3、提取数据帧到LockedRect，它包括两个数据对象：pitch每行字节数，pBits第一个字节地址
//		//	//并锁定数据，这样当我们读数据的时候，kinect就不会去修改它
//  //          pTexture->LockRect(0, &LockedRect, NULL, 0); 
//		//	//4.4、确认获得的数据是否有效
//  //          if( LockedRect.Pitch != 0 ) 
//  //          { 
//		//		//4.5、将数据转换为OpenCV的Mat格式
//		//		for (int i=0; i<colorImage.rows; i++) 
//  //              {
//		//			uchar *ptr = colorImage.ptr<uchar>(i);  //第i行的指针
//		//			
//		//			//每个字节代表一个颜色信息，直接使用uchar
//  //                  uchar *pBuffer = (uchar*)(LockedRect.pBits) + i * LockedRect.Pitch;
//  //                  for (int j=0; j<colorImage.cols; j++) 
//  //                  { 
//  //                      ptr[3*j] = pBuffer[4*j];  //内部数据是4个字节，0-1-2是BGR，第4个现在未使用 
//  //                      ptr[3*j+1] = pBuffer[4*j+1]; 
//  //                      ptr[3*j+2] = pBuffer[4*j+2]; 
//  //                  } 
//		//		} 
//  //              imshow("colorImage", colorImage); //显示图像 
//  //          } 
//  //          else 
//  //          { 
//  //              cout<<"Buffer length of received texture is bogus\r\n"<<endl; 
//  //          }
//
//		//	//5、这帧已经处理完了，所以将其解锁
//		//	pTexture->UnlockRect(0);
//  //          //6、释放本帧数据，准备迎接下一帧 
//  //          NuiImageStreamReleaseFrame(colorStreamHandle, pColorImageFrame ); 
//  //      }
//		//7.1、无限等待新的数据，等到后返回
//					cout<<6<<endl;
//
//        if (WaitForSingleObject(nextDepthFrameEvent, INFINITE)==0) 
//        { 
//						cout<<9<<endl;
//
//			//7.2、从刚才打开数据流的流句柄中得到该帧数据，读取到的数据地址存于pImageFrame
//            hr = NuiImageStreamGetNextFrame(depthStreamHandle, 0, &pDepthImageFrame); 
//			if (FAILED(hr))
//			{
//				cout<<"Could not get color image"<<endl; 
//				NuiShutdown();
//				return -1;
//			}
//						cout<<0<<endl;
//
//			INuiFrameTexture * pTexture = pDepthImageFrame->pFrameTexture;
//			NUI_LOCKED_RECT LockedRect;
//			cout<<1<<endl;
//			//7.3、提取数据帧到LockedRect，它包括两个数据对象：pitch每行字节数，pBits第一个字节地址
//			//并锁定数据，这样当我们读数据的时候，kinect就不会去修改它
//            pTexture->LockRect(0, &LockedRect, NULL, 0); 
//			//7.4、确认获得的数据是否有效
//						cout<<2<<endl;
//
//            if( LockedRect.Pitch != 0 ) 
//            { 
//				//7.5、将数据转换为OpenCV的Mat格式
//				for (int i=0; i<depthImage.rows; i++) 
//                {
//								cout<<3<<endl;
//
//					uchar *ptr = depthImage.ptr<uchar>(i);  //第i行的指针
//					
//					//每个字节代表一个颜色信息，直接使用uchar
//                    uchar *pBuffer = (uchar*)(LockedRect.pBits) + i * LockedRect.Pitch;
//                    USHORT *pBufferRun = (USHORT *)pBuffer;
//					for (int j=0; j<depthImage.cols; j++) 
//                    { 
//									cout<<4<<endl;
//
//                        ptr[j] = 255-(BYTE)(256*pBufferRun[j]/0x1fff); //将数据归一化处理*
//                    } 
//								cout<<5<<endl;
//
//				} 
//                imshow("depthImage", depthImage); //显示图像 
//            } 
//            else 
//            { 
//                cout<<"Buffer length of received texture is bogus\r\n"<<endl; 
//            }
//
//			//5、这帧已经处理完了，所以将其解锁
//			pTexture->UnlockRect(0);
//            //6、释放本帧数据，准备迎接下一帧 
//            NuiImageStreamReleaseFrame(depthStreamHandle, pDepthImageFrame ); 
//        }
//		cout<<"a"<<endl;
//        if (cvWaitKey(20) == 27) 
//            break; 
//    } 
//    //7、关闭NUI链接 
//    NuiShutdown(); 
//	return 0;
//}