//
//  yas_observer_pool_tests.mm
//

#import <XCTest/XCTest.h>
#import <chaining/chaining.h>

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
    auto holder = value::holder<int>::make_shared(1);
    int received = 0;

    {
        observer_pool pool;

        pool.add_observer(holder->chain().perform([&received](int const &value) { received = value; }).sync());

        XCTAssertEqual(received, 1);

        holder->set_value(2);

        XCTAssertEqual(received, 2);
    }

    holder->set_value(3);

    XCTAssertEqual(received, 2);
}

- (void)test_remove_observer {
    auto holder = value::holder<int>::make_shared(1);
    int received = 0;

    observer_pool pool;

    any_observer_ptr observer = holder->chain().perform([&received](int const &value) { received = value; }).sync();

    pool.add_observer(observer);

    XCTAssertEqual(received, 1);

    pool.remove_observer(observer);

    holder->set_value(2);

    XCTAssertEqual(received, 1);
}

- (void)test_invalidate {
    auto holder = value::holder<int>::make_shared(1);
    int received = 0;

    observer_pool pool;

    pool.add_observer(holder->chain().perform([&received](int const &value) { received = value; }).sync());

    XCTAssertEqual(received, 1);

    pool.invalidate();

    holder->set_value(2);

    XCTAssertEqual(received, 1);
}

- (void)test_add_observer_by_plus_equal {
    auto holder = value::holder<int>::make_shared(1);
    int received = 0;

    {
        observer_pool pool;

        pool += holder->chain().perform([&received](int const &value) { received = value; }).sync();

        XCTAssertEqual(received, 1);

        holder->set_value(2);

        XCTAssertEqual(received, 2);
    }

    holder->set_value(3);

    XCTAssertEqual(received, 2);
}

- (void)test_make_shared {
    auto holder = value::holder<int>::make_shared(1);
    int received = 0;

    auto pool = observer_pool::make_shared();

    pool->add_observer(holder->chain().perform([&received](int const &value) { received = value; }).sync());

    XCTAssertEqual(received, 1);

    holder->set_value(2);

    XCTAssertEqual(received, 2);

    pool.reset();

    holder->set_value(3);

    XCTAssertEqual(received, 2);
}

@end
