//
//  yas_handler_tests.mm
//

#import <XCTest/XCTest.h>
#import <chaining/chaining.h>

using namespace yas;

@interface yas_handler_tests : XCTestCase

@end

@implementation yas_handler_tests

- (void)test_handler {
    using namespace chaining;

    bool is_called = false;

    joint_handler_f<int> raw_handler = [&is_called](int const &, any_joint &) { is_called = true; };

    auto const handler = any_handler::make_shared(std::move(raw_handler));

    std::shared_ptr<sender<int> const> const sender = notifier<int>::make_shared();
    any_joint_ptr any_joint = chaining::make_joint(to_weak(sender));
    joint_handler_f<int> const &calling_handler = handler->get<int>();

    calling_handler(10, *any_joint);

    XCTAssertTrue(is_called);
}

@end
