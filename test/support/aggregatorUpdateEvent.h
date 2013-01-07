#include <string>

#include "disruptor/interface.h"

#include "long_event.h"

#ifndef AGGREGATOR_UPDATE_EVENT_H_ // NOLINT
#define AGGREGATOR_UPDATE_EVENT_H_ // NOLINT

/*
 * @brief aggregator update object
 */
class AggregatorUpdate
{
public:
    AggregatorUpdate() : id_(-1), desc_("Not Set") {}

    const int64_t getId() const { return id_; }
    const std::string& getDesc() const { return desc_; }

    void setId(const int64_t id)
    {
        id_ = id;
    }

    void setDesc(const std::string& desc)
    {
        desc_ = desc;
    }

private:
    int64_t id_;
    std::string desc_;
};

/*
 * @brief ring buffer entry which encapsulates updates received
 * from ART
 */

class AggregatorUpdateEvent : public disruptor::test::LongEvent
{
 public:
    AggregatorUpdateEvent(const int64_t& value = 0) : LongEvent(value) {}

    const std::string& getRawByteStream() const { return rawByteStream_; }
    AggregatorUpdate& getAggregatorUpdate() { return aggregatorUpdate_; }

    void setRawByteStream(const std::string& byteStream)
    {
        rawByteStream_ = byteStream;
    }


 private:
    std::string rawByteStream_;
    AggregatorUpdate aggregatorUpdate_;
};

/*
 * @brief Factory to pre-allocate aggregator update events
 */

class AggregatorUpdateEventFactory : public disruptor::EventFactoryInterface<AggregatorUpdateEvent>
{
 public:
    virtual AggregatorUpdateEvent* NewInstance(const int& size) const {
        return new AggregatorUpdateEvent[size];
    }
};

#endif // AGGREGATOR_UPDATE_EVENT_H_
