//
//  yas_multimap_holder_chain_tests.mm
//

#import <XCTest/XCTest.h>
#import <chaining/yas_chaining_alias.h>
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

    std::vector<event> received;

    auto chain = holder.chain().perform([&received](auto const &event) { received.push_back(event); }).sync();

    XCTAssertEqual(received.size(), 1);
    XCTAssertEqual(received.at(0).type(), event_type::fetched);
    XCTAssertEqual((received.at(0).get<multimap::fetched_event<int, std::string>>().elements),
                   (std::multimap<int, std::string>{{{0, "10"}, {1, "11"}, {2, "12"}}}));
}

- (void)test_chain_any_by_replace {
    multimap::holder<int, std::string> holder{{{0, "10"}, {1, "11"}, {2, "12"}}};

    std::vector<event> received;

    auto chain = holder.chain().perform([&received](auto const &event) { received.push_back(event); }).end();

    holder.replace({{3, "13"}, {4, "14"}});

    XCTAssertEqual(received.size(), 1);
    XCTAssertEqual(received.at(0).type(), event_type::any);
    XCTAssertEqual((received.at(0).get<multimap::any_event<int, std::string>>().elements),
                   (std::multimap<int, std::string>{{3, "13"}, {4, "14"}}));
}

- (void)test_chain_any_by_clear {
    multimap::holder<int, std::string> holder{{{0, "10"}, {1, "11"}, {2, "12"}}};

    std::vector<event> received;

    auto chain = holder.chain().perform([&received](auto const &event) { received.push_back(event); }).end();

    holder.clear();

    XCTAssertEqual(received.size(), 1);
    XCTAssertEqual(received.at(0).type(), event_type::any);
    XCTAssertEqual((received.at(0).get<multimap::any_event<int, std::string>>().elements.size()), 0);
}

- (void)test_chain_inserted {
    multimap::holder<int, std::string> holder{{{0, "10"}, {1, "11"}, {2, "12"}}};

    std::vector<event> received_events;
    std::vector<std::multimap<int, std::string>> received_elements;

    auto chain = holder.chain()
                     .perform([&received_events, &received_elements](auto const &event) {
                         received_events.push_back(event);
                         if (event.type() == event_type::inserted) {
                             received_elements.push_back(
                                 event.template get<multimap::inserted_event<int, std::string>>().elements);
                         }
                     })
                     .end();

    holder.insert({{3, "13"}});

    XCTAssertEqual(received_events.size(), 1);
    XCTAssertEqual(received_events.at(0).type(), event_type::inserted);
    XCTAssertEqual(received_elements.size(), 1);
    XCTAssertEqual(received_elements.at(0), (std::multimap<int, std::string>{{3, "13"}}));
}

- (void)test_chain_erased {
    multimap::holder<int, std::string> holder{{{0, "10"}, {1, "11"}, {2, "12"}}};

    std::vector<event> received_events;
    std::vector<std::multimap<int, std::string>> received_elements;

    auto chain = holder.chain()
                     .perform([&received_events, &received_elements](auto const &event) {
                         received_events.push_back(event);
                         if (event.type() == event_type::erased) {
                             received_elements.push_back(
                                 event.template get<multimap::erased_event<int, std::string>>().elements);
                         }
                     })
                     .end();

    holder.erase_for_key(1);

    XCTAssertEqual(received_events.size(), 1);
    XCTAssertEqual(received_events.at(0).type(), event_type::erased);
    XCTAssertEqual(received_elements.size(), 1);
    XCTAssertEqual(received_elements.at(0), (std::multimap<int, std::string>{{1, "11"}}));
}

- (void)test_chain_relayed {
    holder<std::string> holder1{"1"};
    holder<std::string> holder2{"2"};
    multimap::holder<int, chaining::holder<std::string>> map_holder{{{1, holder1}, {2, holder2}}};

    std::vector<event> received_events;
    std::vector<std::tuple<int, holder<std::string>, std::string>> received_relayed_events;

    auto chain =
        map_holder.chain()
            .perform([&received_events, &received_relayed_events](auto const &event) {
                received_events.push_back(event);
                if (event.type() == event_type::relayed) {
                    auto const &relayed = event.template get<multimap::relayed_event<int, holder<std::string>>>();
                    received_relayed_events.push_back(std::make_tuple(relayed.key, relayed.value, relayed.relayed));
                }
            })
            .end();

    holder1.set_value("3");

    XCTAssertEqual(received_events.size(), 1);
    XCTAssertEqual(received_events.at(0).type(), event_type::relayed);
    XCTAssertEqual(received_relayed_events.size(), 1);
    XCTAssertEqual(std::get<0>(received_relayed_events.at(0)), 1);
    XCTAssertEqual(std::get<1>(received_relayed_events.at(0)), (holder<std::string>{"3"}));
    XCTAssertEqual(std::get<2>(received_relayed_events.at(0)), "3");

    holder2.set_value("4");

    XCTAssertEqual(received_events.size(), 2);
    XCTAssertEqual(received_events.at(1).type(), event_type::relayed);
    XCTAssertEqual(received_relayed_events.size(), 2);
    XCTAssertEqual(std::get<0>(received_relayed_events.at(1)), 2);
    XCTAssertEqual(std::get<1>(received_relayed_events.at(1)), (holder<std::string>{"4"}));
    XCTAssertEqual(std::get<2>(received_relayed_events.at(1)), "4");
}

