//
//  yas_vector_holder_chain_tests.mm
//

#import <XCTest/XCTest.h>
#import <chaining/yas_chaining_alias.h>
#import <chaining/yas_chaining_holder.h>
#import <chaining/yas_chaining_vector_holder.h>

using namespace yas;
using namespace yas::chaining;

@interface yas_vector_holder_chain_tests : XCTestCase

@end

@implementation yas_vector_holder_chain_tests

- (void)setUp {
    [super setUp];
}

- (void)tearDown {
    [super tearDown];
}

- (void)test_chain_fetched_by_sync {
    vector::holder<int> holder{{0, 1, 2}};

    std::vector<event> received;

    auto chain = holder.chain().perform([&received](auto const &event) { received.push_back(event); }).sync();

    XCTAssertEqual(received.size(), 1);
    XCTAssertEqual(received.at(0).type(), event_type::fetched);
    XCTAssertEqual(received.at(0).get<vector::fetched_event<int>>().elements, (std::vector{0, 1, 2}));
}

- (void)test_chain_any_by_replace {
    vector::holder<int> holder{{0, 1, 2}};

    std::vector<event> received;

    auto chain = holder.chain().perform([&received](auto const &event) { received.push_back(event); }).end();

    holder.replace({3, 4});

    XCTAssertEqual(received.size(), 1);
    XCTAssertEqual(received.at(0).type(), event_type::any);
    XCTAssertEqual(received.at(0).get<vector::any_event<int>>().elements, (std::vector{3, 4}));
}

- (void)test_chain_any_by_clear {
    vector::holder<int> holder{{0, 1, 2}};

    std::vector<event> received;

    auto chain = holder.chain().perform([&received](auto const &event) { received.push_back(event); }).end();

    holder.clear();

    XCTAssertEqual(received.size(), 1);
    XCTAssertEqual(received.at(0).type(), event_type::any);
    XCTAssertEqual(received.at(0).get<vector::any_event<int>>().elements.size(), 0);
}

- (void)test_chain_inserted {
    vector::holder<int> holder{{0, 1, 2}};

    std::vector<event> received;

    auto chain = holder.chain().perform([&received](auto const &event) { received.push_back(event); }).end();

    holder.insert(3, 1);

    XCTAssertEqual(received.size(), 1);
    XCTAssertEqual(received.at(0).type(), event_type::inserted);
    XCTAssertEqual(received.at(0).get<vector::inserted_event<int>>().index, 1);
    XCTAssertEqual(received.at(0).get<vector::inserted_event<int>>().element, 3);
}

- (void)test_chain_erased {
    vector::holder<int> holder{{0, 1, 2}};

    std::vector<event> received;

    auto chain = holder.chain().perform([&received](auto const &event) { received.push_back(event); }).end();

    holder.erase_at(1);

    XCTAssertEqual(received.size(), 1);
    XCTAssertEqual(received.at(0).type(), event_type::erased);
    XCTAssertEqual(received.at(0).get<vector::erased_event<int>>().index, 1);
}

- (void)test_chain_replace {
    vector::holder<int> holder{{0, 1, 2}};

    std::vector<event> received;

    auto chain = holder.chain().perform([&received](auto const &event) { received.push_back(event); }).end();

    holder.replace(10, 1);

    XCTAssertEqual(received.size(), 1);
    XCTAssertEqual(received.at(0).type(), event_type::replaced);
    XCTAssertEqual(received.at(0).get<vector::replaced_event<int>>().index, 1);
    XCTAssertEqual(received.at(0).get<vector::replaced_event<int>>().element, 10);
}

- (void)test_chain_relayed {
    value::holder<int> holder{1};
    vector::holder<value::holder<int>> vector_holder{{holder}};

    std::vector<event> received;
    std::vector<int> relayed;

    auto chain = vector_holder.chain()
                     .perform([&received, &relayed](auto const &event) {
                         received.push_back(event);
                         relayed.push_back(event.template get<vector::relayed_event<value::holder<int>>>().relayed);
                     })
                     .end();

    holder.set_value(2);

    XCTAssertEqual(received.size(), 1);
    XCTAssertEqual(received.at(0).type(), event_type::relayed);
    XCTAssertEqual(relayed.size(), 1);
    XCTAssertEqual(relayed.at(0), 2);
}

