//
//  yas_flow_holder_tests.mm
//

#import <XCTest/XCTest.h>
#import "yas_chaining.h"

using namespace yas;

@interface yas_flow_holder_tests : XCTestCase

@end

@implementation yas_flow_holder_tests

- (void)setUp {
    [super setUp];
}

- (void)tearDown {
    [super tearDown];
}

- (void)test_getter_setter {
    chaining::holder<int> holder{1};

    XCTAssertEqual(holder.value(), 1);

    holder.set_value(2);

    XCTAssertEqual(holder.value(), 2);
}

- (void)test_chain {
    chaining::holder<int> holder{10};

    int received = -1;

    auto flow = holder.chain().perform([&received](int const &value) { received = value; }).sync();

    XCTAssertEqual(received, 10);

    holder.set_value(20);

    XCTAssertEqual(received, 20);
}

- (void)test_receive {
    chaining::holder<int> holder{100};
    chaining::notifier<int> notifier;

    auto flow = notifier.chain().receive(holder.receiver()).end();

    XCTAssertEqual(holder.value(), 100);

    notifier.notify(200);

    XCTAssertEqual(holder.value(), 200);
}

- (void)test_recursive_flow {
    chaining::holder<int> holder1{123};
    chaining::holder<int> holder2{456};

    auto flow1 = holder1.chain().receive(holder2.receiver()).sync();

    XCTAssertEqual(holder1.value(), 123);
    XCTAssertEqual(holder2.value(), 123);

    auto flow2 = holder2.chain().receive(holder1.receiver()).sync();

    XCTAssertEqual(holder1.value(), 123);
    XCTAssertEqual(holder2.value(), 123);

    holder1.set_value(789);

    XCTAssertEqual(holder1.value(), 789);
    XCTAssertEqual(holder2.value(), 789);

    holder2.set_value(0);

    XCTAssertEqual(holder1.value(), 0);
    XCTAssertEqual(holder2.value(), 0);
}

- (void)test_is_equal {
    chaining::holder<int> holder1a{1};
    chaining::holder<int> holder1b{1};
    chaining::holder<int> holder2{2};

    XCTAssertTrue(holder1a == holder1a);
    XCTAssertTrue(holder1a == holder1b);
    XCTAssertFalse(holder1a == holder2);

    XCTAssertFalse(holder1a != holder1a);
    XCTAssertFalse(holder1a != holder1b);
    XCTAssertTrue(holder1a != holder2);
}

@end
