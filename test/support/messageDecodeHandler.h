#include <string>
#include <sstream>
#include <algorithm>

#include "disruptor/interface.h"
#include "aggregatorUpdateEvent.h"

#ifndef MESSAGE_DECODE_HANDLER_H_ // NOLINT
#define MESSAGE_DECODE_HANDLER_H_ // NOLINT


class MessageDecodeBatchHandler : public disruptor::EventHandlerInterface<AggregatorUpdateEvent>
{
 public:
    virtual void OnEvent(const int64_t& sequence,
                         const bool& end_of_batch,
                         AggregatorUpdateEvent* event)
    {
        if (event)
        {
#if defined(DEBUG)
            std::stringstream ss;
            ss << std::this_thread::get_id() << " : Decoding Message with Received Sequence [" << sequence
                << "] and event Sequence [" << event->value() << "]" << std::endl;
            //std::cout << ss.str();
            _bufferedLog_.push_back(ss.str());
#endif

            AggregatorUpdate& update(event->getAggregatorUpdate());
            update.setId(sequence);
            update.setDesc(event->getRawByteStream());

            if (event->value() != sequence || sequence == 4)
            {
                // dump all the received sequences
                //for_each(begin(receivedSequence_), end(receivedSequence_), [](int i){ std::cout << i << std::endl; });

                std::stringstream ss;
                ss << "FATAL : Message Decoder event sequence number [" << event->value()
                    << "] is not same as received sequence [" << sequence << "]";

                throw std::runtime_error(ss.str());
            }
            else
            {
                // receivedSequence_.push_back(sequence);
            }

            event->set_value(sequence);
        }
    };

    virtual void OnStart() { start_ = std::chrono::high_resolution_clock::now();}
    virtual void OnShutdown()
    {
        end_ = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double,std::ratio<1,1>> d = std::chrono::duration_cast<std::chrono::duration<double>>(end_ - start_);
        std::stringstream ss;
        ss << "Message Decoder Thread duration = " << d.count() << " seconds" << std::endl;
        std::cout << ss.str();
    } 
 
 private:
    std::chrono::high_resolution_clock::time_point start_;
    std::chrono::high_resolution_clock::time_point end_;
    //std::vector<int64_t> receivedSequence_;
};

#endif // DISRUPTOR_TEST_LONG_EVENT_H_ NOLINT
