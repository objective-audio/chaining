//
//  yas_multimap_holder_chain_tests.mm
//

#import <XCTest/XCTest.h>
#import <chaining/yas_chaining_multimap_holder.h>
#import <chaining/yas_chaining_value_holder.h>
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
    auto holder = multimap::holder<int, std::string>::make_shared({{0, "10"}, {1, "11"}, {2, "12"}});

    std::vector<event> received;

    auto chain = holder->chain().perform([&received](auto const &event) { received.push_back(event); }).sync();

    XCTAssertEqual(received.size(), 1);
    XCTAssertEqual(received.at(0).type(), event_type::fetched);
    XCTAssertEqual((received.at(0).get<multimap::fetched_event<int, std::string>>().elements),
                   (std::multimap<int, std::string>{{{0, "10"}, {1, "11"}, {2, "12"}}}));
}

- (void)test_chain_any_by_replace {
    auto holder = multimap::holder<int, std::string>::make_shared({{0, "10"}, {1, "11"}, {2, "12"}});

    std::vector<event> received;

    auto chain = holder->chain().perform([&received](auto const &event) { received.push_back(event); }).end();

    holder->replace({{3, "13"}, {4, "14"}});

    XCTAssertEqual(received.size(), 1);
    XCTAssertEqual(received.at(0).type(), event_type::any);
    XCTAssertEqual((received.at(0).get<multimap::any_event<int, std::string>>().elements),
                   (std::multimap<int, std::string>{{3, "13"}, {4, "14"}}));
}

- (void)test_chain_any_by_clear {
    auto holder = multimap::holder<int, std::string>::make_shared({{0, "10"}, {1, "11"}, {2, "12"}});

    std::vector<event> received;

    auto chain = holder->chain().perform([&received](auto const &event) { received.push_back(event); }).end();

    holder->clear();

    XCTAssertEqual(received.size(), 1);
    XCTAssertEqual(received.at(0).type(), event_type::any);
    XCTAssertEqual((received.at(0).get<multimap::any_event<int, std::string>>().elements.size()), 0);
}

- (void)test_chain_inserted {
    auto holder = multimap::holder<int, std::string>::make_shared({{0, "10"}, {1, "11"}, {2, "12"}});

    std::vector<event> received_events;
    std::vector<std::multimap<int, std::string>> received_elements;

    auto chain = holder->chain()
                     .perform([&received_events, &received_elements](auto const &event) {
                         received_events.push_back(event);
                         if (event.type() == event_type::inserted) {
                             received_elements.push_back(
                                 event.template get<multimap::inserted_event<int, std::string>>().elements);
                         }
                     })
                     .end();

    holder->insert({{3, "13"}});

    XCTAssertEqual(received_events.size(), 1);
    XCTAssertEqual(received_events.at(0).type(), event_type::inserted);
    XCTAssertEqual(received_elements.size(), 1);
    XCTAssertEqual(received_elements.at(0), (std::multimap<int, std::string>{{3, "13"}}));
}

- (void)test_chain_erased {
    auto holder = multimap::holder<int, std::string>::make_shared({{0, "10"}, {1, "11"}, {2, "12"}});

    std::vector<event> received_events;
    std::vector<std::multimap<int, std::string>> received_elements;

    auto chain = holder->chain()
                     .perform([&received_events, &received_elements](auto const &event) {
                         received_events.push_back(event);
                         if (event.type() == event_type::erased) {
                             received_elements.push_back(
                                 event.template get<multimap::erased_event<int, std::string>>().elements);
                         }
                     })
                     .end();

    holder->erase_for_key(1);

    XCTAssertEqual(received_events.size(), 1);
    XCTAssertEqual(received_events.at(0).type(), event_type::erased);
    XCTAssertEqual(received_elements.size(), 1);
    XCTAssertEqual(received_elements.at(0), (std::multimap<int, std::string>{{1, "11"}}));
}

