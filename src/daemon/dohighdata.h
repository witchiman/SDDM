
#include <errno.h>
#include <signal.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <dlfcn.h>
#include <termios.h>
#include <fcntl.h>
#include <pthread.h>

#include "XG_Protocol.h"

typedef int (*SendDataCallBack)(int total, int sent);

#define DEMO_MODEL 		0  //指静脉模块操作应用实例,指静脉设备支持UART串口或USB HID模式，不支持USB CDROM驱动模式
#define DEMO_LIB_1    1  //1:1验证算法库应用实例，精度较低，免费提供
#define DEMO_LIB_N    2  //1：N识别算法库应用实例，精度高，收费
#define DEMO_MODE     DEMO_MODEL

#define USB_DEV_N			2 	//最大支持USB设备的个数，编号从1开始




#define DataDeal_SUCCESS  1
#define DataDeal_FAIL  0


#define ENROLL_BUF_SIZE  20*1024
#define FILENAME_LEN    200
static char* so_file = "/usr/lib/libvein.so";
static char sQuality[3][20] = {"bad", "medium", "good"};
static int EnrollTempNum = 0;
static int EnrollMaxUser = 0;
static int EnrollUserNum = 0;
static int MaxTempNum;

static char mode[10] = "0";
static int ThreadStop = 1;
int usbNum = 0;

struct Thread_Argv
{
    UINT8 iDevAddress;
    HANDLE hDevHandle;
};

struct Thread_Argv thArgv;

//指静脉模块设备操作相关函数
typedef struct _FUN_COM
{
    //获取版本号
    int (*XG_GetVeinLibVer) (char *pVer);

    //获取当前连接的USB指静脉设备的个数，USB驱动必须是HID模式
    int (*XG_DetectUsbDev) ();

    //打开并连接指静脉设备
    int (*XG_OpenVeinDev) (char* pDev, int Baud, int Addr, UINT8 Password[16], int Len, PHANDLE pHandle);

    //关闭指静脉设备
    int (*XG_CloseVeinDev) (UINT8 Addr, HANDLE Handle);

    //发送一个指令包
    int (*XG_SendPacket) (UINT8 Addr, UINT8 Cmd, UINT8 Encode, UINT8 Len, UINT8* pData, HANDLE Handle);

    //接收一个指令包
    int (*XG_RecvPacket)(UINT8 Addr, UINT8* pData, HANDLE Handle);

    //写入数据
    int (*XG_WriteData) (UINT8 Addr, UINT8 Cmd, UINT8* pData, UINT32 size, HANDLE Handle);

    //读取数据
    int (*XG_ReadData) (UINT8 Addr, UINT8 Cmd, UINT8* pData, UINT32 size, HANDLE Handle);

    //更新指静脉固件
    int (*XG_Upgrade) (int iAddr, const char* fname, HANDLE Handle);

    //写入指静脉登记数据
    int (*XG_WriteDevEnrollData) (int iAddr, UINT32 User, UINT8 *pBuf, HANDLE Handle);

    //读取指静脉登记数据
    int (*XG_ReadDevEnrollData) (int iAddr, UINT32 User, UINT8 *pBuf, UINT32 *pSize, HANDLE Handle);

    //获取数据包发送状态，固件升级时间较长用于进度条
    int (*XG_SetCallBack) (HANDLE Handle, SendDataCallBack pSendData);

} FunCom_t, *pFunCom_t;

