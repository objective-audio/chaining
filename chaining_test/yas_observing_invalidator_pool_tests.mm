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

- (void)test_cancel {
    std::vector<int> called;

    auto const notifier = observing::notifier<int>::make_shared();

    canceller_pool pool;

    notifier->observe([&called](int const &value) { called.emplace_back(value); })->add_to(pool);

    XCTAssertEqual(called.size(), 0);

    notifier->notify(1);

    XCTAssertEqual(called.size(), 1);
    XCTAssertEqual(called.at(0), 1);

    pool.cancel();

    notifier->notify(2);

    XCTAssertEqual(called.size(), 1);
}

- (void)test_nest {
    std::vector<int> called;

    auto const notifier = observing::notifier<int>::make_shared();

    auto pool1 = canceller_pool::make_shared();

    notifier->observe([&called](int const &value) { called.emplace_back(value); })->add_to(*pool1);

    canceller_pool pool2;

    pool2.add_canceller(pool1);

    XCTAssertEqual(called.size(), 0);

    notifier->notify(3);

    XCTAssertEqual(called.size(), 1);
    XCTAssertEqual(called.at(0), 3);

    pool2.cancel();

    notifier->notify(4);

    XCTAssertEqual(called.size(), 1);
}

- (void)test_add_to {
    std::vector<int> called;

    auto const notifier = observing::notifier<int>::make_shared();

    auto pool1 = canceller_pool::make_shared();
    canceller_pool pool2;

    notifier->observe([&called](int const &value) { called.emplace_back(value); })->add_to(*pool1);

    pool1->add_to(pool2);

    XCTAssertEqual(called.size(), 0);

    notifier->notify(3);

    XCTAssertEqual(called.size(), 1);
    XCTAssertEqual(called.at(0), 3);

    pool2.cancel();

    notifier->notify(4);

    XCTAssertEqual(called.size(), 1);
}

- (void)test_set_to {
    std::vector<int> called;

    auto const notifier = observing::notifier<int>::make_shared();

    auto pool = canceller_pool::make_shared();
    cancellable_ptr canceller = nullptr;

    notifier->observe([&called](int const &value) { called.emplace_back(value); })->add_to(*pool);

    pool->set_to(canceller);

    XCTAssertEqual(called.size(), 0);

    notifier->notify(3);

    XCTAssertEqual(called.size(), 1);
    XCTAssertEqual(called.at(0), 3);

    canceller->cancel();

    notifier->notify(4);

    XCTAssertEqual(called.size(), 1);
}

- (void)test_has_cancellable {
    auto const notifier = observing::notifier<int>::make_shared();

    canceller_pool pool;

    XCTAssertFalse(pool.has_cancellable());

    auto canceller1 = notifier->observe([](int const &) {});

    pool.add_canceller(canceller1);

    XCTAssertTrue(pool.has_cancellable());

    auto canceller2 = notifier->observe([](int const &) {});

    pool.add_canceller(canceller2);

    XCTAssertTrue(pool.has_cancellable());

    canceller1->cancel();

    XCTAssertTrue(pool.has_cancellable());

    canceller2->cancel();

    XCTAssertFalse(pool.has_cancellable());

    notifier->observe([](int const &) {})->add_to(pool);

    XCTAssertTrue(pool.has_cancellable());

    pool.cancel();

    XCTAssertFalse(pool.has_cancellable());
}

@end
