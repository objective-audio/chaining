//
//  yas_observing_map_holder_tests.mm
//

#import <XCTest/XCTest.h>
#import <chaining/chaining.h>

using namespace yas;
using namespace yas::observing;

@interface yas_observing_map_holder_tests : XCTestCase

@end

@implementation yas_observing_map_holder_tests

- (void)test_make {
    auto const holder = map::holder<int, std::string>::make_shared();

    XCTAssertEqual(holder->elements().size(), 0);
}

- (void)test_make_by_copy {
    std::map<int, std::string> const map{{1, "1"}, {2, "2"}};
    auto const holder = map::holder<int, std::string>::make_shared(map);

    XCTAssertEqual(holder->elements().size(), 2);
    XCTAssertEqual(holder->elements(), map);
}

- (void)test_make_by_move {
    std::map<int, std::string> map{{1, "1"}, {2, "2"}};
    auto const holder = map::holder<int, std::string>::make_shared(std::move(map));

    XCTAssertEqual(holder->elements(), (std::map<int, std::string>{{1, "1"}, {2, "2"}}));
}

- (void)test_at {
    auto const holder = map::holder<int, std::string>::make_shared({{1, "1"}, {2, "2"}});

    XCTAssertEqual(holder->at(1), "1");
    XCTAssertEqual(holder->at(2), "2");
}

- (void)test_contains {
    std::map<int, std::string> const map{{1, "1"}};
    auto const holder = map::holder<int, std::string>::make_shared(map);

    XCTAssertFalse(holder->contains(0));
    XCTAssertTrue(holder->contains(1));
    XCTAssertFalse(holder->contains(2));
}

- (void)test_size {
    auto const holder = map::holder<int, std::string>::make_shared();

    XCTAssertEqual(holder->size(), 0);

    holder->replace({{1, "1"}});

    XCTAssertEqual(holder->size(), 1);

    holder->replace({{2, "2"}, {3, "3"}});

    XCTAssertEqual(holder->size(), 2);
}

- (void)test_replace_by_copy {
    auto const holder = map::holder<int, std::string>::make_shared({{1, "1"}});

    std::map<int, std::string> const map{{2, "2"}, {3, "3"}};
    holder->replace(map);

    XCTAssertEqual(holder->elements().size(), 2);
    XCTAssertEqual(holder->elements(), map);
}

- (void)test_replace_by_move {
    auto const holder = map::holder<int, std::string>::make_shared({{1, "1"}});

    std::map<int, std::string> map{{2, "2"}, {3, "3"}};
    holder->replace(std::move(map));

    XCTAssertEqual(holder->elements(), (std::map<int, std::string>{{2, "2"}, {3, "3"}}));
}

- (void)test_insert_or_replace {
    auto const holder = map::holder<int, std::string>::make_shared({{1, "1"}});

    holder->insert_or_replace(2, "2");

    XCTAssertEqual(holder->elements(), (std::map<int, std::string>{{1, "1"}, {2, "2"}}));

    holder->insert_or_replace(1, "a");

    XCTAssertEqual(holder->elements(), (std::map<int, std::string>{{1, "a"}, {2, "2"}}));
}

- (void)test_erase {
    auto const holder = map::holder<int, std::string>::make_shared({{1, "1"}, {2, "a"}, {3, "a"}});

    auto const erased = holder->erase(2);

    XCTAssertEqual(holder->elements(), (std::map<int, std::string>{{1, "1"}, {3, "a"}}));
    XCTAssertEqual(erased, (std::map<int, std::string>{{2, "a"}}));
}

- (void)test_clear {
    auto const holder = map::holder<int, std::string>::make_shared({{1, "1"}, {2, "a"}});

    holder->clear();

    XCTAssertEqual(holder->size(), 0);
}

- (void)test_observe_with_synced {
    auto const holder = map::holder<int, std::string>::make_shared({{1, "1"}, {2, "2"}});

    struct called_event {
        map::event_type type;
        std::optional<int> key;
        std::optional<std::string> element;
    };

    std::vector<called_event> called;

    auto canceller = holder->observe(
        [&called](auto const &event) {
            std::optional<int> key = event.key ? std::optional<int>{*event.key} : std::nullopt;
            std::optional<std::string> element =
                event.element ? std::optional<std::string>{*event.element} : std::nullopt;
            called.emplace_back(called_event{.type = event.type, .key = std::move(key), .element = std::move(element)});
        },
        true);

    XCTAssertEqual(called.size(), 1);
    XCTAssertEqual(called.at(0).type, map::event_type::any);
    XCTAssertEqual(called.at(0).key, std::nullopt);
    XCTAssertEqual(called.at(0).element, std::nullopt);

    holder->replace({{3, "3"}, {4, "4"}});

    XCTAssertEqual(called.size(), 2);
    XCTAssertEqual(called.at(1).type, map::event_type::any);
    XCTAssertEqual(called.at(1).key, std::nullopt);
    XCTAssertEqual(called.at(1).element, std::nullopt);

    holder->insert_or_replace(4, "a");

    XCTAssertEqual(called.size(), 3);
    XCTAssertEqual(called.at(2).type, map::event_type::replaced);
    XCTAssertEqual(called.at(2).key, 4);
    XCTAssertEqual(called.at(2).element, "a");

    holder->insert_or_replace(5, "5");

    XCTAssertEqual(called.size(), 4);
    XCTAssertEqual(called.at(3).type, map::event_type::inserted);
    XCTAssertEqual(called.at(3).key, 5);
    XCTAssertEqual(called.at(3).element, "5");

    holder->erase(3);

    XCTAssertEqual(called.size(), 5);
    XCTAssertEqual(called.at(4).type, map::event_type::erased);
    XCTAssertEqual(called.at(4).key, 3);
    XCTAssertEqual(called.at(4).element, "3");

    holder->clear();

    XCTAssertEqual(called.size(), 6);
    XCTAssertEqual(called.at(5).type, map::event_type::any);
    XCTAssertEqual(called.at(5).key, std::nullopt);
    XCTAssertEqual(called.at(5).element, std::nullopt);

    holder->erase(1);
    holder->clear();

    XCTAssertEqual(called.size(), 6);

    canceller->cancel();
}

@end
