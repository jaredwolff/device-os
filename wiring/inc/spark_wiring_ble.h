/*
 * Copyright (c) 2018 Particle Industries, Inc.  All rights reserved.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation, either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, see <http://www.gnu.org/licenses/>.
 */

#ifndef SPARK_WIRING_BLE_H
#define SPARK_WIRING_BLE_H

#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "spark_wiring_platform.h"

#if Wiring_BLE

#include "system_error.h"
#include "spark_wiring_string.h"
#include "spark_wiring_vector.h"
#include "spark_wiring_flags.h"
#include "ble_hal.h"
#include "check.h"
#include <memory>

using spark::Vector;
using particle::Flags;

class BleScanResult;
class BlePeerDevice;

// Forward declaration
class BleCharacteristicImpl;
class BleServiceImpl;
class BleGattServerImpl;
class BleGattClientImpl;
class BleBroadcasterImpl;
class BleObserverImpl;
class BlePeripheralImpl;
class BleCentralImpl;

enum class BleUuidType {
    SHORT = 0,
    LONG = 1
};

enum class BleUuidOrder {
    MSB = 0,
    LSB = 1
};

struct BleDeviceRoleType; // Tag type for BLE device role flags
typedef Flags<BleDeviceRoleType, uint8_t> BleDeviceRoles;
typedef BleDeviceRoles::FlagType BleDeviceRole;
namespace ROLE {
    const BleDeviceRole INVALID(BLE_ROLE_INVALID);
    const BleDeviceRole PERIPHERAL(BLE_ROLE_PERIPHERAL);
    const BleDeviceRole CENTRAL(BLE_ROLE_CENTRAL);
    const BleDeviceRole ALL(0xff);
}

struct BleCharacteristicPropertyType; // Tag type for BLE attribute property flags
typedef Flags<BleCharacteristicPropertyType, uint8_t> BleCharacteristicProperties;
typedef BleCharacteristicProperties::FlagType BleCharacteristicProperty;
namespace PROPERTY {
    const BleCharacteristicProperty NONE(0);
    const BleCharacteristicProperty READ(BLE_SIG_CHAR_PROP_READ);
    const BleCharacteristicProperty WRITE(BLE_SIG_CHAR_PROP_WRITE);
    const BleCharacteristicProperty WRITE_WO_RSP(BLE_SIG_CHAR_PROP_WRITE_WO_RESP);
    const BleCharacteristicProperty NOTIFY(BLE_SIG_CHAR_PROP_NOTIFY);
    const BleCharacteristicProperty INDICATE(BLE_SIG_CHAR_PROP_INDICATE);
}

namespace PARTICLE_BLE {
    const uint8_t BLE_USER_DEFAULT_SVC_UUID[BLE_SIG_UUID_128BIT_LEN] = {
        0x2d, 0x49, 0xc0, 0x26, 0xf0, 0xdb, 0xce, 0x06, 0x7a, 0x33, 0x6f, 0x12, 0x00, 0x00, 0xf3, 0x5d
    };
}

typedef uint16_t BleConnectionHandle;
typedef uint16_t BleAttributeHandle;

typedef void (*OnDataReceivedCallback)(const uint8_t* data, size_t len, const BlePeerDevice& peer, void* context);
typedef void (*OnScanResultCallback)(const BleScanResult* device);
typedef void (*OnConnectedCallback)(const BlePeerDevice& peer);
typedef void (*OnDisconnectedCallback)(const BlePeerDevice& peer);

class BleAddress : public hal_ble_addr_t {
public:
    void operator = (hal_ble_addr_t addr) {
        this->addr_type = addr.addr_type;
        memcpy(this->addr, addr.addr, BLE_SIG_ADDR_LEN);
    }

    uint8_t operator [] (uint8_t i) const {
        if (i >= BLE_SIG_ADDR_LEN) {
            return 0;
        }
        return addr[i];
    }

