/*
 *
 *    Copyright (c) 2018 Nest Labs, Inc.
 *    All rights reserved.
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

/**
 *    @file
 *          Provides implementations for the OpenWeave and LwIP logging functions
 *          on Nordic nRF52 platforms.
 */


#define NRF_LOG_MODULE_NAME weave
#include "nrf_log.h"
NRF_LOG_MODULE_REGISTER();

#include <Weave/DeviceLayer/internal/WeaveDeviceLayerInternal.h>
#include <Weave/Support/logging/WeaveLogging.h>

#if WEAVE_DEVICE_CONFIG_ENABLE_THREAD
#include <openthread/platform/logging.h>
#endif // WEAVE_DEVICE_CONFIG_ENABLE_THREAD

using namespace ::nl::Weave;
using namespace ::nl::Weave::DeviceLayer;
using namespace ::nl::Weave::DeviceLayer::Internal;

namespace {

void GetModuleName(char *buf, uint8_t module)
{
    if (module == ::nl::Weave::Logging::kLogModule_DeviceLayer)
    {
        memcpy(buf, "DL", 3);
    }
    else
    {
        ::nl::Weave::Logging::GetModuleName(buf, module);
    }
}

} // unnamed namespace

namespace nl {
namespace Weave {
namespace DeviceLayer {

/**
 * Called whenever a log message is emitted by Weave or LwIP.
 *
 * This function is intended be overridden by the application to, e.g.,
 * schedule output of queued log entries.
 */
void __attribute__((weak)) OnLogOutput(void)
{
}

} // namespace DeviceLayer
} // namespace Weave
} // namespace nl


namespace nl {
namespace Weave {
namespace Logging {

/**
 * OpenWeave log output function.
 */
void Log(uint8_t module, uint8_t category, const char *msg, ...)
{
    va_list v;

    (void)module;
    (void)category;

    va_start(v, msg);

#if NRF_LOG_ENABLED

    if (IsCategoryEnabled(category))
    {
        {
            char formattedMsg[256];
            size_t formattedMsgLen;

            constexpr size_t maxPrefixLen = nlWeaveLoggingModuleNameLen + 3;
            static_assert(sizeof(formattedMsg) > maxPrefixLen);

            // Form the log prefix, e.g. "[DL] "
            formattedMsg[0] = '[';
            ::GetModuleName(formattedMsg + 1, module);
            formattedMsgLen = strlen(formattedMsg);
            formattedMsg[formattedMsgLen++] = ']';
            formattedMsg[formattedMsgLen++] = ' ';

            // Append the log message.
            vsnprintf(formattedMsg + formattedMsgLen, sizeof(formattedMsg) - formattedMsgLen, msg, v);

            // Invoke the NRF logging library to log the message.
            switch (category) {
            case kLogCategory_Error:
                NRF_LOG_ERROR("%s", NRF_LOG_PUSH(formattedMsg));
                break;
            case kLogCategory_Progress:
            case kLogCategory_Retain:
            default:
                NRF_LOG_INFO("%s", NRF_LOG_PUSH(formattedMsg));
                break;
            case kLogCategory_Detail:
                NRF_LOG_DEBUG("%s", NRF_LOG_PUSH(formattedMsg));
                break;
            }
        }

        // Let the application know that a log message has been emitted.
        DeviceLayer::OnLogOutput();
    }

#endif // NRF_LOG_ENABLED

    va_end(v);
}

} // namespace Logging
} // namespace Weave
} // namespace nl


#undef NRF_LOG_MODULE_NAME
#define NRF_LOG_MODULE_NAME lwip
#include "nrf_log.h"
NRF_LOG_MODULE_REGISTER();

/**
 * LwIP log output function.
 */
extern "C"
void LwIPLog(const char *msg, ...)
{
    va_list v;

    va_start(v, msg);

#if NRF_LOG_ENABLED
    {
        char formattedMsg[256];

        // Append the log message.
        size_t len = vsnprintf(formattedMsg, sizeof(formattedMsg), msg, v);

        while (len > 0 && isspace(formattedMsg[len-1]))
        {
            len--;
            formattedMsg[len] = 0;
        }

        // Invoke the NRF logging library to log the message.
        NRF_LOG_DEBUG("%s", NRF_LOG_PUSH(formattedMsg));
    }

    // Let the application know that a log message has been emitted.
    DeviceLayer::OnLogOutput();

#endif // NRF_LOG_ENABLED

    va_end(v);
}

#if WEAVE_DEVICE_CONFIG_ENABLE_THREAD

#undef NRF_LOG_MODULE_NAME
#define NRF_LOG_MODULE_NAME thread
#include "nrf_log.h"
NRF_LOG_MODULE_REGISTER();

extern "C"
void otPlatLog(otLogLevel aLogLevel, otLogRegion aLogRegion, const char *aFormat, ...)
{
    va_list v;

    (void)aLogLevel;
    (void)aLogRegion;

    va_start(v, aFormat);

#if NRF_LOG_ENABLED
    {
        char formattedMsg[256];

        // Append the log message.
        vsnprintf(formattedMsg, sizeof(formattedMsg), aFormat, v);

        // Invoke the NRF logging library to log the message.
        switch (aLogLevel) {
        case OT_LOG_LEVEL_CRIT:
            NRF_LOG_ERROR("%s", NRF_LOG_PUSH(formattedMsg));
            break;
        case OT_LOG_LEVEL_WARN:
            NRF_LOG_WARNING("%s", NRF_LOG_PUSH(formattedMsg));
            break;
        case OT_LOG_LEVEL_NOTE:
        case OT_LOG_LEVEL_INFO:
        default:
            NRF_LOG_INFO("%s", NRF_LOG_PUSH(formattedMsg));
            break;
        case OT_LOG_LEVEL_DEBG:
            NRF_LOG_DEBUG("%s", NRF_LOG_PUSH(formattedMsg));
            break;
        }
    }

    // Let the application know that a log message has been emitted.
    DeviceLayer::OnLogOutput();

#endif // NRF_LOG_ENABLED

    va_end(v);
}

#endif // WEAVE_DEVICE_CONFIG_ENABLE_THREAD