- (void)test_chain_relayed {
    auto holder1 = value::holder<std::string>::make_shared("1");
    auto holder2 = value::holder<std::string>::make_shared("2");
    auto map_holder = multimap::holder<int, value::holder<std::string>>::make_shared({{1, *holder1}, {2, *holder2}});

    std::vector<event> received_events;
    std::vector<std::tuple<int, value::holder<std::string>, std::string>> received_relayed_events;

    auto chain =
        map_holder->chain()
            .perform([&received_events, &received_relayed_events](auto const &event) {
                received_events.push_back(event);
                if (event.type() == event_type::relayed) {
                    auto const &relayed =
                        event.template get<multimap::relayed_event<int, value::holder<std::string>>>();
                    received_relayed_events.push_back(std::make_tuple(relayed.key, relayed.value, relayed.relayed));
                }
            })
            .end();

    holder1->set_value("3");

    auto holder3 = value::holder<std::string>::make_shared("3");
    XCTAssertEqual(received_events.size(), 1);
    XCTAssertEqual(received_events.at(0).type(), event_type::relayed);
    XCTAssertEqual(received_relayed_events.size(), 1);
    XCTAssertEqual(std::get<0>(received_relayed_events.at(0)), 1);
    XCTAssertEqual(std::get<1>(received_relayed_events.at(0)), *holder3);
    XCTAssertEqual(std::get<2>(received_relayed_events.at(0)), "3");

    holder2->set_value("4");

    auto holder4 = value::holder<std::string>::make_shared("4");
    XCTAssertEqual(received_events.size(), 2);
    XCTAssertEqual(received_events.at(1).type(), event_type::relayed);
    XCTAssertEqual(received_relayed_events.size(), 2);
    XCTAssertEqual(std::get<0>(received_relayed_events.at(1)), 2);
    XCTAssertEqual(std::get<1>(received_relayed_events.at(1)), *holder4);
    XCTAssertEqual(std::get<2>(received_relayed_events.at(1)), "4");
}

- (void)test_chain_relayed_after_inserted {
    auto map_holder = multimap::holder<int, value::holder<std::string>>::make_shared();

    auto holder1 = value::holder<std::string>::make_shared("1");
    map_holder->insert(1, *holder1);

    auto holder2 = value::holder<std::string>::make_shared("2");
    map_holder->insert(2, *holder2);

    std::vector<event> received_events;
    std::vector<std::tuple<int, value::holder<std::string>, std::string>> received_relayed_events;

    auto chain =
        map_holder->chain()
            .perform([&received_events, &received_relayed_events](auto const &event) {
                received_events.push_back(event);
                if (event.type() == event_type::relayed) {
                    auto const &relayed =
                        event.template get<multimap::relayed_event<int, value::holder<std::string>>>();
                    received_relayed_events.push_back(std::make_tuple(relayed.key, relayed.value, relayed.relayed));
                }
            })
            .end();

    holder1->set_value("3");

    auto holder3 = value::holder<std::string>::make_shared("3");
    XCTAssertEqual(received_events.size(), 1);
    XCTAssertEqual(received_events.at(0).type(), event_type::relayed);
    XCTAssertEqual(std::get<0>(received_relayed_events.at(0)), 1);
    XCTAssertEqual(std::get<1>(received_relayed_events.at(0)), *holder3);
    XCTAssertEqual(std::get<2>(received_relayed_events.at(0)), "3");

    holder2->set_value("4");

    auto holder4 = value::holder<std::string>::make_shared("4");
    XCTAssertEqual(received_events.size(), 2);
    XCTAssertEqual(received_events.at(1).type(), event_type::relayed);
    XCTAssertEqual(received_relayed_events.size(), 2);
    XCTAssertEqual(std::get<0>(received_relayed_events.at(1)), 2);
    XCTAssertEqual(std::get<1>(received_relayed_events.at(1)), *holder4);
    XCTAssertEqual(std::get<2>(received_relayed_events.at(1)), "4");
}

