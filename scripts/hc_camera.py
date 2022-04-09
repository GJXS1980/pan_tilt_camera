#! /usr/bin/env python3
# coding=gbk

import numpy as np
import os
import ctypes
 
 
#��ȡ���еĿ��ļ���һ���б�
path = "/home/grantli/test/src/pan_tilt_camera/lib/"
def file_name(file_dir):
    pathss=[]
    for root, dirs, files in os.walk(file_dir):
        for file in files:
          pathss.append(path+file)
    return pathss
 
dll_list=file_name(path)
 
lUserID = 0
lChannel=1
def callCpp(func_name,*args):
    for HK_dll in dll_list:
        try:
            lib = ctypes.cdll.LoadLibrary(HK_dll)
            try:
                value = eval("lib.%s"%func_name)(*args)
                # print("���õĿ⣺"+HK_dll)
                # print("ִ�гɹ�,����ֵ��"+str(value))
                return value
            except:
                continue
        except:
            # print("���ļ�����ʧ�ܣ�"+HK_dll)
            continue
    # print("û���ҵ��ӿڣ�")
    return False
 
# region ����
#�������ṹ��
class LPNET_DVR_DEVICEINFO_V30(ctypes.Structure):
    _fields_ = [
        ("sSerialNumber", ctypes.c_byte * 48),
        ("byAlarmInPortNum", ctypes.c_byte),
        ("byAlarmOutPortNum", ctypes.c_byte),
        ("byDiskNum", ctypes.c_byte),
        ("byDVRType", ctypes.c_byte),
        ("byChanNum", ctypes.c_byte),
        ("byStartChan", ctypes.c_byte),
        ("byAudioChanNum", ctypes.c_byte),
        ("byIPChanNum", ctypes.c_byte),
        ("byZeroChanNum", ctypes.c_byte),
        ("byMainProto", ctypes.c_byte),
        ("bySubProto", ctypes.c_byte),
        ("bySupport", ctypes.c_byte),
        ("bySupport1", ctypes.c_byte),
        ("bySupport2", ctypes.c_byte),
        ("wDevType", ctypes.c_uint16),
        ("bySupport3", ctypes.c_byte),
        ("byMultiStreamProto", ctypes.c_byte),
        ("byStartDChan", ctypes.c_byte),
        ("byStartDTalkChan", ctypes.c_byte),
        ("byHighDChanNum", ctypes.c_byte),
        ("bySupport4", ctypes.c_byte),
        ("byLanguageType", ctypes.c_byte),
        ("byVoiceInChanNum", ctypes.c_byte),
        ("byStartVoiceInChanNo", ctypes.c_byte),
        ("byRes3", ctypes.c_byte * 2),
        ("byMirrorChanNum", ctypes.c_byte),
        ("wStartMirrorChanNo", ctypes.c_uint16),
        ("byRes2", ctypes.c_byte * 2)]
 
#�û�ע���豸 �����룬��Ҫ�޸�IP,�˺š�����
def NET_DVR_Login_V30(sDVRIP = "192.168.5.64",wDVRPort = 8000,sUserName = "admin",sPassword = "abcd1234"):
    init_res = callCpp("NET_DVR_Init")#SDK��ʼ��
    if init_res:
        print("SDK��ʼ���ɹ�")
        error_info = callCpp("NET_DVR_GetLastError")
    else:
        error_info = callCpp("NET_DVR_GetLastError")
        print("SDK��ʼ������" + str(error_info))
        return False
 
    set_overtime = callCpp("NET_DVR_SetConnectTime",5000,4)#���ó�ʱ
    if set_overtime:
        print("���ó�ʱʱ��ɹ�")
    else:
        error_info = callCpp("NET_DVR_GetLastError")
        print("���ó�ʱ������Ϣ��" + str(error_info))
        return False
 
    #�û�ע���豸
    #c++���ݽ�ȥ����byte�����ݣ���Ҫת��byte�ʹ���ȥ�����������
    sDVRIP = bytes(sDVRIP,"ascii")
    sUserName = bytes(sUserName,"ascii")
    sPassword = bytes(sPassword,"ascii")
    print( "����ת���ɹ�")
    DeviceInfo = LPNET_DVR_DEVICEINFO_V30()
    print(DeviceInfo)
    lUserID = callCpp("NET_DVR_Login_V30",sDVRIP,wDVRPort,sUserName,sPassword,ctypes.byref(DeviceInfo))
    print("��¼�ɹ����û�ID��"+str(lUserID))
    if lUserID == -1:
        error_info = callCpp("NET_DVR_GetLastError")
        print("��¼������Ϣ��" + str(error_info))
        return error_info
    else:
        return lUserID