    bool operator == (const BleAddress& addr) const {
        if (this->addr_type == addr.addr_type && !memcmp(this->addr, addr.addr, BLE_SIG_ADDR_LEN)) {
            return true;
        }
        return false;
    }
};

class BleAdvertisingParams : public hal_ble_adv_params_t {
};

class BleConnectionParams : public hal_ble_conn_params_t {
};

class BleScanParams : public hal_ble_scan_params_t {
};

class BleCharacteristicHandles : public hal_ble_char_handles_t {
public:
    const BleCharacteristicHandles& operator = (const hal_ble_char_handles_t& halHandles) {
        this->decl_handle = halHandles.decl_handle;
        this->value_handle = halHandles.value_handle;
        this->user_desc_handle = halHandles.user_desc_handle;
        this->cccd_handle = halHandles.cccd_handle;
        this->sccd_handle = halHandles.sccd_handle;
        return *this;
    }
};


class BleUuid {
public:
    BleUuid();
    BleUuid(const BleUuid& uuid);
    BleUuid(const uint8_t* uuid128, BleUuidOrder order=BleUuidOrder::LSB);
    BleUuid(uint16_t uuid16, BleUuidOrder order=BleUuidOrder::LSB);
    BleUuid(const uint8_t* uuid128, uint16_t uuid16, BleUuidOrder order=BleUuidOrder::LSB);
    BleUuid(const String& uuid);
    BleUuid(const char* uuid);
    ~BleUuid();

    bool isValid(void) const;

    BleUuidType type(void) const {
        return type_;
    }

    BleUuidOrder order(void) const {
        return order_;
    }

    uint16_t shorted(void) const {
        return shortUUID_;
    }

    void full(uint8_t uuid128[BLE_SIG_UUID_128BIT_LEN]) const {
        memcpy(uuid128, fullUUID_, BLE_SIG_UUID_128BIT_LEN);
    }

    const uint8_t* full(void) const {
        return fullUUID_;
    }

    bool operator == (const BleUuid& uuid) const;

private:
    BleUuidType type_;
    BleUuidOrder order_;
    uint16_t shortUUID_;
    uint8_t fullUUID_[BLE_SIG_UUID_128BIT_LEN];

    int8_t toInt(char c);
    void setUuid(const String& str);
};


class iBeacon {
public:
    uint16_t major;
    uint16_t minor;
    BleUuid uuid;
    int8_t measurePower;

    static const uint16_t APPLE_COMPANY_ID = 0x004C;
    static const uint8_t BEACON_TYPE_IBEACON = 0x02;

    iBeacon() : major(0), minor(0), measurePower(0) {
    }

    template<typename T>
    iBeacon(uint16_t major, uint16_t minor, T uuid, int8_t measurePower)
        : major(major), minor(minor), uuid(uuid), measurePower(measurePower) {
    }

    ~iBeacon() {
    }
};


class BleAdvertisingData {
public:
    static const size_t MAX_LEN = BLE_MAX_ADV_DATA_LEN;

    BleAdvertisingData();
    ~BleAdvertisingData();

    size_t set(const uint8_t* buf, size_t len);

    size_t append(uint8_t type, const uint8_t* buf, size_t len, bool force=false);
    size_t appendLocalName(const char* name, bool force = false);
    size_t appendCustomData(const uint8_t* buf, size_t len, bool force=false);

    template<typename T>
    size_t appendServiceUUID(T uuid, bool force=false) {
        BleUuid tempUUID(uuid);
        if (tempUUID.type() == BleUuidType::SHORT) {
            uint16_t uuid16 = tempUUID.shorted();
            return append(BLE_SIG_AD_TYPE_16BIT_SERVICE_UUID_COMPLETE, reinterpret_cast<const uint8_t*>(&uuid16), sizeof(uint16_t), force);
        }
        else {
            return append(BLE_SIG_AD_TYPE_128BIT_SERVICE_UUID_COMPLETE, tempUUID.full(), BLE_SIG_UUID_128BIT_LEN, force);
        }
    }

    void clear(void);
    void remove(uint8_t type);

