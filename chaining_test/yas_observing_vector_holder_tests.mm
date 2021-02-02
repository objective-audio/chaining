//
//  yas_observing_vector_holder_tests.mm
//

#import <XCTest/XCTest.h>
#import <chaining/chaining.h>

using namespace yas;
using namespace yas::observing;

@interface yas_observing_vector_holder_tests : XCTestCase

@end

@implementation yas_observing_vector_holder_tests

- (void)test_make {
    auto const holder = vector::holder<int>::make_shared();

    XCTAssertEqual(holder->size(), 0);
    XCTAssertEqual(holder->value(), std::vector<int>{});
}

- (void)test_make_by_copy {
    std::vector<int> const vector{1, 2};
    auto const holder = vector::holder<int>::make_shared(vector);

    XCTAssertEqual(holder->size(), 2);
    XCTAssertEqual(holder->value(), (std::vector<int>{1, 2}));
}

- (void)test_make_by_move {
    std::vector<int> vector{3, 4};
    auto const holder = vector::holder<int>::make_shared(std::move(vector));

    XCTAssertEqual(holder->size(), 2);
    XCTAssertEqual(holder->value(), (std::vector<int>{3, 4}));
}

- (void)test_at {
    auto const holder = vector::holder<int>::make_shared({5, 6, 7});

    XCTAssertEqual(holder->at(0), 5);
    XCTAssertEqual(holder->at(1), 6);
    XCTAssertEqual(holder->at(2), 7);
}

- (void)test_replace_all_by_copy {
    auto const holder = vector::holder<int>::make_shared({10, 11});

    std::vector<int> const vector{12, 13};

    holder->replace(vector);

    XCTAssertEqual(holder->value(), (std::vector<int>{12, 13}));
}

- (void)test_replace_all_by_move {
    auto const holder = vector::holder<int>::make_shared({20, 21});

    std::vector<int> vector{22, 23};

    holder->replace(std::move(vector));

    XCTAssertEqual(holder->value(), (std::vector<int>{22, 23}));
}

- (void)test_replace_element_by_copy {
    auto const holder = vector::holder<int>::make_shared({30, 31, 32});

    int const value = 33;

    holder->replace(value, 1);

    XCTAssertEqual(holder->value(), (std::vector<int>{30, 33, 32}));
}

- (void)test_replace_element_by_move {
    auto const holder = vector::holder<int>::make_shared({40, 41, 42});

    int value = 43;

    holder->replace(std::move(value), 1);

    XCTAssertEqual(holder->value(), (std::vector<int>{40, 43, 42}));
}

- (void)test_push_back_by_copy {
    auto const holder = vector::holder<int>::make_shared({50, 51});

    int const value = 52;

    holder->push_back(value);

    XCTAssertEqual(holder->value(), (std::vector<int>{50, 51, 52}));
}

- (void)test_push_back_by_move {
    auto const holder = vector::holder<int>::make_shared({60, 61});

    int value = 62;

    holder->push_back(std::move(value));

    XCTAssertEqual(holder->value(), (std::vector<int>{60, 61, 62}));
}

- (void)test_insert_by_copy {
    auto const holder = vector::holder<int>::make_shared({70, 71});

    int const value = 72;

    holder->insert(value, 1);

    XCTAssertEqual(holder->value(), (std::vector<int>{70, 72, 71}));
}

- (void)test_insert_by_move {
    auto const holder = vector::holder<int>::make_shared({80, 81});

    int value = 82;

    holder->insert(std::move(value), 1);

    XCTAssertEqual(holder->value(), (std::vector<int>{80, 82, 81}));
}

- (void)test_erase {
    auto const holder = vector::holder<int>::make_shared({90, 91, 92});

    int const erased = holder->erase(1);

    XCTAssertEqual(erased, 91);
    XCTAssertEqual(holder->value(), (std::vector<int>{90, 92}));
}

- (void)test_clear {
    auto const holder = vector::holder<int>::make_shared({100, 101});

    holder->clear();

    XCTAssertEqual(holder->value(), (std::vector<int>{}));
}

- (void)test_observe {
    auto const holder = vector::holder<int>::make_shared({200, 201});

    std::vector<vector::holder<int>::event_type> called_event_types;

    auto canceller = holder->observe(
        [&called_event_types](auto const &event) { called_event_types.emplace_back(event.type); }, true);

    XCTAssertEqual(called_event_types.size(), 1);
    XCTAssertEqual(called_event_types.at(0), vector::holder<int>::event_type::any);

    holder->replace(std::vector<int>{210, 211, 212});

    XCTAssertEqual(called_event_types.size(), 2);
    XCTAssertEqual(called_event_types.at(1), vector::holder<int>::event_type::any);

    holder->replace(220, 1);

    XCTAssertEqual(called_event_types.size(), 3);
    XCTAssertEqual(called_event_types.at(2), vector::holder<int>::event_type::replaced);

    holder->push_back(221);

    XCTAssertEqual(called_event_types.size(), 4);
    XCTAssertEqual(called_event_types.at(3), vector::holder<int>::event_type::inserted);

    holder->insert(222, 0);

    XCTAssertEqual(called_event_types.size(), 5);
    XCTAssertEqual(called_event_types.at(4), vector::holder<int>::event_type::inserted);

    holder->erase(1);

    XCTAssertEqual(called_event_types.size(), 6);
    XCTAssertEqual(called_event_types.at(5), vector::holder<int>::event_type::erased);

    holder->clear();

    XCTAssertEqual(called_event_types.size(), 7);
    XCTAssertEqual(called_event_types.at(6), vector::holder<int>::event_type::any);

    holder->clear();
    XCTAssertEqual(called_event_types.size(), 7);
}

@end
