//
//  yas_flow_fetcher_tests.mm
//

#import <XCTest/XCTest.h>
#import <chaining/yas_chaining_fetcher.h>
#import <chaining/yas_chaining_notifier.h>

using namespace yas;

@interface yas_flow_fetcher_tests : XCTestCase

@end

@implementation yas_flow_fetcher_tests

- (void)setUp {
    [super setUp];
}

- (void)tearDown {
    [super tearDown];
}

- (void)test_make_shared {
    auto fetcher = chaining::fetcher<int>::make_shared([]() { return 1; });

    XCTAssertTrue(fetcher);
}

- (void)test_fetched_value {
    int sending = 1;

    auto fetcher = chaining::fetcher<int>::make_shared([&sending] { return sending; });

    XCTAssertEqual(fetcher->fetched_value(), 1);
}

- (void)test_push {
    int sending = 1;

    auto fetcher = chaining::fetcher<int>::make_shared([&sending] { return sending; });

    int notified = -1;

    auto flow = fetcher->chain().perform([&notified](int const &value) { notified = value; }).end();

    XCTAssertEqual(notified, -1);

    fetcher->push();

    XCTAssertEqual(notified, 1);
}

- (void)test_push_with_value {
    int sending = 1;

    auto fetcher = chaining::fetcher<int>::make_shared([&sending] { return sending; });

    int notified = -1;

    auto flow = fetcher->chain().perform([&notified](int const &value) { notified = value; }).end();

    XCTAssertEqual(notified, -1);

    fetcher->push(2);

    XCTAssertEqual(notified, 2);
}

- (void)test_sync {
    int sending = 1;

    auto fetcher = chaining::fetcher<int>::make_shared([&sending] { return sending; });

    int notified = -1;

    auto flow = fetcher->chain().perform([&notified](int const &value) { notified = value; }).sync();

    XCTAssertEqual(notified, 1);

    sending = 2;

    fetcher->push();

    XCTAssertEqual(notified, 2);
}

- (void)test_receive {
    auto notifier = chaining::notifier<std::nullptr_t>::make_shared();

    int sending = 1;

    auto fetcher = chaining::fetcher<int>::make_shared([&sending] { return sending; });

    int notified = -1;

    auto flow = fetcher->chain().perform([&notified](int const &value) { notified = value; }).sync();

    XCTAssertEqual(notified, 1);

    sending = 2;

    auto receive_flow = notifier->chain().send_to(fetcher).end();

    notifier->notify(nullptr);

    XCTAssertEqual(notified, 2);
}

@end
