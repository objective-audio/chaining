//
//  yas_caller_tests.mm
//

#import <XCTest/XCTest.h>
#import <chaining/chaining.h>

using namespace yas;
using namespace yas::observing;

@interface yas_observing_caller_tests : XCTestCase

@end

@implementation yas_observing_caller_tests

- (void)test_call {
    observing::caller<int> caller;

    std::vector<int> called1;
    std::vector<int> called2;

    auto canceller1 = caller.add([&called1](int const &value) { called1.emplace_back(value); });
    auto canceller2 = caller.add([&called2](int const &value) { called2.emplace_back(value); });

    XCTAssertEqual(called1.size(), 0);
    XCTAssertEqual(called2.size(), 0);

    caller.call(1);

    XCTAssertEqual(called1.size(), 1);
    XCTAssertEqual(called1.at(0), 1);
    XCTAssertEqual(called2.size(), 1);
    XCTAssertEqual(called2.at(0), 1);

    canceller1->invalidate();

    caller.call(2);

    XCTAssertEqual(called1.size(), 1);
    XCTAssertEqual(called2.size(), 2);
    XCTAssertEqual(called2.at(1), 2);
}

- (void)test_ignore {
    observing::caller<int> caller;

    std::vector<int> called;

    auto canceller = caller.add([&called](int const &value) { called.emplace_back(value); });

    XCTAssertEqual(called.size(), 0);

    caller.call(3);

    XCTAssertEqual(called.size(), 1);
    XCTAssertEqual(called.at(0), 3);

    canceller->ignore();

    caller.call(4);

    XCTAssertEqual(called.size(), 2);
    XCTAssertEqual(called.at(1), 4);
}

- (void)test_destruct_caller {
    std::vector<int> called;

    canceller_ptr canceller = nullptr;

    {
        observing::caller<int> caller;

        canceller = caller.add([&called](int const &value) { called.emplace_back(value); });

        caller.call(5);

        XCTAssertEqual(called.size(), 1);
        XCTAssertEqual(called.at(0), 5);
    }

    XCTAssertEqual(called.size(), 1);
}

- (void)test_destruct_canceller {
    observing::caller<int> caller;

    std::vector<int> called1;

    {
        auto canceller1 = caller.add([&called1](int const &value) { called1.emplace_back(value); });

        XCTAssertEqual(called1.size(), 0);

        caller.call(6);

        XCTAssertEqual(called1.size(), 1);
        XCTAssertEqual(called1.at(0), 6);
    }

    caller.call(7);

    XCTAssertEqual(called1.size(), 1);
}

@end