    size_t get(uint8_t* buf, size_t len) const;
    size_t get(uint8_t type, uint8_t* buf, size_t len) const;

    const uint8_t* data(void) const;
    size_t length(void) const;

    String deviceName(void) const;
    size_t deviceName(uint8_t* buf, size_t len) const;
    size_t serviceUUID(BleUuid* uuids, size_t count) const;
    size_t customData(uint8_t* buf, size_t len) const;

    size_t operator () (uint8_t* buf, size_t len) const {
        return get(buf, len);
    }

    bool contains (uint8_t type) const;

private:
    uint8_t selfData_[BLE_MAX_ADV_DATA_LEN];
    size_t selfLen_;

    size_t serviceUUID(uint8_t type, BleUuid* uuids, size_t count) const;
    static size_t locate(const uint8_t* buf, size_t len, uint8_t type, size_t* offset);
};


class BleCharacteristic {
public:
    BleCharacteristic();
    BleCharacteristic(const BleCharacteristic& characteristic);
    BleCharacteristic(const char* desc, BleCharacteristicProperties properties, OnDataReceivedCallback callback=nullptr);

    template<typename T>
    BleCharacteristic(const char* desc, BleCharacteristicProperties properties, T charUuid, T svcUuid, OnDataReceivedCallback callback=nullptr) {
        BleUuid cUuid(charUuid);
        BleUuid sUuid(svcUuid);
        construct(desc, properties, cUuid, sUuid, callback);
    }

    ~BleCharacteristic();

    BleCharacteristic& operator = (const BleCharacteristic& characteristic);

    bool valid(void) {
        return impl() != nullptr;
    }

    BleUuid UUID(void) const;
    BleCharacteristicProperties properties(void) const;

    size_t getValue(uint8_t* buf, size_t len) const;
    size_t getValue(String& str) const;

    template<typename T>
    size_t getValue(T* val) const {
        size_t len = sizeof(T);
        return getValue(reinterpret_cast<uint8_t*>(val), len);
    }

    size_t setValue(const uint8_t* buf, size_t len);
    size_t setValue(const String& str);
    size_t setValue(const char* str);

    template<typename T>
    size_t setValue(T val) {
        uint8_t buf[BLE_MAX_ATTR_VALUE_PACKET_SIZE];
        size_t len = sizeof(T) > BLE_MAX_ATTR_VALUE_PACKET_SIZE ? BLE_MAX_ATTR_VALUE_PACKET_SIZE : sizeof(T);
        for (size_t i = 0, j = len - 1; i < len; i++, j--) {
            buf[i] = reinterpret_cast<const uint8_t*>(&val)[j];
        }
        return setValue(buf, len);
    }

    void onDataReceived(OnDataReceivedCallback callback);

    BleCharacteristicImpl* impl(void) const {
        return impl_.get();
    }

private:
    std::shared_ptr<BleCharacteristicImpl> impl_;
    void construct(const char* desc, BleCharacteristicProperties properties, BleUuid& charUuid, BleUuid& svcUuid, OnDataReceivedCallback callback);
};


class BleService {
public:
    BleService();
    BleService(const BleUuid& uuid);
    ~BleService();

    BleServiceImpl* impl(void) const {
        return impl_.get();
    }

private:
    std::shared_ptr<BleServiceImpl> impl_;
};


class BleScanResult {
public:
    BleAddress address;
    BleAdvertisingData advertisingData;
    BleAdvertisingData scanResponse;
    int8_t rssi;
};


class BlePeerDevice {
public:
    BleDeviceRoles role;
    BleAddress address;
    BleConnectionParams connParams;
    BleConnectionHandle connHandle;
    int8_t rssi;

    BlePeerDevice();
    BlePeerDevice(const BleAddress& addr);
    ~BlePeerDevice();

    BleCharacteristic characteristic(const char* desc) const;
    BleCharacteristic characteristic(const BleUuid& uuid) const;

    bool connected(void) const;

    BleGattServerImpl* gattsProxy(void) {
        return gattsProxy_.get();
    }

