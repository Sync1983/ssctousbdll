#define STATE_GET_CLASS_DEVS			0x1000
#define STATE_ENUM_DEVICE_INTERFACE		0x1001
#define STATE_ALLOC_TEXT				0x1002
#define STATE_GET_DEVICE_IFACE_DETAIL	0x1003

#define EP_GET_STATUS	0
#define EP_RAW_DATA		1
#define	EP_SET_STATUS	2

#if defined(USB_IMPORT)
#pragma message("import")
#define DLLSPEC  __declspec(dllexport)
DEFINE_GUID(GUID_DEVINTERFACE_SyncStepControl,
	0xec4430e3, 0x362f, 0x4fd3, 0xb8, 0x28, 0x45, 0x17, 0x71, 0x71, 0xeb, 0x10);

#else
#pragma message("export")
#define DLLSPEC  __declspec(dllimport)

#endif

#pragma pack(1)

typedef struct {
	LPWSTR	name;
	HANDLE	hDev;
	WINUSB_INTERFACE_HANDLE		iHandle;
	USB_INTERFACE_DESCRIPTOR	ifaceDescriptor;
	WINUSB_PIPE_INFORMATION	   *pipeInfo;
	WINUSB_INTERFACE_HANDLE		assocIFace;
} DEVICE_INFO;

typedef struct {
	WORD	posX;
	WORD	posY;
	WORD	posZ;
	BYTE	coils;
} DEVICE_POINT, *LPDEVICE_POINT;

typedef struct {
	DEVICE_POINT point;
	BYTE	status;
	BYTE	inputs;
	DWORD	position;
} DEVICE_STATUS, *LPDEVICE_STATUS;

typedef struct  {
	UINT8 bmRequestType;   /**< Type of the request. */
	UINT8 bRequest;        /**< Request command code. */
	UINT8 wValueL;          /**< wValue parameter of the request. */
	UINT8 wValueH;          /**< wValue parameter of the request. */
	UINT8 wIndexL;          /**< wIndex parameter of the request. */
	UINT8 wIndexH;          /**< wIndex parameter of the request. */
	UINT8 wLengthL;         /**< Length of the data to transfer in bytes. */
	UINT8 wLengthH;         /**< Length of the data to transfer in bytes. */
	UINT8 extend[8];
} USB_Request_Header, *LPUSB_Request_Header;

#pragma pack()

typedef DEVICE_INFO *LPDEVICE_INFO;

extern "C" LPDEVICE_INFO DLLSPEC initDeviceInfoStruct();
extern "C" LPWSTR DLLSPEC getName(LPDEVICE_INFO device);
extern "C" int DLLSPEC getUSBname(LPDEVICE_INFO device);
extern "C" int DLLSPEC initUsb(LPDEVICE_INFO device);
extern "C" int DLLSPEC getStatus(LPDEVICE_INFO device, LPDEVICE_STATUS status);
extern "C" int DLLSPEC setStatus(LPDEVICE_INFO device, LPDEVICE_STATUS status);
extern "C" int DLLSPEC sendPoint(LPDEVICE_INFO device, LPDEVICE_POINT status);
extern "C" int DLLSPEC sendCommand(LPDEVICE_INFO device, UINT8 cmd, UINT16 wIndex, UINT16 wValue, void *Buf, UINT8 bufSize);
extern "C" int DLLSPEC freeUSB(LPDEVICE_INFO device);