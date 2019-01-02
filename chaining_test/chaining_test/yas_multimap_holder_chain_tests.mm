//
//  yas_multimap_holder_chain_tests.mm
//

#import <XCTest/XCTest.h>
#import <chaining/yas_chaining_holder.h>
#import <chaining/yas_chaining_multimap_holder.h>
#import <string>

using namespace yas;
using namespace yas::chaining;

@interface yas_multimap_holder_chain_tests : XCTestCase

@end

@implementation yas_multimap_holder_chain_tests

- (void)setUp {
}

- (void)tearDown {
}

- (void)test_chain_fetched_by_sync {
    multimap::holder<int, std::string> holder{{{0, "10"}, {1, "11"}, {2, "12"}}};

    std::vector<multimap::event<int, std::string>> received;

    auto chain = holder.chain().perform([&received](auto const &event) { received.push_back(event); }).sync();

    XCTAssertEqual(received.size(), 1);
    XCTAssertEqual(received.at(0).type, multimap::event_type::fetched);
    XCTAssertEqual((received.at(0).elements), (std::multimap<int, std::string>{{{0, "10"}, {1, "11"}, {2, "12"}}}));
}

- (void)test_chain_any_by_replace {
    multimap::holder<int, std::string> holder{{{0, "10"}, {1, "11"}, {2, "12"}}};

    std::vector<multimap::event<int, std::string>> received;

    auto chain = holder.chain().perform([&received](auto const &event) { received.push_back(event); }).end();

    holder.replace({{3, "13"}, {4, "14"}});

    XCTAssertEqual(received.size(), 1);
    XCTAssertEqual(received.at(0).type, multimap::event_type::any);
    XCTAssertEqual((received.at(0).elements), (std::multimap<int, std::string>{{3, "13"}, {4, "14"}}));
}

- (void)test_chain_any_by_clear {
    multimap::holder<int, std::string> holder{{{0, "10"}, {1, "11"}, {2, "12"}}};

    std::vector<multimap::event<int, std::string>> received;

    auto chain = holder.chain().perform([&received](auto const &event) { received.push_back(event); }).end();

    holder.clear();

    XCTAssertEqual(received.size(), 1);
    XCTAssertEqual(received.at(0).type, multimap::event_type::any);
    XCTAssertEqual((received.at(0).elements.size()), 0);
}

- (void)test_chain_inserted {
    multimap::holder<int, std::string> holder{{{0, "10"}, {1, "11"}, {2, "12"}}};

    std::vector<multimap::event<int, std::string>> received_events;
    std::vector<std::multimap<int, std::string>> received_elements;

    auto chain = holder.chain()
                     .perform([&received_events, &received_elements](auto const &event) {
                         received_events.push_back(event);
                         if (event.type == multimap::event_type::inserted) {
                             received_elements.push_back(event.elements);
                         }
                     })
                     .end();

    holder.insert({{3, "13"}});

    XCTAssertEqual(received_events.size(), 1);
    XCTAssertEqual(received_events.at(0).type, multimap::event_type::inserted);
    XCTAssertEqual(received_elements.size(), 1);
    XCTAssertEqual(received_elements.at(0), (std::multimap<int, std::string>{{3, "13"}}));
}

- (void)test_chain_erased {
    multimap::holder<int, std::string> holder{{{0, "10"}, {1, "11"}, {2, "12"}}};

    std::vector<multimap::event<int, std::string>> received_events;
    std::vector<std::multimap<int, std::string>> received_elements;

    auto chain = holder.chain()
                     .perform([&received_events, &received_elements](auto const &event) {
                         received_events.push_back(event);
                         if (event.type == multimap::event_type::erased) {
                             received_elements.push_back(event.elements);
                         }
                     })
                     .end();

    holder.erase_for_key(1);

    XCTAssertEqual(received_events.size(), 1);
    XCTAssertEqual(received_events.at(0).type, multimap::event_type::erased);
    XCTAssertEqual(received_elements.size(), 1);
    XCTAssertEqual(received_elements.at(0), (std::multimap<int, std::string>{{1, "11"}}));
}

- (void)test_chain_relayed {
    holder<std::string> holder1{"1"};
    holder<std::string> holder2{"2"};
    multimap::holder<int, chaining::holder<std::string>> map_holder{{{1, holder1}, {2, holder2}}};

    std::vector<multimap::event<int, chaining::holder<std::string>>> received_events;
    std::vector<std::multimap<int, chaining::holder<std::string>>> received_elements;

    auto chain = map_holder.chain()
                     .perform([&received_events, &received_elements](auto const &event) {
                         received_events.push_back(event);
                         if (event.type == multimap::event_type::relayed) {
                             received_elements.push_back(event.elements);
                         }
                     })
                     .end();

    holder1.set_value("3");

    XCTAssertEqual(received_events.size(), 1);
    XCTAssertEqual(received_events.at(0).type, multimap::event_type::relayed);
    XCTAssertEqual(received_elements.size(), 1);
    XCTAssertEqual(received_elements.at(0),
                   (std::multimap<int, chaining::holder<std::string>>{{1, chaining::holder<std::string>{"3"}}}));

    holder2.set_value("4");

    XCTAssertEqual(received_events.size(), 2);
    XCTAssertEqual(received_events.at(1).type, multimap::event_type::relayed);
    XCTAssertEqual(received_elements.size(), 2);
    XCTAssertEqual(received_elements.at(1),
                   (std::multimap<int, chaining::holder<std::string>>{{2, chaining::holder<std::string>{"4"}}}));
}

