#include <initguid.h>
#include <Windows.h>
#include <guiddef.h>
#include <SetupAPI.h>
#include <Strsafe.h>
#include <winusb.h>
#include <SetupAPI.h>
#include <stdlib.h>
#include <Windows.h>
#include "SSCtoUSBdll.h"

int freeUSB(LPDEVICE_INFO device) {
	if (device->pipeInfo) {
		delete[] device->pipeInfo;
	}
	if (device->iHandle) {
		WinUsb_Free(device->iHandle);
	}
	if (device->hDev) {
		CloseHandle(device->hDev);
	}
	if (device->name) {
		free(device->name);
	}	
	return 0;
}

LPDEVICE_INFO initDeviceInfoStruct() {
	LPDEVICE_INFO strct;
	strct = (LPDEVICE_INFO)malloc(sizeof(DEVICE_INFO));
	memset(strct, 0, sizeof(DEVICE_INFO));
	return strct;
}

LPWSTR getName(LPDEVICE_INFO device) {
	return device->name;
}

int getUSBname(LPDEVICE_INFO device) {
	int			state	= 0;	
	LPGUID		DEV_GUID = (LPGUID)&GUID_DEVINTERFACE_SyncStepControl;
	HDEVINFO	HardwareDeviceInfo;
	SP_DEVICE_INTERFACE_DATA			DeviceInterfaceData;
	PSP_DEVICE_INTERFACE_DETAIL_DATA	DeviceInterfaceDetailData = NULL;
	int			bResult;
	ULONG		Length, RequiredLength = 0;	

	HardwareDeviceInfo = SetupDiGetClassDevs(DEV_GUID, NULL, NULL, (DIGCF_PRESENT | DIGCF_DEVICEINTERFACE));

	if (HardwareDeviceInfo == INVALID_HANDLE_VALUE) {
		state = STATE_GET_CLASS_DEVS;
		return state;
	}

	DeviceInterfaceData.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);

	bResult = SetupDiEnumDeviceInterfaces(HardwareDeviceInfo, 0, DEV_GUID, 0, &DeviceInterfaceData);
	if (!bResult) {
		state = STATE_ENUM_DEVICE_INTERFACE;
		SetupDiDestroyDeviceInfoList(HardwareDeviceInfo);
		return state;
	}

	SetupDiGetDeviceInterfaceDetail(HardwareDeviceInfo, &DeviceInterfaceData, NULL, 0, &RequiredLength, NULL);

	DeviceInterfaceDetailData = (PSP_DEVICE_INTERFACE_DETAIL_DATA)LocalAlloc(LMEM_FIXED, RequiredLength);
	if (DeviceInterfaceDetailData == NULL) {
		state = STATE_ALLOC_TEXT;
		SetupDiDestroyDeviceInfoList(HardwareDeviceInfo);
		return state;
	}

	DeviceInterfaceDetailData->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);
	Length = RequiredLength;
	bResult = SetupDiGetDeviceInterfaceDetail(HardwareDeviceInfo, &DeviceInterfaceData, DeviceInterfaceDetailData, Length, &RequiredLength, NULL);

	if (!bResult) {
		state = STATE_GET_DEVICE_IFACE_DETAIL;
		SetupDiDestroyDeviceInfoList(HardwareDeviceInfo);
		LocalFree(DeviceInterfaceDetailData);
		return state;
	}

	RequiredLength = (RequiredLength + 1) * 2;				//Convert size to UTF16 format

	device->name = (LPWSTR)malloc(RequiredLength);
	memchr((void*)device->name, 0, RequiredLength);

	MultiByteToWideChar(CP_ACP, 0, DeviceInterfaceDetailData->DevicePath, -1, device->name, RequiredLength);

	SetupDiDestroyDeviceInfoList(HardwareDeviceInfo);
	LocalFree(DeviceInterfaceDetailData);	
	
	return 1;
}

int	initUsb(LPDEVICE_INFO device) {	

	device->hDev = CreateFileW(	device->name, 
						GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, 
						NULL, 
						OPEN_EXISTING, 
						FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED, 
						NULL);

	if (device->hDev == NULL) {
		return -1;
	}

	if (WinUsb_Initialize(device->hDev, &device->iHandle) == 0) {
		return -1;
	}

	if (WinUsb_QueryInterfaceSettings(device->iHandle, 0, &device->ifaceDescriptor) == 0) {
		return -1;
	}
	
	WinUsb_GetAssociatedInterface(device->iHandle, 0, &device->assocIFace);

	WORD epCount = device->ifaceDescriptor.bNumEndpoints;
	device->pipeInfo = new WINUSB_PIPE_INFORMATION[epCount];
	int state = 0;
	for (WORD i = 0; i < epCount; i++) {
		if (WinUsb_QueryPipe(device->iHandle, 0, i, &device->pipeInfo[i]) == 0) {
			state++;
		}
	}

	if (state == epCount) {
		return -1;
	}
	

	return  0;
}

int getStatus(LPDEVICE_INFO device, LPDEVICE_STATUS status) {
	ULONG readLen;	

	if (WinUsb_ReadPipe(device->iHandle, 
						device->pipeInfo[EP_GET_STATUS].PipeId, 
						(PUCHAR) status, 
						sizeof(DEVICE_STATUS), 
						&readLen, 
						NULL) == 0) {
		return 0;
	}
	else if (readLen != sizeof(DEVICE_STATUS)) {
		return 0;
	}
	return 1;
}

int setStatus(LPDEVICE_INFO device, LPDEVICE_STATUS status) {
	ULONG writeLen;

	if (WinUsb_WritePipe(	device->iHandle,
							device->pipeInfo[EP_SET_STATUS].PipeId,
							(PUCHAR)status,
							sizeof(DEVICE_STATUS),
							&writeLen,
							NULL) == 0) {
		return 0;
	}
	else if (writeLen != sizeof(DEVICE_STATUS)) {
		return 0;
	}
	return 1;
}

int sendPoint(LPDEVICE_INFO device, LPDEVICE_POINT point) {
	ULONG writeLen;

	if (WinUsb_WritePipe(	device->iHandle,
							device->pipeInfo[EP_RAW_DATA].PipeId,
							(PUCHAR)point,
							sizeof(DEVICE_POINT),
							&writeLen,
							NULL) == 0) {
		return 0;
	}
	else if (writeLen != sizeof(DEVICE_POINT)) {
		return 0;
	}
	return 1;
}

int sendCommand(LPDEVICE_INFO device, UINT8 cmd, UINT16 wIndex, UINT16 wValue, void *Buf, UINT8 bufSize) {
	ULONG writeLen, sendLen = sizeof(WINUSB_SETUP_PACKET);
	WINUSB_SETUP_PACKET setup;
	
	if (Buf != NULL) {
		sendLen += bufSize;
	}

	setup.RequestType	= 0x80;
	setup.Request		= cmd;
	setup.Index			= wIndex;
	setup.Value			= wValue;
	setup.Length		= sendLen;	

	if (WinUsb_ControlTransfer(	device->iHandle,
								setup,
								(PUCHAR)Buf,
								bufSize,
								&writeLen,
								NULL) == 0) {
		return 0;
	}
	else if ( writeLen != sendLen ) {
		return 0;
	}

	return 1;
}