//指静脉算法库操作相关函数
typedef struct _FUN_VEIN
{
    //创建算法库实例
    int (*XGV_CreateVein) (PHANDLE pHandle, int UserNum);

    //销毁算法库实例
    int (*XGV_DestroyVein) (HANDLE hHandle);

    //登记
    int (*XGV_Enroll) (HANDLE hHandle, UINT32 User, UINT8* pBuf, UINT32 size, UINT8 CheckSameFinger, UINT16* pQuality);

    //验证，一般1：N识别比对时使用
    int (*XGV_Verify) (HANDLE hHandle, UINT32* pUser, UINT32 Num, UINT8* pBuf, UINT32 size, UINT8 Group, UINT16* pQuality);

    //1:1验证
    UINT8* (*XGV_CharaVerify) (HANDLE hHandle, UINT8* pEnroll, int EnrollSize, UINT8* pChara, int CharaSize);

    //获取1:1验证成功后自学习后的登记数据，可替换原有的登记数据
    int (*XGV_GetCharaVerifyLearn) (HANDLE hHandle, UINT8* pBuf);

    //设置安全等级，1,2,3，安全等级越高误识率越低
    int (*XGV_SetSecurity) (HANDLE hHandle, UINT8 Level);

    //获取登记数据
    int (*XGV_GetEnrollData) (HANDLE hHandle, UINT32 User, UINT8* pData, UINT32* pSize);

    //获取登记数据登记时候指定的用户ID
    int (*XGV_GetEnrollUserId) (HANDLE hHandle, UINT8* pData);

    //保存登记数据
    int (*XGV_SaveEnrollData) (HANDLE hHandle, UINT32 User, UINT8 Group, UINT8* pData, UINT16 Size);

    //删除登记数据
    int (*XGV_DelEnrollData) (HANDLE hHandle, UINT32 User);

    //获取登记用户数
    int (*XGV_GetEnrollNum) (HANDLE hHandle, UINT32 *pMaxUser, UINT32 *pMaxTempNum, UINT32 *pEnrollUser, UINT32 *pEnrollTempNum);

    //获取空ID
    int (*XGV_GetEnptyID) (HANDLE hHandle, UINT32 *pEnptyId, UINT32 StartId, UINT32 EndId);

    //获取指定用户登记的模板数
    int (*XGV_GetUserTempNum) (HANDLE hHandle, UINT32 User, UINT32 *pTempNum);
} FunVein_t, *pFunVein_t;

static FunCom_t m_ComFun;
static FunVein_t m_VeinFun;





static HANDLE hDevHandle = 0;

static HANDLE hDevHandleArray[USB_DEV_N];
static UINT8 DevId[USB_DEV_N] = {1, 2};
static int ThreadStopArray[USB_DEV_N] = {1, 1};

static unsigned char iDevAddress = 0;
static HANDLE VeinHandle = 0;
UINT8 charaData[ENROLL_BUF_SIZE];

static struct	Thread_Argv thArgvArray[USB_DEV_N];




int GetChara(UINT8* charaBuf, UINT8 iDevAddress, HANDLE hDevHandle);
void DelUser(UINT32 User, UINT8 iDevAddress, HANDLE hDevHandle);




static void* GetDlFun(void *Handle, char *pFunName)
{
    char* eInfo = NULL;
    void* fun = NULL;

    fun = dlsym(Handle, pFunName);
    eInfo = dlerror();
    if(eInfo != NULL)
    {
        printf("FUN:%s error:%s\n", pFunName, eInfo);
        return NULL;
    }
    return fun;
}