- (void)test_chain_relayed_after_inserted {
    multimap::holder<int, chaining::holder<std::string>> map_holder;

    holder<std::string> holder1{"1"};
    map_holder.insert(1, holder1);

    holder<std::string> holder2{"2"};
    map_holder.insert(2, holder2);

    std::vector<multimap::event<int, chaining::holder<std::string>>> received_events;
    std::vector<std::multimap<int, chaining::holder<std::string>>> received_elements;

    auto chain = map_holder.chain()
                     .perform([&received_events, &received_elements](auto const &event) {
                         received_events.push_back(event);
                         if (event.type == multimap::event_type::relayed) {
                             received_elements.push_back(event.elements);
                         }
                     })
                     .end();

    holder1.set_value("3");

    XCTAssertEqual(received_events.size(), 1);
    XCTAssertEqual(received_events.at(0).type, multimap::event_type::relayed);
    XCTAssertEqual(received_elements.size(), 1);
    XCTAssertEqual(received_elements.at(0),
                   (std::multimap<int, chaining::holder<std::string>>{{1, chaining::holder<std::string>{"3"}}}));

    holder2.set_value("4");

    XCTAssertEqual(received_events.size(), 2);
    XCTAssertEqual(received_events.at(1).type, multimap::event_type::relayed);
    XCTAssertEqual(received_elements.size(), 2);
    XCTAssertEqual(received_elements.at(1),
                   (std::multimap<int, chaining::holder<std::string>>{{2, chaining::holder<std::string>{"4"}}}));
}

- (void)test_chain_relayed_after_replaced {
    holder<int> holder1{1};
    holder<int> holder2{2};
    multimap::holder<int, chaining::holder<int>> map_holder{{{1, holder1}, {2, holder2}}};

    holder<int> holder3{3};
    holder<int> holder4{4};
    map_holder.replace({{3, holder3}, {4, holder4}});

    std::vector<multimap::event<int, chaining::holder<int>>> received_events;
    std::vector<std::multimap<int, chaining::holder<int>>> received_elements;

    auto chain = map_holder.chain()
                     .perform([&received_events, &received_elements](auto const &event) {
                         received_events.push_back(event);
                         if (event.type == multimap::event_type::relayed) {
                             received_elements.push_back(event.elements);
                         }
                     })
                     .end();

    holder1.set_value(10);
    holder2.set_value(20);

    XCTAssertEqual(received_events.size(), 0);

    holder3.set_value(30);

    XCTAssertEqual(received_events.size(), 1);
    XCTAssertEqual(received_events.at(0).type, multimap::event_type::relayed);
    XCTAssertEqual(received_elements.size(), 1);
    XCTAssertEqual(received_elements.at(0),
                   (std::multimap<int, chaining::holder<int>>{{3, chaining::holder<int>{30}}}));

    holder4.set_value(40);

    XCTAssertEqual(received_events.size(), 2);
    XCTAssertEqual(received_events.at(1).type, multimap::event_type::relayed);
    XCTAssertEqual(received_elements.size(), 2);
    XCTAssertEqual(received_elements.at(1),
                   (std::multimap<int, chaining::holder<int>>{{4, chaining::holder<int>{40}}}));
}

- (void)test_chain_not_relayed_after_erased {
    holder<int> holder1{1};
    holder<int> holder2{2};
    multimap::holder<int, chaining::holder<int>> map_holder{{{1, holder1}, {2, holder2}}};

    map_holder.erase_for_key(1);

    std::vector<multimap::event<int, chaining::holder<int>>> received_events;
    std::vector<std::multimap<int, chaining::holder<int>>> received_elements;

    auto chain = map_holder.chain()
                     .perform([&received_events, &received_elements](auto const &event) {
                         received_events.push_back(event);
                         if (event.type == multimap::event_type::relayed) {
                             received_elements.push_back(event.elements);
                         }
                     })
                     .end();

    holder1.set_value(3);

    XCTAssertEqual(received_events.size(), 0);

    holder2.set_value(4);

    XCTAssertEqual(received_events.size(), 1);
}

- (void)test_chain_not_relayed_after_clear {
    holder<int> holder1{1};
    holder<int> holder2{2};
    multimap::holder<int, chaining::holder<int>> map_holder{{{1, holder1}, {2, holder2}}};

    map_holder.clear();

    std::vector<multimap::event<int, chaining::holder<int>>> received_events;

    auto chain =
        map_holder.chain().perform([&received_events](auto const &event) { received_events.push_back(event); }).end();

    holder1.set_value(3);
    holder2.set_value(4);

    XCTAssertEqual(received_events.size(), 0);
}

- (void)test_chain_not_relayed_after_replaced {
    holder<int> holder1{1};
    holder<int> holder2{2};
    multimap::holder<int, chaining::holder<int>> map_holder{{{1, holder1}, {2, holder2}}};

    holder<int> holder3{3};
    map_holder.replace({{3, holder3}});

    std::vector<multimap::event<int, chaining::holder<int>>> received_events;

    auto chain =
        map_holder.chain().perform([&received_events](auto const &event) { received_events.push_back(event); }).end();

    holder1.set_value(10);
    holder2.set_value(20);

    XCTAssertEqual(received_events.size(), 0);

    holder3.set_value(30);

    XCTAssertEqual(received_events.size(), 1);
}

@end