# endregion
 
# region Ԥ��
#����Ԥ���ṹ��
class NET_DVR_PREVIEWINFO(ctypes.Structure):
    _fields_ = [
        ("lChannel", ctypes.c_long),
        ("lLinkMode", ctypes.c_long),
        ("hPlayWnd", ctypes.c_void_p),
        ("sMultiCastIP", ctypes.c_char_p),
        ("byProtoType", ctypes.c_byte),
        ("byRes", ctypes.c_byte * 3)]
# Ԥ��ʵ��
def Preview():
    lpPreviewInfo=NET_DVR_PREVIEWINFO()
    # hPlayWnd��Ҫ���봴��ͼ�δ��ڵ�handle,û�������޷�ʵ��BMPץͼ
    lpPreviewInfo.hPlayWnd=0
    lpPreviewInfo.lChannel=1
    lpPreviewInfo.dwLinkMode=0
    lpPreviewInfo.sMultiCastIP=None
    m_lRealHandle=callCpp("NET_DVR_RealPlay_V30",lUserID,ctypes.byref(lpPreviewInfo),None,None,True)
    if(m_lRealHandle<0):
        error_info = callCpp("NET_DVR_GetLastError")
        print("Ԥ��ʧ�ܣ�" + str(error_info))
    else:
        print("Ԥ���ɹ�")
    return m_lRealHandle
# endregion
 
 
# # region ץͼ
# # BMPץͼԤ����ʱ��hPlayWnd��ʾ���ڲ���Ϊnone
# def Get_BMPPicture():
#     sBmpPicFileName = bytes("pytest.bmp", "ascii")
#     if(callCpp("NET_DVR_CapturePicture",m_lRealHandle,sBmpPicFileName)==False):
#         error_info = callCpp("NET_DVR_GetLastError")
#         print("ץͼʧ�ܣ�" + str(error_info))
#     else:
#         print("ץͼ�ɹ�")
#
# ץͼ���ݽṹ��
class NET_DVR_JPEGPARA(ctypes.Structure):
    _fields_ = [
        ("wPicSize", ctypes.c_ushort),
        ("wPicQuality", ctypes.c_ushort)]
 
# jpegץͼhPlayWnd��ʾ������Ϊnone������ȱ��ɼ�ͼƬ�ٶ���
def Get_JPEGpicture():
    sJpegPicFileName = bytes("pytest.jpg", "ascii")
    lpJpegPara=NET_DVR_JPEGPARA()
    lpJpegPara.wPicSize=0
    lpJpegPara.wPicQuality=0
    if (callCpp("NET_DVR_CaptureJPEGPicture", lUserID, lChannel, ctypes.byref(lpJpegPara), sJpegPicFileName)== False):
        error_info = callCpp("NET_DVR_GetLastError")
        print("ץͼʧ�ܣ�" + str(error_info))
    else:
        print("ץͼ�ɹ�")
# endregion
 
#�����ѧ�䱶�ṹ��
 
# ��ѧ�䱶�ṹ��
class NET_DVR_FOCUSMODE_CFG(ctypes.Structure):
    _fields_ = [
        ("dwSize", ctypes.c_uint32),
        ("byFocusMode", ctypes.c_byte),
        ("byAutoFocusMode", ctypes.c_byte),
        ("wMinFocusDistance", ctypes.c_uint16),
        ("byZoomSpeedLevel", ctypes.c_byte),
        ("byFocusSpeedLevel", ctypes.c_byte),
        ("byOpticalZoom", ctypes.c_byte),
        ("byDigtitalZoom", ctypes.c_byte),
        ("fOpticalZoomLevel", ctypes.c_float),
        ("dwFocusPos", ctypes.c_uint32),
        ("byFocusDefinitionDisplay", ctypes.c_byte),
        ("byFocusSensitivity", ctypes.c_byte),
        ("byRes1", ctypes.c_byte*2),
        ("dwRelativeFocusPos", ctypes.c_uint32),
        ("byRes", ctypes.c_byte * 48)]
 