- (void)test_chain_relayed_after_inserted {
    multimap::holder<int, chaining::holder<std::string>> map_holder;

    holder<std::string> holder1{"1"};
    map_holder.insert(1, holder1);

    holder<std::string> holder2{"2"};
    map_holder.insert(2, holder2);

    std::vector<event> received_events;
    std::vector<std::tuple<int, holder<std::string>, std::string>> received_relayed_events;

    auto chain =
        map_holder.chain()
            .perform([&received_events, &received_relayed_events](auto const &event) {
                received_events.push_back(event);
                if (event.type() == event_type::relayed) {
                    auto const &relayed = event.template get<multimap::relayed_event<int, holder<std::string>>>();
                    received_relayed_events.push_back(std::make_tuple(relayed.key, relayed.value, relayed.relayed));
                }
            })
            .end();

    holder1.set_value("3");

    XCTAssertEqual(received_events.size(), 1);
    XCTAssertEqual(received_events.at(0).type(), event_type::relayed);
    XCTAssertEqual(std::get<0>(received_relayed_events.at(0)), 1);
    XCTAssertEqual(std::get<1>(received_relayed_events.at(0)), (holder<std::string>{"3"}));
    XCTAssertEqual(std::get<2>(received_relayed_events.at(0)), "3");

    holder2.set_value("4");

    XCTAssertEqual(received_events.size(), 2);
    XCTAssertEqual(received_events.at(1).type(), event_type::relayed);
    XCTAssertEqual(received_relayed_events.size(), 2);
    XCTAssertEqual(std::get<0>(received_relayed_events.at(1)), 2);
    XCTAssertEqual(std::get<1>(received_relayed_events.at(1)), (holder<std::string>{"4"}));
    XCTAssertEqual(std::get<2>(received_relayed_events.at(1)), "4");
}

- (void)test_chain_relayed_after_replaced {
    holder<int> holder1{1};
    holder<int> holder2{2};
    multimap::holder<int, chaining::holder<int>> map_holder{{{1, holder1}, {2, holder2}}};

    holder<int> holder3{3};
    holder<int> holder4{4};
    map_holder.replace({{3, holder3}, {4, holder4}});

    std::vector<event> received_events;
    std::vector<std::tuple<int, holder<int>, int>> received_relayed_events;

    auto chain =
        map_holder.chain()
            .perform([&received_events, &received_relayed_events](auto const &event) {
                received_events.push_back(event);
                if (event.type() == event_type::relayed) {
                    auto const &relayed = event.template get<multimap::relayed_event<int, holder<int>>>();
                    received_relayed_events.push_back(std::make_tuple(relayed.key, relayed.value, relayed.relayed));
                }
            })
            .end();

    holder1.set_value(10);
    holder2.set_value(20);

    XCTAssertEqual(received_events.size(), 0);

    holder3.set_value(30);

    XCTAssertEqual(received_events.size(), 1);
    XCTAssertEqual(received_events.at(0).type(), event_type::relayed);
    XCTAssertEqual(received_relayed_events.size(), 1);
    XCTAssertEqual(std::get<0>(received_relayed_events.at(0)), 3);
    XCTAssertEqual(std::get<1>(received_relayed_events.at(0)), (holder<int>{30}));
    XCTAssertEqual(std::get<2>(received_relayed_events.at(0)), 30);

    holder4.set_value(40);

    XCTAssertEqual(received_events.size(), 2);
    XCTAssertEqual(received_events.at(1).type(), event_type::relayed);
    XCTAssertEqual(received_relayed_events.size(), 2);
    XCTAssertEqual(std::get<0>(received_relayed_events.at(1)), 4);
    XCTAssertEqual(std::get<1>(received_relayed_events.at(1)), (holder<int>{40}));
    XCTAssertEqual(std::get<2>(received_relayed_events.at(1)), 40);
}

- (void)test_chain_not_relayed_after_erased {
    holder<int> holder1{1};
    holder<int> holder2{2};
    multimap::holder<int, chaining::holder<int>> map_holder{{{1, holder1}, {2, holder2}}};

    map_holder.erase_for_key(1);

    std::vector<event> received_events;
    std::vector<std::tuple<int, holder<int>, int>> received_relayed_events;

    auto chain =
        map_holder.chain()
            .perform([&received_events, &received_relayed_events](auto const &event) {
                received_events.push_back(event);
                if (event.type() == event_type::relayed) {
                    auto const &relayed = event.template get<multimap::relayed_event<int, holder<int>>>();
                    received_relayed_events.push_back(std::make_tuple(relayed.key, relayed.value, relayed.relayed));
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

    std::vector<event> received_events;

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

    std::vector<event> received_events;

    auto chain =
        map_holder.chain().perform([&received_events](auto const &event) { received_events.push_back(event); }).end();

    holder1.set_value(10);
    holder2.set_value(20);

    XCTAssertEqual(received_events.size(), 0);

    holder3.set_value(30);

    XCTAssertEqual(received_events.size(), 1);
}

- (void)test_alias {
    multimap::holder<int, std::string> holder{{{0, "10"}, {1, "11"}, {2, "12"}}};
    auto alias = make_alias(holder);

    XCTAssertEqual(alias.raw(), (std::multimap<int, std::string>{{{0, "10"}, {1, "11"}, {2, "12"}}}));

    std::vector<chaining::event> received;

    auto observer = alias.chain().perform([&received](auto const &event) { received.push_back(event); }).sync();

    XCTAssertEqual(received.size(), 1);
    XCTAssertEqual(received.at(0).type(), event_type::fetched);

    holder.insert({{3, "13"}});

    XCTAssertEqual(received.size(), 2);
    XCTAssertEqual(received.at(1).type(), event_type::inserted);
}

@end
