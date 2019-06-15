//
//  yas_map_holder_tests.mm
//

#import <XCTest/XCTest.h>
#import <chaining/yas_chaining_map_holder.h>

using namespace yas;
using namespace yas::chaining;

@interface yas_map_holder_tests : XCTestCase

@end

@implementation yas_map_holder_tests

- (void)setUp {
}

- (void)tearDown {
}

- (void)test_size {
    map::holder<int, std::string> holder0;
    map::holder<int, std::string> holder1{{{1, "11"}}};
    map::holder<int, std::string> holder3{{{0, "10"}, {1, "11"}, {2, "12"}}};

    XCTAssertEqual(holder0.size(), 0);
    XCTAssertEqual(holder1.size(), 1);
    XCTAssertEqual(holder3.size(), 3);
}

- (void)test_has_value {
    map::holder<int, std::string> holder{{{0, "10"}, {1, "11"}}};

    XCTAssertTrue(holder.has_value(0));
    XCTAssertTrue(holder.has_value(1));

    XCTAssertFalse(holder.has_value(2));
}

- (void)test_at {
    map::holder<int, std::string> holder{{{0, "10"}, {1, "11"}}};

    XCTAssertEqual(holder.at(0), "10");
    XCTAssertEqual(holder.at(1), "11");
}

- (void)test_replace_all {
    map::holder<int, std::string> holder{{{0, "10"}, {1, "11"}}};

    XCTAssertEqual(holder.size(), 2);
    XCTAssertEqual(holder.raw(), (std::map<int, std::string>{{0, "10"}, {1, "11"}}));

    holder.replace_all({{2, "12"}});

    XCTAssertEqual(holder.size(), 1);
    XCTAssertEqual(holder.raw(), (std::map<int, std::string>{{2, "12"}}));
}

- (void)test_replace_by_insert_or_replace {
    map::holder<int, std::string> holder{{{0, "10"}, {1, "11"}}};

    holder.insert_or_replace(0, "100");

    XCTAssertEqual(holder.size(), 2);
    XCTAssertEqual(holder.raw(), (std::map<int, std::string>{{0, "100"}, {1, "11"}}));
}

- (void)test_insert_by_insert_or_replace {
    map::holder<int, std::string> holder{{{0, "10"}, {2, "12"}}};

    XCTAssertEqual(holder.size(), 2);
    XCTAssertEqual(holder.raw(), (std::map<int, std::string>{{0, "10"}, {2, "12"}}));

    holder.insert_or_replace(1, "11");

    XCTAssertEqual(holder.size(), 3);
    XCTAssertEqual(holder.raw(), (std::map<int, std::string>{{0, "10"}, {1, "11"}, {2, "12"}}));
}

- (void)test_insert_map {
    map::holder<int, std::string> holder{{{1, "11"}}};

    XCTAssertEqual(holder.size(), 1);
    XCTAssertEqual(holder.raw(), (std::map<int, std::string>{{1, "11"}}));

    holder.insert(std::map<int, std::string>{{{0, "10"}, {2, "12"}}});

    XCTAssertEqual(holder.size(), 3);
    XCTAssertEqual(holder.raw(), (std::map<int, std::string>{{0, "10"}, {1, "11"}, {2, "12"}}));
}

- (void)test_erase_if {
    map::holder<int, std::string> holder{{{0, "10"}, {1, "11"}, {2, "12"}}};

    auto const erased = holder.erase_if([](int const &key, std::string const &) { return key == 1; });

    XCTAssertEqual(erased.size(), 1);
    XCTAssertEqual(erased.begin()->first, 1);
    XCTAssertEqual(erased.begin()->second, "11");
}

- (void)test_erase_for_value {
    map::holder<int, std::string> holder{{{0, "10"}, {1, "11"}, {2, "12"}}};

    auto const erased = holder.erase_for_value("11");

    XCTAssertEqual(erased.size(), 1);
    XCTAssertEqual(erased.begin()->first, 1);
    XCTAssertEqual(erased.begin()->second, "11");
}

- (void)test_erase_for_key {
    map::holder<int, std::string> holder{{{0, "10"}, {1, "11"}, {2, "12"}}};

    auto const erased = holder.erase_for_key(1);

    XCTAssertEqual(erased.size(), 1);
    XCTAssertEqual(erased.begin()->first, 1);
    XCTAssertEqual(erased.begin()->second, "11");
}

- (void)test_clear {
    map::holder<int, std::string> holder{{{0, "10"}, {1, "11"}}};

    XCTAssertEqual(holder.size(), 2);

    holder.clear();

    XCTAssertEqual(holder.size(), 0);
}

- (void)test_receiver {
    map::holder<int, std::string> holder{{{0, "10"}, {1, "11"}}};

    holder.receivable().receive_value(map::make_fetched_event(std::map<int, std::string>{{2, "12"}}));

    XCTAssertEqual(holder.size(), 1);
    XCTAssertEqual(holder.at(2), "12");

    holder.receivable().receive_value(map::make_any_event(std::map<int, std::string>{{3, "13"}, {4, "14"}}));

    XCTAssertEqual(holder.size(), 2);
    XCTAssertEqual(holder.at(3), "13");
    XCTAssertEqual(holder.at(4), "14");

    holder.receivable().receive_value(map::make_inserted_event(std::map<int, std::string>{{5, "15"}, {6, "16"}}));

    XCTAssertEqual(holder.size(), 4);
    XCTAssertEqual(holder.at(3), "13");
    XCTAssertEqual(holder.at(4), "14");
    XCTAssertEqual(holder.at(5), "15");
    XCTAssertEqual(holder.at(6), "16");

    holder.receivable().receive_value(map::make_erased_event(std::map<int, std::string>{{4, "14"}, {5, "15"}}));

    XCTAssertEqual(holder.size(), 2);
    XCTAssertEqual(holder.at(3), "13");
    XCTAssertEqual(holder.at(6), "16");

    holder.receivable().receive_value(map::make_replaced_event<int, std::string>(3, "23"));

    XCTAssertEqual(holder.size(), 2);
    XCTAssertEqual(holder.at(3), "23");
    XCTAssertEqual(holder.at(6), "16");
}

@end
