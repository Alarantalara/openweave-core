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
 *          Utilities for accessing persisted device configuration on
 *          platforms based on the Nordic nRF5 SDK.
 */

#ifndef NRF5_CONFIG_H
#define NRF5_CONFIG_H

#include <functional>

#include <Weave/DeviceLayer/internal/WeaveDeviceLayerInternal.h>

#include "fds.h"

namespace nl {
namespace Weave {
namespace DeviceLayer {
namespace Internal {

constexpr inline uint32_t NRF5ConfigKey(uint16_t fileId, uint16_t recordId)
{
    return static_cast<uint32_t>(fileId) << 16 | recordId;
}


/**
 * Provides functions and definitions for accessing persisted device configuration
 * on platforms based on the Nordic nRF5 SDK.
 *
 * This implementation uses the Nordic Flash Data Storage (FDS) library as the
 * underlying storage layer.
 *
 * NOTE: This class is designed to be mixed-in to the concrete subclass of the
 * GenericConfigurationManagerImpl<> template.  When used this way, the class
 * naturally provides implementations for the delegated members referenced by
 * the template class (e.g. the ReadConfigValue() method).
 */
class NRF5Config
{
public:

    // Limits/definitions imposed by the Nordic SDK
    static constexpr uint16_t kFDSFileIdMin                     = 0x0000; /**< Minimum value that can be used for a FDS file id (per Nordic SDK) */
    static constexpr uint16_t kFDSFileIdMax                     = 0xBFFF; /**< Maximum value that can be used for a FDS file id (per Nordic SDK) */
    static constexpr uint16_t kFDSRecordKeyMin                  = 0x0001; /**< Minimum value that can be used for a FDS record key (per Nordic SDK) */
    static constexpr uint16_t kFDSRecordKeyMax                  = 0xBFFF; /**< Maximum value that can be used for a FDS record key (per Nordic SDK) */

    // FDS file ids used by the OpenWeave Device Layer
    static constexpr uint16_t kFileId_WeaveFactory              = 0x235A; /**< FDS file containing persistent config values set at manufacturing time.
                                                                           *   Retained during factory reset. */
    static constexpr uint16_t kFileId_WeaveConfig               = 0x235B; /**< FDS file containing dynamic config values set at runtime.
                                                                           *   Cleared during factory reset. */
    static constexpr uint16_t kFileId_WeaveCounter              = 0x235C; /**< FDS file containing dynamic counter values set at runtime.
                                                                           *   Retained during factory reset. */

    // API data type used to represent the combination of a FDS file id and record key.
    using Key = uint32_t;

    // Key definitions for well-known configuration values.
    static constexpr Key kConfigKey_SerialNum                   = NRF5ConfigKey(kFileId_WeaveFactory, 0x0001);
    static constexpr Key kConfigKey_DeviceId                    = NRF5ConfigKey(kFileId_WeaveFactory, 0x0002);
    static constexpr Key kConfigKey_DeviceCert                  = NRF5ConfigKey(kFileId_WeaveFactory, 0x0003);
    static constexpr Key kConfigKey_DevicePrivateKey            = NRF5ConfigKey(kFileId_WeaveFactory, 0x0004);
    static constexpr Key kConfigKey_ManufacturingDate           = NRF5ConfigKey(kFileId_WeaveFactory, 0x0005);
    static constexpr Key kConfigKey_PairingCode                 = NRF5ConfigKey(kFileId_WeaveFactory, 0x0006);
    static constexpr Key kConfigKey_FabricId                    = NRF5ConfigKey(kFileId_WeaveConfig,  0x0007);
    static constexpr Key kConfigKey_ServiceConfig               = NRF5ConfigKey(kFileId_WeaveConfig,  0x0008);
    static constexpr Key kConfigKey_PairedAccountId             = NRF5ConfigKey(kFileId_WeaveConfig,  0x0009);
    static constexpr Key kConfigKey_ServiceId                   = NRF5ConfigKey(kFileId_WeaveConfig,  0x000A);
    static constexpr Key kConfigKey_FabricSecret                = NRF5ConfigKey(kFileId_WeaveConfig,  0x000B);
    static constexpr Key kConfigKey_LastUsedEpochKeyId          = NRF5ConfigKey(kFileId_WeaveConfig,  0x000C);
    static constexpr Key kConfigKey_FailSafeArmed               = NRF5ConfigKey(kFileId_WeaveConfig,  0x000D);
    static constexpr Key kConfigKey_GroupKey                    = NRF5ConfigKey(kFileId_WeaveConfig,  0x000E);

