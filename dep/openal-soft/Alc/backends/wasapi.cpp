/**
 * OpenAL cross platform audio library
 * Copyright (C) 2011 by authors.
 * This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 *  License along with this library; if not, write to the
 *  Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 * Or go to http://www.gnu.org/copyleft/lgpl.html
 */

#include "config.h"

#include "backends/wasapi.h"

#include <stdlib.h>
#include <stdio.h>
#include <memory.h>

#include <wtypes.h>
#include <mmdeviceapi.h>
#include <audioclient.h>
#include <cguid.h>
#include <devpropdef.h>
#include <mmreg.h>
#include <propsys.h>
#include <propkey.h>
#include <devpkey.h>
#ifndef _WAVEFORMATEXTENSIBLE_
#include <ks.h>
#include <ksmedia.h>
#endif

#include <deque>
#include <mutex>
#include <atomic>
#include <thread>
#include <vector>
#include <string>
#include <future>
#include <algorithm>
#include <functional>
#include <condition_variable>

#include "alMain.h"
#include "alu.h"
#include "ringbuffer.h"
#include "compat.h"
#include "converter.h"


/* Some headers seem to define these as macros for __uuidof, which is annoying
 * since some headers don't declare them at all. Hopefully the ifdef is enough
 * to tell if they need to be declared.
 */
#ifndef KSDATAFORMAT_SUBTYPE_PCM
DEFINE_GUID(KSDATAFORMAT_SUBTYPE_PCM, 0x00000001, 0x0000, 0x0010, 0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71);
#endif
#ifndef KSDATAFORMAT_SUBTYPE_IEEE_FLOAT
DEFINE_GUID(KSDATAFORMAT_SUBTYPE_IEEE_FLOAT, 0x00000003, 0x0000, 0x0010, 0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71);
#endif

DEFINE_DEVPROPKEY(DEVPKEY_Device_FriendlyName, 0xa45c254e, 0xdf1c, 0x4efd, 0x80,0x20, 0x67,0xd1,0x46,0xa8,0x50,0xe0, 14);
DEFINE_PROPERTYKEY(PKEY_AudioEndpoint_FormFactor, 0x1da5d803, 0xd492, 0x4edd, 0x8c,0x23, 0xe0,0xc0,0xff,0xee,0x7f,0x0e, 0);
DEFINE_PROPERTYKEY(PKEY_AudioEndpoint_GUID, 0x1da5d803, 0xd492, 0x4edd, 0x8c, 0x23,0xe0, 0xc0,0xff,0xee,0x7f,0x0e, 4 );


namespace {

#define MONO SPEAKER_FRONT_CENTER
#define STEREO (SPEAKER_FRONT_LEFT|SPEAKER_FRONT_RIGHT)
#define QUAD (SPEAKER_FRONT_LEFT|SPEAKER_FRONT_RIGHT|SPEAKER_BACK_LEFT|SPEAKER_BACK_RIGHT)
#define X5DOT1 (SPEAKER_FRONT_LEFT|SPEAKER_FRONT_RIGHT|SPEAKER_FRONT_CENTER|SPEAKER_LOW_FREQUENCY|SPEAKER_SIDE_LEFT|SPEAKER_SIDE_RIGHT)
#define X5DOT1REAR (SPEAKER_FRONT_LEFT|SPEAKER_FRONT_RIGHT|SPEAKER_FRONT_CENTER|SPEAKER_LOW_FREQUENCY|SPEAKER_BACK_LEFT|SPEAKER_BACK_RIGHT)
#define X6DOT1 (SPEAKER_FRONT_LEFT|SPEAKER_FRONT_RIGHT|SPEAKER_FRONT_CENTER|SPEAKER_LOW_FREQUENCY|SPEAKER_BACK_CENTER|SPEAKER_SIDE_LEFT|SPEAKER_SIDE_RIGHT)
#define X7DOT1 (SPEAKER_FRONT_LEFT|SPEAKER_FRONT_RIGHT|SPEAKER_FRONT_CENTER|SPEAKER_LOW_FREQUENCY|SPEAKER_BACK_LEFT|SPEAKER_BACK_RIGHT|SPEAKER_SIDE_LEFT|SPEAKER_SIDE_RIGHT)
#define X7DOT1_WIDE (SPEAKER_FRONT_LEFT|SPEAKER_FRONT_RIGHT|SPEAKER_FRONT_CENTER|SPEAKER_LOW_FREQUENCY|SPEAKER_BACK_LEFT|SPEAKER_BACK_RIGHT|SPEAKER_FRONT_LEFT_OF_CENTER|SPEAKER_FRONT_RIGHT_OF_CENTER)

#define REFTIME_PER_SEC ((REFERENCE_TIME)10000000)

#define DEVNAME_HEAD "OpenAL Soft on "


/* Scales the given value using 64-bit integer math, ceiling the result. */
inline int64_t ScaleCeil(int64_t val, int64_t new_scale, int64_t old_scale)
{
    return (val*new_scale + old_scale-1) / old_scale;
}


struct PropVariant {
    PROPVARIANT mProp;

public:
    PropVariant() { PropVariantInit(&mProp); }
    ~PropVariant() { clear(); }

    void clear() { PropVariantClear(&mProp); }

    PROPVARIANT* get() noexcept { return &mProp; }

    PROPVARIANT& operator*() noexcept { return mProp; }
    const PROPVARIANT& operator*() const noexcept { return mProp; }

    PROPVARIANT* operator->() noexcept { return &mProp; }
    const PROPVARIANT* operator->() const noexcept { return &mProp; }
};

struct DevMap {
    std::string name;
    std::string endpoint_guid; // obtained from PKEY_AudioEndpoint_GUID , set to "Unknown device GUID" if absent.
    std::wstring devid;

