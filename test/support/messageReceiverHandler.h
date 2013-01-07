#include <string>
#include <sstream>

#include "disruptor/interface.h"
#include "aggregatorUpdateEvent.h"

#ifndef MESSAGE_RECEIVER_EVENT_H_ // NOLINT
#define MESSAGE_RECEIVER_EVENT_H_ // NOLINT


class MessageReceiverBatchHandler : public disruptor::EventHandlerInterface<AggregatorUpdateEvent> {
 public:
    virtual void OnEvent(const int64_t& sequence,
                         const bool& end_of_batch,
                         AggregatorUpdateEvent* event) {
        if (event)
        {
            //std::cout << "Received MessageReceiver Event with Sequence : " << sequence << std::endl;
            event->set_value(sequence);
        }
    };

    virtual void OnStart() {}
    virtual void OnShutdown() {}
};

class MessageReceiverEventTranslator : public disruptor::EventTranslatorOneArgInterface<AggregatorUpdateEvent, QpidDataType> {
 public:
    virtual void TranslateTo(const int64_t& sequence, AggregatorUpdateEvent* event, const QpidDataType& data) {
#if defined(DEBUG)
        std::stringstream ss;
        ss << std::this_thread::get_id() << " : Translating MessageReceiver Event with Sequence [" << sequence << "]" << std::endl;
        //std::cout << ss.str();
        _bufferedLog_.push_back(ss.str());
#endif
        event->set_value(sequence);

        event->setRawByteStream(data);
    };

};

#endif // DISRUPTOR_TEST_LONG_EVENT_H_ NOLINT