    // Range of FDS record keys used to store Weave persisted counter values.
    static constexpr uint16_t kPersistedCounterRecordKeyBase    = kFDSRecordKeyMin;
                                                                          /**< Base record key for records containing Weave persisted counter values.
                                                                           *   The Weave counter id is added to this value to form the FDS record key.*/
    static constexpr uint16_t kPersistedCounterRecordKeyMax     = kFDSRecordKeyMax;
                                                                          /**< Max record key for records containing Weave persisted counter values. */

    static WEAVE_ERROR Init(void);

    // Configuration methods used by the GenericConfigurationManagerImpl<> template.
    static WEAVE_ERROR ReadConfigValue(Key key, bool & val);
    static WEAVE_ERROR ReadConfigValue(Key key, uint32_t & val);
    static WEAVE_ERROR ReadConfigValue(Key key, uint64_t & val);
    static WEAVE_ERROR ReadConfigValueStr(Key key, char * buf, size_t bufSize, size_t & outLen);
    static WEAVE_ERROR ReadConfigValueBin(Key key, uint8_t * buf, size_t bufSize, size_t & outLen);
    static WEAVE_ERROR WriteConfigValue(Key key, bool val);
    static WEAVE_ERROR WriteConfigValue(Key key, uint32_t val);
    static WEAVE_ERROR WriteConfigValue(Key key, uint64_t val);
    static WEAVE_ERROR WriteConfigValueStr(Key key, const char * str);
    static WEAVE_ERROR WriteConfigValueStr(Key key, const char * str, size_t strLen);
    static WEAVE_ERROR WriteConfigValueBin(Key key, const uint8_t * data, size_t dataLen);
    static WEAVE_ERROR ClearConfigValue(Key key);
    static bool ConfigValueExists(Key key);
    static WEAVE_ERROR FactoryResetConfig(void);

    static void RunConfigUnitTest(void);

protected:

    struct FDSAsyncOp
    {
        enum
        {
            kAddRecord                      = 0,
            kUpdateRecord,
            kAddOrUpdateRecord,
            kDeleteRecord,
            kDeleteRecordByKey,
            kDeleteFile,
            kGC,
            kInit,
            kWaitQueueSpaceAvailable
        };

        fds_record_desc_t RecordDesc;
        const uint8_t * RecordData;
        uint32_t RecordDataLengthWords;
        ret_code_t Result;
        uint16_t FileId;
        uint16_t RecordKey;
        uint8_t OpType;

        inline FDSAsyncOp(uint8_t opType) : OpType(opType) { }
    };

    using ForEachRecordFunct = std::function<WEAVE_ERROR(const fds_flash_record_t & rec, bool & deleteRec)>;

    static constexpr uint16_t kFDSWordSize = 4;

    static FDSAsyncOp * volatile sActiveAsyncOp;
    static SemaphoreHandle_t sAsyncOpCompletionSem;

    static constexpr uint16_t GetFileId(uint32_t key);
    static constexpr uint16_t GetRecordKey(uint32_t key);
    static WEAVE_ERROR OpenRecord(NRF5Config::Key key, fds_record_desc_t & recDesc, fds_flash_record_t & rec);
    static WEAVE_ERROR ForEachRecord(uint16_t fileId, uint16_t recordKey, ForEachRecordFunct funct);
    static WEAVE_ERROR DoAsyncFDSOp(FDSAsyncOp & asyncOp);
    static constexpr uint16_t FDSWords(size_t s);

private:
    static void HandleFDSEvent(const fds_evt_t * fdsEvent);
    static WEAVE_ERROR MapFDSError(ret_code_t fdsRes);
};

/**
 * Extract an FDS file id from a Key value.
 */
inline constexpr uint16_t NRF5Config::GetFileId(Key key)
{
    return static_cast<uint16_t>(key >> 16);
}

/**
 * Extract an FDS record key from a Key value.
 */
inline constexpr uint16_t NRF5Config::GetRecordKey(Key key)
{
    return static_cast<uint16_t>(key);
}

/**
 * Number of FDS words needed to hold a given size object.
 */
inline constexpr uint16_t NRF5Config::FDSWords(size_t s)
{
    return (s + (kFDSWordSize - 1)) / kFDSWordSize;
}

} // namespace Internal
} // namespace DeviceLayer
} // namespace Weave
} // namespace nl

#endif // NRF5_CONFIG_H
