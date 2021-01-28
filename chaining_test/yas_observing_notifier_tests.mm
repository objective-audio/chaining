//
//  yas_observing_notifier_tests.mm
//

#import <XCTest/XCTest.h>
#import <chaining/chaining.h>

using namespace yas;
using namespace yas::observing;

@interface yas_observing_notifier_tests : XCTestCase

@end

@implementation yas_observing_notifier_tests

- (void)test_notify {
    auto const notifier = observing::notifier<int>::make_shared();

    std::vector<int> called1;
    std::vector<int> called2;

    auto const canceller1 = notifier->observe([&called1](int const &value) { called1.emplace_back(value); });
    auto const canceller2 = notifier->observe([&called2](int const &value) { called2.emplace_back(value); });

    XCTAssertEqual(called1.size(), 0);
    XCTAssertEqual(called2.size(), 0);

    notifier->notify(1);

    XCTAssertEqual(called1.size(), 1);
    XCTAssertEqual(called1.at(0), 1);
    XCTAssertEqual(called2.size(), 1);
    XCTAssertEqual(called2.at(0), 1);

    canceller1->cancel();

    notifier->notify(2);

    XCTAssertEqual(called1.size(), 1);
    XCTAssertEqual(called2.size(), 2);
    XCTAssertEqual(called2.at(1), 2);
}

@end
