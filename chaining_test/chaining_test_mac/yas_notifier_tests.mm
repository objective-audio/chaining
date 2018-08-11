//
//  yas_flow_notifier_tests.mm
//

#import <XCTest/XCTest.h>
#import "yas_chaining.h"

using namespace yas;

@interface yas_flow_notifier_tests : XCTestCase

@end

@implementation yas_flow_notifier_tests

- (void)setUp {
    [super setUp];
}

- (void)tearDown {
    [super tearDown];
}

- (void)test_notifier_begin {
    int received = -1;

    chaining::notifier<int> notifier;

    auto flow = notifier.chain().perform([&received](int const &value) { received = value; }).end();

    XCTAssertEqual(received, -1);

    notifier.notify(2);

    XCTAssertEqual(received, 2);
}

- (void)test_notifier_many_begin {
    int received1 = -1;
    int received2 = -1;

    chaining::notifier<int> notifier;

    auto flow1 = notifier.chain().perform([&received1](int const &value) { received1 = value; }).end();
    auto flow2 = notifier.chain().perform([&received2](int const &value) { received2 = value; }).end();

    notifier.notify(3);

    XCTAssertEqual(received1, 3);
    XCTAssertEqual(received2, 3);
}

- (void)test_notifier_receiver {
    int received = -1;

    chaining::notifier<int> notifier1;
    chaining::notifier<int> notifier2;

    auto flow1 = notifier1.chain().receive(notifier2.receiver()).end();
    auto flow2 = notifier2.chain().perform([&received](int const &value) { received = value; }).end();

    notifier1.notify(4);

    XCTAssertEqual(received, 4);
}

- (void)test_notifier_block_recursive_call {
    int received = -1;

    chaining::notifier<int> notifier;

    chaining::receiver<int> receiver{[&notifier, &received](int const &value) {
        received = value;
        notifier.notify(value + 1);
    }};

    auto flow = notifier.chain().receive(receiver).end();

    notifier.notify(1);

    XCTAssertEqual(received, 1);
}

@end
