//
//  yas_to_tests.mm
//

#import <XCTest/XCTest.h>
#import <string>
#import "yas_chaining.h"

using namespace yas;

@interface yas_to_tests : XCTestCase

@end

@implementation yas_to_tests

- (void)setUp {
    [super setUp];
}

- (void)tearDown {
    [super tearDown];
}

- (void)test_to {
    chaining::notifier<int> notifier;

    int received = -1;

    auto chain = notifier.chain()
                     .to([](int const &value) { return value + 1; })
                     .perform([&received](int const &value) { received = value; })
                     .end();

    notifier.notify(10);

    XCTAssertEqual(received, 11);
}

- (void)test_to_type {
    chaining::notifier<int> notifier;

    std::string received = "";

    auto chain = notifier.chain()
                     .guard([](int const &) { return true; })
                     .to([](int const &value) { return value > 0; })
                     .guard([](bool const &) { return true; })
                     .to([](bool const &value) { return value ? "true" : "false"; })
                     .guard([](std::string const &) { return true; })
                     .perform([&received](std::string const &value) { received = value; })
                     .end();

    notifier.notify(0);

    XCTAssertEqual(received, "false");

    notifier.notify(1);

    XCTAssertEqual(received, "true");
}

- (void)test_to_value {
    chaining::notifier<int> notifier;

    std::string received = "";

    auto chain = notifier.chain()
                     .to_value(std::string("test_value"))
                     .perform([&received](std::string const &value) { received = value; })
                     .end();

    notifier.notify(0);

    XCTAssertEqual(received, "test_value");
}

- (void)test_to_null {
    chaining::notifier<int> notifier;

    bool called = false;

    auto chain = notifier.chain().to_null().perform([&called](std::nullptr_t const &) { called = true; }).end();

    notifier.notify(1);

    XCTAssertTrue(called);
}

- (void)test_to_tuple {
    chaining::notifier<int> notifier;

    std::optional<std::tuple<int>> called;

    auto chain = notifier.chain().to_tuple().perform([&called](std::tuple<int> const &value) { called = value; }).end();

    notifier.notify(1);

    XCTAssertTrue(called);
    XCTAssertEqual(std::get<0>(*called), 1);
}

- (void)test_to_tuple_from_tuple {
    chaining::notifier<std::tuple<int>> notifier;

    std::optional<std::tuple<int>> called;

    auto chain = notifier.chain().to_tuple().perform([&called](auto const &value) { called = value; }).end();

    notifier.notify(std::make_tuple(int(1)));

    XCTAssertTrue(called);
    XCTAssertEqual(std::get<0>(*called), 1);
}

- (void)test_to_tuple_from_pair {
    chaining::notifier<std::pair<int, std::string>> notifier;

    std::optional<std::tuple<int, std::string>> called;

    auto chain = notifier.chain().to_tuple().perform([&called](auto const &value) { called = value; }).end();

    notifier.notify(std::make_pair(int(1), std::string("2")));

    XCTAssertTrue(called);
    XCTAssertEqual(std::get<0>(*called), 1);
    XCTAssertEqual(std::get<1>(*called), "2");
}

@end
