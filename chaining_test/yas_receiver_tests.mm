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

@end
