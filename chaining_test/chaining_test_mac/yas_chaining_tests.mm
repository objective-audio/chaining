//
//  yas_chaining_tests.mm
//

#import <XCTest/XCTest.h>
#import <string>
#import "yas_chaining.h"
#import "yas_chaining_fetcher.h"
#import "yas_chaining_notifier.h"

using namespace yas;

@interface yas_chaining_tests : XCTestCase

@end

@implementation yas_chaining_tests

- (void)setUp {
    [super setUp];
}

- (void)tearDown {
    [super tearDown];
}

- (void)test_to {
    chaining::notifier<int> notifier;

    int received = -1;

    auto chain = notifier.chain()
                     .to([](int const &value) { return value + 1; })
                     .perform([&received](int const &value) { received = value; })
                     .end();

    notifier.notify(10);

    XCTAssertEqual(received, 11);
}

- (void)test_to_type {
    chaining::notifier<int> notifier;

    std::string received = "";

    auto chain = notifier.chain()
                     .guard([](int const &) { return true; })
                     .to([](int const &value) { return value > 0; })
                     .guard([](bool const &) { return true; })
                     .to([](bool const &value) { return value ? "true" : "false"; })
                     .guard([](std::string const &) { return true; })
                     .perform([&received](std::string const &value) { received = value; })
                     .end();

    notifier.notify(0);

    XCTAssertEqual(received, "false");

    notifier.notify(1);

    XCTAssertEqual(received, "true");
}

- (void)test_to_value {
    chaining::notifier<int> notifier;

    std::string received = "";

    auto chain = notifier.chain()
                     .to_value(std::string("test_value"))
                     .perform([&received](std::string const &value) { received = value; })
                     .end();

    notifier.notify(0);

    XCTAssertEqual(received, "test_value");
}

- (void)test_to_null {
    chaining::notifier<int> notifier;

    bool called = false;

    auto chain = notifier.chain().to_null().perform([&called](std::nullptr_t const &) { called = true; }).end();

    notifier.notify(1);

    XCTAssertTrue(called);
}

- (void)test_to_tuple {
    chaining::notifier<int> notifier;

    opt_t<std::tuple<int>> called;

    auto chain = notifier.chain().to_tuple().perform([&called](std::tuple<int> const &value) { called = value; }).end();

    notifier.notify(1);

    XCTAssertTrue(called);
    XCTAssertEqual(std::get<0>(*called), 1);
}

- (void)test_to_tuple_from_tuple {
    chaining::notifier<std::tuple<int>> notifier;

    opt_t<std::tuple<int>> called;

    auto chain = notifier.chain().to_tuple().perform([&called](auto const &value) { called = value; }).end();

    notifier.notify(std::make_tuple(int(1)));

    XCTAssertTrue(called);
    XCTAssertEqual(std::get<0>(*called), 1);
}

- (void)test_to_tuple_from_pair {
    chaining::notifier<std::pair<int, std::string>> notifier;

    opt_t<std::tuple<int, std::string>> called;

    auto chain = notifier.chain().to_tuple().perform([&called](auto const &value) { called = value; }).end();

    notifier.notify(std::make_pair(int(1), std::string("2")));

    XCTAssertTrue(called);
    XCTAssertEqual(std::get<0>(*called), 1);
    XCTAssertEqual(std::get<1>(*called), "2");
}

- (void)test_sync {
    chaining::fetcher<int> fetcher{[] { return 100; }};

    int received = -1;

    auto chain = fetcher.chain().perform([&received](int const &value) { received = value; }).end();
    chain.sync();

    XCTAssertEqual(received, 100);
}

- (void)test_sync_by_observer {
    chaining::fetcher<int> fetcher{[] { return 100; }};

    int received = -1;

    chaining::observer chain = fetcher.chain().perform([&received](int const &value) { received = value; }).end();
    chain.sync();

    XCTAssertEqual(received, 100);
}

- (void)test_sync_many_chain {
    chaining::fetcher<int> fetcher{[] { return 100; }};

    int received1 = -1;
    int received2 = -1;

    auto chain1 = fetcher.chain().perform([&received1](int const &value) { received1 = value; }).end();
    auto chain2 = fetcher.chain().perform([&received2](int const &value) { received2 = value; }).end();

    chain1.sync();

    XCTAssertEqual(received1, 100);
    XCTAssertEqual(received2, -1);
}

- (void)test_sync_end {
    chaining::fetcher<int> fetcher{[] { return 100; }};

    int received = -1;

    auto chain = fetcher.chain().perform([&received](int const &value) { received = value; }).sync();

    XCTAssertEqual(received, 100);
}

