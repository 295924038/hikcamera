#include "hikcamera.h"
#include <iostream>
#include <thread>
#include <QDebug>
#include <boost/date_time.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/filesystem.hpp>
#include "base/data_types.h"
#include "camera_image.h"

using namespace std;
using namespace chrono;
using namespace cv;

#define MAX_BUF_SIZE (1920*1200*3)

camera::camera():
    _status{DISCONNECTED},
    m_hDevHandle{nullptr}
{
    connectDevice();
}

camera::~camera()
{
    disconnectDevice();
}

void camera::connectDevice()
{
    std::lock_guard<std::recursive_mutex> lck(_mtx);
    if( DISCONNECTED == _status)
    {
        unsigned nIndex = 0;
        int nRet = MV_OK;
        MV_CC_DEVICE_INFO_LIST m_stDevList;
        memset(&m_stDevList, 0, sizeof(MV_CC_DEVICE_INFO_LIST));
        // ch:枚举子网内所有设备
        nRet = EnumDevices(MV_USB_DEVICE, &m_stDevList);
        if (MV_OK != nRet)
        {
            qDebug()<<"Enum devices error"<<endl;
            return;
        }

        cout<<"device count : "<< m_stDevList.nDeviceNum <<endl;

        if(!IsDeviceAccessible(m_stDevList.pDeviceInfo[nIndex], MV_ACCESS_Control))
        {
            qDebug()<<"access device failed!"<<nRet<<endl;
            return;
        }
        nRet = Open(m_stDevList.pDeviceInfo[nIndex]);
        if (MV_OK != nRet)
        {
            qDebug()<<"Open device failed!"<<nRet<<endl;
            return;
        }
        m_ptrFrameBuf = nullptr;
        m_ptrFrameBuf = (unsigned char*)malloc(MAX_BUF_SIZE);
        nRet = SetEnumValue("PixelFormat",PixelType_Gvsp_RGB8_Packed);
        if (MV_OK != nRet)
        {
            qDebug()<<"set pixelFormat rgb8 failed!"<<nRet<<endl;
            return;
        }
        nRet = SetEnumValue("TriggerMode",MV_TRIGGER_MODE_OFF);
        if (MV_OK != nRet)
        {
            qDebug()<<"set triggerMode off failed!"<<nRet<<endl;
            return;
        }
        nRet = SetEnumValue("ExposureAuto",MV_EXPOSURE_AUTO_MODE_OFF);
        if (MV_OK != nRet)
        {
            qDebug()<<"set exposureAuto off failed!"<<nRet<<endl;
            return;
        }

        nRet = StartGrabbing();
        if (MV_OK != nRet)
        {
            qDebug()<<"Start grabbing failed"<<nRet<<endl;
            return;
        }
        _status = CONNECTED;
    }
}

void camera::disconnectDevice()
{
    std::lock_guard<std::recursive_mutex> lck(_mtx);
    if(CONNECTED == _status)
    {
        StopGrabbing();
    }
    if(m_ptrFrameBuf)
    {
        free(m_ptrFrameBuf);
        delete m_ptrFrameBuf;
    }
    Close();
    _status = DISCONNECTED;
}

cv::Mat camera::takePicture()
{
    std::lock_guard<std::recursive_mutex> lck(_mtx);
    int nRet = MV_OK;
    cv::Mat matRet;
    if( DISCONNECTED == _status || nullptr == m_hDevHandle)
    {
        qDebug()<<"device disconnected or devHandle is NULL!"<<endl;
        return matRet;
    }
    try
    {
        //        unsigned char* m_ptrFrameBuf = {0};
        //        m_ptrFrameBuf = (unsigned char*)malloc(MAX_BUF_SIZE*2);
        //        if(m_ptrFrameBuf == NULL)
        //        {
        //            cout<<"malloc faield"<<endl;
        //            return matRet;
        //        }
        MV_FRAME_OUT_INFO_EX pFrameInfo = {0};
        memset(&pFrameInfo,0,sizeof(MV_FRAME_OUT_INFO_EX));

        nRet = GetOneFrameTimeout(m_ptrFrameBuf,MAX_BUF_SIZE,&pFrameInfo,1000);
        if(MV_OK != nRet)
        {
            qDebug()<<"get frame failed"<<nRet<<endl;
            _status = ERR;
            throw "get oneFrame fialed!";
        }

        RGB2BGR(m_ptrFrameBuf, pFrameInfo.nWidth, pFrameInfo.nHeight);
        matRet = cv::Mat(pFrameInfo.nHeight, pFrameInfo.nWidth, CV_8UC3, m_ptrFrameBuf);
        if(matRet.data == NULL)
        {
            cout<<"get no image"<<endl;
            return matRet;
        }
        {
            std::lock_guard<std::recursive_mutex> lck(_mtxMat) ;
            _mat = matRet ;
        }
        _timePicture = steady_clock::now() ;
        memset(m_ptrFrameBuf,0,sizeof(MAX_BUF_SIZE*2));
        //        free(m_ptrFrameBuf);
        //        m_ptrFrameBuf = nullptr;

    }catch(const char * errInfo)
    {
        qDebug()<< errInfo << endl;
        resetCamera();
    }
    return _mat;
}

void camera::resetCamera()
{
    disconnectDevice();
    connectDevice();
}

int camera::setGain(float fValue)
{
    return MV_CC_SetGain(m_hDevHandle,fValue);
}

int camera::setExposure(float fValue)
{
    return MV_CC_SetExposureTime(m_hDevHandle, fValue);
}