    template<typename T0, typename T1, typename T2>
    DevMap(T0&& name_, T1&& guid_, T2&& devid_)
      : name{std::forward<T0>(name_)}
      , endpoint_guid{std::forward<T1>(guid_)}
      , devid{std::forward<T2>(devid_)}
    { }
};

bool checkName(const al::vector<DevMap> &list, const std::string &name)
{
    return std::find_if(list.cbegin(), list.cend(),
        [&name](const DevMap &entry) -> bool
        { return entry.name == name; }
    ) != list.cend();
}

al::vector<DevMap> PlaybackDevices;
al::vector<DevMap> CaptureDevices;


using NameGUIDPair = std::pair<std::string,std::string>;
NameGUIDPair get_device_name_and_guid(IMMDevice *device)
{
    std::string name{DEVNAME_HEAD};
    std::string guid;

    IPropertyStore *ps;
    HRESULT hr = device->OpenPropertyStore(STGM_READ, &ps);
    if(FAILED(hr))
    {
        WARN("OpenPropertyStore failed: 0x%08lx\n", hr);
        return { name+"Unknown Device Name", "Unknown Device GUID" };
    }

    PropVariant pvprop;
    hr = ps->GetValue(reinterpret_cast<const PROPERTYKEY&>(DEVPKEY_Device_FriendlyName), pvprop.get());
    if(FAILED(hr))
    {
        WARN("GetValue Device_FriendlyName failed: 0x%08lx\n", hr);
        name += "Unknown Device Name";
    }
    else if(pvprop->vt == VT_LPWSTR)
        name += wstr_to_utf8(pvprop->pwszVal);
    else
    {
        WARN("Unexpected PROPVARIANT type: 0x%04x\n", pvprop->vt);
        name += "Unknown Device Name";
    }

    pvprop.clear();
    hr = ps->GetValue(reinterpret_cast<const PROPERTYKEY&>(PKEY_AudioEndpoint_GUID), pvprop.get());
    if(FAILED(hr))
    {
        WARN("GetValue AudioEndpoint_GUID failed: 0x%08lx\n", hr);
        guid = "Unknown Device GUID";
    }
    else if(pvprop->vt == VT_LPWSTR)
        guid = wstr_to_utf8(pvprop->pwszVal);
    else
    {
        WARN("Unexpected PROPVARIANT type: 0x%04x\n", pvprop->vt);
        guid = "Unknown Device GUID";
    }

    ps->Release();

    return {name, guid};
}

void get_device_formfactor(IMMDevice *device, EndpointFormFactor *formfactor)
{
    IPropertyStore *ps;
    HRESULT hr = device->OpenPropertyStore(STGM_READ, &ps);
    if(FAILED(hr))
    {
        WARN("OpenPropertyStore failed: 0x%08lx\n", hr);
        return;
    }

    PropVariant pvform;
    hr = ps->GetValue(reinterpret_cast<const PROPERTYKEY&>(PKEY_AudioEndpoint_FormFactor), pvform.get());
    if(FAILED(hr))
        WARN("GetValue AudioEndpoint_FormFactor failed: 0x%08lx\n", hr);
    else if(pvform->vt == VT_UI4)
        *formfactor = static_cast<EndpointFormFactor>(pvform->ulVal);
    else if(pvform->vt == VT_EMPTY)
        *formfactor = UnknownFormFactor;
    else
        WARN("Unexpected PROPVARIANT type: 0x%04x\n", pvform->vt);

    ps->Release();
}


void add_device(IMMDevice *device, const WCHAR *devid, al::vector<DevMap> &list)
{
    std::string basename, guidstr;
    std::tie(basename, guidstr) = get_device_name_and_guid(device);

    int count{1};
    std::string newname{basename};
    while(checkName(list, newname))
    {
        newname = basename;
        newname += " #";
        newname += std::to_string(++count);
    }
    list.emplace_back(std::move(newname), std::move(guidstr), devid);
    const DevMap &newentry = list.back();

    TRACE("Got device \"%s\", \"%s\", \"%ls\"\n", newentry.name.c_str(),
          newentry.endpoint_guid.c_str(), newentry.devid.c_str());
}

WCHAR *get_device_id(IMMDevice *device)
{
    WCHAR *devid;

    HRESULT hr = device->GetId(&devid);
    if(FAILED(hr))
    {
        ERR("Failed to get device id: %lx\n", hr);
        return nullptr;
    }

    return devid;
}

HRESULT probe_devices(IMMDeviceEnumerator *devenum, EDataFlow flowdir, al::vector<DevMap> &list)
{
    IMMDeviceCollection *coll;
    HRESULT hr{devenum->EnumAudioEndpoints(flowdir, DEVICE_STATE_ACTIVE, &coll)};
    if(FAILED(hr))
    {
        ERR("Failed to enumerate audio endpoints: 0x%08lx\n", hr);
        return hr;
    }

    IMMDevice *defdev{nullptr};
    WCHAR *defdevid{nullptr};
    UINT count{0};
    hr = coll->GetCount(&count);
    if(SUCCEEDED(hr) && count > 0)
    {
        list.clear();
        list.reserve(count);

        hr = devenum->GetDefaultAudioEndpoint(flowdir, eMultimedia, &defdev);
    }
    if(SUCCEEDED(hr) && defdev != nullptr)
    {
        defdevid = get_device_id(defdev);
        if(defdevid)
            add_device(defdev, defdevid, list);
    }

    for(UINT i{0};i < count;++i)
    {
        IMMDevice *device;
        hr = coll->Item(i, &device);
        if(FAILED(hr)) continue;

        WCHAR *devid{get_device_id(device)};
        if(devid)
        {
            if(!defdevid || wcscmp(devid, defdevid) != 0)
                add_device(device, devid, list);
            CoTaskMemFree(devid);
        }
        device->Release();
    }

    if(defdev) defdev->Release();
    if(defdevid) CoTaskMemFree(defdevid);
    coll->Release();

    return S_OK;
}


enum class MsgType : unsigned int {
    OpenDevice,
    ResetDevice,
    StartDevice,
    StopDevice,
    CloseDevice,
    EnumeratePlayback,
    EnumerateCapture,
    QuitThread,

    Count
};

constexpr char MessageStr[static_cast<unsigned int>(MsgType::Count)][20]{
    "Open Device",
    "Reset Device",
    "Start Device",
    "Stop Device",
    "Close Device",
    "Enumerate Playback",
    "Enumerate Capture",
    "Quit"
};


/* Proxy interface used by the message handler. */
struct WasapiProxy {
    virtual HRESULT openProxy() = 0;
    virtual void closeProxy() = 0;

    virtual HRESULT resetProxy() = 0;
    virtual HRESULT startProxy() = 0;
    virtual void  stopProxy() = 0;

    struct Msg {
        MsgType mType;
        WasapiProxy *mProxy;
        std::promise<HRESULT> mPromise;
    };
    static std::deque<Msg> mMsgQueue;
    static std::mutex mMsgQueueLock;
    static std::condition_variable mMsgQueueCond;

    std::future<HRESULT> pushMessage(MsgType type)
    {
        std::promise<HRESULT> promise;
        std::future<HRESULT> future{promise.get_future()};
        { std::lock_guard<std::mutex> _{mMsgQueueLock};
            mMsgQueue.emplace_back(Msg{type, this, std::move(promise)});
        }
        mMsgQueueCond.notify_one();
        return future;
    }

    static std::future<HRESULT> pushMessageStatic(MsgType type)
    {
        std::promise<HRESULT> promise;
        std::future<HRESULT> future{promise.get_future()};
        { std::lock_guard<std::mutex> _{mMsgQueueLock};
            mMsgQueue.emplace_back(Msg{type, nullptr, std::move(promise)});
        }
        mMsgQueueCond.notify_one();
        return future;
    }

    static bool popMessage(Msg &msg)
    {
        std::unique_lock<std::mutex> lock{mMsgQueueLock};
        while(mMsgQueue.empty())
            mMsgQueueCond.wait(lock);
        msg = std::move(mMsgQueue.front());
        mMsgQueue.pop_front();
        return msg.mType != MsgType::QuitThread;
    }

    static int messageHandler(std::promise<HRESULT> *promise);

