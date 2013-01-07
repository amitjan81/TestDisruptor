#include <string>
#include <sstream>

#include "disruptor/interface.h"
#include "disruptor/event_publisher.h"

#include "aggregatorUpdateEvent.h"
#include "keyValueUpdateEvent.h"
#include "keyValueEventTranslator.h"

#ifndef BUSINESS_LOGIC_HANDLER_H_ // NOLINT
#define BUSINESS_LOGIC_HANDLER_H_ // NOLINT


class BusinessLogicBatchHandler : public disruptor::EventHandlerInterface<AggregatorUpdateEvent>
{
 public:
    BusinessLogicBatchHandler(disruptor::EventPublisher<KeyValueUpdateEvent>& publisher) : publisher_(publisher),
        translator_(new KeyValueEventTranslator)
    {}
    virtual void OnEvent(const int64_t& sequence,
                         const bool& end_of_batch,
                         AggregatorUpdateEvent* event)
    {
        if (event)
        {
            //std::stringstream ss;
            AggregatorUpdate& update(event->getAggregatorUpdate());

            //ss << "Run business Logic for id [" << update.getId() << "] and desc ["
            //    << update.getDesc() << "]" << std::endl;
            //std::cout << ss.str();

            if (update.getId() != sequence)
            {
                std::cout << "Sequence Number  in update [" << update.getId()
                    << "] and received sequence [" << sequence << "] doesn't match" << std::endl;
                exit(-1);
            }
            event->set_value(sequence);

            KeyValueUpdate up;
            up.setValue(sequence);
            publisher_.PublishEvent(translator_.get(), up);
        }
    };
    
    virtual void OnStart() { start_ = std::chrono::high_resolution_clock::now();}
    virtual void OnShutdown()
    {
        end_ = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double,std::ratio<1,1>> d = std::chrono::duration_cast<std::chrono::duration<double>>(end_ - start_);
        std::stringstream ss;
        ss << "Business Logic Thread duration = " << d.count() << " seconds" << std::endl;
        std::cout << ss.str();
    } 
 
 private:
    std::chrono::high_resolution_clock::time_point start_;
    std::chrono::high_resolution_clock::time_point end_;
    std::unique_ptr<KeyValueEventTranslator> translator_;
    disruptor::EventPublisher<KeyValueUpdateEvent> publisher_;
};

#endif // BUSINESS_LOGIC_HANDLER_H_
