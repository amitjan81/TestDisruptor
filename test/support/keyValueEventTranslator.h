#include <string>

#include "disruptor/interface.h"
#include "keyValueUpdateEvent.h"

#ifndef KEYVALUE_EVENT_TRANSLATOR_H_ // NOLINT
#define KEYVALUE_EVENT_TRANSLATOR_H_ // NOLINT


class KeyValueEventTranslator : public disruptor::EventTranslatorOneArgInterface<KeyValueUpdateEvent, KeyValueUpdate> {
 public:
    virtual void TranslateTo(const int64_t& sequence, KeyValueUpdateEvent* event, const KeyValueUpdate& data) {
        //std::cout << "Translating MessageReceiver Event with Sequence : " << sequence << std::endl;
        event->set_value(sequence);

        event->setKeyValueUpdate(data);
    };

};

#endif // DISRUPTOR_TEST_LONG_EVENT_H_ NOLINT