    static constexpr inline const char *CurrentPrefix() noexcept { return "WasapiProxy::"; }
};
std::deque<WasapiProxy::Msg> WasapiProxy::mMsgQueue;
std::mutex WasapiProxy::mMsgQueueLock;
std::condition_variable WasapiProxy::mMsgQueueCond;

int WasapiProxy::messageHandler(std::promise<HRESULT> *promise)
{
    TRACE("Starting message thread\n");

    HRESULT cohr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
    if(FAILED(cohr))
    {
        WARN("Failed to initialize COM: 0x%08lx\n", cohr);
        promise->set_value(cohr);
        return 0;
    }

    void *ptr{};
    HRESULT hr{CoCreateInstance(CLSID_MMDeviceEnumerator, nullptr, CLSCTX_INPROC_SERVER,
        IID_IMMDeviceEnumerator, &ptr)};
    if(FAILED(hr))
    {
        WARN("Failed to create IMMDeviceEnumerator instance: 0x%08lx\n", hr);
        promise->set_value(hr);
        CoUninitialize();
        return 0;
    }
    auto Enumerator = static_cast<IMMDeviceEnumerator*>(ptr);
    Enumerator->Release();
    Enumerator = nullptr;
    CoUninitialize();

    TRACE("Message thread initialization complete\n");
    promise->set_value(S_OK);
    promise = nullptr;

    TRACE("Starting message loop\n");
    ALuint deviceCount{0};
    Msg msg;
    while(popMessage(msg))
    {
        TRACE("Got message \"%s\" (0x%04x, this=%p)\n",
            MessageStr[static_cast<unsigned int>(msg.mType)], static_cast<unsigned int>(msg.mType),
            msg.mProxy);

        switch(msg.mType)
        {
        case MsgType::OpenDevice:
            hr = cohr = S_OK;
            if(++deviceCount == 1)
                hr = cohr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
            if(SUCCEEDED(hr))
                hr = msg.mProxy->openProxy();
            msg.mPromise.set_value(hr);

            if(FAILED(hr))
            {
                if(--deviceCount == 0 && SUCCEEDED(cohr))
                    CoUninitialize();
            }
            continue;

        case MsgType::ResetDevice:
            hr = msg.mProxy->resetProxy();
            msg.mPromise.set_value(hr);
            continue;

        case MsgType::StartDevice:
            hr = msg.mProxy->startProxy();
            msg.mPromise.set_value(hr);
            continue;

        case MsgType::StopDevice:
            msg.mProxy->stopProxy();
            msg.mPromise.set_value(S_OK);
            continue;

        case MsgType::CloseDevice:
            msg.mProxy->closeProxy();
            msg.mPromise.set_value(S_OK);

            if(--deviceCount == 0)
                CoUninitialize();
            continue;

        case MsgType::EnumeratePlayback:
        case MsgType::EnumerateCapture:
            hr = cohr = S_OK;
            if(++deviceCount == 1)
                hr = cohr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
            if(SUCCEEDED(hr))
                hr = CoCreateInstance(CLSID_MMDeviceEnumerator, nullptr, CLSCTX_INPROC_SERVER, IID_IMMDeviceEnumerator, &ptr);
            if(FAILED(hr))
                msg.mPromise.set_value(hr);
            else
            {
                Enumerator = static_cast<IMMDeviceEnumerator*>(ptr);

                if(msg.mType == MsgType::EnumeratePlayback)
                    hr = probe_devices(Enumerator, eRender, PlaybackDevices);
                else if(msg.mType == MsgType::EnumerateCapture)
                    hr = probe_devices(Enumerator, eCapture, CaptureDevices);
                msg.mPromise.set_value(hr);

                Enumerator->Release();
                Enumerator = nullptr;
            }

            if(--deviceCount == 0 && SUCCEEDED(cohr))
                CoUninitialize();
            continue;

        default:
            ERR("Unexpected message: %u\n", static_cast<unsigned int>(msg.mType));
            msg.mPromise.set_value(E_FAIL);
            continue;
        }
    }
    TRACE("Message loop finished\n");

    return 0;
}


struct WasapiPlayback final : public BackendBase, WasapiProxy {
    WasapiPlayback(ALCdevice *device) noexcept : BackendBase{device} { }
    ~WasapiPlayback() override;

    int mixerProc();

    ALCenum open(const ALCchar *name) override;
    HRESULT openProxy() override;
    void closeProxy() override;

    ALCboolean reset() override;
    HRESULT resetProxy() override;
    ALCboolean start() override;
    HRESULT startProxy() override;
    void stop() override;
    void stopProxy() override;

    ClockLatency getClockLatency() override;

    std::wstring mDevId;

    IMMDevice *mMMDev{nullptr};
    IAudioClient *mClient{nullptr};
    IAudioRenderClient *mRender{nullptr};
    HANDLE mNotifyEvent{nullptr};

    std::atomic<UINT32> mPadding{0u};

    std::atomic<bool> mKillNow{true};
    std::thread mThread;

