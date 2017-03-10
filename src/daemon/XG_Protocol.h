
#ifndef  _XG_PROTOCOL_
#define  _XG_PROTOCOL_

#if	defined(__cplusplus)
extern	"C"	{
#endif	/* defined(__cplusplus) */

#if !defined NULL
	#define NULL            0
#endif

typedef signed char         INT8, *PINT8;
typedef signed short        INT16, *PINT16;
typedef signed int          INT32, *PINT32;
typedef unsigned char       UINT8, *PUINT8;
typedef unsigned short      UINT16, *PUINT16;
typedef unsigned int        UINT32, *PUINT32;
typedef char 				*LPCSTR;
typedef void 				*HANDLE;
typedef HANDLE 				*PHANDLE;

/******************************通讯协议*****************************************/
#define XG_PREFIX_CODE                0xAABB //包标识

#define USB_DATA_PACKET_SIZE          4096-2
#define COM_DATA_PACKET_SIZE          512-2 //串口数据包最大为512字节
#define HID_DATA_PACKET_SIZE          (60*60)-2

/*设备相关指令*/
#define XG_CMD_CONNECTION             0x01 //连接设备,必须带8位以上密码，默认密码全为0(0x30)
#define XG_CMD_CLOSE_CONNECTION       0x02 //关闭连接
#define XG_CMD_GET_SYSTEM_INFO        0x03 //获取版本号和设置信息
#define XG_CMD_FACTORY_SETTING        0x04 //恢复出厂设置
#define XG_CMD_SET_DEVICE_ID          0x05 //设置设备编号0-255
#define XG_CMD_SET_BAUDRATE           0x06 //设置波特率0-4
#define XG_CMD_SET_SECURITYLEVEL      0x07 //设置安全等级0-4
#define XG_CMD_SET_TIMEOUT            0x08 //设置等待手指放入超时1-255秒
#define XG_CMD_SET_DUP_CHECK          0x09 //设置重复登录检查0-1
#define XG_CMD_SET_PASSWORD           0x0A //设置通信密码
#define XG_CMD_CHECK_PASSWORD         0x0B //检查密码是否正确
#define XG_CMD_REBOOT                 0x0C //复位重启设备
#define XG_CMD_SET_SAME_FV            0x0D //登记的时候检查是否为同一根手指
#define XG_CMD_SET_USB_MODE           0x0E //设置USB驱动模式 

/*识别相关指令*/
#define XG_CMD_FINGER_STATUS          0x10 //检测手指放置状态
#define XG_CMD_CLEAR_ENROLL           0x11 //清除指定ID登录数据
#define XG_CMD_CLEAR_ALL_ENROLL       0x12 //清除所有ID登录数据
#define XG_CMD_GET_EMPTY_ID           0x13 //获取空（无登录数据）ID
#define XG_CMD_GET_ENROLL_INFO        0x14 //获取总登录用户数和模板数
#define XG_CMD_GET_ID_INFO            0x15 //获取指定ID登录信息
#define XG_CMD_ENROLL                 0x16 //指定ID登录
#define XG_CMD_VERIFY                 0x17 //1:1认证或1:N识别
#define XG_CMD_IDENTIFY_FREE          0x18 //FREE识别模式，自动识别并发送状态
#define XG_CMD_CANCEL                 0x19 //取消FREE识别模式
#define XG_CMD_RAM_MODE               0x1A //内存操作模式
#define XG_CMD_VERIFY_MULTI           0x1B //连续多个ID验证

/******************************高级指令****************************************/
/*数据读写相关指令*/
#define XG_CMD_READ_DATA              0x20 //从设备读取数据
#define XG_CMD_WRITE_DATA             0x21 //写入数据到设备
#define XG_CMD_READ_ENROLL            0x22 //读取指定ID登录数据
#define XG_CMD_WRITE_ENROLL           0x23 //写入（覆盖）指定ID登录数据
#define XG_CMD_GET_VEIN_IMAGE         0X24 //采集指静脉图像数据到主机
#define XG_CMD_WRITE_IMAGE            0x25 //写入图像数据
#define XG_CMD_GET_DEBUG              0x26 //读取调试信息
#define XG_CMD_UPGRADE                0x27 //写入升级程序，重启后自动升级
#define XG_CMD_GET_CHARA              0x28 //采集并读取特征到主机 
#define XG_CMD_READ_USER_DATA         0x29 //从用户扩展存储区读取数据，最大4K
#define XG_CMD_WRITE_USER_DATA        0x2A //写入数据到用户扩展存储区，最大4K

/******************************扩展指令****************************************/
/*数据读写相关指令*/
#define XG_CMD_GET_DOORCTRL           0x30 //获取门禁设置 
#define XG_CMD_SET_DOORCTRL           0x31 //设置门禁设置
#define XG_CMD_OPEN_DOOR              0x32 //开门
#define XG_CMD_READ_LOG               0x33 //读取门禁出入日志
#define XG_CMD_SET_DEVNAME            0x34 //设置设备名称
#define XG_CMD_GET_DATETIME           0x35 //获取门禁实时时钟
#define XG_CMD_SET_DATETIME           0x36 //设置门禁实时时钟
#define XG_CMD_KEY_CTRL               0x37 //发送按键
#define XG_CMD_ENROLL_EXT             0x38 //扩展语音登记
#define XG_CMD_VERIFY_EXT             0x39 //扩展语音验证
#define XG_CMD_DEL_LOG                0x3a //删除门禁日志
#define XG_CMD_PLAY_VOICE             0x3b //播放语音
#define XG_CMD_GET_USER_NAME          0x3C //读取用户名称
#define XG_CMD_SET_USER_NAME          0x3D //修改写入用户名称
#define XG_CMD_GET_USER_CG            0x3E //读取用户卡号和组号 
#define XG_CMD_SET_USER_CG            0x3F //修改写入用户卡号和组号 
#define XG_CMD_DEL_USER_INFO          0x40 //情况所有用户信息 
#define XG_CMD_GET_ADMIN_PWD          0x41 //读取管理员密码，一个管理员可设置一个密码
#define XG_CMD_SET_ADMIN_PWD          0x42 //修改写入管理员密码 


/******************************错误代码****************************************/
#define	XG_ERR_SUCCESS                0x00 //操作成功
#define	XG_ERR_FAIL                   0x01 //操作失败
#define XG_ERR_COM                    0x02 //通讯错误
#define XG_ERR_DATA                   0x03 //数据校验错误
#define XG_ERR_INVALID_PWD            0x04 //密码错误
#define XG_ERR_INVALID_PARAM          0x05 //参数错误
#define XG_ERR_INVALID_ID             0x06 //ID错误
#define XG_ERR_EMPTY_ID               0x07 //指定ID为空（无登录数据）
#define XG_ERR_NOT_ENOUGH             0x08 //无足够登录空间
#define XG_ERR_NO_SAME_FINGER         0x09 //不是同一根手指
#define XG_ERR_DUPLICATION_ID         0x0A //有相同登录ID
#define XG_ERR_TIME_OUT               0x0B //等待手指输入超时
#define XG_ERR_VERIFY                 0x0C //认证失败
#define XG_ERR_NO_NULL_ID             0x0D //已无空ID
#define XG_ERR_BREAK_OFF              0x0E //操作中断 
#define XG_ERR_NO_CONNECT             0x0F //未连接
#define XG_ERR_NO_SUPPORT             0x10 //不支持此操作
#define XG_ERR_NO_VEIN                0x11 //无静脉数据 
#define XG_ERR_MEMORY                 0x12 //内存不足
#define XG_ERR_NO_DEV                 0x13 //设备不存在
#define XG_ERR_ADDRESS                0x14 //设备地址错误
#define XG_ERR_NO_FILE                0x15 //文件不存在
#define XG_ERR_LICENSE                0x80

/******************************状态代码****************************************/
#define XG_INPUT_FINGER               0x20 //请求放入手指
#define XG_RELEASE_FINGER             0x21 //请求拿开手指

#define API_EXPORTED	

///////////////////////////////////////////////////////////////////////////////////
//算法库操作函数
int API_EXPORTED	XG_GetVeinLibVer(char *pVer);

	/*********************************************************************
	功能：初始化静脉识别算法库
	参数：
	pHandle:句柄指针
	UserNum:用户数,1:1验证此参数为0或1， 1：N验证此参数最大为10000,可分组验证
	返回值：
	XG_ERR_INVALID_PARAM：参数错误
	XG_ERR_SUCCESS：初始化成功
	XG_ERR_FAIL：初始化失败
	**********************************************************************/
extern int API_EXPORTED	XG_CreateVein(PHANDLE pHandle, int UserNum);

	/*********************************************************************
	功能：退出关闭静脉识别算法库
	参数：
	无
	返回值：
	XG_ERR_SUCCESS：关闭成功
	**********************************************************************/
int API_EXPORTED	XG_DestroyVein(HANDLE hHandle);

int API_EXPORTED XG_ImgVeinChara(HANDLE hHandle, UINT8* pImg, UINT8* pChara, UINT32* pSize, UINT16* pQuality);

	/*********************************************************************
	功能：增加一个静脉特征到内存暂存
	参数：
	User(输入):用户编号，取值范围为1-最大用户编号
	pBuf(输入):静脉数据，图像数据或特征数据
	size(输入):静脉数据大小
	CheckSameFinger(输入):是否进行同一根手指的检测0：不检测，1：检测
	pQuality(输出):特征质量
	返回值：
	XG_ERR_INVALID_PARAM：参数错误
	XG_ERR_DATA：数据错误
	XG_ERR_NO_SAME_FINGER：不是同一根手指
	XG_ERR_SUCCESS：登记成功
	XG_ERR_FAIL：登记失败
	**********************************************************************/
int API_EXPORTED XG_Enroll(HANDLE hHandle, UINT32 User, UINT8* pBuf, UINT32 size, UINT8 CheckSameFinger, UINT16* pQuality);

	/*********************************************************************
	功能：验证特征
	参数：
	pUser(输入/输出):用户编号指针，0：1：N识别模式， 1-最大用户编号：1:1验证模式
	                 返回为0表示验证失败，1-最大用户编号为验证成功的用户编号
	pBuf(输入):静脉数据，图像数据或特征数据
	size(输入):静脉数据大小
	Group(输入):组号
	pQuality(输出):特征质量
	返回值：
	XG_ERR_INVALID_PARAM：参数错误
	XG_ERR_DATA：数据错误
	XG_ERR_SUCCESS：验证成功
	XG_ERR_FAIL：验证失败
	**********************************************************************/
int API_EXPORTED XG_Verify(HANDLE hHandle, UINT32* pUser, UINT8* pBuf, UINT32 size, UINT8 Group, UINT16* pQuality);

/*********************************************************************
	功能：多ID验证特征,适合一个用户登记多根手指的情况,必须是连续的多个ID
	参数：
	pUser(输入/输出):开始验证的ID号,取值范围1-最大ID号
	                 返回为0表示验证失败，1-最大用户编号为验证成功的用户编号
	Num:pUser开始的连续比对ID数
	pBuf(输入):静脉数据，图像数据或特征数据
	size(输入):静脉数据大小
	Group(输入):组号
	pQuality(输出):特征质量
	返回值：
	XG_ERR_INVALID_PARAM：参数错误
	XG_ERR_DATA：数据错误
	XG_ERR_SUCCESS：验证成功
	XG_ERR_FAIL：验证失败
	**********************************************************************/
int API_EXPORTED XG_MultiVerify(HANDLE hHandle, UINT32* pUser, UINT32 Num, UINT8* pBuf, UINT32 size, UINT8 Group, UINT16* pQuality);

	/*********************************************************************
	功能：设置安全等级
	参数：
	Level(输入):安全等级，1-3
	返回值：
	XG_ERR_SUCCESS：初始化成功
	XG_ERR_FAIL：初始化失败
	**********************************************************************/
int API_EXPORTED XG_SetSecurity(HANDLE hHandle, UINT8 Level);

	/*********************************************************************
	功能：获取已登记数据
	参数：
	User(输入):用户编号，0：读取当前内存登记数据，1-最大用户编号：指定用户编号登记数据
	pData(输出):读取登记数据的数据缓冲区指针
	pSize(输出):读取的登记数据大小
	返回值：
	XG_ERR_INVALID_PARAM：参数错误
	XG_ERR_SUCCESS：读取成功
	XG_ERR_FAIL：读取失败
	**********************************************************************/
int API_EXPORTED XG_GetEnrollData (HANDLE hHandle, UINT32 User, UINT8* pData, UINT32* pSize);

	/*********************************************************************
	功能：获取登记数据pData保存的用户ID
	参数：
	pData(输入):外部登记数据的数据缓冲区指针
	返回值：
	< 0：数据错误
	> 0：用户编号
	**********************************************************************/
int API_EXPORTED XG_GetEnrollUserId (HANDLE hHandle, UINT8* pData);

	/*********************************************************************
	功能：保存内存（或外部）登记数据到识别存储空间或内存
	参数：
	User(输入):用户编号，0：保存pData指定的登记数据到内存，1-最大用户编号：指定用户编号登记数据
	Group(输入):用户分组
	pData(输入):外部登记数据的数据缓冲区指针，为NULL时表示保存内部登记数据
	Size(输入):外部登记数据大小
	返回值：
	XG_ERR_INVALID_PARAM：参数错误
	XG_ERR_SUCCESS：保存成功
	XG_ERR_FAIL：保存失败
	**********************************************************************/
int API_EXPORTED XG_SaveEnrollData (HANDLE hHandle, UINT32 User, UINT8 Group, UINT8* pData, UINT16 Size);

	/*********************************************************************
	功能：删除指定用户编号存储空间和内存登记数据
	参数：
	User(输入):用户编号，1-最大用户编号：指定用户编号登记数据
	返回值：
	XG_ERR_INVALID_PARAM：参数错误
	XG_ERR_SUCCESS：保存成功
	**********************************************************************/
int API_EXPORTED XG_DelEnrollData (HANDLE hHandle, UINT32 User);

	/*********************************************************************
	功能：获取登记信息，有最大登记用户数和模板数，已登记用户数和模板数
	参数：
	pMaxUser(输出):指针
	pMaxTempNum(输出):指针
	pEnrollUser(输出):指针
	pEnrollTempNum(输出):指针
	返回值：
	XG_ERR_SUCCESS：成功
	**********************************************************************/
int API_EXPORTED XG_GetEnrollNum(HANDLE hHandle, UINT32 *pMaxUser, UINT32 *pMaxTempNum, UINT32 *pEnrollUser, UINT32 *pEnrollTempNum);

	/*********************************************************************
	功能：获取空（无登记）的用户编号
	参数：
	pEnptyId(输出):指针
	StartId和EndId：在此范围内获取空的用户编号
	返回值：
	XG_ERR_NO_NULL_ID：登记空间已满
	XG_ERR_SUCCESS：成功
	**********************************************************************/
int API_EXPORTED XG_GetEnptyID(HANDLE hHandle, UINT32 *pEnptyId, UINT32 StartId, UINT32 EndId);

/*********************************************************************
	功能：1:1验证
	参数：
	void* pEnroll:登记特征数据，base64编码字符串或二进制数据
	void* pChara：需要比对的静脉特征数据，base64编码字符串或二进制数据
	返回值：
	非空：验证成功，自学习后的特征登记数据
	null：验证失败
	**********************************************************************/
UINT8* XGV_CharaVerify(HANDLE hHandle, UINT8* pEnroll, int EnrollSize, UINT8* pChara, int CharaSize);

//获取自学习后的模板数据
int XGV_GetCharaVerifyLearn(HANDLE hHandle, UINT8 *pBuf);

/*********************************************************************
	功能：获取指定用户编号的登记模板数
	参数：
	User:用户ID
	pTempNum：输出模板数
	返回值：
	XG_ERR_SUCCESS：成功
	**********************************************************************/
int API_EXPORTED XG_GetUserTempNum(HANDLE hHandle, UINT32 User, UINT32 *pTempNum);

int API_EXPORTED	XG_DecodeEnrollData(UINT8 *pSrc, UINT8 *pDest);

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//设备操作函数
int API_EXPORTED	XG_GetComApiVer(char *pVer);

int API_EXPORTED XG_DetectUsbDev();
/*********************************************************************
功能：打开设备，如果打开设备成功同时发送XG_CMD_CONNECTION指令
参数：
Port(输入):端口号，0[USB端口]，1-10[COM1-COM10]
Baud(输入):波特率, 0-4[9600,19200,38400,57600,115200]
Addr(输入):如Port = 0，此参数用于选择多个USB设备，CDROM驱动优先；如Port>0此参数为设备地址
Password(输入/输出):输入时为连接密码，连接成功后输出设备名
Len(输入):密码长度>=8
返回值：
XG_ERR_SUCCESS:连接成功
XG_ERR_FAIL:连接失败
**********************************************************************/
int XG_OpenVeinDev(char* pDev, int Baud, int Addr, UINT8 Password[16], int Len, PHANDLE pHandle);

/*********************************************************************
功能：发送XG_CMD_CLOSE_CONNECTION指令成功后关闭设备
参数：
Addr(输入):设备地址,0[广播，所有设备都接收]，1-255[指定接收设备地址]
返回值：
XG_ERR_SUCCESS:关闭成功
XG_ERR_FAIL:关闭失败
**********************************************************************/
int API_EXPORTED XG_CloseVeinDev(UINT8 Addr, HANDLE Handle);

/*********************************************************************
功能：端口已经打开的情况下发送XG_CMD_CONNECTION连接指令
参数：
Addr(输入):设备地址,用于485联网时连接设备选择
Password(输入/输出):输入时为连接密码，连接成功后输出设备名
Len(输入):密码长度>=8
返回值：
XG_ERR_SUCCESS:连接成功
XG_ERR_FAIL:连接失败
**********************************************************************/
int API_EXPORTED  XG_Connect(UINT8 Addr, UINT8* Password, UINT8 Len, HANDLE Handle);

/*********************************************************************
功能：发送XG_CMD_CLOSE_CONNECTION断开连接指令
参数：
Addr(输入):设备地址,用于485联网时连接设备选择
返回值：
XG_ERR_SUCCESS:成功
XG_ERR_FAIL:失败
**********************************************************************/
int API_EXPORTED  XG_Disconnect(UINT8 Addr, HANDLE Handle);

/*********************************************************************
功能：发送一个指令包
参数：
Addr(输入):设备地址,0[广播，所有设备都接收]，1-255[指定接收设备地址]
Cmd(输入):指令号
Encode(输入):数据编码格式
Len(输入):数据长度
pData(输入):包数据
返回值：
XG_ERR_SUCCESS:指令包发送成功
XG_ERR_INVALID_PARAM:参数非法
XG_ERR_COM:设备或通信错误
**********************************************************************/
int API_EXPORTED	XG_SendPacket(UINT8 Addr, UINT8 Cmd, UINT8 Encode, UINT8 Len, UINT8* pData, HANDLE Handle);

/*********************************************************************
功能：接收一个指令包
参数：
Addr(输入):设备地址,0[广播，所有设备都接收]，1-255[指定接收设备地址]
pPacket(输出):接收到的返回包
返回值：
XG_ERR_SUCCESS:接收成功
XG_ERR_INVALID_PARAM:参数非法
XG_ERR_COM:设备或通信错误
XG_ERR_DATA:数据校验错误
**********************************************************************/
int API_EXPORTED	XG_RecvPacket(UINT8 Addr, UINT8* pData, HANDLE Handle);

/*********************************************************************
功能：从设备读取数据
参数：
Addr(输入):设备地址,0[广播，所有设备都接收]，1-255[指定接收设备地址]
Cmd(输入):指令号
pData(输出):用于读取数据的缓冲区
size(输入):读取数据的大小(字节)
返回值：
XG_ERR_SUCCESS:数据读取成功
XG_ERR_INVALID_PARAM:参数非法
XG_ERR_COM:设备或通信错误
XG_ERR_DATA:数据校验错误
XG_ERR_FAIL:读取数据失败
**********************************************************************/
int API_EXPORTED	XG_ReadData(UINT8 Addr, UINT8 Cmd, UINT8* pData, UINT32 size, HANDLE Handle);

/*********************************************************************
功能：写入数据到设备
参数：
Addr(输入):设备地址,0[广播，所有设备都接收]，1-255[指定接收设备地址]
Cmd(输入):指令号
pData(输入):需要写入的数据
size(输入):写入数据的大小(字节)
返回值：
XG_ERR_SUCCESS:数据写入成功
XG_ERR_INVALID_PARAM:参数非法
XG_ERR_COM:设备或通信错误
XG_ERR_DATA:数据校验错误
XG_ERR_FAIL:写入数据失败
**********************************************************************/
int API_EXPORTED	XG_WriteData(UINT8 Addr, UINT8 Cmd, UINT8* pData, UINT32 size, HANDLE Handle);

int API_EXPORTED	XG_ReadDevEnrollData(int iAddr, UINT32 User, UINT8 *pBuf, UINT32 *pSize, HANDLE Handle);
int API_EXPORTED	XG_WriteDevEnrollData(int iAddr, UINT32 User, UINT8 *pBuf, HANDLE Handle);
int API_EXPORTED	XG_ReadDevFlashData(int iAddr, UINT8 *pBuf, int offset, int size, HANDLE Handle);
int API_EXPORTED	XG_WriteDevFlashData(int iAddr, UINT8 *pBuf, int offset, int size, HANDLE Handle);

int API_EXPORTED	XG_Upgrade(int iAddr, const char* fname, HANDLE Handle);

int API_EXPORTED	XG_GetFingerStatus(int iAddr, HANDLE Handle);
int API_EXPORTED	XG_GetVeinChara(int iAddr, UINT8* pBuf, UINT32 *pSize, HANDLE Handle);

int XGV_VeinBmpMatch(char* fname1, char* fname2, int width, int height, int th);

#if	defined(__cplusplus)
}
#endif	/* defined(__cplusplus) */

#endif
