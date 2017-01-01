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

#pragma pack()

typedef DEVICE_INFO *LPDEVICE_INFO;

extern "C" LPDEVICE_INFO DLLSPEC initDeviceInfoStruct();
extern "C" LPWSTR DLLSPEC getName(LPDEVICE_INFO device);
extern "C" int DLLSPEC getUSBname(LPDEVICE_INFO device);
extern "C" int DLLSPEC initUsb(LPDEVICE_INFO device);
extern "C" int DLLSPEC getStatus(LPDEVICE_INFO device, LPDEVICE_STATUS status);
extern "C" int DLLSPEC setStatus(LPDEVICE_INFO device, LPDEVICE_STATUS status);
extern "C" int DLLSPEC sendPoint(LPDEVICE_INFO device, LPDEVICE_POINT status);
extern "C" int DLLSPEC freeUSB(LPDEVICE_INFO device);