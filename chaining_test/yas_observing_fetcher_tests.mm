//
//  yas_observing_fetcher_tests.mm
//

#import <XCTest/XCTest.h>
#import <chaining/chaining.h>

using namespace yas;
using namespace yas::observing;

@interface yas_observing_fetcher_tests : XCTestCase

@end

@implementation yas_observing_fetcher_tests

- (void)test_fetched_value {
    std::optional<int> value = std::nullopt;

    auto const fetcher = observing::fetcher<int>::make_shared([&value] { return value; });

    XCTAssertFalse(fetcher->fetched_value().has_value());

    value = 1;

    auto const fetched = fetcher->fetched_value();

    XCTAssertTrue(fetched.has_value());
    XCTAssertEqual(fetched.value(), 1);
}

- (void)test_observe {
    std::vector<int> called1;
    std::vector<int> called2;

    std::optional<int> value = std::nullopt;

    auto const fetcher = observing::fetcher<int>::make_shared([&value] { return value; });

    auto canceller1 = fetcher->observe([&called1](int const &value) { called1.emplace_back(value); });

    XCTAssertEqual(called1.size(), 0);

    value = 1;

    auto canceller2 = fetcher->observe([&called2](int const &value) { called2.emplace_back(value); });

    XCTAssertEqual(called1.size(), 0);
    XCTAssertEqual(called2.size(), 1);
    XCTAssertEqual(called2.at(0), 1);

    fetcher->push();

    XCTAssertEqual(called1.size(), 1);
    XCTAssertEqual(called1.at(0), 1);
    XCTAssertEqual(called2.size(), 2);
    XCTAssertEqual(called2.at(1), 1);

    fetcher->push(2);

    XCTAssertEqual(called1.size(), 2);
    XCTAssertEqual(called1.at(1), 2);
    XCTAssertEqual(called2.size(), 3);
    XCTAssertEqual(called2.at(2), 2);
}

@end