    static constexpr inline const char *CurrentPrefix() noexcept { return "WasapiPlayback::"; }
    DEF_NEWDEL(WasapiPlayback)
};

WasapiPlayback::~WasapiPlayback()
{
    pushMessage(MsgType::CloseDevice).wait();

    if(mNotifyEvent != nullptr)
        CloseHandle(mNotifyEvent);
    mNotifyEvent = nullptr;
}


FORCE_ALIGN int WasapiPlayback::mixerProc()
{
    HRESULT hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
    if(FAILED(hr))
    {
        ERR("CoInitializeEx(nullptr, COINIT_MULTITHREADED) failed: 0x%08lx\n", hr);
        aluHandleDisconnect(mDevice, "COM init failed: 0x%08lx", hr);
        return 1;
    }

    SetRTPriority();
    althrd_setname(MIXER_THREAD_NAME);

    const ALuint update_size{mDevice->UpdateSize};
    const UINT32 buffer_len{mDevice->BufferSize};
    while(!mKillNow.load(std::memory_order_relaxed))
    {
        UINT32 written;
        hr = mClient->GetCurrentPadding(&written);
        if(FAILED(hr))
        {
            ERR("Failed to get padding: 0x%08lx\n", hr);
            aluHandleDisconnect(mDevice, "Failed to retrieve buffer padding: 0x%08lx", hr);
            break;
        }
        mPadding.store(written, std::memory_order_relaxed);

        ALuint len{buffer_len - written};
        if(len < update_size)
        {
            DWORD res{WaitForSingleObjectEx(mNotifyEvent, 2000, FALSE)};
            if(res != WAIT_OBJECT_0)
                ERR("WaitForSingleObjectEx error: 0x%lx\n", res);
            continue;
        }
        len -= len%update_size;

        BYTE *buffer;
        hr = mRender->GetBuffer(len, &buffer);
        if(SUCCEEDED(hr))
        {
            lock();
            aluMixData(mDevice, buffer, len);
            mPadding.store(written + len, std::memory_order_relaxed);
            unlock();
            hr = mRender->ReleaseBuffer(len, 0);
        }
        if(FAILED(hr))
        {
            ERR("Failed to buffer data: 0x%08lx\n", hr);
            aluHandleDisconnect(mDevice, "Failed to send playback samples: 0x%08lx", hr);
            break;
        }
    }
    mPadding.store(0u, std::memory_order_release);

    CoUninitialize();
    return 0;
}


ALCboolean MakeExtensible(WAVEFORMATEXTENSIBLE *out, const WAVEFORMATEX *in)
{
    *out = WAVEFORMATEXTENSIBLE{};
    if(in->wFormatTag == WAVE_FORMAT_EXTENSIBLE)
        *out = *(const WAVEFORMATEXTENSIBLE*)in;
    else if(in->wFormatTag == WAVE_FORMAT_PCM)
    {
        out->Format = *in;
        out->Format.wFormatTag = WAVE_FORMAT_EXTENSIBLE;
        out->Format.cbSize = sizeof(*out) - sizeof(*in);
        if(out->Format.nChannels == 1)
            out->dwChannelMask = MONO;
        else if(out->Format.nChannels == 2)
            out->dwChannelMask = STEREO;
        else
            ERR("Unhandled PCM channel count: %d\n", out->Format.nChannels);
        out->SubFormat = KSDATAFORMAT_SUBTYPE_PCM;
    }
    else if(in->wFormatTag == WAVE_FORMAT_IEEE_FLOAT)
    {
        out->Format = *in;
        out->Format.wFormatTag = WAVE_FORMAT_EXTENSIBLE;
        out->Format.cbSize = sizeof(*out) - sizeof(*in);
        if(out->Format.nChannels == 1)
            out->dwChannelMask = MONO;
        else if(out->Format.nChannels == 2)
            out->dwChannelMask = STEREO;
        else
            ERR("Unhandled IEEE float channel count: %d\n", out->Format.nChannels);
        out->SubFormat = KSDATAFORMAT_SUBTYPE_IEEE_FLOAT;
    }
    else
    {
        ERR("Unhandled format tag: 0x%04x\n", in->wFormatTag);
        return ALC_FALSE;
    }
    return ALC_TRUE;
}

ALCenum WasapiPlayback::open(const ALCchar *name)
{
    HRESULT hr{S_OK};

    mNotifyEvent = CreateEventW(nullptr, FALSE, FALSE, nullptr);
    if(mNotifyEvent == nullptr)
    {
        ERR("Failed to create notify events: %lu\n", GetLastError());
        hr = E_FAIL;
    }

    if(SUCCEEDED(hr))
    {
        if(name)
        {
            if(PlaybackDevices.empty())
                pushMessage(MsgType::EnumeratePlayback).wait();

            hr = E_FAIL;
            auto iter = std::find_if(PlaybackDevices.cbegin(), PlaybackDevices.cend(),
                [name](const DevMap &entry) -> bool
                { return entry.name == name || entry.endpoint_guid == name; }
            );
            if(iter == PlaybackDevices.cend())
            {
                std::wstring wname{utf8_to_wstr(name)};
                iter = std::find_if(PlaybackDevices.cbegin(), PlaybackDevices.cend(),
                    [&wname](const DevMap &entry) -> bool
                    { return entry.devid == wname; }
                );
            }
            if(iter == PlaybackDevices.cend())
                WARN("Failed to find device name matching \"%s\"\n", name);
            else
            {
                mDevId = iter->devid;
                mDevice->DeviceName = iter->name;
                hr = S_OK;
            }
        }
    }

    if(SUCCEEDED(hr))
        hr = pushMessage(MsgType::OpenDevice).get();

    if(FAILED(hr))
    {
        if(mNotifyEvent != nullptr)
            CloseHandle(mNotifyEvent);
        mNotifyEvent = nullptr;

        mDevId.clear();

        ERR("Device init failed: 0x%08lx\n", hr);
        return ALC_INVALID_VALUE;
    }

    return ALC_NO_ERROR;
}

HRESULT WasapiPlayback::openProxy()
{
    void *ptr;
    HRESULT hr{CoCreateInstance(CLSID_MMDeviceEnumerator, nullptr, CLSCTX_INPROC_SERVER, IID_IMMDeviceEnumerator, &ptr)};
    if(SUCCEEDED(hr))
    {
        auto Enumerator = static_cast<IMMDeviceEnumerator*>(ptr);
        if(mDevId.empty())
            hr = Enumerator->GetDefaultAudioEndpoint(eRender, eMultimedia, &mMMDev);
        else
            hr = Enumerator->GetDevice(mDevId.c_str(), &mMMDev);
        Enumerator->Release();
    }
    if(SUCCEEDED(hr))
        hr = mMMDev->Activate(IID_IAudioClient, CLSCTX_INPROC_SERVER, nullptr, &ptr);
    if(SUCCEEDED(hr))
    {
        mClient = static_cast<IAudioClient*>(ptr);
        if(mDevice->DeviceName.empty())
            mDevice->DeviceName = get_device_name_and_guid(mMMDev).first;
    }

    if(FAILED(hr))
    {
        if(mMMDev)
            mMMDev->Release();
        mMMDev = nullptr;
    }

    return hr;
}

void WasapiPlayback::closeProxy()
{
    if(mClient)
        mClient->Release();
    mClient = nullptr;

    if(mMMDev)
        mMMDev->Release();
    mMMDev = nullptr;
}


ALCboolean WasapiPlayback::reset()
{
    HRESULT hr{pushMessage(MsgType::ResetDevice).get()};
    return SUCCEEDED(hr) ? ALC_TRUE : ALC_FALSE;
}

HRESULT WasapiPlayback::resetProxy()
{
    if(mClient)
        mClient->Release();
    mClient = nullptr;

    void *ptr;
    HRESULT hr = mMMDev->Activate(IID_IAudioClient, CLSCTX_INPROC_SERVER, nullptr, &ptr);
    if(FAILED(hr))
    {
        ERR("Failed to reactivate audio client: 0x%08lx\n", hr);
        return hr;
    }
    mClient = static_cast<IAudioClient*>(ptr);

    WAVEFORMATEX *wfx;
    hr = mClient->GetMixFormat(&wfx);
    if(FAILED(hr))
    {
        ERR("Failed to get mix format: 0x%08lx\n", hr);
        return hr;
    }

    WAVEFORMATEXTENSIBLE OutputType;
    if(!MakeExtensible(&OutputType, wfx))
    {
        CoTaskMemFree(wfx);
        return E_FAIL;
    }
    CoTaskMemFree(wfx);
    wfx = nullptr;

    const REFERENCE_TIME per_time{mDevice->UpdateSize * REFTIME_PER_SEC / mDevice->Frequency};
    const REFERENCE_TIME buf_time{mDevice->BufferSize * REFTIME_PER_SEC / mDevice->Frequency};

    if(!(mDevice->Flags&DEVICE_FREQUENCY_REQUEST))
        mDevice->Frequency = OutputType.Format.nSamplesPerSec;
    if(!(mDevice->Flags&DEVICE_CHANNELS_REQUEST))
    {
        if(OutputType.Format.nChannels == 1 && OutputType.dwChannelMask == MONO)
            mDevice->FmtChans = DevFmtMono;
        else if(OutputType.Format.nChannels == 2 && OutputType.dwChannelMask == STEREO)
            mDevice->FmtChans = DevFmtStereo;
        else if(OutputType.Format.nChannels == 4 && OutputType.dwChannelMask == QUAD)
            mDevice->FmtChans = DevFmtQuad;
        else if(OutputType.Format.nChannels == 6 && OutputType.dwChannelMask == X5DOT1)
            mDevice->FmtChans = DevFmtX51;
        else if(OutputType.Format.nChannels == 6 && OutputType.dwChannelMask == X5DOT1REAR)
            mDevice->FmtChans = DevFmtX51Rear;
        else if(OutputType.Format.nChannels == 7 && OutputType.dwChannelMask == X6DOT1)
            mDevice->FmtChans = DevFmtX61;
        else if(OutputType.Format.nChannels == 8 && (OutputType.dwChannelMask == X7DOT1 || OutputType.dwChannelMask == X7DOT1_WIDE))
            mDevice->FmtChans = DevFmtX71;
        else
            ERR("Unhandled channel config: %d -- 0x%08lx\n", OutputType.Format.nChannels, OutputType.dwChannelMask);
    }

    switch(mDevice->FmtChans)
    {
        case DevFmtMono:
            OutputType.Format.nChannels = 1;
            OutputType.dwChannelMask = MONO;
            break;
        case DevFmtAmbi3D:
            mDevice->FmtChans = DevFmtStereo;
            /*fall-through*/
        case DevFmtStereo:
            OutputType.Format.nChannels = 2;
            OutputType.dwChannelMask = STEREO;
            break;
        case DevFmtQuad:
            OutputType.Format.nChannels = 4;
            OutputType.dwChannelMask = QUAD;
            break;
        case DevFmtX51:
            OutputType.Format.nChannels = 6;
            OutputType.dwChannelMask = X5DOT1;
            break;
        case DevFmtX51Rear:
            OutputType.Format.nChannels = 6;
            OutputType.dwChannelMask = X5DOT1REAR;
            break;
        case DevFmtX61:
            OutputType.Format.nChannels = 7;
            OutputType.dwChannelMask = X6DOT1;
            break;
        case DevFmtX71:
            OutputType.Format.nChannels = 8;
            OutputType.dwChannelMask = X7DOT1;
            break;
    }
    switch(mDevice->FmtType)
    {
        case DevFmtByte:
            mDevice->FmtType = DevFmtUByte;
            /* fall-through */
        case DevFmtUByte:
            OutputType.Format.wBitsPerSample = 8;
            OutputType.Samples.wValidBitsPerSample = 8;
            OutputType.SubFormat = KSDATAFORMAT_SUBTYPE_PCM;
            break;
        case DevFmtUShort:
            mDevice->FmtType = DevFmtShort;
            /* fall-through */
        case DevFmtShort:
            OutputType.Format.wBitsPerSample = 16;
            OutputType.Samples.wValidBitsPerSample = 16;
            OutputType.SubFormat = KSDATAFORMAT_SUBTYPE_PCM;
            break;
        case DevFmtUInt:
            mDevice->FmtType = DevFmtInt;
            /* fall-through */
        case DevFmtInt:
            OutputType.Format.wBitsPerSample = 32;
            OutputType.Samples.wValidBitsPerSample = 32;
            OutputType.SubFormat = KSDATAFORMAT_SUBTYPE_PCM;
            break;
        case DevFmtFloat:
            OutputType.Format.wBitsPerSample = 32;
            OutputType.Samples.wValidBitsPerSample = 32;
            OutputType.SubFormat = KSDATAFORMAT_SUBTYPE_IEEE_FLOAT;
            break;
    }
    OutputType.Format.nSamplesPerSec = mDevice->Frequency;

    OutputType.Format.nBlockAlign = OutputType.Format.nChannels *
                                    OutputType.Format.wBitsPerSample / 8;
    OutputType.Format.nAvgBytesPerSec = OutputType.Format.nSamplesPerSec *
                                        OutputType.Format.nBlockAlign;

    hr = mClient->IsFormatSupported(AUDCLNT_SHAREMODE_SHARED, &OutputType.Format, &wfx);
    if(FAILED(hr))
    {
        ERR("Failed to check format support: 0x%08lx\n", hr);
        hr = mClient->GetMixFormat(&wfx);
    }
    if(FAILED(hr))
    {
        ERR("Failed to find a supported format: 0x%08lx\n", hr);
        return hr;
    }

    if(wfx != nullptr)
    {
        if(!MakeExtensible(&OutputType, wfx))
        {
            CoTaskMemFree(wfx);
            return E_FAIL;
        }
        CoTaskMemFree(wfx);
        wfx = nullptr;

        mDevice->Frequency = OutputType.Format.nSamplesPerSec;
        if(OutputType.Format.nChannels == 1 && OutputType.dwChannelMask == MONO)
            mDevice->FmtChans = DevFmtMono;
        else if(OutputType.Format.nChannels == 2 && OutputType.dwChannelMask == STEREO)
            mDevice->FmtChans = DevFmtStereo;
        else if(OutputType.Format.nChannels == 4 && OutputType.dwChannelMask == QUAD)
            mDevice->FmtChans = DevFmtQuad;
        else if(OutputType.Format.nChannels == 6 && OutputType.dwChannelMask == X5DOT1)
            mDevice->FmtChans = DevFmtX51;
        else if(OutputType.Format.nChannels == 6 && OutputType.dwChannelMask == X5DOT1REAR)
            mDevice->FmtChans = DevFmtX51Rear;
        else if(OutputType.Format.nChannels == 7 && OutputType.dwChannelMask == X6DOT1)
            mDevice->FmtChans = DevFmtX61;
        else if(OutputType.Format.nChannels == 8 && (OutputType.dwChannelMask == X7DOT1 || OutputType.dwChannelMask == X7DOT1_WIDE))
            mDevice->FmtChans = DevFmtX71;
        else
        {
            ERR("Unhandled extensible channels: %d -- 0x%08lx\n", OutputType.Format.nChannels, OutputType.dwChannelMask);
            mDevice->FmtChans = DevFmtStereo;
            OutputType.Format.nChannels = 2;
            OutputType.dwChannelMask = STEREO;
        }

        if(IsEqualGUID(OutputType.SubFormat, KSDATAFORMAT_SUBTYPE_PCM))
        {
            if(OutputType.Format.wBitsPerSample == 8)
                mDevice->FmtType = DevFmtUByte;
            else if(OutputType.Format.wBitsPerSample == 16)
                mDevice->FmtType = DevFmtShort;
            else if(OutputType.Format.wBitsPerSample == 32)
                mDevice->FmtType = DevFmtInt;
            else
            {
                mDevice->FmtType = DevFmtShort;
                OutputType.Format.wBitsPerSample = 16;
            }
        }
        else if(IsEqualGUID(OutputType.SubFormat, KSDATAFORMAT_SUBTYPE_IEEE_FLOAT))
        {
            mDevice->FmtType = DevFmtFloat;
            OutputType.Format.wBitsPerSample = 32;
        }
        else
        {
            ERR("Unhandled format sub-type\n");
            mDevice->FmtType = DevFmtShort;
            OutputType.Format.wBitsPerSample = 16;
            OutputType.SubFormat = KSDATAFORMAT_SUBTYPE_PCM;
        }
        OutputType.Samples.wValidBitsPerSample = OutputType.Format.wBitsPerSample;
    }

    EndpointFormFactor formfactor = UnknownFormFactor;
    get_device_formfactor(mMMDev, &formfactor);
    mDevice->IsHeadphones = (mDevice->FmtChans == DevFmtStereo &&
        (formfactor == Headphones || formfactor == Headset));

    SetDefaultWFXChannelOrder(mDevice);

    hr = mClient->Initialize(AUDCLNT_SHAREMODE_SHARED, AUDCLNT_STREAMFLAGS_EVENTCALLBACK, buf_time,
        0, &OutputType.Format, nullptr);
    if(FAILED(hr))
    {
        ERR("Failed to initialize audio client: 0x%08lx\n", hr);
        return hr;
    }

    UINT32 buffer_len, min_len;
    REFERENCE_TIME min_per;
    hr = mClient->GetDevicePeriod(&min_per, nullptr);
    if(SUCCEEDED(hr))
        hr = mClient->GetBufferSize(&buffer_len);
    if(FAILED(hr))
    {
        ERR("Failed to get audio buffer info: 0x%08lx\n", hr);
        return hr;
    }

    /* Find the nearest multiple of the period size to the update size */
    if(min_per < per_time)
        min_per *= maxu((per_time + min_per/2) / min_per, 1u);
    min_len = (UINT32)ScaleCeil(min_per, mDevice->Frequency, REFTIME_PER_SEC);
    min_len = minu(min_len, buffer_len/2);

    mDevice->UpdateSize = min_len;
    mDevice->BufferSize = buffer_len;

    hr = mClient->SetEventHandle(mNotifyEvent);
    if(FAILED(hr))
    {
        ERR("Failed to set event handle: 0x%08lx\n", hr);
        return hr;
    }

    return hr;
}


ALCboolean WasapiPlayback::start()
{
    HRESULT hr{pushMessage(MsgType::StartDevice).get()};
    return SUCCEEDED(hr) ? ALC_TRUE : ALC_FALSE;
}

HRESULT WasapiPlayback::startProxy()
{
    ResetEvent(mNotifyEvent);

    HRESULT hr = mClient->Start();
    if(FAILED(hr))
    {
        ERR("Failed to start audio client: 0x%08lx\n", hr);
        return hr;
    }

    void *ptr;
    hr = mClient->GetService(IID_IAudioRenderClient, &ptr);
    if(SUCCEEDED(hr))
    {
        mRender = static_cast<IAudioRenderClient*>(ptr);
        try {
            mKillNow.store(false, std::memory_order_release);
            mThread = std::thread{std::mem_fn(&WasapiPlayback::mixerProc), this};
        }
        catch(...) {
            mRender->Release();
            mRender = nullptr;
            ERR("Failed to start thread\n");
            hr = E_FAIL;
        }
    }

    if(FAILED(hr))
        mClient->Stop();

    return hr;
}


void WasapiPlayback::stop()
{ pushMessage(MsgType::StopDevice).wait(); }

void WasapiPlayback::stopProxy()
{
    if(!mRender || !mThread.joinable())
        return;

    mKillNow.store(true, std::memory_order_release);
    mThread.join();

    mRender->Release();
    mRender = nullptr;
    mClient->Stop();
}


ClockLatency WasapiPlayback::getClockLatency()
{
    ClockLatency ret;

    lock();
    ret.ClockTime = GetDeviceClockTime(mDevice);
    ret.Latency  = std::chrono::seconds{mPadding.load(std::memory_order_relaxed)};
    ret.Latency /= mDevice->Frequency;
    unlock();

    return ret;
}


struct WasapiCapture final : public BackendBase, WasapiProxy {
    WasapiCapture(ALCdevice *device) noexcept : BackendBase{device} { }
    ~WasapiCapture() override;