static int space_vein_init()
{
    int i;
    char* eInfo;
    void *hLibHandle = NULL;

    //memset(&m_ComFun, 0, sizeof(m_ComFun));
    memset(&m_VeinFun, 0, sizeof(m_VeinFun));

    printf("Open so:%s\n", so_file);
    hLibHandle = dlopen(so_file, RTLD_LAZY);
    if(hLibHandle == NULL)
    {
        eInfo = dlerror();
        printf("libVeinApi.so error...\n%s\n",eInfo);
        printf("VeinApiLinux Error\n");
        return -1;
    }

    m_ComFun.XG_GetVeinLibVer = GetDlFun(hLibHandle, "XG_GetVeinLibVer");
    m_ComFun.XG_DetectUsbDev = GetDlFun(hLibHandle, "XG_DetectUsbDev");
    m_ComFun.XG_OpenVeinDev = GetDlFun(hLibHandle, "XG_OpenVeinDev");
    m_ComFun.XG_CloseVeinDev = GetDlFun(hLibHandle, "XG_CloseVeinDev");
    m_ComFun.XG_SendPacket = GetDlFun(hLibHandle, "XG_SendPacket");
    m_ComFun.XG_RecvPacket = GetDlFun(hLibHandle, "XG_RecvPacket");
    m_ComFun.XG_WriteData = GetDlFun(hLibHandle, "XG_WriteData");
    m_ComFun.XG_ReadData = GetDlFun(hLibHandle, "XG_ReadData");
    m_ComFun.XG_Upgrade = GetDlFun(hLibHandle, "XG_Upgrade");
    m_ComFun.XG_WriteDevEnrollData = GetDlFun(hLibHandle, "XG_WriteDevEnrollData");
    m_ComFun.XG_ReadDevEnrollData = GetDlFun(hLibHandle, "XG_ReadDevEnrollData");
    m_ComFun.XG_SetCallBack = GetDlFun(hLibHandle, "XG_SetCallBack");

    m_VeinFun.XGV_CreateVein = GetDlFun(hLibHandle, "XGV_CreateVein");
    m_VeinFun.XGV_DestroyVein = GetDlFun(hLibHandle, "XGV_DestroyVein");
    m_VeinFun.XGV_SetSecurity = GetDlFun(hLibHandle, "XGV_SetSecurity");
    m_VeinFun.XGV_Enroll = GetDlFun(hLibHandle, "XGV_Enroll");
    m_VeinFun.XGV_Verify = GetDlFun(hLibHandle, "XGV_Verify");
    m_VeinFun.XGV_CharaVerify = GetDlFun(hLibHandle, "XGV_CharaVerify");
    m_VeinFun.XGV_GetCharaVerifyLearn = GetDlFun(hLibHandle, "XGV_GetCharaVerifyLearn");
    m_VeinFun.XGV_SaveEnrollData = GetDlFun(hLibHandle, "XGV_SaveEnrollData");
    m_VeinFun.XGV_GetEnrollData = GetDlFun(hLibHandle, "XGV_GetEnrollData");
    m_VeinFun.XGV_DelEnrollData = GetDlFun(hLibHandle, "XGV_DelEnrollData");
    m_VeinFun.XGV_GetEnrollNum = GetDlFun(hLibHandle, "XGV_GetEnrollNum");
    m_VeinFun.XGV_GetEnptyID = GetDlFun(hLibHandle, "XGV_GetEnptyID");
    m_VeinFun.XGV_GetUserTempNum = GetDlFun(hLibHandle, "XGV_GetUserTempNum");
    m_VeinFun.XGV_GetEnrollUserId = GetDlFun(hLibHandle, "XGV_GetEnrollUserId");

    printf("space vein init \n");
   return 0;
}



static void GetInput(char* input)
{
    char str[255];
    int len = 0;

    fflush(stdin);
    fgets(str, 255, stdin);
    len = strlen(str);
//printf("fgets:%s, len = %d\n", str, len);
    strncpy(input, str, len - 1);
    input[len -1] = 0;
}



static void getiDevAdr(char *str)
{

        memset(&hDevHandle, 0, sizeof(hDevHandle));


        iDevAddress = atoi(str);
        if(iDevAddress < 1 || iDevAddress > USB_DEV_N)
        {
            printf("DevID Error!\n");

        }
        memcpy(&hDevHandle, &hDevHandleArray[iDevAddress - 1], sizeof(hDevHandleArray[iDevAddress - 1]));
       // identifyTid = identifyTidArray[iDevAddress - 1];
        printf("iDevAddress %d, hDevHandle 0x%X\n", iDevAddress, (int)hDevHandleArray[iDevAddress - 1]);
        printf("get device adress success \n");

}





void ReadXgd(HANDLE hHandle, int start, int end)
{
    int i;
    int ret = 0;

    for (i = start; i < end; i++)
    {
        FILE *fp = NULL;
        char fname[200];
        UINT8 EnrollBuf[ENROLL_BUF_SIZE];
        int EnrollBufSize = 0;
        UINT32 EnrollId = 0, VerifyId = 0;

        sprintf(fname, "FV_U%d.xgd", i + 1 - start);

        fp = fopen(fname, "rb");
        if(fp)
        {
            fseek(fp, 0, SEEK_END);
            EnrollBufSize = ftell(fp);
            fseek(fp, 0, SEEK_SET);
            if(EnrollBufSize <= 0)
            {
                printf("USER %d, EnrollBufSize %d, error\n", i + 1, EnrollBufSize);
                continue;
            }
            printf("USER %d, EnrollBufSize %d\n", i+1, EnrollBufSize);
            fread(EnrollBuf, EnrollBufSize, 1, fp);
            fclose(fp);
            EnrollId = m_VeinFun.XGV_GetEnrollUserId(VeinHandle, EnrollBuf);
            if(EnrollId < 0)
            {
                printf("USER %d, EnrollId ERROR\n", i + 1);
                continue;
            }
            printf("Importing user %d enroll data to file %s\n", i+1, fname);
            ret = m_VeinFun.XGV_DelEnrollData(VeinHandle, i + 1);
            ret = m_VeinFun.XGV_SaveEnrollData(VeinHandle, i + 1, 0, EnrollBuf, EnrollBufSize);
            if(ret != XG_ERR_SUCCESS)
            {
                printf("USER %d, XG_SaveEnrollData ERROR %d\n", i + 1, ret);
            }
        }
    }
}



