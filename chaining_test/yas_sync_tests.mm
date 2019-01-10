//
//  yas_sync_tests.mm
//

#import <XCTest/XCTest.h>
#import <chaining/yas_chaining_umbrella.h>

using namespace yas;

@interface yas_sync_tests : XCTestCase

@end

@implementation yas_sync_tests

- (void)setUp {
    [super setUp];
}

- (void)tearDown {
    [super tearDown];
}

- (void)test_sync {
    chaining::fetcher<int> fetcher{[] { return 100; }};

    int received = -1;

    auto chain = fetcher.chain().perform([&received](int const &value) { received = value; }).end();
    chain.broadcast();

    XCTAssertEqual(received, 100);
}

- (void)test_sync_by_observer {
    chaining::fetcher<int> fetcher{[] { return 100; }};

    int received = -1;

    chaining::any_observer chain = fetcher.chain().perform([&received](int const &value) { received = value; }).end();
    chain.broadcast();

    XCTAssertEqual(received, 100);
}

- (void)test_sync_many_chain {
    chaining::fetcher<int> fetcher{[] { return 100; }};

    int received1 = -1;
    int received2 = -1;

    auto chain1 = fetcher.chain().perform([&received1](int const &value) { received1 = value; }).end();
    auto chain2 = fetcher.chain().perform([&received2](int const &value) { received2 = value; }).end();

    chain1.broadcast();

    XCTAssertEqual(received1, 100);
    XCTAssertEqual(received2, -1);
}

- (void)test_sync_end {
    chaining::fetcher<int> fetcher{[] { return 100; }};

    int received = -1;

    auto chain = fetcher.chain().perform([&received](int const &value) { received = value; }).sync();

    XCTAssertEqual(received, 100);
}

- (void)test_sync_with_combined_sub_sender {
    chaining::fetcher<int> fetcher{[]() { return 123; }};
    chaining::fetcher<int> sub_fetcher{[]() { return 456; }};

    std::vector<std::pair<int, int>> received;

    auto chain = fetcher.chain()
                     .combine(sub_fetcher.chain())
                     .perform([&received](auto const &pair) { received.emplace_back(pair); })
                     .sync();

    // 2つのsenderから来た値が両方揃ってから受け取った
    XCTAssertEqual(received.size(), 1);
    XCTAssertEqual(received.at(0).first, 123);
    XCTAssertEqual(received.at(0).second, 456);
}

- (void)test_sync_with_merged_sub_sender {
    chaining::fetcher<int> fetcher{[]() { return 78; }};
    chaining::fetcher<int> sub_fetcher{[]() { return 90; }};

    std::vector<int> received;

    auto chain = fetcher.chain()
                     .merge(sub_fetcher.chain())
                     .perform([&received](int const &value) { received.emplace_back(value); })
                     .sync();

    // main -> sub の順番で実行される
    XCTAssertEqual(received.size(), 2);
    XCTAssertEqual(received.at(0), 78);
    XCTAssertEqual(received.at(1), 90);
}

@end
