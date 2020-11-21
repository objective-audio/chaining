//
//  yas_chain_tests.mm
//

#import <XCTest/XCTest.h>
#import <chaining/chaining.h>
#import <string>

using namespace yas;

@interface yas_chaining_tests : XCTestCase

@end

@implementation yas_chaining_tests

- (void)setUp {
    [super setUp];
}

- (void)tearDown {
    [super tearDown];
}

- (void)test_guard {
    float received = -1.0f;

    auto notifier = chaining::notifier<int>::make_shared();

    auto chain = notifier->chain()
                     .to([](int const &value) { return value; })
                     .guard([](float const &value) { return value > 2.5f; })
                     .perform([&received](float const &value) { received = value; })
                     .end();

    notifier->notify(2);

    XCTAssertEqual(received, -1.0f);

    notifier->notify(3);

    XCTAssertEqual(received, 3.0f);
}

- (void)test_merge {
    std::string received;

    auto notifier = chaining::notifier<int>::make_shared();
    auto sub_notifier = chaining::notifier<float>::make_shared();

    auto sub_chain = sub_notifier->chain().to([](float const &value) { return std::to_string(int(value)); });

    auto chain = notifier->chain()
                     .to([](int const &value) { return std::to_string(value); })
                     .merge(std::move(sub_chain))
                     .perform([&received](std::string const &value) { received = value; })
                     .end();

    notifier->notify(10);

    XCTAssertEqual(received, "10");

    sub_notifier->notify(20.0f);

    XCTAssertEqual(received, "20");
}

- (void)test_pair {
    auto main_notifier = chaining::notifier<int>::make_shared();
    auto sub_notifier = chaining::notifier<std::string>::make_shared();

    using opt_pair_t = std::pair<std::optional<int>, std::optional<std::string>>;

    opt_pair_t received;

    auto sub_chain = sub_notifier->chain();
    auto main_chain = main_notifier->chain()
                          .pair(std::move(sub_chain))
                          .perform([&received](auto const &value) { received = value; })
                          .end();

    main_notifier->notify(1);

    XCTAssertEqual(*received.first, 1);
    XCTAssertFalse(!!received.second);

    sub_notifier->notify("test_text");

    XCTAssertFalse(!!received.first);
    XCTAssertEqual(*received.second, "test_text");
}

- (void)test_combine {
    auto main_notifier = chaining::notifier<int>::make_shared();
    auto sub_notifier = chaining::notifier<std::string>::make_shared();

    using opt_pair_t = std::optional<std::pair<int, std::string>>;

    opt_pair_t received;

    auto sub_chain = sub_notifier->chain();
    auto main_chain = main_notifier->chain()
                          .combine(std::move(sub_chain))
                          .perform([&received](auto const &value) { received = value; })
                          .end();

    main_notifier->notify(1);

    XCTAssertFalse(received);

    sub_notifier->notify("test_text");

    XCTAssertTrue(received);
    XCTAssertEqual(received->first, 1);
    XCTAssertEqual(received->second, "test_text");
}

- (void)test_combine_tuples {
    auto main_notifier = chaining::notifier<int>::make_shared();
    auto sub_notifier = chaining::notifier<std::string>::make_shared();
    auto sub_notifier2 = chaining::notifier<float>::make_shared();

    auto sub_chain = sub_notifier->chain().to_tuple();
    auto main_chain = main_notifier->chain().to_tuple();
    auto sub_chain2 = sub_notifier2->chain().to_tuple();

    std::optional<std::tuple<int, std::string, float>> received;

    auto chain = main_chain.combine(std::move(sub_chain))
                     .combine(std::move(sub_chain2))
                     .perform([&received](std::tuple<int, std::string, float> const &value) { received = value; })
                     .end();

    main_notifier->notify(33);
    sub_notifier->notify("44");
    sub_notifier2->notify(55.0f);

    XCTAssertTrue(received);
    XCTAssertEqual(std::get<0>(*received), 33);
    XCTAssertEqual(std::get<1>(*received), "44");
    XCTAssertEqual(std::get<2>(*received), 55.0f);
}

- (void)test_throws_end_not_called {
    auto notifier = chaining::notifier<int>::make_shared();
    auto chain = notifier->chain();

    XCTAssertThrows(notifier->notify(0));
}

@end
