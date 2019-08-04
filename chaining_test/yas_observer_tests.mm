//
//  yas_observer_tests.mm
//

#import <XCTest/XCTest.h>
#import <chaining/yas_chaining_observer.h>
#import <chaining/yas_chaining_value_holder.h>

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
    auto holder = value::holder<int>::make_shared(1);

    int received = 0;

    any_observer_ptr observer = holder->chain().perform([&received](int const &value) { received = value; }).sync();

    XCTAssertEqual(received, 1);

    observer->invalidate();

    holder->set_value(2);

    XCTAssertEqual(received, 1);
}

- (void)test_invalidate_with_sub_joints {
    auto main_holder = value::holder<int>::make_shared(1);
    auto sub_holder = value::holder<int>::make_shared(2);

    int received = 0;

    any_observer_ptr observer = main_holder->chain()
                                    .merge(sub_holder->chain())
                                    .perform([&received](int const &value) { received = value; })
                                    .sync();

    XCTAssertEqual(received, 2);

    observer->invalidate();

    main_holder->set_value(3);

    XCTAssertEqual(received, 2);

    sub_holder->set_value(4);

    XCTAssertEqual(received, 2);
}

@end
