//
//  yas_value_holder_tests.mm
//

#import <XCTest/XCTest.h>
#import <chaining/chaining.h>

using namespace yas;
using namespace yas::chaining;

@interface yas_value_holder_tests : XCTestCase

@end

@implementation yas_value_holder_tests

- (void)setUp {
    [super setUp];
}

- (void)tearDown {
    [super tearDown];
}

- (void)test_make_shared {
    auto holder = value::holder<int>::make_shared(1);

    XCTAssertTrue(holder);
}

- (void)test_getter_setter {
    auto holder = value::holder<int>::make_shared(1);

    XCTAssertEqual(holder->value(), 1);

    holder->set_value(2);

    XCTAssertEqual(holder->value(), 2);
}

- (void)test_const_getter {
    auto const holder = value::holder<int>::make_shared(1);

    XCTAssertEqual(holder->value(), 1);
}

- (void)test_chain {
    auto holder = value::holder<int>::make_shared(10);

    int received = -1;

    auto flow = holder->chain().perform([&received](int const &value) { received = value; }).sync();

    XCTAssertEqual(received, 10);

    holder->set_value(20);

    XCTAssertEqual(received, 20);
}

- (void)test_receive {
    auto holder = value::holder<int>::make_shared(100);
    auto notifier = chaining::notifier<int>::make_shared();

    auto flow = notifier->chain().send_to(holder).end();

    XCTAssertEqual(holder->value(), 100);

    notifier->notify(200);

    XCTAssertEqual(holder->value(), 200);
}

- (void)test_ignore_recursive {
    auto holder1 = value::holder<int>::make_shared(123);
    auto holder2 = value::holder<int>::make_shared(456);

    auto flow1 = holder1->chain().send_to(holder2).sync();

    XCTAssertEqual(holder1->value(), 123);
    XCTAssertEqual(holder2->value(), 123);

    auto flow2 = holder2->chain().send_to(holder1).sync();

    XCTAssertEqual(holder1->value(), 123);
    XCTAssertEqual(holder2->value(), 123);

    holder1->set_value(789);

    XCTAssertEqual(holder1->value(), 789);
    XCTAssertEqual(holder2->value(), 789);

    holder2->set_value(0);

    XCTAssertEqual(holder1->value(), 0);
    XCTAssertEqual(holder2->value(), 0);
}

@end
