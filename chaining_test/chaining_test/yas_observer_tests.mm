//
//  yas_observer_tests.mm
//

#import <XCTest/XCTest.h>
#import <chaining/yas_chaining_holder.h>
#import <chaining/yas_chaining_observer.h>

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

- (void)test_invalidate_with_sub_joints {
    holder<int> main_holder{1};
    holder<int> sub_holder{2};

    int received = 0;

    any_observer observer = main_holder.chain()
                                .merge(sub_holder.chain())
                                .perform([&received](int const &value) { received = value; })
                                .sync();

    XCTAssertEqual(received, 2);

    observer.invalidate();

    main_holder.set_value(3);

    XCTAssertEqual(received, 2);

    sub_holder.set_value(4);

    XCTAssertEqual(received, 2);
}

- (void)test_create_any_observer_with_null {
    any_observer observer{nullptr};

    XCTAssertFalse(observer);
}

@end