void ReadEnroll(UINT8 iDevAddress, HANDLE hDevHandle)
{

    int UserId = 0;
    char input[255];
    printf("Please input user ID:");
    GetInput(input);
    UserId = atoi(input);
    if(UserId < 1 || UserId > EnrollMaxUser)
    {
        printf("User %d illegal ID, ReadEnroll Fail\n", UserId);
        return;
    }
    if(m_VeinFun.XGV_GetUserTempNum && VeinHandle)
    {
        int ret = 0;
        char input[255];
        int size = 0;
        unsigned char buf[ENROLL_BUF_SIZE];
        UINT32 TempNum = 0;
        ret = m_VeinFun.XGV_GetUserTempNum(VeinHandle, UserId, &TempNum);
        if(ret == XG_ERR_SUCCESS)
        {
            if(TempNum > 0)
            {
                printf("User %d registered %d templates\n", UserId, TempNum);
                ret  =  m_VeinFun.XGV_GetEnrollData(VeinHandle, UserId, buf, &size);
                if(ret == XG_ERR_SUCCESS)
                {
                    if(size > 0)
                    {
                            char fname[100];
                            FILE* fp;
                            printf("XGV_GetEnrollData SUCCESS, size %d\n", size);
                            sprintf(fname, "FV_U%d.xgd", UserId);
                            fp = fopen(fname, "wb");
                            if(fp)
                            {
                                fwrite(buf, size, 1, fp);
                                fclose(fp);
                                printf("Enroll data save to %s file\n", fname);
                            }
                    }
                }
            }
            else
                printf("User %d not registered\n", UserId);
        }
        else
        {
            printf("ReadInfo Fail, ERROR %d\n", ret);
        }
    }
    return;


}

void WriteEnroll(UINT8 iDevAddress, HANDLE hDevHandle)
{

    FILE *fp = NULL;
    char fname[200];
    char input[100];
    int ret = 0;
    int size = 0;
    int UserId = 0;
    unsigned char data[ENROLL_BUF_SIZE];

    printf("Please input file name:");
    GetInput(fname);
    fp = fopen(fname, "rb");
    if(fp)
    {
        fseek(fp, 0, SEEK_END);
        size = ftell(fp);
        fseek(fp, 0, SEEK_SET);
        fread(data, size, 1, fp);
        fclose(fp);
    } else {
        printf("file name %s no find\n", fname);
        return;
    }

    printf("Please input user id:");
    GetInput(input);
    UserId = atoi(input);


    if(UserId < 1 || UserId > EnrollMaxUser)
    {
        printf("User %d illegal ID, WriteEnroll Fail\n", UserId);
        return;
    }
    ret = m_VeinFun.XGV_DelEnrollData(VeinHandle, UserId);
    ret = m_VeinFun.XGV_SaveEnrollData(VeinHandle, UserId, 0, data, size);
    if(ret == XG_ERR_INVALID_ID)
    {
            printf("User %d illegal\n", UserId);
            ret = XG_ERR_INVALID_ID;
    }

    if(ret == XG_ERR_SUCCESS)
    {
        printf("write user %d enroll file %s ok\n", UserId, fname);
    } else {
        printf("write user %d enroll file %s fail  ERROR: %d\n", UserId, fname, ret);
    }

}

