#include <windows.h>
#include <iostream> 
#include <NuiApi.h>
#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
using namespace std;
using namespace cv;
//Kinect¼�Ʋ�ͬʱ�洢��ȺͲ�ɫͼ��
//kinect 
int main()
{
	cout << "input 1 to  store video , input 0 is not"<<endl;
	int store = 0;
	cin >> store;

 string colorImgPath="F:\\kinect_vedio\\color\\";
 string depthImgPath="F:\\kinect_vedio\\depth\\";
 Mat image_rgb, image_depth;
 image_rgb.create(480, 640, CV_8UC3); //��ʼ��3ͨ���Ĳ�ɫͼ��
 image_depth.create(480, 640, CV_8UC1);//��ʼ����ͨ�������ͼ��

 //1����ʼ��NUI ʹ�ò�ɫ���ݺͲ�����Ϸ��ID���������
 HRESULT hr = NuiInitialize(NUI_INITIALIZE_FLAG_USES_COLOR | NUI_INITIALIZE_FLAG_USES_DEPTH);
 if (FAILED(hr))
 {
  cout<<"NuiInitialize failed"<<endl;
  return hr;
 }

 //2�������¼����
 //������ȡ��һ֡���ź��¼����������KINECT�Ƿ���Կ�ʼ��ȡ��һ֡����
 HANDLE nextColorFrameEvent = CreateEvent( NULL, TRUE, FALSE, NULL );
 HANDLE colorStreamHandle = NULL; //�����ɫͼ���������ľ����������ȡ����
 HANDLE nextDepthFrameEvent = CreateEvent( NULL, TRUE, FALSE, NULL );
 HANDLE depthStreamHandle = NULL; //�������ͼ���������ľ����������ȡ����

 //3����KINECT�豸�Ĳ�ɫͼ��Ϣͨ��������colorStreamHandle��������ľ�����Ա����Ժ��ȡ
 hr = NuiImageStreamOpen(NUI_IMAGE_TYPE_COLOR, NUI_IMAGE_RESOLUTION_640x480,
  0, 2, nextColorFrameEvent, &colorStreamHandle);
 if( FAILED( hr ) )//�ж��Ƿ���ȡ��ȷ
 {
  cout<<"Could not open color image stream video"<<endl;
  NuiShutdown();
  return hr;
 }
 hr = NuiImageStreamOpen(NUI_IMAGE_TYPE_DEPTH, NUI_IMAGE_RESOLUTION_640x480,
  0, 2, nextDepthFrameEvent, &depthStreamHandle);
 if( FAILED( hr ) )//�ж��Ƿ���ȡ��ȷ
 {
  cout<<"Could not open depth image stream video"<<endl;
  NuiShutdown();
  return hr;
 }
 namedWindow("colorImage", CV_WINDOW_AUTOSIZE);
 namedWindow("depthImage", CV_WINDOW_AUTOSIZE);
 //4����ʼ��ȡ��ɫͼ���ݺ����ͼ����
 int num_colorImg=0;
 int num_depthImg=0;
 while(1)
 {
  const NUI_IMAGE_FRAME * pImageFrame_rgb = NULL;
  const NUI_IMAGE_FRAME * pImageFrame_depth = NULL;

  //4.1�����޵ȴ��µĲ�ɫ���ݣ��ȵ��󷵻�
  if (WaitForSingleObject(nextColorFrameEvent, INFINITE)==0)
  {
   //4.2���ӸղŴ���������������еõ���֡���ݣ���ȡ�������ݵ�ַ����pImageFrame
   hr = NuiImageStreamGetNextFrame(colorStreamHandle, 0, &pImageFrame_rgb);
   if (FAILED(hr))
   {
    cout<<"Could not get color image"<<endl;
    NuiShutdown();
    return -1;
   }

   INuiFrameTexture * pTexture = pImageFrame_rgb->pFrameTexture;
   NUI_LOCKED_RECT LockedRect;

   //4.3����ȡ����֡��LockedRect���������������ݶ���pitchÿ���ֽ�����pBits��һ���ֽڵ�ַ
   //���������ݣ����������Ƕ����ݵ�ʱ��kinect�Ͳ���ȥ�޸���
   pTexture->LockRect(0, &LockedRect, NULL, 0);
   //4.4��ȷ�ϻ�õ������Ƿ���Ч
   if( LockedRect.Pitch != 0 )
   {   

    //4.5��������ת��ΪOpenCV��Mat��ʽ
    for (int i=0; i<image_rgb.rows; i++) 
    {
     uchar *ptr = image_rgb.ptr(i);  //��i�е�ָ��

     //ÿ���ֽڴ���һ����ɫ��Ϣ��ֱ��ʹ��uchar
     uchar *pBuffer = (uchar*)(LockedRect.pBits) + i * LockedRect.Pitch;
    for (int j=0; j<image_rgb.cols; j++) 
     {
      ptr[3*j] = pBuffer[4*j];  //�ڲ�������4���ֽڣ�0-1-2��BGR����4������δʹ��
      ptr[3*j+1] = pBuffer[4*j+1];
      ptr[3*j+2] = pBuffer[4*j+2];
     }
    }
    imshow("colorImage", image_rgb); //��ʾͼ��
	
	if (store == 1){
		imwrite(colorImgPath + to_string(num_colorImg) + ".jpg", image_rgb);
	}
	num_colorImg++;
   }
   else
   {
    cout<<"Buffer length of received texture is bogus\r\n"<<endl;
   }
   //5����֡�Ѿ��������ˣ����Խ������
   pTexture->UnlockRect(0);
   //6���ͷű�֡���ݣ�׼��ӭ����һ֡
   NuiImageStreamReleaseFrame(colorStreamHandle, pImageFrame_rgb );
  }
  //7.1�����޵ȴ��µ�������ݣ��ȵ��󷵻�
  if (WaitForSingleObject(nextDepthFrameEvent, INFINITE)==0)
  {
   //7.2���ӸղŴ���������������еõ���֡���ݣ���ȡ�������ݵ�ַ����pImageFrame
   hr = NuiImageStreamGetNextFrame(depthStreamHandle, 0, &pImageFrame_depth);
   if (FAILED(hr))
   {
    cout<<"Could not get color image"<<endl;
    NuiShutdown();
    return -1;
   }

   INuiFrameTexture * pTexture = pImageFrame_depth->pFrameTexture;
   NUI_LOCKED_RECT LockedRect;

   //7.3����ȡ����֡��LockedRect���������������ݶ���pitchÿ���ֽ�����pBits��һ���ֽڵ�ַ
   //���������ݣ����������Ƕ����ݵ�ʱ��kinect�Ͳ���ȥ�޸���
   pTexture->LockRect(0, &LockedRect, NULL, 0);
   //7.4��ȷ�ϻ�õ������Ƿ���Ч
   if( LockedRect.Pitch != 0 )
   {

    //7.5��������ת��ΪOpenCV��Mat��ʽ
    for (int i=0; i<image_depth.rows; i++) 
    {
     uchar *ptr = image_depth.ptr(i);  //��i�е�ָ��

     //ÿ���ֽڴ���һ����ɫ��Ϣ��ֱ��ʹ��uchar
     uchar *pBuffer = (uchar*)(LockedRect.pBits) + i * LockedRect.Pitch;
     USHORT *pBufferRun = (USHORT *)pBuffer;//������Ҫת������Ϊÿ�����������2���ֽڣ�Ӧ��BYTEת��USHORT
    for (int j=0; j<image_depth.cols; j++) 
     {
      ptr[j] = 255-(BYTE)(256*pBufferRun[j]/0x1fff); //�����ݹ�һ������*
     }
    }
    imshow("depthImage", image_depth); //��ʾͼ��
	if (store == 1){
		imwrite(depthImgPath + to_string(num_depthImg) + ".jpg", image_depth);
	}
	num_depthImg++;
   }
   else
   {
    cout<<"Buffer length of received texture is bogus\r\n"<<endl;
   }

   //8����֡�Ѿ��������ˣ����Խ������
   pTexture->UnlockRect(0);
   //9���ͷű�֡���ݣ�׼��ӭ����һ֡
   NuiImageStreamReleaseFrame(depthStreamHandle, pImageFrame_depth );
  }

  if (cvWaitKey(20) == 27)
   break;
 }
 //10���ر�NUI����
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
//    //1����ʼ��NUI 
//    HRESULT hr = NuiInitialize(NUI_INITIALIZE_FLAG_USES_COLOR|NUI_INITIALIZE_FLAG_USES_DEPTH); 
//    if (FAILED(hr)) 
//    { 
//        cout<<"NuiInitialize failed"<<endl; 
//        return hr; 
//    } 
//
//    //2�������¼���� 
//	//������ȡ��һ֡���ź��¼����������KINECT�Ƿ���Կ�ʼ��ȡ��һ֡����
//    HANDLE nextColorFrameEvent = CreateEvent( NULL, TRUE, FALSE, NULL );
//    HANDLE colorStreamHandle = NULL; //�����ɫͼ���������ľ����������ȡ���� 
// 
//	HANDLE nextDepthFrameEvent = CreateEvent( NULL, TRUE, FALSE, NULL );
//    HANDLE depthStreamHandle = NULL; //�������ͼ���������ľ����������ȡ���� 
//    //3����KINECT�豸�Ĳ�ɫͼ��Ϣͨ��������colorStreamHandle��������ľ�����Ա����Ժ��ȡ
//	hr = NuiImageStreamOpen(NUI_IMAGE_TYPE_COLOR, NUI_IMAGE_RESOLUTION_640x480, 
//							0, 2, nextColorFrameEvent, &colorStreamHandle); 
//    if( FAILED( hr ) )//�ж��Ƿ���ȡ��ȷ 
//    { 
//        cout<<"Could not open color image stream video"<<endl; 
//        NuiShutdown(); 
//        return hr; 
//    }
//	namedWindow("colorImage", CV_WINDOW_AUTOSIZE);
//
//	hr = NuiImageStreamOpen(NUI_IMAGE_TYPE_COLOR, NUI_IMAGE_RESOLUTION_640x480, 
//							0, 2, nextDepthFrameEvent, &depthStreamHandle); 
//   if( FAILED( hr ) )//�ж��Ƿ���ȡ��ȷ 
//    { 
//        cout<<"Could not open color image stream video"<<endl; 
//        NuiShutdown(); 
//        return hr; 
//    }
//    namedWindow("depthImage", CV_WINDOW_AUTOSIZE);
//	//4����ʼ��ȡ��ɫͼ���� 
//    while(1) 
//    { 
//        const NUI_IMAGE_FRAME * pColorImageFrame = NULL;
//		const NUI_IMAGE_FRAME * pDepthImageFrame = NULL;
//
//		//////4.1�����޵ȴ��µ����ݣ��ȵ��󷵻�
//  //      if (WaitForSingleObject(nextColorFrameEvent, INFINITE)==0) 
//  //      { 
//		//	//4.2���ӸղŴ���������������еõ���֡���ݣ���ȡ�������ݵ�ַ����pImageFrame
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
//		//	//4.3����ȡ����֡��LockedRect���������������ݶ���pitchÿ���ֽ�����pBits��һ���ֽڵ�ַ
//		//	//���������ݣ����������Ƕ����ݵ�ʱ��kinect�Ͳ���ȥ�޸���
//  //          pTexture->LockRect(0, &LockedRect, NULL, 0); 
//		//	//4.4��ȷ�ϻ�õ������Ƿ���Ч
//  //          if( LockedRect.Pitch != 0 ) 
//  //          { 
//		//		//4.5��������ת��ΪOpenCV��Mat��ʽ
//		//		for (int i=0; i<colorImage.rows; i++) 
//  //              {
//		//			uchar *ptr = colorImage.ptr<uchar>(i);  //��i�е�ָ��
//		//			
//		//			//ÿ���ֽڴ���һ����ɫ��Ϣ��ֱ��ʹ��uchar
//  //                  uchar *pBuffer = (uchar*)(LockedRect.pBits) + i * LockedRect.Pitch;
//  //                  for (int j=0; j<colorImage.cols; j++) 
//  //                  { 
//  //                      ptr[3*j] = pBuffer[4*j];  //�ڲ�������4���ֽڣ�0-1-2��BGR����4������δʹ�� 
//  //                      ptr[3*j+1] = pBuffer[4*j+1]; 
//  //                      ptr[3*j+2] = pBuffer[4*j+2]; 
//  //                  } 
//		//		} 
//  //              imshow("colorImage", colorImage); //��ʾͼ�� 
//  //          } 
//  //          else 
//  //          { 
//  //              cout<<"Buffer length of received texture is bogus\r\n"<<endl; 
//  //          }
//
//		//	//5����֡�Ѿ��������ˣ����Խ������
//		//	pTexture->UnlockRect(0);
//  //          //6���ͷű�֡���ݣ�׼��ӭ����һ֡ 
//  //          NuiImageStreamReleaseFrame(colorStreamHandle, pColorImageFrame ); 
//  //      }
//		//7.1�����޵ȴ��µ����ݣ��ȵ��󷵻�
//					cout<<6<<endl;
//
//        if (WaitForSingleObject(nextDepthFrameEvent, INFINITE)==0) 
//        { 
//						cout<<9<<endl;
//
//			//7.2���ӸղŴ���������������еõ���֡���ݣ���ȡ�������ݵ�ַ����pImageFrame
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
//			//7.3����ȡ����֡��LockedRect���������������ݶ���pitchÿ���ֽ�����pBits��һ���ֽڵ�ַ
//			//���������ݣ����������Ƕ����ݵ�ʱ��kinect�Ͳ���ȥ�޸���
//            pTexture->LockRect(0, &LockedRect, NULL, 0); 
//			//7.4��ȷ�ϻ�õ������Ƿ���Ч
//						cout<<2<<endl;
//
//            if( LockedRect.Pitch != 0 ) 
//            { 
//				//7.5��������ת��ΪOpenCV��Mat��ʽ
//				for (int i=0; i<depthImage.rows; i++) 
//                {
//								cout<<3<<endl;
//
//					uchar *ptr = depthImage.ptr<uchar>(i);  //��i�е�ָ��
//					
//					//ÿ���ֽڴ���һ����ɫ��Ϣ��ֱ��ʹ��uchar
//                    uchar *pBuffer = (uchar*)(LockedRect.pBits) + i * LockedRect.Pitch;
//                    USHORT *pBufferRun = (USHORT *)pBuffer;
//					for (int j=0; j<depthImage.cols; j++) 
//                    { 
//									cout<<4<<endl;
//
//                        ptr[j] = 255-(BYTE)(256*pBufferRun[j]/0x1fff); //�����ݹ�һ������*
//                    } 
//								cout<<5<<endl;
//
//				} 
//                imshow("depthImage", depthImage); //��ʾͼ�� 
//            } 
//            else 
//            { 
//                cout<<"Buffer length of received texture is bogus\r\n"<<endl; 
//            }
//
//			//5����֡�Ѿ��������ˣ����Խ������
//			pTexture->UnlockRect(0);
//            //6���ͷű�֡���ݣ�׼��ӭ����һ֡ 
//            NuiImageStreamReleaseFrame(depthStreamHandle, pDepthImageFrame ); 
//        }
//		cout<<"a"<<endl;
//        if (cvWaitKey(20) == 27) 
//            break; 
//    } 
//    //7���ر�NUI���� 
//    NuiShutdown(); 
//	return 0;
//}