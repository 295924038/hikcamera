#ifndef HIKCAMERA_H
#define HIKCAMERA_H

#include "MvCameraControl.h"
#include <opencv2/opencv.hpp>
#include <mutex>
#include <chrono>

#include "base/data_types.h"

#ifndef MV_NULL
#define MV_NULL    0
#endif

class camera
{
public:
    camera();
    ~camera();

    enum Status{CONNECTED,DISCONNECTED,ERR};

    void connectDevice();

    void disconnectDevice();

    cv::Mat takePicture();

    int setGain(float fValue) ;

    int setExposure(float fValue) ;

    void setRoi(int witdh,int height,int offsetX,int offsetY);

    void resetCamera();

    const cv::Mat getImg() const;

    // ch:枚举设备 | en:Enumerate Device
    static int EnumDevices(unsigned int nTLayerType, MV_CC_DEVICE_INFO_LIST* pstDevList);

    // ch:判断设备是否可达 | en:Is the device accessible
    static bool IsDeviceAccessible(MV_CC_DEVICE_INFO* pstDevInfo, unsigned int nAccessMode);

    // ch:打开设备 | en:Open Device
    int Open(MV_CC_DEVICE_INFO* pstDeviceInfo);

    // ch:关闭设备 | en:Close Device
    int Close();

    // ch:判断相机是否处于连接状态 | en:Is The Device Connected
    bool IsDeviceConnected();

    // ch:开启抓图 | en:Start Grabbing
    int StartGrabbing();

    // ch:停止抓图 | en:Stop Grabbing
    int StopGrabbing();

    // ch:主动获取一帧图像数据 | en:Get one frame initiatively
    int GetOneFrameTimeout(unsigned char* pData, unsigned int nDataSize, MV_FRAME_OUT_INFO_EX* pFrameInfo, int nMsec);

    // ch:设置SDK内部图像缓存节点个数 | en:Set the number of the internal image cache nodes in SDK
    int SetImageNodeNum(unsigned int nNum);

    // ch:获取设备信息 | en:Get device information
    int GetDeviceInfo(MV_CC_DEVICE_INFO* pstDevInfo);

    // ch:获取U3V相机的统计信息 | en:Get detect info of U3V camera
    int GetU3VAllMatchInfo(MV_MATCH_INFO_USB_DETECT* pMatchInfoUSBDetect);

    // ch:获取和设置Int型参数，如 Width和Height，详细内容参考SDK安装目录下的 MvCameraNode.xlsx 文件
    // en:Get Int type parameters, such as Width and Height, for details please refer to MvCameraNode.xlsx file under SDK installation directory
    int GetIntValue(IN const char* strKey, OUT MVCC_INTVALUE_EX *pIntValue);
    int SetIntValue(IN const char* strKey, IN int64_t nValue);

    // ch:获取和设置Enum型参数，如 PixelFormat，详细内容参考SDK安装目录下的 MvCameraNode.xlsx 文件
    // en:Get Enum type parameters, such as PixelFormat, for details please refer to MvCameraNode.xlsx file under SDK installation directory
    int GetEnumValue(IN const char* strKey, OUT MVCC_ENUMVALUE *pEnumValue);
    int SetEnumValue(IN const char* strKey, IN unsigned int nValue);
    int SetEnumValueByString(IN const char* strKey, IN const char* sValue);

    // ch:获取和设置Float型参数，如 ExposureTime和Gain，详细内容参考SDK安装目录下的 MvCameraNode.xlsx 文件
    // en:Get Float type parameters, such as ExposureTime and Gain, for details please refer to MvCameraNode.xlsx file under SDK installation directory
    int GetFloatValue(IN const char* strKey, OUT MVCC_FLOATVALUE *pFloatValue);
    int SetFloatValue(IN const char* strKey, IN float fValue);

    // ch:获取和设置Bool型参数，如 ReverseX，详细内容参考SDK安装目录下的 MvCameraNode.xlsx 文件
    // en:Get Bool type parameters, such as ReverseX, for details please refer to MvCameraNode.xlsx file under SDK installation directory
    int GetBoolValue(IN const char* strKey, OUT bool *pbValue);
    int SetBoolValue(IN const char* strKey, IN bool bValue);

    // ch:获取和设置String型参数，如 DeviceUserID，详细内容参考SDK安装目录下的 MvCameraNode.xlsx 文件UserSetSave
    // en:Get String type parameters, such as DeviceUserID, for details please refer to MvCameraNode.xlsx file under SDK installation directory
    int GetStringValue(IN const char* strKey, MVCC_STRINGVALUE *pStringValue);
    int SetStringValue(IN const char* strKey, IN const char * strValue);


private:
    void* m_hDevHandle;
    Status _status;
    std::recursive_mutex _mtx;
    mutable std::recursive_mutex _mtxMat;
    cv::Mat _mat;
    Millsecond _timePicture;
    unsigned char* m_ptrFrameBuf;

};

int RGB2BGR( unsigned char* pRgbData, unsigned int nWidth, unsigned int nHeight );
#endif // HIKCAMERA_H