int GetFingerStatus(UINT8 iDevAddress, HANDLE hDevHandle)
{
    int ret = -1;
    ret = m_ComFun.XG_SendPacket(iDevAddress, XG_CMD_FINGER_STATUS, 0, 0, NULL, hDevHandle) ;
    if(ret == XG_ERR_SUCCESS)
    {
        UINT8 data[16];
        ret = m_ComFun.XG_RecvPacket(iDevAddress, data, hDevHandle);
        if(ret > 0)
        {
            if(data[0] == XG_ERR_SUCCESS)
            {
                ret = data[1];
            }
        }
        else
        {
            printf("GetFingerStatus XG_RecvPacket ret %d, data[1] %d\n",ret, data[1]);
            ret = -1;
        }
    }else
    {
        printf("GetFingerStatus XG_SendPacket ret %d\n", ret);
    }
    return ret;
}

int WaitFingerInput(UINT8 iDevAddress, HANDLE hDevHandle)
{
    int FingerStatus = 0;
    time_t StartTime = 0, CurTime = 0;
    StartTime = time(NULL);
    while(1)
    {
        CurTime = time(NULL);
        if((CurTime - StartTime) > 10) return XG_ERR_TIME_OUT;
        FingerStatus = GetFingerStatus(iDevAddress, hDevHandle);
        if(FingerStatus < 0) return -1;
        if(FingerStatus > 0) return 1;
    }
    return 0;
}

int WaitFingerOutput(UINT8 iDevAddress, HANDLE hDevHandle)
{
    int FingerStatus = 0;
    time_t StartTime = 0, CurTime = 0;

    StartTime = time(NULL);

    while(1)
    {
        CurTime = time(NULL);
        if((CurTime - StartTime) > 10) return XG_ERR_TIME_OUT;
        FingerStatus = GetFingerStatus(iDevAddress, hDevHandle);
        if(FingerStatus < 0) return -1;
        if(FingerStatus == 0) return 1;
    }
    return 0;
}

void Enroll(int UserId, UINT8 iDevAddress, HANDLE hDevHandle)
{

    int ret = 0;
    int i, MaxEnrollNum = 3;
    int size = 0;
    unsigned char CharaBuf[ENROLL_BUF_SIZE];
    unsigned char EnrollBuf[ENROLL_BUF_SIZE];
    int id = UserId;
    if(DEMO_MODE == DEMO_LIB_N)
    {
        if(id < 1 || id > EnrollMaxUser)
        {
            printf("User %d illegal ID, Enroll Fail\n", id);
            return;
        }
    }
    if(DEMO_MODE == DEMO_LIB_1)
    {
        id = 1;
    }
    ret = m_VeinFun.XGV_DelEnrollData(VeinHandle, id);
    /*注意：必须保证每个User有独立的ID：UserId，否则有可能无法导入特征认证*/
    printf("MaxTempNum %d\n", MaxTempNum);
    for(i = 0; i < 3; i++)
    {
        if(i == 0)
        {
            printf("Please input finger...\n");
        } else {
            printf("Please input finger again...\n");
        }

        if(WaitFingerInput(iDevAddress, hDevHandle) == 1)
        {
            UINT8 data[2*1024];
            size = GetChara(data, iDevAddress, hDevHandle);
            if(size > 0)
            {
//#if (true)//手指重复登记检查
//				if(true)
//				{
//					UINT32 User = 0;
//					ret = m_VeinFun.XGV_Verify(VeinHandle, &User, 0, data, size, 0, 0);
//					if(ret == XG_ERR_SUCCESS)
//					{
//						ret = XG_ERR_DUPLICATION_ID;
//						printf("该手指已登记\n");
//						break;
//					}
//				}
//#endif
                ret = m_VeinFun.XGV_Enroll(VeinHandle, id, data, size, 0, 0);
                if(ret == XG_ERR_SUCCESS)
                {
                    WaitFingerOutput(iDevAddress, hDevHandle);
                } else if(ret == XG_ERR_NO_SAME_FINGER) {
                    printf("Not the same finger\n");
                    break;
                } else {
                    printf("Enroll Fail\n");
                    break;
                }
            } else {
                printf("GetChara Error, Exit...\n");
                ret = XG_ERR_NO_SUPPORT;
                break;
            }
        } else {
            printf("NO Finger, Timeout and Exit...\n");
            ret = XG_ERR_TIME_OUT;
            break;
        }
    }

    if(ret == XG_ERR_SUCCESS)
    {
        m_VeinFun.XGV_SaveEnrollData(VeinHandle, id, 0, NULL, 0);
        printf("User %d Enroll Success\n", UserId);

        if(m_VeinFun.XGV_GetEnrollData(VeinHandle, id, EnrollBuf, (unsigned int*)&size) == XG_ERR_SUCCESS)
        {
            FILE *fp = NULL;
            char fname[FILENAME_LEN];

            sprintf(fname, "/etc/space/FV_U%d.xgd", UserId);
            fp = fopen(fname, "wb");
            if(fp)
            {
                fwrite(EnrollBuf, size, 1, fp);
                fclose(fp);
                printf("Enroll Success, Save User %d Enroll Data to File %s \n", UserId, fname);
            }
        }
    } else {
        printf("Enroll Fail\n");
    }



}