    int recordProc();

    ALCenum open(const ALCchar *name) override;
    HRESULT openProxy() override;
    void closeProxy() override;

    HRESULT resetProxy() override;
    ALCboolean start() override;
    HRESULT startProxy() override;
    void stop() override;
    void stopProxy() override;

    ALCenum captureSamples(void *buffer, ALCuint samples) override;
    ALCuint availableSamples() override;

    std::wstring mDevId;

    IMMDevice *mMMDev{nullptr};
    IAudioClient *mClient{nullptr};
    IAudioCaptureClient *mCapture{nullptr};
    HANDLE mNotifyEvent{nullptr};

    ChannelConverterPtr mChannelConv;
    SampleConverterPtr mSampleConv;
    RingBufferPtr mRing;

    std::atomic<bool> mKillNow{true};
    std::thread mThread;

    static constexpr inline const char *CurrentPrefix() noexcept { return "WasapiCapture::"; }
    DEF_NEWDEL(WasapiCapture)
};

WasapiCapture::~WasapiCapture()
{
    pushMessage(MsgType::CloseDevice).wait();

    if(mNotifyEvent != nullptr)
        CloseHandle(mNotifyEvent);
    mNotifyEvent = nullptr;
}


FORCE_ALIGN int WasapiCapture::recordProc()
{
    HRESULT hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
    if(FAILED(hr))
    {
        ERR("CoInitializeEx(nullptr, COINIT_MULTITHREADED) failed: 0x%08lx\n", hr);
        aluHandleDisconnect(mDevice, "COM init failed: 0x%08lx", hr);
        return 1;
    }

    althrd_setname(RECORD_THREAD_NAME);

    al::vector<float> samples;
    while(!mKillNow.load(std::memory_order_relaxed))
    {
        UINT32 avail;
        hr = mCapture->GetNextPacketSize(&avail);
        if(FAILED(hr))
            ERR("Failed to get next packet size: 0x%08lx\n", hr);
        else if(avail > 0)
        {
            UINT32 numsamples;
            DWORD flags;
            BYTE *rdata;

            hr = mCapture->GetBuffer(&rdata, &numsamples, &flags, nullptr, nullptr);
            if(FAILED(hr))
                ERR("Failed to get capture buffer: 0x%08lx\n", hr);
            else
            {
                if(mChannelConv)
                {
                    samples.resize(numsamples*2);
                    mChannelConv->convert(rdata, samples.data(), numsamples);
                    rdata = reinterpret_cast<BYTE*>(samples.data());
                }

                auto data = mRing->getWriteVector();

                size_t dstframes;
                if(mSampleConv)
                {
                    const ALvoid *srcdata{rdata};
                    auto srcframes = static_cast<ALsizei>(numsamples);

                    dstframes = mSampleConv->convert(&srcdata, &srcframes, data.first.buf,
                        static_cast<ALsizei>(minz(data.first.len, INT_MAX)));
                    if(srcframes > 0 && dstframes == data.first.len && data.second.len > 0)
                    {
                        /* If some source samples remain, all of the first dest
                         * block was filled, and there's space in the second
                         * dest block, do another run for the second block.
                         */
                        dstframes += mSampleConv->convert(&srcdata, &srcframes, data.second.buf,
                            static_cast<ALsizei>(minz(data.second.len, INT_MAX)));
                    }
                }
                else
                {
                    const auto framesize = static_cast<ALuint>(mDevice->frameSizeFromFmt());
                    size_t len1 = minz(data.first.len, numsamples);
                    size_t len2 = minz(data.second.len, numsamples-len1);

                    memcpy(data.first.buf, rdata, len1*framesize);
                    if(len2 > 0)
                        memcpy(data.second.buf, rdata+len1*framesize, len2*framesize);
                    dstframes = len1 + len2;
                }

                mRing->writeAdvance(dstframes);

                hr = mCapture->ReleaseBuffer(numsamples);
                if(FAILED(hr)) ERR("Failed to release capture buffer: 0x%08lx\n", hr);
            }
        }

        if(FAILED(hr))
        {
            aluHandleDisconnect(mDevice, "Failed to capture samples: 0x%08lx", hr);
            break;
        }

        DWORD res{WaitForSingleObjectEx(mNotifyEvent, 2000, FALSE)};
        if(res != WAIT_OBJECT_0)
            ERR("WaitForSingleObjectEx error: 0x%lx\n", res);
    }

    CoUninitialize();
    return 0;
}


ALCenum WasapiCapture::open(const ALCchar *name)
{
    HRESULT hr{S_OK};

    mNotifyEvent = CreateEventW(nullptr, FALSE, FALSE, nullptr);
    if(mNotifyEvent == nullptr)
    {
        ERR("Failed to create notify event: %lu\n", GetLastError());
        hr = E_FAIL;
    }

    if(SUCCEEDED(hr))
    {
        if(name)
        {
            if(CaptureDevices.empty())
                pushMessage(MsgType::EnumerateCapture).wait();

            hr = E_FAIL;
            auto iter = std::find_if(CaptureDevices.cbegin(), CaptureDevices.cend(),
                [name](const DevMap &entry) -> bool
                { return entry.name == name || entry.endpoint_guid == name; }
            );
            if(iter == CaptureDevices.cend())
            {
                std::wstring wname{utf8_to_wstr(name)};
                iter = std::find_if(CaptureDevices.cbegin(), CaptureDevices.cend(),
                    [&wname](const DevMap &entry) -> bool
                    { return entry.devid == wname; }
                );
            }
            if(iter == CaptureDevices.cend())
                WARN("Failed to find device name matching \"%s\"\n", name);
            else
            {
                mDevId = iter->devid;
                mDevice->DeviceName = iter->name;
                hr = S_OK;
            }
        }
    }

    if(SUCCEEDED(hr))
        hr = pushMessage(MsgType::OpenDevice).get();

    if(FAILED(hr))
    {
        if(mNotifyEvent != nullptr)
            CloseHandle(mNotifyEvent);
        mNotifyEvent = nullptr;

        mDevId.clear();

        ERR("Device init failed: 0x%08lx\n", hr);
        return ALC_INVALID_VALUE;
    }

    hr = pushMessage(MsgType::ResetDevice).get();
    if(FAILED(hr))
    {
        if(hr == E_OUTOFMEMORY)
            return ALC_OUT_OF_MEMORY;
        return ALC_INVALID_VALUE;
    }

    return ALC_NO_ERROR;
}

HRESULT WasapiCapture::openProxy()
{
    void *ptr;
    HRESULT hr{CoCreateInstance(CLSID_MMDeviceEnumerator, nullptr, CLSCTX_INPROC_SERVER,
        IID_IMMDeviceEnumerator, &ptr)};
    if(SUCCEEDED(hr))
    {
        auto Enumerator = static_cast<IMMDeviceEnumerator*>(ptr);
        if(mDevId.empty())
            hr = Enumerator->GetDefaultAudioEndpoint(eCapture, eMultimedia, &mMMDev);
        else
            hr = Enumerator->GetDevice(mDevId.c_str(), &mMMDev);
        Enumerator->Release();
    }
    if(SUCCEEDED(hr))
        hr = mMMDev->Activate(IID_IAudioClient, CLSCTX_INPROC_SERVER, nullptr, &ptr);
    if(SUCCEEDED(hr))
    {
        mClient = static_cast<IAudioClient*>(ptr);
        if(mDevice->DeviceName.empty())
            mDevice->DeviceName = get_device_name_and_guid(mMMDev).first;
    }

    if(FAILED(hr))
    {
        if(mMMDev)
            mMMDev->Release();
        mMMDev = nullptr;
    }

    return hr;
}

void WasapiCapture::closeProxy()
{
    if(mClient)
        mClient->Release();
    mClient = nullptr;

    if(mMMDev)
        mMMDev->Release();
    mMMDev = nullptr;
}

HRESULT WasapiCapture::resetProxy()
{
    if(mClient)
        mClient->Release();
    mClient = nullptr;

    void *ptr;
    HRESULT hr{mMMDev->Activate(IID_IAudioClient, CLSCTX_INPROC_SERVER, nullptr, &ptr)};
    if(FAILED(hr))
    {
        ERR("Failed to reactivate audio client: 0x%08lx\n", hr);
        return hr;
    }
    mClient = static_cast<IAudioClient*>(ptr);

    // Make sure buffer is at least 100ms in size
    REFERENCE_TIME buf_time{mDevice->BufferSize * REFTIME_PER_SEC / mDevice->Frequency};
    buf_time = maxu64(buf_time, REFTIME_PER_SEC/10);

    WAVEFORMATEXTENSIBLE OutputType;
    OutputType.Format.wFormatTag = WAVE_FORMAT_EXTENSIBLE;
    switch(mDevice->FmtChans)
    {
        case DevFmtMono:
            OutputType.Format.nChannels = 1;
            OutputType.dwChannelMask = MONO;
            break;
        case DevFmtStereo:
            OutputType.Format.nChannels = 2;
            OutputType.dwChannelMask = STEREO;
            break;
        case DevFmtQuad:
            OutputType.Format.nChannels = 4;
            OutputType.dwChannelMask = QUAD;
            break;
        case DevFmtX51:
            OutputType.Format.nChannels = 6;
            OutputType.dwChannelMask = X5DOT1;
            break;
        case DevFmtX51Rear:
            OutputType.Format.nChannels = 6;
            OutputType.dwChannelMask = X5DOT1REAR;
            break;
        case DevFmtX61:
            OutputType.Format.nChannels = 7;
            OutputType.dwChannelMask = X6DOT1;
            break;
        case DevFmtX71:
            OutputType.Format.nChannels = 8;
            OutputType.dwChannelMask = X7DOT1;
            break;

        case DevFmtAmbi3D:
            return E_FAIL;
    }
    switch(mDevice->FmtType)
    {
        /* NOTE: Signedness doesn't matter, the converter will handle it. */
        case DevFmtByte:
        case DevFmtUByte:
            OutputType.Format.wBitsPerSample = 8;
            OutputType.SubFormat = KSDATAFORMAT_SUBTYPE_PCM;
            break;
        case DevFmtShort:
        case DevFmtUShort:
            OutputType.Format.wBitsPerSample = 16;
            OutputType.SubFormat = KSDATAFORMAT_SUBTYPE_PCM;
            break;
        case DevFmtInt:
        case DevFmtUInt:
            OutputType.Format.wBitsPerSample = 32;
            OutputType.SubFormat = KSDATAFORMAT_SUBTYPE_PCM;
            break;
        case DevFmtFloat:
            OutputType.Format.wBitsPerSample = 32;
            OutputType.SubFormat = KSDATAFORMAT_SUBTYPE_IEEE_FLOAT;
            break;
    }
    OutputType.Samples.wValidBitsPerSample = OutputType.Format.wBitsPerSample;
    OutputType.Format.nSamplesPerSec = mDevice->Frequency;

    OutputType.Format.nBlockAlign = OutputType.Format.nChannels *
                                    OutputType.Format.wBitsPerSample / 8;
    OutputType.Format.nAvgBytesPerSec = OutputType.Format.nSamplesPerSec *
                                        OutputType.Format.nBlockAlign;
    OutputType.Format.cbSize = sizeof(OutputType) - sizeof(OutputType.Format);

    WAVEFORMATEX *wfx;
    hr = mClient->IsFormatSupported(AUDCLNT_SHAREMODE_SHARED, &OutputType.Format, &wfx);
    if(FAILED(hr))
    {
        ERR("Failed to check format support: 0x%08lx\n", hr);
        return hr;
    }

    mSampleConv = nullptr;
    mChannelConv = nullptr;

    if(wfx != nullptr)
    {
        if(!(wfx->nChannels == OutputType.Format.nChannels ||
             (wfx->nChannels == 1 && OutputType.Format.nChannels == 2) ||
             (wfx->nChannels == 2 && OutputType.Format.nChannels == 1)))
        {
            ERR("Failed to get matching format, wanted: %s %s %uhz, got: %d channel%s %d-bit %luhz\n",
                DevFmtChannelsString(mDevice->FmtChans), DevFmtTypeString(mDevice->FmtType),
                mDevice->Frequency, wfx->nChannels, (wfx->nChannels==1)?"":"s", wfx->wBitsPerSample,
                wfx->nSamplesPerSec);
            CoTaskMemFree(wfx);
            return E_FAIL;
        }

        if(!MakeExtensible(&OutputType, wfx))
        {
            CoTaskMemFree(wfx);
            return E_FAIL;
        }
        CoTaskMemFree(wfx);
        wfx = nullptr;
    }

    DevFmtType srcType;
    if(IsEqualGUID(OutputType.SubFormat, KSDATAFORMAT_SUBTYPE_PCM))
    {
        if(OutputType.Format.wBitsPerSample == 8)
            srcType = DevFmtUByte;
        else if(OutputType.Format.wBitsPerSample == 16)
            srcType = DevFmtShort;
        else if(OutputType.Format.wBitsPerSample == 32)
            srcType = DevFmtInt;
        else
        {
            ERR("Unhandled integer bit depth: %d\n", OutputType.Format.wBitsPerSample);
            return E_FAIL;
        }
    }
    else if(IsEqualGUID(OutputType.SubFormat, KSDATAFORMAT_SUBTYPE_IEEE_FLOAT))
    {
        if(OutputType.Format.wBitsPerSample == 32)
            srcType = DevFmtFloat;
        else
        {
            ERR("Unhandled float bit depth: %d\n", OutputType.Format.wBitsPerSample);
            return E_FAIL;
        }
    }
    else
    {
        ERR("Unhandled format sub-type\n");
        return E_FAIL;
    }

    if(mDevice->FmtChans == DevFmtMono && OutputType.Format.nChannels == 2)
    {
        mChannelConv = CreateChannelConverter(srcType, DevFmtStereo, mDevice->FmtChans);
        if(!mChannelConv)
        {
            ERR("Failed to create %s stereo-to-mono converter\n", DevFmtTypeString(srcType));
            return E_FAIL;
        }
        TRACE("Created %s stereo-to-mono converter\n", DevFmtTypeString(srcType));
        /* The channel converter always outputs float, so change the input type
         * for the resampler/type-converter.
         */
        srcType = DevFmtFloat;
    }
    else if(mDevice->FmtChans == DevFmtStereo && OutputType.Format.nChannels == 1)
    {
        mChannelConv = CreateChannelConverter(srcType, DevFmtMono, mDevice->FmtChans);
        if(!mChannelConv)
        {
            ERR("Failed to create %s mono-to-stereo converter\n", DevFmtTypeString(srcType));
            return E_FAIL;
        }
        TRACE("Created %s mono-to-stereo converter\n", DevFmtTypeString(srcType));
        srcType = DevFmtFloat;
    }

    if(mDevice->Frequency != OutputType.Format.nSamplesPerSec || mDevice->FmtType != srcType)
    {
        mSampleConv = CreateSampleConverter(srcType, mDevice->FmtType, mDevice->channelsFromFmt(),
            OutputType.Format.nSamplesPerSec, mDevice->Frequency, BSinc24Resampler);
        if(!mSampleConv)
        {
            ERR("Failed to create converter for %s format, dst: %s %uhz, src: %s %luhz\n",
                DevFmtChannelsString(mDevice->FmtChans), DevFmtTypeString(mDevice->FmtType),
                mDevice->Frequency, DevFmtTypeString(srcType), OutputType.Format.nSamplesPerSec);
            return E_FAIL;
        }
        TRACE("Created converter for %s format, dst: %s %uhz, src: %s %luhz\n",
              DevFmtChannelsString(mDevice->FmtChans), DevFmtTypeString(mDevice->FmtType),
              mDevice->Frequency, DevFmtTypeString(srcType), OutputType.Format.nSamplesPerSec);
    }

    hr = mClient->Initialize(AUDCLNT_SHAREMODE_SHARED, AUDCLNT_STREAMFLAGS_EVENTCALLBACK, buf_time,
        0, &OutputType.Format, nullptr);
    if(FAILED(hr))
    {
        ERR("Failed to initialize audio client: 0x%08lx\n", hr);
        return hr;
    }

    UINT32 buffer_len;
    REFERENCE_TIME min_per;
    hr = mClient->GetDevicePeriod(&min_per, nullptr);
    if(SUCCEEDED(hr))
        hr = mClient->GetBufferSize(&buffer_len);
    if(FAILED(hr))
    {
        ERR("Failed to get buffer size: 0x%08lx\n", hr);
        return hr;
    }
    mDevice->UpdateSize = static_cast<ALuint>(ScaleCeil(min_per, mDevice->Frequency,
        REFTIME_PER_SEC));
    mDevice->BufferSize = buffer_len;

    buffer_len = maxu(mDevice->BufferSize, buffer_len);
    mRing = CreateRingBuffer(buffer_len, mDevice->frameSizeFromFmt(), false);
    if(!mRing)
    {
        ERR("Failed to allocate capture ring buffer\n");
        return E_OUTOFMEMORY;
    }

    hr = mClient->SetEventHandle(mNotifyEvent);
    if(FAILED(hr))
    {
        ERR("Failed to set event handle: 0x%08lx\n", hr);
        return hr;
    }

    return hr;
}


ALCboolean WasapiCapture::start()
{
    HRESULT hr{pushMessage(MsgType::StartDevice).get()};
    return SUCCEEDED(hr) ? ALC_TRUE : ALC_FALSE;
}

HRESULT WasapiCapture::startProxy()
{
    ResetEvent(mNotifyEvent);

    HRESULT hr{mClient->Start()};
    if(FAILED(hr))
    {
        ERR("Failed to start audio client: 0x%08lx\n", hr);
        return hr;
    }

    void *ptr;
    hr = mClient->GetService(IID_IAudioCaptureClient, &ptr);
    if(SUCCEEDED(hr))
    {
        mCapture = static_cast<IAudioCaptureClient*>(ptr);
        try {
            mKillNow.store(false, std::memory_order_release);
            mThread = std::thread{std::mem_fn(&WasapiCapture::recordProc), this};
        }
        catch(...) {
            mCapture->Release();
            mCapture = nullptr;
            ERR("Failed to start thread\n");
            hr = E_FAIL;
        }
    }

    if(FAILED(hr))
    {
        mClient->Stop();
        mClient->Reset();
    }

    return hr;
}


void WasapiCapture::stop()
{ pushMessage(MsgType::StopDevice).wait(); }

void WasapiCapture::stopProxy()
{
    if(!mCapture || !mThread.joinable())
        return;

    mKillNow.store(true, std::memory_order_release);
    mThread.join();

    mCapture->Release();
    mCapture = nullptr;
    mClient->Stop();
    mClient->Reset();
}


ALCuint WasapiCapture::availableSamples()
{ return (ALCuint)mRing->readSpace(); }

ALCenum WasapiCapture::captureSamples(void *buffer, ALCuint samples)
{
    mRing->read(buffer, samples);
    return ALC_NO_ERROR;
}

} // namespace


