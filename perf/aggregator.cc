#include <sys/time.h>

#include <exception>
#include <functional>
#include <iostream>
#include <memory>
#include <thread>
#include <vector>
#include <string>

std::vector<std::string> _bufferedLog_;

#include <disruptor/ring_buffer.h>
#include <disruptor/event_publisher.h>
#include <disruptor/event_processor.h>
#include <disruptor/exception_handler.h>

#include "../test/support/aggregatorUpdateEvent.h"
#include "../test/support/messageReceiverHandler.h"
#include "../test/support/messageDecodeHandler.h"
#include "../test/support/businessLogicHandler.h"
#include "../test/support/keyValuePublisherBatchHandler.h"

using namespace disruptor;

int main(int arc, char** argv) {
    long buffer_size = 1024L * 1024L * 30;
    //long iterations =  1000L * 1000L * 30;
    long iterations = 100L;

    AggregatorUpdateEventFactory aggregatorUpdateEventFactory;
    RingBuffer<AggregatorUpdateEvent> ringBufferAggregatorUpdate(&aggregatorUpdateEventFactory,
                                       buffer_size,
                                       kSingleThreadedStrategy,
                                       kBusySpinStrategy);

    KeyValueUpdateEventFactory keyValueUpdateEventFactory;
    RingBuffer<KeyValueUpdateEvent> ringBufferKeyValueUpdate(&keyValueUpdateEventFactory,
                                       buffer_size,
                                       kSingleThreadedStrategy,
                                       kBusySpinStrategy);

    // one exception handler
    FatalExceptionHandler<AggregatorUpdateEvent> aggregatorUpdateExceptionHandler;
    FatalExceptionHandler<KeyValueUpdateEvent> keyValueUpdateExceptionHandler;

    std::vector<Sequence*> sequenceToTrackAggregatorUpdate(0);

    // translator and publisher
    std::unique_ptr<MessageReceiverEventTranslator> translator(new MessageReceiverEventTranslator);
    EventPublisher<AggregatorUpdateEvent> publisher(&ringBufferAggregatorUpdate);

    EventPublisher<KeyValueUpdateEvent> keyValuePub(&ringBufferKeyValueUpdate);

    // event handlers
    MessageDecodeBatchHandler decodeHandler;
    BusinessLogicBatchHandler businessLogicHandler(keyValuePub);
    KeyValuePublisherBatchHandler keyValuePublisherBatchHandler;

    std::unique_ptr<ProcessingSequenceBarrier> first_barrier(
        ringBufferAggregatorUpdate.NewBarrier(sequenceToTrackAggregatorUpdate));
    BatchEventProcessor<AggregatorUpdateEvent> first_processor(&ringBufferAggregatorUpdate,
                                              (SequenceBarrierInterface*) first_barrier.get(),
                                              &decodeHandler,
                                              &aggregatorUpdateExceptionHandler);

    sequenceToTrackAggregatorUpdate.clear();
    sequenceToTrackAggregatorUpdate.push_back(first_processor.GetSequence());

    std::unique_ptr<ProcessingSequenceBarrier> second_barrier(
        ringBufferAggregatorUpdate.NewBarrier(sequenceToTrackAggregatorUpdate));
    BatchEventProcessor<AggregatorUpdateEvent> second_processor(&ringBufferAggregatorUpdate,
                                              (SequenceBarrierInterface*) second_barrier.get(),
                                              &businessLogicHandler,
                                              &aggregatorUpdateExceptionHandler);
    sequenceToTrackAggregatorUpdate.clear();
    sequenceToTrackAggregatorUpdate.push_back(second_processor.GetSequence());

    std::unique_ptr<ProcessingSequenceBarrier> third_barrier(
        ringBufferKeyValueUpdate.NewBarrier(sequenceToTrackAggregatorUpdate));
    BatchEventProcessor<KeyValueUpdateEvent> third_processor(&ringBufferKeyValueUpdate,
                                              (SequenceBarrierInterface*) third_barrier.get(),
                                              &keyValuePublisherBatchHandler,
                                              &keyValueUpdateExceptionHandler);

    try {
        std::thread first_consumer(std::ref<BatchEventProcessor<AggregatorUpdateEvent>>(first_processor));
        std::thread second_consumer(std::ref<BatchEventProcessor<AggregatorUpdateEvent>>(second_processor));
        std::thread third_consumer(std::ref<BatchEventProcessor<KeyValueUpdateEvent>>(third_processor));

        struct timeval start_time, end_time;

        gettimeofday(&start_time, NULL);

        QpidDataType data("Aggregator update data : ");
        for (long i=0; i<iterations; i++) {
            publisher.PublishEvent(translator.get(), data);
        }

        long expected_sequence = ringBufferAggregatorUpdate.GetCursor();
        while (second_processor.GetSequence()->sequence() < expected_sequence)
        {
            std::chrono::milliseconds dura(2);
            std::this_thread::sleep_for(dura);
        }

        gettimeofday(&end_time, NULL);

        double start, end;
        start = start_time.tv_sec + ((double) start_time.tv_usec / 1000000);
        end = end_time.tv_sec + ((double) end_time.tv_usec / 1000000);

        std::cout.precision(15);
        std::cout << "Aggregator performance: ";
        std::cout << (iterations * 1.0) / (end - start)
                  << " ops/secs" << std::endl;

        first_processor.Halt();
        second_processor.Halt();
        third_processor.Halt();

        first_consumer.join();
        second_consumer.join();
        third_consumer.join();

        return EXIT_SUCCESS;
    }
    //catch (const std::exception& e) {
    catch (...) {
        std::cout << "Caught Exception " << std::endl;
        first_processor.Halt();
        second_processor.Halt();
        third_processor.Halt();

        for_each(begin(_bufferedLog_), end(_bufferedLog_),
                [](std::string& ss){ std::cout << ss; });
        //std::cout << e.what() << std::endl;
        exit(-1);
    }
}

