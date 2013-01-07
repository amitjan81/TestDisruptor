#include <string>
#include <sstream>

#include "disruptor/interface.h"
#include "keyValueUpdateEvent.h"

#ifndef KEYVALUE_UPDATE_HANDLER_H_ // NOLINT
#define KEYVALUE_UPDATE_HANDLER_H_ // NOLINT


class KeyValuePublisherBatchHandler : public disruptor::EventHandlerInterface<KeyValueUpdateEvent>
{
 public:
    KeyValuePublisherBatchHandler() : value_(0) {}
    virtual void OnEvent(const int64_t& sequence,
                         const bool& end_of_batch,
                         KeyValueUpdateEvent* event)
    {
        if (event)
        {
            KeyValueUpdate& update(event->getKeyValueUpdate());
            const std::string& key(update.getKey());
            const int64_t& value(update.getValue());
            value_ += value;

#if 0
            std::stringstream ss;
            ss << "Received Value [" << value << "] and Total Value ["
                << value_ << "]" << std::endl;
            std::cout << ss.str();
#endif

            event->set_value(sequence);
        }
    };

    virtual void OnStart() { start_ = std::chrono::high_resolution_clock::now();}
    virtual void OnShutdown()
    {
        end_ = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double,std::ratio<1,1>> d = std::chrono::duration_cast<std::chrono::duration<double>>(end_ - start_);
        std::stringstream ss;
        ss << "KeyValue Publisher Thread duration = " << d.count() << " seconds" << std::endl;
        std::cout << ss.str();
    } 
 
 private:
    std::chrono::high_resolution_clock::time_point start_;
    std::chrono::high_resolution_clock::time_point end_;
    int64_t value_;
};

#endif