//1：1验证

int ReadCharaVerify(UINT32 User)
{
    int ret = 0;
    int size = 0;
    unsigned char buf[ENROLL_BUF_SIZE];
    FILE *fp = NULL;
    char fname[200];
    int charaSize = 0;
    unsigned char charaData[ENROLL_BUF_SIZE];

    sprintf(fname, "/etc/space/FV_U%d.xgd", User);
    fp = fopen(fname, "rb");
    if(fp)
    {
        fseek(fp, 0, SEEK_END);
        size = ftell(fp);
        fseek(fp, 0, SEEK_SET);
        fread(buf, size, 1, fp);
        fclose(fp);
    } else {
        printf("file %s no find\n", fname);
        return;
    }
    printf("read %s size %d\n",fname, size);

    printf("Please input file name:");
    GetInput(fname);
    fp = fopen(fname, "rb");
    if(fp)
    {
        fseek(fp, 0, SEEK_END);
        charaSize = ftell(fp);
        fseek(fp, 0, SEEK_SET);
        fread(charaData, charaSize, 1, fp);
        fclose(fp);
    } else {
        printf("file %s no find\n",fname );
        return;
    }
    printf("read chara.xgd size %d\n", charaSize);
    if(charaSize > 0)
    {
        UINT8 *p = m_VeinFun.XGV_CharaVerify(VeinHandle, buf, size, charaData, charaSize);
        if(p != NULL)
        {
            /*验证成功后有必要更新登记信息，因为算法库有自学习功能，需要把优化后的特征信息保存起来*/
            /*注意：自学习功能会把没登记的特征自动补上*/
            /*为什么要自学习：气温的变化对静脉特征质量会有影响，小孩的静脉也会有个生长的问题，所以需要学习*/
            ret = m_VeinFun.XGV_GetEnrollData(VeinHandle, User, buf, &size);
            if(ret == XG_ERR_SUCCESS)
            {

                FILE *fp = NULL;
                sprintf(fname, "FV_U%d.xgd", User);
                fp = fopen(fname, "wb");
                if(fp)
                {
                    fwrite(buf, size, 1, fp);
                    fclose(fp);
                    printf("New Enroll Save to File %s OK\n", fname);
                }
                printf("Verify OK\n");
		return 1;
            }
        }
        else
        {
            printf("Verify Fail\n");
	    return 0;
        }
    }
}

int GetCharaVerify(UINT32 User)
{
    int ret = 0;
    int size = 0;
    unsigned char buf[ENROLL_BUF_SIZE];
    FILE *fp = NULL;
    char fname[200];
    int charaSize = 0;
    unsigned char charaData[ENROLL_BUF_SIZE];

    sprintf(fname, "/etc/space/FV_U%d.xgd", User);
    fp = fopen(fname, "rb");
    if(fp)
    {
        fseek(fp, 0, SEEK_END);
        size = ftell(fp);
        fseek(fp, 0, SEEK_SET);
        fread(buf, size, 1, fp);
        fclose(fp);
    } else {
        printf("file %s no find\n", fname);
        return;
    }
    printf("read %s size %d\n",fname, size);

    charaSize = GetChara(charaData, iDevAddress, hDevHandle);
    if(charaSize > 0)
    {
        UINT8 *p = m_VeinFun.XGV_CharaVerify(VeinHandle, buf, size, charaData, charaSize);
        if(p != NULL)
        {
		return 1;
            printf("Verify OK\n");
        }
        else
        {
	return 0;
            printf("Verify Fail\n");
        }
    }
}




