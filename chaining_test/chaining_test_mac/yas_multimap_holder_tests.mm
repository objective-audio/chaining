//
//  yas_multimap_holder_tests.mm
//

#import <XCTest/XCTest.h>
#import "yas_chaining_multimap_holder.h"

using namespace yas;
using namespace yas::chaining;

@interface yas_multimap_holder_tests : XCTestCase

@end

@implementation yas_multimap_holder_tests

- (void)setUp {
}

- (void)tearDown {
}

- (void)test_get_raw {
    multimap::holder<int, std::string> holder{{{0, "10"}, {1, "11"}, {2, "12"}}};

    XCTAssertEqual(holder.raw(), (std::multimap<int, std::string>{{0, "10"}, {1, "11"}, {2, "12"}}));
}

- (void)test_size {
    multimap::holder<int, std::string> holder0;
    multimap::holder<int, std::string> holder1{{{1, "11"}}};
    multimap::holder<int, std::string> holder3{{{0, "10"}, {1, "11"}, {2, "12"}}};

    XCTAssertEqual(holder0.size(), 0);
    XCTAssertEqual(holder1.size(), 1);
    XCTAssertEqual(holder3.size(), 3);
}

- (void)test_replace {
    multimap::holder<int, std::string> holder{{{0, "10"}, {1, "11"}}};

    XCTAssertEqual(holder.size(), 2);
    XCTAssertEqual(holder.raw(), (std::multimap<int, std::string>{{0, "10"}, {1, "11"}}));

    holder.replace({{2, "12"}});

    XCTAssertEqual(holder.size(), 1);
    XCTAssertEqual(holder.raw(), (std::multimap<int, std::string>{{2, "12"}}));
}

- (void)test_insert_multimap {
    multimap::holder<int, std::string> holder{{{1, "11"}}};

    XCTAssertEqual(holder.size(), 1);
    XCTAssertEqual(holder.raw(), (std::multimap<int, std::string>{{1, "11"}}));

    holder.insert(std::multimap<int, std::string>{{{0, "10"}, {2, "12"}}});

    XCTAssertEqual(holder.size(), 3);
    XCTAssertEqual(holder.raw(), (std::multimap<int, std::string>{{0, "10"}, {1, "11"}, {2, "12"}}));
}

- (void)test_insert_single {
    multimap::holder<int, std::string> holder{{{0, "10"}, {2, "12"}}};

    XCTAssertEqual(holder.size(), 2);
    XCTAssertEqual(holder.raw(), (std::multimap<int, std::string>{{0, "10"}, {2, "12"}}));

    holder.insert(1, "11");

    XCTAssertEqual(holder.size(), 3);
    XCTAssertEqual(holder.raw(), (std::multimap<int, std::string>{{0, "10"}, {1, "11"}, {2, "12"}}));
}

- (void)test_erase_if {
    multimap::holder<int, std::string> holder{{{0, "10"}, {1, "11"}, {2, "12"}}};

    auto const erased = holder.erase_if([](int const &key, std::string const &) { return key == 1; });

    XCTAssertEqual(erased.size(), 1);
    XCTAssertEqual(erased.begin()->first, 1);
    XCTAssertEqual(erased.begin()->second, "11");
}

- (void)test_erase_for_value {
    multimap::holder<int, std::string> holder{{{0, "10"}, {1, "11"}, {2, "12"}}};

    auto const erased = holder.erase_for_value("11");

    XCTAssertEqual(erased.size(), 1);
    XCTAssertEqual(erased.begin()->first, 1);
    XCTAssertEqual(erased.begin()->second, "11");
}

- (void)test_erase_for_key {
    multimap::holder<int, std::string> holder{{{0, "10"}, {1, "11"}, {2, "12"}}};

    auto const erased = holder.erase_for_key(1);

    XCTAssertEqual(erased.size(), 1);
    XCTAssertEqual(erased.begin()->first, 1);
    XCTAssertEqual(erased.begin()->second, "11");
}

- (void)test_clear {
    multimap::holder<int, std::string> holder{{{0, "10"}, {1, "11"}}};

    XCTAssertEqual(holder.size(), 2);

    holder.clear();

    XCTAssertEqual(holder.size(), 0);
}

- (void)test_make_fetched_event {
    std::multimap<int, std::string> elements{{0, "10"}, {1, "11"}};
    multimap::event<int, std::string> event = multimap::make_fetched_event(elements);

    XCTAssertEqual(event.type(), multimap::event_type::fetched);
    XCTAssertEqual((event.get<multimap::fetched_event<int, std::string>>().elements), elements);
}

- (void)test_make_any_event {
    std::multimap<int, std::string> elements{{0, "10"}, {1, "11"}};
    multimap::event<int, std::string> event = multimap::make_any_event(elements);

    XCTAssertEqual(event.type(), multimap::event_type::any);
    XCTAssertEqual((event.get<multimap::any_event<int, std::string>>().elements), elements);
}

- (void)test_make_inserted_event {
    std::multimap<int, std::string> elements{{0, "10"}, {1, "11"}};
    multimap::event<int, std::string> event = multimap::make_inserted_event(elements);

    XCTAssertEqual(event.type(), multimap::event_type::inserted);
    XCTAssertEqual((event.get<multimap::inserted_event<int, std::string>>().elements), elements);
}

- (void)test_make_erased_event {
    std::multimap<int, std::string> elements{{0, "10"}, {1, "11"}};
    multimap::event<int, std::string> event = multimap::make_erased_event(elements);

    XCTAssertEqual(event.type(), multimap::event_type::erased);
    XCTAssertEqual((event.get<multimap::erased_event<int, std::string>>().elements), elements);
}

- (void)test_make_relayed_event {
    int const key = 1;
    std::string const value = "11";
    multimap::event<int, std::string> event = multimap::make_relayed_event(value, key);

    XCTAssertEqual(event.type(), multimap::event_type::relayed);
    XCTAssertEqual((event.get<multimap::relayed_event<int, std::string>>().key), 1);
    XCTAssertEqual((event.get<multimap::relayed_event<int, std::string>>().value), "11");
}

@end
