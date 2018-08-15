//
//  yas_observer_tests.mm
//

#import <XCTest/XCTest.h>
#import "yas_chaining_holder.h"
#import "yas_chaining_observer.h"

using namespace yas;
using namespace yas::chaining;

@interface yas_observer_tests : XCTestCase

@end

@implementation yas_observer_tests

- (void)setUp {
    [super setUp];
}

- (void)tearDown {
    [super tearDown];
}

- (void)test_invalidate {
    holder<int> holder{1};

    int received = 0;

    any_observer observer = holder.chain().perform([&received](int const &value) { received = value; }).sync();

    XCTAssertEqual(received, 1);

    observer.invalidate();

    holder.set_value(2);

    XCTAssertEqual(received, 1);
}

@end
