#include <sys/time.h>

#include <exception>
#include <functional>
#include <iostream>
#include <memory>
#include <thread>

#include <disruptor/ring_buffer.h>
#include <disruptor/event_publisher.h>
#include <disruptor/event_processor.h>
#include <disruptor/exception_handler.h>

#include "../test/support/stub_event.h"

using namespace disruptor;

int main(int arc, char** argv) {
    int buffer_size = 1024 * 8;
    long iterations = 1000L * 1000L * 30;
    //long iterations = 4L;

    test::StubEventFactory stub_factory;
    RingBuffer<test::StubEvent> ring_buffer(&stub_factory,
                                       buffer_size,
                                       kSingleThreadedStrategy,
                                       kBusySpinStrategy);

    // one exception handler
    IgnoreExceptionHandler<test::StubEvent> stub_exception_handler;
    // one event handler
    test::StubBatchHandler stub_handler;
    //test::StubBatchHandler2 stub_handler2;

    std::vector<Sequence*> sequence_to_track(0);

    std::unique_ptr<ProcessingSequenceBarrier> first_barrier(
        ring_buffer.NewBarrier(sequence_to_track));
    BatchEventProcessor<test::StubEvent> first_processor(&ring_buffer,
                                              (SequenceBarrierInterface*) first_barrier.get(),
                                              &stub_handler,
                                              &stub_exception_handler);

    sequence_to_track.clear();
    sequence_to_track.push_back(first_processor.GetSequence());

    std::unique_ptr<ProcessingSequenceBarrier> second_barrier(
        ring_buffer.NewBarrier(sequence_to_track));
    BatchEventProcessor<test::StubEvent> second_processor(&ring_buffer,
                                              (SequenceBarrierInterface*) second_barrier.get(),
                                              &stub_handler,
                                              &stub_exception_handler);
    sequence_to_track.clear();
    sequence_to_track.push_back(second_processor.GetSequence());


    std::thread first_consumer(std::ref<BatchEventProcessor<test::StubEvent>>(first_processor));
    std::thread second_consumer(std::ref<BatchEventProcessor<test::StubEvent>>(second_processor));

    struct timeval start_time, end_time;

    gettimeofday(&start_time, NULL);

    std::unique_ptr<test::StubEventTranslator> translator(new test::StubEventTranslator);
    EventPublisher<test::StubEvent> publisher(&ring_buffer);
    for (long i=0; i<iterations; i++) {
        publisher.PublishEvent(translator.get());
    }

    long expected_sequence = ring_buffer.GetCursor();
    while (second_processor.GetSequence()->sequence() < expected_sequence) {}

    gettimeofday(&end_time, NULL);

    first_processor.Halt();
    second_processor.Halt();

    first_consumer.join();
    second_consumer.join();

    double start, end;
    start = start_time.tv_sec + ((double) start_time.tv_usec / 1000000);
    end = end_time.tv_sec + ((double) end_time.tv_usec / 1000000);

    std::cout.precision(15);
    std::cout << "1P-3EP-PIPELINE performance: ";
    std::cout << (iterations * 1.0) / (end - start)
              << " ops/secs" << std::endl;

    return EXIT_SUCCESS;
}

