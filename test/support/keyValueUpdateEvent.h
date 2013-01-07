#include <string>

#include "disruptor/interface.h"

#include "long_event.h"

#ifndef KEYVALUE_UPDATE_EVENT_H_ // NOLINT
#define KEYVALUE_UPDATE_EVENT_H_ // NOLINT

/*
 * @brief Key Value update object
 */
class KeyValueUpdate
{
public:
    KeyValueUpdate() : key_("id"), value_(-1) {}

    const int64_t getValue() const { return value_; }
    const std::string& getKey() const { return key_; }

    void setValue(const int64_t value)
    {
        value_ = value;
    }

    void setKey(const std::string& key)
    {
        key_ = key;
    }

private:
    int64_t value_;
    std::string key_;
};

/*
 * @brief ring buffer entry which encapsulates updates to be
 * sent to ART
 */

class KeyValueUpdateEvent : public disruptor::test::LongEvent
{
 public:
    KeyValueUpdateEvent(const int64_t& value = 0) : LongEvent(value) {}

    const std::string& getRawByteStream() const { return rawByteStream_; }
    KeyValueUpdate& getKeyValueUpdate() { return keyValueUpdate_; }

    void setRawByteStream(const std::string& byteStream)
    {
        rawByteStream_ = byteStream;
    }

    void setKeyValueUpdate(const KeyValueUpdate& update)
    {
        keyValueUpdate_ = update;
    }

 private:
    std::string rawByteStream_;
    KeyValueUpdate keyValueUpdate_;
};

/*
 * @brief Factory to pre-allocate keyValue update events
 */

class KeyValueUpdateEventFactory : public disruptor::EventFactoryInterface<KeyValueUpdateEvent>
{
 public:
    virtual KeyValueUpdateEvent* NewInstance(const int& size) const {
        return new KeyValueUpdateEvent[size];
    }
};

#endif // KEYVALUE_UPDATE_EVENT_H_