//1:N识别
int Identify(UINT8 iDevAddress, HANDLE hDevHandle)
{

    printf("Please Input Finger...\n");
    if(WaitFingerInput(iDevAddress, hDevHandle) == 1)
    {
            UINT8 buf[2*1024];
            int size = 0;
            int ret;
            int User = 0;
            size = GetChara(buf, iDevAddress, hDevHandle);
            if(size > 0)
            {
                if(m_VeinFun.XGV_Verify && VeinHandle)
                {
                    ret = m_VeinFun.XGV_Verify(VeinHandle, &User, 0, buf, size, 0, 0);
                    if(ret == XG_ERR_SUCCESS)
                    {
                        printf("User %d Identify OK!!!!\n", User);
                        return User;
                    }
                }
            }
            printf("Identify Fail\n");
    }
        return 0;


}

void DelUser(UINT32 User, UINT8 iDevAddress, HANDLE hDevHandle)
{

    if(User < 1 || User > EnrollMaxUser)
    {
        printf("User %d Illegal ID, Delete Fail\n", User);
        return;
    }
    int ret = m_VeinFun.XGV_DelEnrollData(VeinHandle, User);
    if(ret == XG_ERR_SUCCESS)
    {
        //删除文件
        char path[16];
        sprintf(path, "FV_U%d.xgd", User);
        remove(path);
        printf("Delete User %d OK\n", User);
    }
    else
    {
        printf("Delete Fail\n");
    }

}

int GetChara(UINT8* charaData, UINT8 iDevAddress, HANDLE hDevHandle)
{
    //printf("Please input finger:\n");
    if(m_ComFun.XG_SendPacket(iDevAddress, XG_CMD_GET_CHARA, 0, 0, NULL, hDevHandle) == XG_ERR_SUCCESS)
    {
        UINT8 data[16];
        for(;;)
        {
            int ret = m_ComFun.XG_RecvPacket(iDevAddress, data, hDevHandle);
            if(ret > 0)
            {
                if(data[0] == XG_ERR_SUCCESS)
                {
                    int quality = data[3];
                    int size = data[1] + (data[2]<<8);
                    int ret = m_ComFun.XG_ReadData(iDevAddress, XG_CMD_GET_CHARA, charaData, size, hDevHandle);
                    if(ret == XG_ERR_SUCCESS)
                    {
                        FILE *fp = NULL;
                        char fname[32];

                        sprintf(fname, "/etc/space/chara.xgd");
                        fp = fopen(fname, "wb");
                        if(fp)
                        {
                            fwrite(charaData, size, 1, fp);
                            fclose(fp);
                            printf("GetChara OK\n");
                        }
                    }
                    else
                    {
                        printf("GetChara Fail, XG_ReadData ERROR: %d\n", ret);
                    }
                    return size;
                } else if(data[0] == XG_ERR_FAIL) {
                    if(data[1] == XG_ERR_TIME_OUT) {
                        printf("No Finger, Timeout...\n");
                    }
                    return 0;
                } else if(data[0] == XG_INPUT_FINGER) {
                    printf("Please Input Finger...\n");
                } else if(data[0] == XG_RELEASE_FINGER) {
                    printf("Please Release Finger...\n");
                } else {
                    printf("GetChara Fail!\n");
                    return 0;
                }
            }
        }
    } else {
        printf("GetChara Fail\n");
    }
    return 0;
}

int UpgradeInfo(int total, int sent)
{
    if(sent == 1)
    {
        printf("Upgrade! Please wait.\n");
    }
    printf("send packet %d, total %d\n", sent, total);
    return 0;
}