- (void)test_chain_relayed_after_inserted {
    vector::holder<value::holder<int>> vector_holder;

    value::holder<int> holder{1};
    vector_holder.insert(holder, 0);

    std::vector<event> received;

    auto chain = vector_holder.chain().perform([&received](auto const &event) { received.push_back(event); }).end();

    holder.set_value(2);

    XCTAssertEqual(received.size(), 1);
}

- (void)test_chain_relayed_after_replaced {
    value::holder<int> holder1{1};
    vector::holder<value::holder<int>> vector_holder{{holder1}};

    value::holder<int> holder2{2};
    vector_holder.replace(holder2, 0);

    std::vector<event> received;

    auto chain = vector_holder.chain().perform([&received](auto const &event) { received.push_back(event); }).end();

    holder2.set_value(20);

    XCTAssertEqual(received.size(), 1);
}

- (void)test_chain_relayed_after_replaced_all {
    value::holder<int> holder1{1};
    value::holder<int> holder2{2};
    vector::holder<value::holder<int>> vector_holder{{holder1, holder2}};

    value::holder<int> holder3{3};
    vector_holder.replace({holder3});

    std::vector<event> received;

    auto chain = vector_holder.chain().perform([&received](auto const &event) { received.push_back(event); }).end();

    holder3.set_value(30);

    XCTAssertEqual(received.size(), 1);
}

- (void)test_chain_not_relayed_after_erased {
    value::holder<int> holder1{1};
    value::holder<int> holder2{2};
    vector::holder<value::holder<int>> vector_holder{{holder1, holder2}};

    vector_holder.erase_at(0);

    std::vector<event> received;

    auto chain = vector_holder.chain().perform([&received](auto const &event) { received.push_back(event); }).end();

    holder1.set_value(3);

    XCTAssertEqual(received.size(), 0);

    holder2.set_value(4);

    XCTAssertEqual(received.size(), 1);
}

- (void)test_chain_not_relayed_after_clear {
    value::holder<int> holder1{1};
    value::holder<int> holder2{2};
    vector::holder<value::holder<int>> vector_holder{{holder1, holder2}};

    vector_holder.clear();

    std::vector<event> received;

    auto chain = vector_holder.chain().perform([&received](auto const &event) { received.push_back(event); }).end();

    holder1.set_value(3);
    holder2.set_value(4);

    XCTAssertEqual(received.size(), 0);
}

- (void)test_chain_not_relayed_after_replaced {
    value::holder<int> holder1{1};
    value::holder<int> holder2{2};
    vector::holder<value::holder<int>> vector_holder{{holder1, holder2}};

    vector_holder.replace(value::holder<int>{10}, 0);

    std::vector<event> received;

    auto chain = vector_holder.chain().perform([&received](auto const &event) { received.push_back(event); }).end();

    holder1.set_value(3);

    XCTAssertEqual(received.size(), 0);

    holder2.set_value(4);

    XCTAssertEqual(received.size(), 1);
}

- (void)test_chain_not_relayed_after_replaced_all {
    value::holder<int> holder1{1};
    value::holder<int> holder2{2};
    vector::holder<value::holder<int>> vector_holder{{holder1, holder2}};

    vector_holder.replace({value::holder<int>{10}});

    std::vector<event> received;

    auto chain = vector_holder.chain().perform([&received](auto const &event) { received.push_back(event); }).end();

    holder1.set_value(3);
    holder2.set_value(4);

    XCTAssertEqual(received.size(), 0);
}

- (void)test_alias {
    vector::holder<int> holder{{0, 1, 2}};
    auto alias = make_alias(holder);

    XCTAssertEqual(alias.raw(), (std::vector<int>{{0, 1, 2}}));

    std::vector<chaining::event> received;

    auto observer = alias.chain().perform([&received](auto const &event) { received.push_back(event); }).sync();

    XCTAssertEqual(received.size(), 1);
    XCTAssertEqual(received.at(0).type(), event_type::fetched);

    holder.push_back(3);

    XCTAssertEqual(received.size(), 2);
    XCTAssertEqual(received.at(1).type(), event_type::inserted);
}

@end
