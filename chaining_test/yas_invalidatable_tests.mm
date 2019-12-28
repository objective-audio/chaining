//
//  yas_chaining_invalidatable_tests.mm
//

#import <XCTest/XCTest.h>
#import <chaining/yas_chaining_invalidatable.h>
#import <chaining/yas_chaining_value_holder.h>

using namespace yas;
using namespace yas::chaining;

@interface yas_invalidatable_tests : XCTestCase

@end

@implementation yas_invalidatable_tests

- (void)setUp {
}

- (void)tearDown {
}

- (void)test_invalidate {
    auto holder = value::holder<int>::make_shared(0);

    std::vector<int> received;

    invalidatable_ptr observer =
        holder->chain().perform([&received](int const &value) { received.push_back(value); }).sync();

    XCTAssertEqual(received.size(), 1);
    XCTAssertEqual(received.at(0), 0);

    holder->set_value(1);

    XCTAssertEqual(received.size(), 2);
    XCTAssertEqual(received.at(1), 1);

    observer->invalidate();

    holder->set_value(2);

    XCTAssertEqual(received.size(), 2);
}

@end