void *IdentifyThread(void *data)
{
        int user = 0;
        int ret = -1;
        struct Thread_Argv *tempArgv;
        tempArgv = (struct Thread_Argv*)data;
#ifdef USB_DEV_N
        while(!ThreadStopArray[(*tempArgv).iDevAddress - 1])
        {
            int status = GetFingerStatus((*tempArgv).iDevAddress, (*tempArgv).hDevHandle);
            if(status == 1 && user == 0)
            {
                user = Identify((*tempArgv).iDevAddress, (*tempArgv).hDevHandle);
            }
            else if(status == 0)
            {
                user = 0;
            }
            else if(status == -1)
            {
//				time_t now;
//				struct tm *timenow;
//				time(&now);
//				timenow = localtime(&now);
//				printf("Local time is %s/n",asctime(timenow));
                    usleep(50*1000);
//					printf("GetFingerStatus Fail\n");
            }
            usleep(50*1000);//The thread delay at least 50 ms
        }
#else
        while(!ThreadStop)
        {
            int status = GetFingerStatus((*tempArgv).iDevAddress, (*tempArgv).hDevHandle);
            if(status == 1 && user == 0)
            {
                user = Identify((*tempArgv).iDevAddress, (*tempArgv).hDevHandle);
            }
            else if(status == 0)
            {
                user = 0;
            }
            else if(status == -1)
            {
//				time_t now;
//				struct tm *timenow;
//				time(&now);
//				timenow = localtime(&now);
//				printf("Local time is %s/n",asctime(timenow));
                    usleep(50*1000);
//					printf("GetFingerStatus Fail\n");
            }
            usleep(50*1000);//The thread delay at least 50 ms
        }
#endif
}



static void space_veindev_data_get(char *dev,int ret)
 {

    if(m_ComFun.XG_OpenVeinDev)
    {
        char Password[16] = "00000000";
        if(strcmp(dev, "usb") == 0 || strcmp(dev, "USB") == 0)
        {

            if(m_ComFun.XG_DetectUsbDev)
            {
                int i = 0;
                usbNum = m_ComFun.XG_DetectUsbDev();
                if(usbNum > 0)
                {
                    for(i = 0; i < usbNum; i++)
                    {
                        sprintf(Password,"00000000");
                        iDevAddress = DevId[i];
                        ret = m_ComFun.XG_OpenVeinDev(dev, 0, iDevAddress, (UINT8*)Password, strlen(Password), &hDevHandleArray[i]);
                        printf("iDevAddress %d, ret %d, hDevHandle1 0x%X\n", iDevAddress, ret, (int)hDevHandleArray[i]);
                        thArgvArray[i].iDevAddress = DevId[i];
                        thArgvArray[i].hDevHandle = hDevHandleArray[i];
                    }
                }
                printf("usbNum %d\n", usbNum);
            }

        } else {
            int baud = 115200; //串口波特率
            ret = m_ComFun.XG_OpenVeinDev(dev, baud, iDevAddress, (UINT8*)Password, strlen(Password), &hDevHandle);
        }


    } else {
        printf("m_ComFun.XG_OpenVeinDev is NULL\n");
        return -1;
    }



    char Ver[100] = "";
    if(m_ComFun.XG_GetVeinLibVer(Ver) == XG_ERR_SUCCESS)
    {
        printf("XG_GetVeinLibVer %s\n", Ver);
    }

    if(m_VeinFun.XGV_CreateVein(&VeinHandle, 100) == XG_ERR_SUCCESS)
    {
        if(VeinHandle == NULL)
        {
            printf("Init Vein Lib Fail, VeinHandle is NULL\n");
            return;
        }
        printf("Init Vein Lib OK\n");
        ret = m_VeinFun.XGV_SetSecurity(VeinHandle, 1);
        printf("XGV_SetSecurity ret %d\n", ret);
        ReadXgd(VeinHandle, 0, 100);
    }

    if(m_VeinFun.XGV_CreateVein(&VeinHandle, 1) == XG_ERR_SUCCESS)
    {
        printf("Init Vein Lib OK\n");
        ret = m_VeinFun.XGV_SetSecurity(VeinHandle, 1);
        printf("XGV_SetSecurity ret %d\n", ret);
    }


    printf("space vein device init success \n");
 }


int enroll_Data(char *method,char *name)
{
    if(strcmp(method, "enroll") == 0)
    {
        Enroll(atoi(name),iDevAddress, hDevHandle);
        return DataDeal_SUCCESS;
    }
    else{
        return DataDeal_FAIL;
        }
}

int verify_Data(char *method,char *name)
{
    if(strcmp(method, "verify") == 0)
    {


       // Verify(atoi(name), iDevAddress, hDevHandle);
       if(1==GetCharaVerify(name))
	{
     	  return 1;
	}
		else
	return 0;   
     }
    else{
        return 0;
        }
}

