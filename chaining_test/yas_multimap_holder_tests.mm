//
//  yas_multimap_holder_tests.mm
//

#import <XCTest/XCTest.h>
#import <chaining/chaining.h>

using namespace yas;
using namespace yas::chaining;

@interface yas_multimap_holder_tests : XCTestCase

@end

@implementation yas_multimap_holder_tests

- (void)setUp {
}

- (void)tearDown {
}

- (void)test_make_shared {
    auto holder = multimap::holder<int, std::string>::make_shared();

    XCTAssertTrue(holder);
}

- (void)test_get_raw {
    auto holder = multimap::holder<int, std::string>::make_shared({{0, "10"}, {1, "11"}, {2, "12"}});

    XCTAssertEqual(holder->raw(), (std::multimap<int, std::string>{{0, "10"}, {1, "11"}, {2, "12"}}));
}

- (void)test_size {
    auto holder0 = multimap::holder<int, std::string>::make_shared();
    auto holder1 = multimap::holder<int, std::string>::make_shared({{1, "11"}});
    auto holder3 = multimap::holder<int, std::string>::make_shared({{0, "10"}, {1, "11"}, {2, "12"}});

    XCTAssertEqual(holder0->size(), 0);
    XCTAssertEqual(holder1->size(), 1);
    XCTAssertEqual(holder3->size(), 3);
}

- (void)test_replace {
    auto holder = multimap::holder<int, std::string>::make_shared({{0, "10"}, {1, "11"}});

    XCTAssertEqual(holder->size(), 2);
    XCTAssertEqual(holder->raw(), (std::multimap<int, std::string>{{0, "10"}, {1, "11"}}));

    holder->replace({{2, "12"}});

    XCTAssertEqual(holder->size(), 1);
    XCTAssertEqual(holder->raw(), (std::multimap<int, std::string>{{2, "12"}}));
}

- (void)test_insert_multimap {
    auto holder = multimap::holder<int, std::string>::make_shared({{1, "11"}});

    XCTAssertEqual(holder->size(), 1);
    XCTAssertEqual(holder->raw(), (std::multimap<int, std::string>{{1, "11"}}));

    holder->insert(std::multimap<int, std::string>{{{0, "10"}, {2, "12"}}});

    XCTAssertEqual(holder->size(), 3);
    XCTAssertEqual(holder->raw(), (std::multimap<int, std::string>{{0, "10"}, {1, "11"}, {2, "12"}}));
}

- (void)test_insert_single {
    auto holder = multimap::holder<int, std::string>::make_shared({{0, "10"}, {2, "12"}});

    XCTAssertEqual(holder->size(), 2);
    XCTAssertEqual(holder->raw(), (std::multimap<int, std::string>{{0, "10"}, {2, "12"}}));

    holder->insert(1, "11");

    XCTAssertEqual(holder->size(), 3);
    XCTAssertEqual(holder->raw(), (std::multimap<int, std::string>{{0, "10"}, {1, "11"}, {2, "12"}}));
}

- (void)test_erase_if {
    auto holder = multimap::holder<int, std::string>::make_shared({{0, "10"}, {1, "11"}, {2, "12"}});

    auto const erased = holder->erase_if([](int const &key, std::string const &) { return key == 1; });

    XCTAssertEqual(erased.size(), 1);
    XCTAssertEqual(erased.begin()->first, 1);
    XCTAssertEqual(erased.begin()->second, "11");
}

- (void)test_erase_for_value {
    auto holder = multimap::holder<int, std::string>::make_shared({{0, "10"}, {1, "11"}, {2, "12"}});

    auto const erased = holder->erase_for_value("11");

    XCTAssertEqual(erased.size(), 1);
    XCTAssertEqual(erased.begin()->first, 1);
    XCTAssertEqual(erased.begin()->second, "11");
}

- (void)test_erase_for_key {
    auto holder = multimap::holder<int, std::string>::make_shared({{0, "10"}, {1, "11"}, {2, "12"}});

    auto const erased = holder->erase_for_key(1);

    XCTAssertEqual(erased.size(), 1);
    XCTAssertEqual(erased.begin()->first, 1);
    XCTAssertEqual(erased.begin()->second, "11");
}

- (void)test_clear {
    auto holder = multimap::holder<int, std::string>::make_shared({{0, "10"}, {1, "11"}});

    XCTAssertEqual(holder->size(), 2);

    holder->clear();

    XCTAssertEqual(holder->size(), 0);
}

@end
