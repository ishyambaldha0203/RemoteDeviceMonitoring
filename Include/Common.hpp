/*************************************************************************************************
 * @file Common.hpp
 *
 * @brief Contains common data structure between devices and monitoring server.
 *
 * These structure is being used as data format to ease the data transfer.
 *
 *************************************************************************************************/
#ifndef _REMOTE_DEVICE_MONITORING_COMMON_HPP
#define _REMOTE_DEVICE_MONITORING_COMMON_HPP

#include <iostream>
#include <cstdint>
#include <cstring>

namespace
{
    namespace Default
    {
        constexpr const uint32_t DeviceNameLength = 64;
        constexpr const uint32_t MaxDeviceSupported = 5;
        constexpr const uint16_t ServerPort = 8100;
        constexpr const char *ServerIp = "127.0.0.1";
    }

    /**
     * @brief A data format used by devices to send data to server.
     */
    struct MessageFrame
    {
        int32_t deviceId;                           ///< A unique device id.
        char deviceName[Default::DeviceNameLength]; ///< The name of the device.
        int32_t data;                               //< A simple integer data.
    };

    /**
     * @brief Used by server to provided the acknowledgement to device.
     */
    struct Response
    {
        int32_t errorCode; ///< To send back the status to device.
    };
} // Anonymous namespace

#endif // !_REMOTE_DEVICE_MONITORING_COMMON_HPP