bool WasapiBackendFactory::init()
{
    static HRESULT InitResult{E_FAIL};

    if(FAILED(InitResult)) try
    {
        std::promise<HRESULT> promise;
        auto future = promise.get_future();

        std::thread{&WasapiProxy::messageHandler, &promise}.detach();
        InitResult = future.get();
    }
    catch(...) {
    }

    return SUCCEEDED(InitResult) ? ALC_TRUE : ALC_FALSE;
}

bool WasapiBackendFactory::querySupport(BackendType type)
{ return type == BackendType::Playback || type == BackendType::Capture; }

void WasapiBackendFactory::probe(DevProbe type, std::string *outnames)
{
    auto add_device = [outnames](const DevMap &entry) -> void
    {
        /* +1 to also append the null char (to ensure a null-separated list and
         * double-null terminated list).
         */
        outnames->append(entry.name.c_str(), entry.name.length()+1);
    };
    HRESULT hr{};
    switch(type)
    {
    case DevProbe::Playback:
        hr = WasapiProxy::pushMessageStatic(MsgType::EnumeratePlayback).get();
        if(SUCCEEDED(hr))
            std::for_each(PlaybackDevices.cbegin(), PlaybackDevices.cend(), add_device);
        break;

    case DevProbe::Capture:
        hr = WasapiProxy::pushMessageStatic(MsgType::EnumerateCapture).get();
        if(SUCCEEDED(hr))
            std::for_each(CaptureDevices.cbegin(), CaptureDevices.cend(), add_device);
        break;
    }
}

BackendPtr WasapiBackendFactory::createBackend(ALCdevice *device, BackendType type)
{
    if(type == BackendType::Playback)
        return BackendPtr{new WasapiPlayback{device}};
    if(type == BackendType::Capture)
        return BackendPtr{new WasapiCapture{device}};
    return nullptr;
}

BackendFactory &WasapiBackendFactory::getFactory()
{
    static WasapiBackendFactory factory{};
    return factory;
}