# ��ȡ��ѧ�䱶ֵ
def get_CamZoom():
    m_struFocusModeCfg = NET_DVR_FOCUSMODE_CFG()
    m_struFocusModeCfg.byRes
    dwReturned = ctypes.c_uint16(0)
    print(callCpp("NET_DVR_GetDVRConfig"))
    if (callCpp("NET_DVR_GetDVRConfig", lUserID, 3305, lChannel, ctypes.byref(m_struFocusModeCfg), 76,
                ctypes.byref(dwReturned)) == False):
        error_info = callCpp("NET_DVR_GetLastError")
        print("��ѧ�䱶��ȡʧ�ܣ�" + str(error_info))
        ctypes.ARRAY()
    else:
        print("��ѧ�䱶��ȡ�ɹ�")
    return m_struFocusModeCfg.fOpticalZoomLevel
 
 
# �޸Ĺ�ѧ�䱶ֵ
def Change_CamZoom(zoomScale):
    m_struFocusModeCfg=NET_DVR_FOCUSMODE_CFG()
    dwReturned=ctypes.c_uint16(0)
    print(callCpp("NET_DVR_GetDVRConfig"))
    if (callCpp("NET_DVR_GetDVRConfig", lUserID, 3305, lChannel,ctypes.byref(m_struFocusModeCfg),76, ctypes.byref(dwReturned))==False):
        error_info = callCpp("NET_DVR_GetLastError")
        print("��ѧ�䱶��ȡʧ�ܣ�" + str(error_info))
    else:
        print("��ѧ�䱶��ȡ�ɹ�")
        print("��ǰ��ѧ�䱶ֵ��"+str(m_struFocusModeCfg.fOpticalZoomLevel))
        m_struFocusModeCfg.fOpticalZoomLevel=zoomScale
        if (callCpp("NET_DVR_SetDVRConfig", lUserID, 3306, lChannel, ctypes.byref(m_struFocusModeCfg),76)==False):
            error_info = callCpp("NET_DVR_GetLastError")
            print("��ѧ�䱶�޸�ʧ�ܣ�" + str(error_info))
        else:
            print("��ѧ�䱶�޸ĳɹ�;�޸ĺ������Ϊ��"+str(m_struFocusModeCfg.fOpticalZoomLevel))
 
# ͸���ӿ���������ṹ��
class NET_DVR_XML_CONFIG_INPUT(ctypes.Structure):
    _fields_ = [
        ("dwSize", ctypes.c_uint32),
        ("lpRequestUrl", ctypes.c_void_p),
        ("dwRequestUrlLen", ctypes.c_uint32),
        ("lpInBuffer", ctypes.c_void_p),
        ("dwInBufferSize", ctypes.c_uint32),
        ("dwRecvTimeOut", ctypes.c_uint32),
        ("byForceEncrpt", ctypes.c_byte),
        ("byRes", ctypes.c_byte*31),]
 
# ͸���ӿ���������ṹ��
class NET_DVR_XML_CONFIG_OUTPUT(ctypes.Structure):
    _fields_ = [
        ("dwSize", ctypes.c_uint32),
        ("lpOutBuffer", ctypes.c_void_p),
        ("dwOutBufferSize", ctypes.c_uint32),
        ("dwReturnedXMLSize", ctypes.c_uint32),
        ("lpStatusBuffer", ctypes.c_void_p),
        ("dwStatusSize", ctypes.c_uint32),
        ("byRes", ctypes.c_byte*31)]
 
 
#����ֲ��۽��;ֲ��ع⹦�ܣ����������������ϽǺ����½ǣ�startX,startY,endX,endY��
# flag=1�ֲ��۽����ܣ�flag!=1�ֲ��ع⹦��
def RegionalCorrection(startX,startY,endX,endY,flag=1):
    # #���崫������
    if(flag==1):
        choise="regionalFocus"
    else:
        choise = "regionalExposure"
    inUrl = "PUT /ISAPI/Image/channels/1/" + choise
    inPutBuffer = "<" + choise + "><StartPoint><positionX>" + str(startX) + "</positionX><positionY>" + str(startY) + "</positionY></StartPoint><EndPoint><positionX>" + str(endX) + "</positionX><positionY>" + str(endY) + "</positionY></EndPoint></" + choise + ">"
 
    szUrl = (ctypes.c_char * 256)()
    struInput = NET_DVR_XML_CONFIG_INPUT()
    struOuput = NET_DVR_XML_CONFIG_OUTPUT()
    struInput.dwSize=ctypes.sizeof(struInput)
    struOuput.dwSize=ctypes.sizeof(struOuput)
    dwBufferLen = 1024 * 1024
    pBuffer = (ctypes.c_char * dwBufferLen)()