void camera::setRoi(int witdh,int height,int offsetX,int offsetY)
{
    MV_CC_SetIntValue(m_hDevHandle,"Width",witdh);
    MV_CC_SetIntValue(m_hDevHandle,"Height",height);
    MV_CC_SetIntValue(m_hDevHandle,"OffsetX",offsetX);
    MV_CC_SetIntValue(m_hDevHandle,"OffsetY",offsetY);
}

int camera::EnumDevices(unsigned int nTLayerType, MV_CC_DEVICE_INFO_LIST* pstDevList)
{
    return MV_CC_EnumDevices(nTLayerType, pstDevList);
}

bool camera::IsDeviceAccessible(MV_CC_DEVICE_INFO* pstDevInfo, unsigned int nAccessMode)
{
    return MV_CC_IsDeviceAccessible(pstDevInfo, nAccessMode);
}

int camera::Open(MV_CC_DEVICE_INFO* pstDeviceInfo)
{
    if (MV_NULL == pstDeviceInfo)
    {
        return MV_E_PARAMETER;
    }

    if (m_hDevHandle)
    {
        return MV_E_CALLORDER;
    }

    int nRet  = MV_CC_CreateHandle(&m_hDevHandle, pstDeviceInfo);
    if (MV_OK != nRet)
    {
        return nRet;
    }

    nRet = MV_CC_OpenDevice(m_hDevHandle);
    if (MV_OK != nRet)
    {
        MV_CC_DestroyHandle(m_hDevHandle);
        m_hDevHandle = MV_NULL;
    }

    return nRet;
}

int camera::Close()
{
    if (MV_NULL == m_hDevHandle)
    {
        return MV_E_HANDLE;
    }

    MV_CC_CloseDevice(m_hDevHandle);

    int nRet = MV_CC_DestroyHandle(m_hDevHandle);
    m_hDevHandle = MV_NULL;

    return nRet;
}

bool camera::IsDeviceConnected()
{
    return MV_CC_IsDeviceConnected(m_hDevHandle);
}

int camera::StartGrabbing()
{
    return MV_CC_StartGrabbing(m_hDevHandle);
}

int camera::StopGrabbing()
{
    return MV_CC_StopGrabbing(m_hDevHandle);
}

int camera::GetOneFrameTimeout(unsigned char* pData, unsigned int nDataSize, MV_FRAME_OUT_INFO_EX* pFrameInfo, int nMsec)
{
    return MV_CC_GetOneFrameTimeout(m_hDevHandle, pData, nDataSize, pFrameInfo, nMsec);
}

int camera::SetImageNodeNum(unsigned int nNum)
{
    return MV_CC_SetImageNodeNum(m_hDevHandle, nNum);
}

int camera::GetDeviceInfo(MV_CC_DEVICE_INFO* pstDevInfo)
{
    return MV_CC_GetDeviceInfo(m_hDevHandle, pstDevInfo);
}

int camera::GetIntValue(IN const char* strKey, OUT MVCC_INTVALUE_EX *pIntValue)
{
    return MV_CC_GetIntValueEx(m_hDevHandle, strKey, pIntValue);
}

int camera::SetIntValue(IN const char* strKey, IN int64_t nValue)
{
    return MV_CC_SetIntValueEx(m_hDevHandle, strKey, nValue);
}

int camera::GetEnumValue(IN const char* strKey, OUT MVCC_ENUMVALUE *pEnumValue)
{
    return MV_CC_GetEnumValue(m_hDevHandle, strKey, pEnumValue);
}

int camera::SetEnumValue(IN const char* strKey, IN unsigned int nValue)
{
    return MV_CC_SetEnumValue(m_hDevHandle, strKey, nValue);
}

int camera::SetEnumValueByString(IN const char* strKey, IN const char* sValue)
{
    return MV_CC_SetEnumValueByString(m_hDevHandle, strKey, sValue);
}

int camera::GetFloatValue(IN const char* strKey, OUT MVCC_FLOATVALUE *pFloatValue)
{
    return MV_CC_GetFloatValue(m_hDevHandle, strKey, pFloatValue);
}

int camera::SetFloatValue(IN const char* strKey, IN float fValue)
{
    return MV_CC_SetFloatValue(m_hDevHandle, strKey, fValue);
}

int camera::GetBoolValue(IN const char* strKey, OUT bool *pbValue)
{
    return MV_CC_GetBoolValue(m_hDevHandle, strKey, pbValue);
}

int camera::SetBoolValue(IN const char* strKey, IN bool bValue)
{
    return MV_CC_SetBoolValue(m_hDevHandle, strKey, bValue);
}

int camera::GetStringValue(IN const char* strKey, MVCC_STRINGVALUE *pStringValue)
{
    return MV_CC_GetStringValue(m_hDevHandle, strKey, pStringValue);
}

int camera::SetStringValue(IN const char* strKey, IN const char* strValue)
{
    return MV_CC_SetStringValue(m_hDevHandle, strKey, strValue);
}

int RGB2BGR( unsigned char* pRgbData, unsigned int nWidth, unsigned int nHeight )
{
    if ( NULL == pRgbData )
    {
        return MV_E_PARAMETER;
    }

    for (unsigned j = 0; j < nHeight; j++)
    {
        for (unsigned int i = 0; i < nWidth; i++)
        {
            unsigned char red = pRgbData[j*(nWidth*3)+i*3];
            pRgbData[j*(nWidth*3)+i*3] = pRgbData[j*(nWidth*3)+i*3+2];
            pRgbData[j*(nWidth*3)+i*3+2] = red;
        }
    }
    return MV_OK;
}

const cv::Mat camera::getImg() const
{
    Millsecond timeNow = steady_clock::now() ;
    if  ((timeNow - _timePicture)  > MillDiff(20)) {
        const_cast<camera*>(this)->takePicture() ;
    }
    return _mat ;
}