- (void)test_chain_relayed_after_replaced {
    auto holder1 = value::holder<int>::make_shared(1);
    auto holder2 = value::holder<int>::make_shared(2);
    auto map_holder = multimap::holder<int, value::holder<int>>::make_shared({{1, *holder1}, {2, *holder2}});

    auto holder3 = value::holder<int>::make_shared(3);
    auto holder4 = value::holder<int>::make_shared(4);
    map_holder->replace({{3, *holder3}, {4, *holder4}});

    std::vector<event> received_events;
    std::vector<std::tuple<int, value::holder<int>, int>> received_relayed_events;

    auto chain =
        map_holder->chain()
            .perform([&received_events, &received_relayed_events](auto const &event) {
                received_events.push_back(event);
                if (event.type() == event_type::relayed) {
                    auto const &relayed = event.template get<multimap::relayed_event<int, value::holder<int>>>();
                    received_relayed_events.push_back(std::make_tuple(relayed.key, relayed.value, relayed.relayed));
                }
            })
            .end();

    holder1->set_value(10);
    holder2->set_value(20);

    XCTAssertEqual(received_events.size(), 0);

    holder3->set_value(30);

    auto holder30 = value::holder<int>::make_shared(30);
    XCTAssertEqual(received_events.size(), 1);
    XCTAssertEqual(received_events.at(0).type(), event_type::relayed);
    XCTAssertEqual(received_relayed_events.size(), 1);
    XCTAssertEqual(std::get<0>(received_relayed_events.at(0)), 3);
    XCTAssertEqual(std::get<1>(received_relayed_events.at(0)), *holder30);
    XCTAssertEqual(std::get<2>(received_relayed_events.at(0)), 30);

    holder4->set_value(40);

    auto holder40 = value::holder<int>::make_shared(40);
    XCTAssertEqual(received_events.size(), 2);
    XCTAssertEqual(received_events.at(1).type(), event_type::relayed);
    XCTAssertEqual(received_relayed_events.size(), 2);
    XCTAssertEqual(std::get<0>(received_relayed_events.at(1)), 4);
    XCTAssertEqual(std::get<1>(received_relayed_events.at(1)), *holder40);
    XCTAssertEqual(std::get<2>(received_relayed_events.at(1)), 40);
}

- (void)test_chain_not_relayed_after_erased {
    auto holder1 = value::holder<int>::make_shared(1);
    auto holder2 = value::holder<int>::make_shared(2);
    auto map_holder = multimap::holder<int, value::holder<int>>::make_shared({{1, *holder1}, {2, *holder2}});

    map_holder->erase_for_key(1);

    std::vector<event> received_events;
    std::vector<std::tuple<int, value::holder<int>, int>> received_relayed_events;

    auto chain =
        map_holder->chain()
            .perform([&received_events, &received_relayed_events](auto const &event) {
                received_events.push_back(event);
                if (event.type() == event_type::relayed) {
                    auto const &relayed = event.template get<multimap::relayed_event<int, value::holder<int>>>();
                    received_relayed_events.push_back(std::make_tuple(relayed.key, relayed.value, relayed.relayed));
                }
            })
            .end();

    holder1->set_value(3);

    XCTAssertEqual(received_events.size(), 0);

    holder2->set_value(4);

    XCTAssertEqual(received_events.size(), 1);
}

- (void)test_chain_not_relayed_after_clear {
    auto holder1 = value::holder<int>::make_shared(1);
    auto holder2 = value::holder<int>::make_shared(2);
    auto map_holder = multimap::holder<int, value::holder<int>>::make_shared({{1, *holder1}, {2, *holder2}});

    map_holder->clear();

    std::vector<event> received_events;

    auto chain =
        map_holder->chain().perform([&received_events](auto const &event) { received_events.push_back(event); }).end();

    holder1->set_value(3);
    holder2->set_value(4);

    XCTAssertEqual(received_events.size(), 0);
}

- (void)test_chain_not_relayed_after_replaced {
    auto holder1 = value::holder<int>::make_shared(1);
    auto holder2 = value::holder<int>::make_shared(2);
    auto map_holder = multimap::holder<int, value::holder<int>>::make_shared({{1, *holder1}, {2, *holder2}});

    auto holder3 = value::holder<int>::make_shared(3);
    map_holder->replace({{3, *holder3}});

    std::vector<event> received_events;

    auto chain =
        map_holder->chain().perform([&received_events](auto const &event) { received_events.push_back(event); }).end();

    holder1->set_value(10);
    holder2->set_value(20);

    XCTAssertEqual(received_events.size(), 0);

    holder3->set_value(30);

    XCTAssertEqual(received_events.size(), 1);
}

@end
