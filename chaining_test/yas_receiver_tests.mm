//
//  yas_receiver_tests.mm
//

#import <XCTest/XCTest.h>
#import <chaining/yas_chaining_umbrella.h>
#import <string>

using namespace yas;

@interface yas_receiver_tests : XCTestCase

@end

@implementation yas_receiver_tests

- (void)setUp {
    [super setUp];
}

- (void)tearDown {
    [super tearDown];
}

- (void)test_receive {
    std::string received = "";

    chaining::notifier<int> notifier;
    chaining::receiver<std::string> receiver{[&received](std::string const &value) { received = value; }};

    auto node = notifier.chain().to([](int const &value) { return std::to_string(value); }).send_to(receiver).end();

    notifier.notify(3);

    XCTAssertEqual(received, "3");
}

- (void)test_receive_tuple {
    chaining::notifier<std::tuple<int, std::string>> notifier;

    int int_received = -1;
    std::string string_received = "";

    chaining::receiver<int> int_receiver{[&int_received](int const &value) { int_received = value; }};
    chaining::receiver<std::string> string_receiver{
        [&string_received](std::string const &value) { string_received = value; }};

    auto chain = notifier.chain().send_to<0>(int_receiver).send_to<1>(string_receiver).end();

    notifier.notify(std::make_tuple(int(10), std::string("20")));

    XCTAssertEqual(int_received, 10);
    XCTAssertEqual(string_received, "20");
}

- (void)test_send_null {
    bool received = false;

    chaining::notifier<int> notifier;
    chaining::receiver<> receiver{[&received]() { received = true; }};

    auto chain = notifier.chain().send_null(receiver).end();

    notifier.notify(4);

    XCTAssertTrue(received);
}

- (void)test_receiver {
    int received = -1;

    chaining::notifier<int> notifier;

    chaining::receiver<int> receiver{[&received](int const &value) { received = value; }};

    auto chain = notifier.chain().send_to(receiver).end();

    notifier.notify(100);

    XCTAssertEqual(received, 100);
}

- (void)test_receive_array {
    chaining::notifier<std::array<int, 2>> notifier;
    int received0 = -1;
    int received1 = -1;

    chaining::receiver<int> receiver0{[&received0](int const &value) { received0 = value; }};
    chaining::receiver<int> receiver1{[&received1](int const &value) { received1 = value; }};
    std::array<chaining::receiver<int>, 2> receivers{receiver0, receiver1};

    chaining::any_observer chain = notifier.chain().send_to(receivers).end();

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

    chaining::any_observer chain = notifier.chain().send_to<0>(receiver0).send_to<1>(receiver1).end();

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

    chaining::any_observer chain = notifier.chain().send_to(receivers).end();

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

    chaining::any_observer chain = notifier.chain().send_to({receiver0, receiver1}).end();

    notifier.notify(std::vector<int>{50, 60});

    XCTAssertEqual(received0, 50);
    XCTAssertEqual(received1, 60);
}

@end