#_____________________________________________put________________________________________________________
    csCommand = bytes(inUrl, "ascii")
    ctypes.memmove(szUrl, csCommand, len(csCommand))
    struInput.lpRequestUrl = ctypes.cast(szUrl,ctypes.c_void_p)
    struInput.dwRequestUrlLen = len(szUrl)
 
    m_csInputParam= bytes(inPutBuffer, "ascii")
    dwInBufferLen = 1024 * 1024
    pInBuffer=(ctypes.c_byte * dwInBufferLen)()
    ctypes.memmove(pInBuffer, m_csInputParam, len(m_csInputParam))
    struInput.lpInBuffer = ctypes.cast(pInBuffer,ctypes.c_void_p)
    struInput.dwInBufferSize = len(m_csInputParam)
 
    struOuput.lpStatusBuffer = ctypes.cast(pBuffer,ctypes.c_void_p)
    struOuput.dwStatusSize = dwBufferLen
 
    if (callCpp("NET_DVR_STDXMLConfig", lUserID, ctypes.byref(struInput), ctypes.byref(struOuput))):
        error_info = callCpp("NET_DVR_GetLastError")
        print("�ϴ��ɹ���" + str(error_info))
    else:
        error_info = callCpp("NET_DVR_GetLastError")
        print("�ϴ�ʧ�ܣ������Ϊ"+ str(error_info))
 
 
#���������������ṹ��
class NET_DVR_ACTIVATECFG(ctypes.Structure):
    _fields_ = [
        ("dwSize", ctypes.c_uint32),
        ("sPassword",ctypes.c_byte*16),
        ("byRes",  ctypes.c_byte*108)]
 
 
#�����������
def OnActivateDevice():
    init_res = callCpp("NET_DVR_Init")#SDK��ʼ��
    if init_res:
        print("SDK��ʼ���ɹ�")
        error_info = callCpp("NET_DVR_GetLastError")
    else:
        error_info = callCpp("NET_DVR_GetLastError")
        print("SDK��ʼ������" + str(error_info))
        return False
 
    set_overtime = callCpp("NET_DVR_SetConnectTime",5000,4)#���ó�ʱ
    if set_overtime:
        print("���ó�ʱʱ��ɹ�")
    else:
        error_info = callCpp("NET_DVR_GetLastError")
        print("���ó�ʱ������Ϣ��" + str(error_info))
        return False
 
    szLan=(ctypes.c_char * 256)()
    pwd=bytes('abcd1234', "ascii")#���������������
    DevAddr=bytes('192.168.5.64', "ascii") #�����ʼĬ��IP��ַ
    struActivateCfg=NET_DVR_ACTIVATECFG()
    struActivateCfg.dwSize=ctypes.sizeof(struActivateCfg)
    ctypes.memmove(struActivateCfg.sPassword, pwd, len(pwd))
    if(callCpp("NET_DVR_ActivateDevice",DevAddr,8000,ctypes.byref(struActivateCfg))):
        error_info = callCpp("NET_DVR_GetLastError")
        print("����ɹ���" + str(error_info))
    else:
        error_info = callCpp("NET_DVR_GetLastError")
        print("����ʧ�ܣ�" + str(error_info))
 
# OnActivateDevice()#�����������
# �������print(ctypes.sizeof(NET_DVR_FOCUSMODE_CFG))
lUserID=NET_DVR_Login_V30()
# ���Ԥ��
m_lRealHandle=Preview()
# Get_JPEGpicture()
# Change_CamZoom(1)
 
#����ֲ��۽��;ֲ��ع⹦�ܣ����������������ϽǺ����½ǣ�startX,startY,endX,endY��
# flag=1�ֲ��۽����ܣ�flag!=1�ֲ��ع⹦��
# RegionalCorrection(20,20,50,50,flag=1)