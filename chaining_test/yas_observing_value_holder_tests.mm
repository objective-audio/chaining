//
//  yas_observing_value_holder_tests.mm
//

#import <XCTest/XCTest.h>
#import <chaining/chaining.h>

using namespace yas;
using namespace yas::observing;

@interface yas_observing_value_holder_tests : XCTestCase

@end

@implementation yas_observing_value_holder_tests

- (void)test_make_by_copy {
    int const value = 10;

    auto const holder = value::holder<int>::make_shared(value);

    XCTAssertEqual(holder->value(), 10);
}

- (void)test_make_by_move {
    int value = 20;

    auto const holder = value::holder<int>::make_shared(std::move(value));

    XCTAssertEqual(holder->value(), 20);
}

- (void)test_value {
    auto const holder = value::holder<int>::make_shared(0);

    XCTAssertEqual(holder->value(), 0);

    holder->set_value(1);

    XCTAssertEqual(holder->value(), 1);

    int const value = 2;

    holder->set_value(value);

    XCTAssertEqual(holder->value(), 2);
}

- (void)test_observe {
    auto const holder = value::holder<int>::make_shared(100);

    std::vector<int> called;

    auto canceller = holder->observe([&called](int const &value) { called.emplace_back(value); });

    XCTAssertEqual(called.size(), 1);
    XCTAssertEqual(called.at(0), 100);

    holder->set_value(101);

    XCTAssertEqual(called.size(), 2);
    XCTAssertEqual(called.at(1), 101);

    holder->set_value(101);

    XCTAssertEqual(called.size(), 2);

    canceller->invalidate();
    holder->set_value(102);

    XCTAssertEqual(called.size(), 2);
}

- (void)test_observe_without_bind {
    auto const holder = value::holder<int>::make_shared(200);

    std::vector<int> called;

    auto canceller = holder->observe([&called](int const &value) { called.emplace_back(value); }, false);

    XCTAssertEqual(called.size(), 0);

    holder->set_value(201);

    XCTAssertEqual(called.size(), 1);
    XCTAssertEqual(called.at(0), 201);
}

@end
