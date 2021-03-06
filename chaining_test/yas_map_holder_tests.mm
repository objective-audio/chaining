//
//  yas_map_holder_tests.mm
//

#import <XCTest/XCTest.h>
#import <chaining/chaining.h>

using namespace yas;
using namespace yas::chaining;

@interface yas_map_holder_tests : XCTestCase

@end

@implementation yas_map_holder_tests

- (void)setUp {
}

- (void)tearDown {
}

- (void)test_make_shared {
    auto holder = map::holder<int, std::string>::make_shared();

    XCTAssertTrue(holder);
}

- (void)test_size {
    auto holder0 = map::holder<int, std::string>::make_shared();
    auto holder1 = map::holder<int, std::string>::make_shared({{1, "11"}});
    auto holder3 = map::holder<int, std::string>::make_shared({{0, "10"}, {1, "11"}, {2, "12"}});

    XCTAssertEqual(holder0->size(), 0);
    XCTAssertEqual(holder1->size(), 1);
    XCTAssertEqual(holder3->size(), 3);
}

- (void)test_has_value {
    auto holder = map::holder<int, std::string>::make_shared({{0, "10"}, {1, "11"}});

    XCTAssertTrue(holder->has_value(0));
    XCTAssertTrue(holder->has_value(1));

    XCTAssertFalse(holder->has_value(2));
}

- (void)test_at {
    auto holder = map::holder<int, std::string>::make_shared({{0, "10"}, {1, "11"}});

    XCTAssertEqual(holder->at(0), "10");
    XCTAssertEqual(holder->at(1), "11");
}

- (void)test_replace_all {
    auto holder = map::holder<int, std::string>::make_shared({{0, "10"}, {1, "11"}});

    XCTAssertEqual(holder->size(), 2);
    XCTAssertEqual(holder->value(), (std::map<int, std::string>{{0, "10"}, {1, "11"}}));

    holder->replace_all({{2, "12"}});

    XCTAssertEqual(holder->size(), 1);
    XCTAssertEqual(holder->value(), (std::map<int, std::string>{{2, "12"}}));
}

- (void)test_replace_by_insert_or_replace {
    auto holder = map::holder<int, std::string>::make_shared({{0, "10"}, {1, "11"}});

    holder->insert_or_replace(0, "100");

    XCTAssertEqual(holder->size(), 2);
    XCTAssertEqual(holder->value(), (std::map<int, std::string>{{0, "100"}, {1, "11"}}));
}

- (void)test_insert_by_insert_or_replace {
    auto holder = map::holder<int, std::string>::make_shared({{0, "10"}, {2, "12"}});

    XCTAssertEqual(holder->size(), 2);
    XCTAssertEqual(holder->value(), (std::map<int, std::string>{{0, "10"}, {2, "12"}}));

    holder->insert_or_replace(1, "11");

    XCTAssertEqual(holder->size(), 3);
    XCTAssertEqual(holder->value(), (std::map<int, std::string>{{0, "10"}, {1, "11"}, {2, "12"}}));
}

- (void)test_insert_map {
    auto holder = map::holder<int, std::string>::make_shared({{1, "11"}});

    XCTAssertEqual(holder->size(), 1);
    XCTAssertEqual(holder->value(), (std::map<int, std::string>{{1, "11"}}));

    holder->insert(std::map<int, std::string>{{{0, "10"}, {2, "12"}}});

    XCTAssertEqual(holder->size(), 3);
    XCTAssertEqual(holder->value(), (std::map<int, std::string>{{0, "10"}, {1, "11"}, {2, "12"}}));
}

- (void)test_erase_if {
    auto holder = map::holder<int, std::string>::make_shared({{0, "10"}, {1, "11"}, {2, "12"}});

    auto const erased = holder->erase_if([](int const &key, std::string const &) { return key == 1; });

    XCTAssertEqual(erased.size(), 1);
    XCTAssertEqual(erased.begin()->first, 1);
    XCTAssertEqual(erased.begin()->second, "11");
}

- (void)test_erase_for_value {
    auto holder = map::holder<int, std::string>::make_shared({{0, "10"}, {1, "11"}, {2, "12"}});

    auto const erased = holder->erase_for_value("11");

    XCTAssertEqual(erased.size(), 1);
    XCTAssertEqual(erased.begin()->first, 1);
    XCTAssertEqual(erased.begin()->second, "11");
}

- (void)test_erase_for_key {
    auto holder = map::holder<int, std::string>::make_shared({{0, "10"}, {1, "11"}, {2, "12"}});

    auto const erased = holder->erase_for_key(1);

    XCTAssertEqual(erased.size(), 1);
    XCTAssertEqual(erased.begin()->first, 1);
    XCTAssertEqual(erased.begin()->second, "11");
}

- (void)test_clear {
    auto holder = map::holder<int, std::string>::make_shared({{0, "10"}, {1, "11"}});

    XCTAssertEqual(holder->size(), 2);

    holder->clear();

    XCTAssertEqual(holder->size(), 0);
}

- (void)test_receiver {
    auto holder = map::holder<int, std::string>::make_shared({{0, "10"}, {1, "11"}});

    holder->receive_value(map::make_fetched_event(std::map<int, std::string>{{2, "12"}}));

    XCTAssertEqual(holder->size(), 1);
    XCTAssertEqual(holder->at(2), "12");

    holder->receive_value(map::make_any_event(std::map<int, std::string>{{3, "13"}, {4, "14"}}));

    XCTAssertEqual(holder->size(), 2);
    XCTAssertEqual(holder->at(3), "13");
    XCTAssertEqual(holder->at(4), "14");

    holder->receive_value(map::make_inserted_event(std::map<int, std::string>{{5, "15"}, {6, "16"}}));

    XCTAssertEqual(holder->size(), 4);
    XCTAssertEqual(holder->at(3), "13");
    XCTAssertEqual(holder->at(4), "14");
    XCTAssertEqual(holder->at(5), "15");
    XCTAssertEqual(holder->at(6), "16");

    holder->receive_value(map::make_erased_event(std::map<int, std::string>{{4, "14"}, {5, "15"}}));

    XCTAssertEqual(holder->size(), 2);
    XCTAssertEqual(holder->at(3), "13");
    XCTAssertEqual(holder->at(6), "16");

    holder->receive_value(map::make_replaced_event<int, std::string>(3, "23"));

    XCTAssertEqual(holder->size(), 2);
    XCTAssertEqual(holder->at(3), "23");
    XCTAssertEqual(holder->at(6), "16");
}

@end
