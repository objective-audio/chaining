//
//  yas_flow_fetcher_tests.mm
//

#import <XCTest/XCTest.h>
#import "yas_chaining.h"

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

- (void)test_sync {
    int sending = 1;

    chaining::fetcher<int> fetcher{[&sending] { return sending; }};

    int notified = -1;

    auto flow = fetcher.chain().perform([&notified](int const &value) { notified = value; }).sync();

    XCTAssertEqual(notified, 1);

    sending = 2;

    fetcher.fetch();

    XCTAssertEqual(notified, 2);
}

- (void)test_receive {
    chaining::notifier<std::nullptr_t> notifier;

    int sending = 1;

    chaining::fetcher<int> fetcher{[&sending] { return sending; }};

    int notified = -1;

    auto flow = fetcher.chain().perform([&notified](int const &value) { notified = value; }).sync();

    XCTAssertEqual(notified, 1);

    sending = 2;

    auto receive_flow = notifier.chain().receive(fetcher.receiver()).end();

    notifier.notify(nullptr);

    XCTAssertEqual(notified, 2);
}

@end