    bool operator == (const BlePeerDevice& device);

private:
    std::shared_ptr<BleGattServerImpl> gattsProxy_;
};


class BleLocalDevice {
public:
    BleAddress address;

    int on(void);
    void off(void);

    int setTxPower(int8_t val) const;
    int8_t txPower(void) const;

    int advertise(void);
    int advertise(BleAdvertisingData* advertisingData, BleAdvertisingData* scanResponse=nullptr);
    int advertise(uint16_t interval);
    int advertise(uint16_t interval, BleAdvertisingData* advertisingData, BleAdvertisingData* scanResponse=nullptr);
    int advertise(uint16_t interval, uint16_t timeout);
    int advertise(uint16_t interval, uint16_t timeout, BleAdvertisingData* advertisingData, BleAdvertisingData* scanResponse=nullptr);
    int advertise(const BleAdvertisingParams& params);
    int advertise(const BleAdvertisingParams& params, BleAdvertisingData* advertisingData, BleAdvertisingData* scanResponse=nullptr);

    int advertise(const iBeacon& iBeacon, bool connectable=false);
    int advertise(uint16_t interval, const iBeacon& iBeacon, bool connectable=false);
    int advertise(uint16_t interval, uint16_t timeout, const iBeacon& iBeacon, bool connectable=false);
    int advertise(const BleAdvertisingParams& params, const iBeacon& iBeacon, bool connectable=false);

    int stopAdvertising(void) const;

    int scan(OnScanResultCallback callback);
    int scan(OnScanResultCallback callback, uint16_t timeout);
    int scan(BleScanResult* results, size_t resultCount);
    int scan(BleScanResult* results, size_t resultCount, uint16_t timeout);
    int scan(BleScanResult* results, size_t resultCount, const BleScanParams& params);
    int stopScanning(void);

    int addCharacteristic(BleCharacteristic& characteristic);
    int addCharacteristic(const char* desc, BleCharacteristicProperties properties, OnDataReceivedCallback callback=nullptr);

    template<typename T>
    int addCharacteristic(const char* desc, BleCharacteristicProperties properties, T charUuid, T svcUuid, OnDataReceivedCallback callback=nullptr) {
        BleCharacteristic characteristic(desc, properties, charUuid, svcUuid, callback);
        return addCharacteristic(characteristic);
    }

    int setPPCP(uint16_t minInterval, uint16_t maxInterval, uint16_t latency=BLE_DEFAULT_SLAVE_LATENCY, uint16_t timeout=BLE_DEFAULT_CONN_SUP_TIMEOUT);

    BlePeerDevice connect(const BleAddress& addr, uint16_t interval, uint16_t latency, uint16_t timeout);
    BlePeerDevice connect(const BleAddress& addr);

    int disconnect(void);
    int disconnect(const BlePeerDevice& peripheral);

    bool connected(void) const;

    void onConnected(OnConnectedCallback callback);
    void onDisconnected(OnDisconnectedCallback callback);

    static BleLocalDevice& getInstance(void);

private:
    OnConnectedCallback connectedCb_;
    OnDisconnectedCallback disconnectedCb_;
    std::unique_ptr<BleGattServerImpl> gattsProxy_;
    std::unique_ptr<BleGattClientImpl> gattcProxy_;
    std::unique_ptr<BleBroadcasterImpl> broadcasterProxy_;
    std::unique_ptr<BleObserverImpl> observerProxy_;
    std::unique_ptr<BlePeripheralImpl> peripheralProxy_;
    std::unique_ptr<BleCentralImpl> centralProxy_;

    BleLocalDevice();
    ~BleLocalDevice();
    static void onBleEvents(const hal_ble_evts_t *event, void* context);
    BlePeerDevice* findPeerDevice(BleConnectionHandle connHandle);
};


extern BleLocalDevice& _fetch_ble();
#define BLE _fetch_ble()


#endif /* Wiring_BLE */

#endif /* SPARK_WIRING_BLE_H */
