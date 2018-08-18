//
//  yas_observer_pool_tests.mm
//

#import <XCTest/XCTest.h>
#import "yas_chaining_holder.h"
#import "yas_chaining_observer_pool.h"

using namespace yas;
using namespace yas::chaining;

@interface yas_chaining_observer_pool_tests : XCTestCase

@end

@implementation yas_chaining_observer_pool_tests

- (void)setUp {
    [super setUp];
}

- (void)tearDown {
    [super tearDown];
}

- (void)test_add_observer {
    observer_pool pool;

    holder<int> holder{1};

    int received = 0;

    pool.add_observer(holder.chain().perform([&received](int const &value) { received = value; }).sync());

    XCTAssertEqual(received, 1);
}

- (void)test_remove_observer {
}

- (void)test_invalidate {
}

@end