- (void)test_sync_with_combined_sub_sender {
    chaining::fetcher<int> fetcher{[]() { return 123; }};
    chaining::fetcher<int> sub_fetcher{[]() { return 456; }};

    std::vector<std::pair<int, int>> received;

    auto chain = fetcher.chain()
                     .combine(sub_fetcher.chain())
                     .perform([&received](auto const &pair) { received.emplace_back(pair); })
                     .sync();

    // 2つのsenderから来た値が両方揃ってから受け取った
    XCTAssertEqual(received.size(), 1);
    XCTAssertEqual(received.at(0).first, 123);
    XCTAssertEqual(received.at(0).second, 456);
}

- (void)test_sync_with_merged_sub_sender {
    chaining::fetcher<int> fetcher{[]() { return 78; }};
    chaining::fetcher<int> sub_fetcher{[]() { return 90; }};

    std::vector<int> received;

    auto chain = fetcher.chain()
                     .merge(sub_fetcher.chain())
                     .perform([&received](int const &value) { received.emplace_back(value); })
                     .sync();

    // main -> sub の順番で実行される
    XCTAssertEqual(received.size(), 2);
    XCTAssertEqual(received.at(0), 78);
    XCTAssertEqual(received.at(1), 90);
}

- (void)test_receive {
    std::string received = "";

    chaining::notifier<int> notifier;
    chaining::receiver<std::string> receiver{[&received](std::string const &value) { received = value; }};

    auto node = notifier.chain().to([](int const &value) { return std::to_string(value); }).receive(receiver).end();

    notifier.notify(3);

    XCTAssertEqual(received, "3");
}

- (void)test_receive_array {
    chaining::notifier<std::array<int, 2>> notifier;
    int received0 = -1;
    int received1 = -1;

    chaining::receiver<int> receiver0{[&received0](int const &value) { received0 = value; }};
    chaining::receiver<int> receiver1{[&received1](int const &value) { received1 = value; }};
    std::array<chaining::receiver<int>, 2> receivers{receiver0, receiver1};

    chaining::observer chain = notifier.chain().receive(receivers).end();

    notifier.notify(std::array<int, 2>{10, 20});

    XCTAssertEqual(received0, 10);
    XCTAssertEqual(received1, 20);
}

- (void)test_receive_array_individual {
    chaining::notifier<std::array<int, 2>> notifier;
    int received0 = -1;
    int received1 = -1;

    chaining::receiver<int> receiver0{[&received0](int const &value) { received0 = value; }};
    chaining::receiver<int> receiver1{[&received1](int const &value) { received1 = value; }};

    chaining::observer chain = notifier.chain().receive<0>(receiver0).receive<1>(receiver1).end();

    notifier.notify(std::array<int, 2>{10, 20});

    XCTAssertEqual(received0, 10);
    XCTAssertEqual(received1, 20);
}

- (void)test_receiver_vector {
    chaining::notifier<std::vector<int>> notifier;
    int received0 = -1;
    int received1 = -1;

    chaining::receiver<int> receiver0{[&received0](int const &value) { received0 = value; }};
    chaining::receiver<int> receiver1{[&received1](int const &value) { received1 = value; }};
    std::vector<chaining::receiver<int>> receivers{receiver0, receiver1};

    chaining::observer chain = notifier.chain().receive(receivers).end();

    notifier.notify(std::vector<int>{30, 40});

    XCTAssertEqual(received0, 30);
    XCTAssertEqual(received1, 40);
}

- (void)test_receiver_initializer_list {
    chaining::notifier<std::vector<int>> notifier;
    int received0 = -1;
    int received1 = -1;

    chaining::receiver<int> receiver0{[&received0](int const &value) { received0 = value; }};
    chaining::receiver<int> receiver1{[&received1](int const &value) { received1 = value; }};

    chaining::observer chain = notifier.chain().receive({receiver0, receiver1}).end();

    notifier.notify(std::vector<int>{50, 60});

    XCTAssertEqual(received0, 50);
    XCTAssertEqual(received1, 60);
}

- (void)test_receive_tuple {
    chaining::notifier<std::tuple<int, std::string>> notifier;

    int int_received = -1;
    std::string string_received = "";

    chaining::receiver<int> int_receiver{[&int_received](int const &value) { int_received = value; }};
    chaining::receiver<std::string> string_receiver{
        [&string_received](std::string const &value) { string_received = value; }};

    auto chain = notifier.chain().receive<0>(int_receiver).receive<1>(string_receiver).end();

    notifier.notify(std::make_tuple(int(10), std::string("20")));

    XCTAssertEqual(int_received, 10);
    XCTAssertEqual(string_received, "20");
}

