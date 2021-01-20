//
//  yas_observing_canceller_pool_tests.mm
//

#import <XCTest/XCTest.h>
#import <chaining/chaining.h>

using namespace yas;
using namespace yas::observing;

@interface yas_observing_canceller_pool_tests : XCTestCase

@end

@implementation yas_observing_canceller_pool_tests

- (void)test_destructor {
    std::vector<int> called;

    auto const notifier = observing::notifier<int>::make_shared();

    {
        canceller_pool pool;

        notifier->observe([&called](int const &value) { called.emplace_back(value); })->add_to(pool);

        XCTAssertEqual(called.size(), 0);

        notifier->notify(1);

        XCTAssertEqual(called.size(), 1);
        XCTAssertEqual(called.at(0), 1);
    }

    notifier->notify(2);

    XCTAssertEqual(called.size(), 1);
}

- (void)test_invalidate {
    std::vector<int> called;

    auto const notifier = observing::notifier<int>::make_shared();

    canceller_pool pool;

    notifier->observe([&called](int const &value) { called.emplace_back(value); })->add_to(pool);

    XCTAssertEqual(called.size(), 0);

    notifier->notify(1);

    XCTAssertEqual(called.size(), 1);
    XCTAssertEqual(called.at(0), 1);

    pool.invalidate();

    notifier->notify(2);

    XCTAssertEqual(called.size(), 1);
}

@end