- (void)test_receive_null {
    bool received = false;

    chaining::notifier<int> notifier;
    chaining::receiver<> receiver{[&received]() { received = true; }};

    auto chain = notifier.chain().receive_null(receiver).end();

    notifier.notify(4);

    XCTAssertTrue(received);
}

- (void)test_receiver {
    int received = -1;

    chaining::notifier<int> notifier;

    chaining::receiver<int> receiver{[&received](int const &value) { received = value; }};

    auto chain = notifier.chain().receive(receiver).end();

    notifier.notify(100);

    XCTAssertEqual(received, 100);
}

- (void)test_guard {
    float received = -1.0f;

    chaining::notifier<int> notifier;

    auto chain = notifier.chain()
                     .to([](int const &value) { return value; })
                     .guard([](float const &value) { return value > 2.5f; })
                     .perform([&received](float const &value) { received = value; })
                     .end();

    notifier.notify(2);

    XCTAssertEqual(received, -1.0f);

    notifier.notify(3);

    XCTAssertEqual(received, 3.0f);
}

- (void)test_merge {
    std::string received;

    chaining::notifier<int> notifier;
    chaining::notifier<float> sub_notifier;

    auto sub_chain = sub_notifier.chain().to([](float const &value) { return std::to_string(int(value)); });

    auto chain = notifier.chain()
                     .to([](int const &value) { return std::to_string(value); })
                     .merge(sub_chain)
                     .perform([&received](std::string const &value) { received = value; })
                     .end();

    notifier.notify(10);

    XCTAssertEqual(received, "10");

    sub_notifier.notify(20.0f);

    XCTAssertEqual(received, "20");
}

- (void)test_pair {
    chaining::notifier<int> main_notifier;
    chaining::notifier<std::string> sub_notifier;

    using opt_pair_t = std::pair<opt_t<int>, opt_t<std::string>>;

    opt_pair_t received;

    auto sub_chain = sub_notifier.chain();
    auto main_chain =
        main_notifier.chain().pair(sub_chain).perform([&received](auto const &value) { received = value; }).end();

    main_notifier.notify(1);

    XCTAssertEqual(*received.first, 1);
    XCTAssertFalse(!!received.second);

    sub_notifier.notify("test_text");

    XCTAssertFalse(!!received.first);
    XCTAssertEqual(*received.second, "test_text");
}

- (void)test_combine {
    chaining::notifier<int> main_notifier;
    chaining::notifier<std::string> sub_notifier;

    using opt_pair_t = opt_t<std::pair<int, std::string>>;

    opt_pair_t received;

    auto sub_chain = sub_notifier.chain();
    auto main_chain =
        main_notifier.chain().combine(sub_chain).perform([&received](auto const &value) { received = value; }).end();

    main_notifier.notify(1);

    XCTAssertFalse(received);

    sub_notifier.notify("test_text");

    XCTAssertTrue(received);
    XCTAssertEqual(received->first, 1);
    XCTAssertEqual(received->second, "test_text");
}

- (void)test_combine_tuples {
    chaining::notifier<int> main_notifier;
    chaining::notifier<std::string> sub_notifier;
    chaining::notifier<float> sub_notifier2;

    auto sub_chain = sub_notifier.chain().to_tuple();
    auto main_chain = main_notifier.chain().to_tuple();
    auto sub_chain2 = sub_notifier2.chain().to_tuple();

    opt_t<std::tuple<int, std::string, float>> received;

    auto chain = main_chain.combine(sub_chain)
                     .combine(sub_chain2)
                     .perform([&received](std::tuple<int, std::string, float> const &value) { received = value; })
                     .end();

    main_notifier.notify(33);
    sub_notifier.notify("44");
    sub_notifier2.notify(55.0f);

    XCTAssertTrue(received);
    XCTAssertEqual(std::get<0>(*received), 33);
    XCTAssertEqual(std::get<1>(*received), "44");
    XCTAssertEqual(std::get<2>(*received), 55.0f);
}

- (void)test_normalize {
    chaining::notifier<int> notifier;

    chaining::chain<std::string, int, int, false> to_chain =
        notifier.chain().to([](int const &value) { return std::to_string(value); });

    chaining::chain<std::string, std::string, int, false> normalized_chain = to_chain.normalize();

    std::string received = "";

    chaining::typed_observer<int> observer =
        normalized_chain.perform([&received](std::string const &value) { received = value; }).end();

    notifier.notify(10);

    XCTAssertEqual(received, "10");
}

@end
