//
//  yas_vector_event_tests.mm
//

#import <XCTest/XCTest.h>
#import <chaining/yas_chaining_holder.h>
#import <chaining/yas_chaining_vector_holder.h>

using namespace yas;
using namespace yas::chaining;

@interface yas_vector_event_tests : XCTestCase

@end

@implementation yas_vector_event_tests

- (void)setUp {
    [super setUp];
}

- (void)tearDown {
    [super tearDown];
}

- (void)test_make_fetched_event {
    std::vector<int> const elements{1, 2, 3};
    auto event = vector::make_fetched_event(elements);

    XCTAssertEqual(event.type(), event_type::fetched);
    XCTAssertEqual(event.get<vector::fetched_event<int>>().elements, (std::vector<int>{1, 2, 3}));
}

- (void)test_make_any_event {
    std::vector<int> const elements{1, 2, 3};
    auto event = vector::make_any_event(elements);

    XCTAssertEqual(event.type(), event_type::any);
    XCTAssertEqual(event.get<vector::any_event<int>>().elements, (std::vector<int>{1, 2, 3}));
}

- (void)test_make_inserted_event {
    int const element = 1;
    auto event = vector::make_inserted_event(element, 2);

    XCTAssertEqual(event.type(), event_type::inserted);
    XCTAssertEqual(event.get<vector::inserted_event<int>>().element, 1);
    XCTAssertEqual(event.get<vector::inserted_event<int>>().index, 2);
}

- (void)test_make_erased_event {
    auto event = vector::make_erased_event<int>(3);

    XCTAssertEqual(event.type(), event_type::erased);
    XCTAssertEqual(event.get<vector::erased_event<int>>().index, 3);
}

- (void)test_make_replaced_event {
    int const element = 4;
    auto event = vector::make_replaced_event(element, 5);

    XCTAssertEqual(event.type(), event_type::replaced);
    XCTAssertEqual(event.get<vector::replaced_event<int>>().element, 4);
    XCTAssertEqual(event.get<vector::replaced_event<int>>().index, 5);
}

- (void)test_make_relayed_event {
    holder<int> const element{6};
    auto event = vector::make_relayed_event(element, 7, 8);

    XCTAssertEqual(event.type(), event_type::relayed);
    XCTAssertEqual(event.get<vector::relayed_event<holder<int>>>().element, holder<int>(6));
    XCTAssertEqual(event.get<vector::relayed_event<holder<int>>>().index, 7);
    XCTAssertEqual(event.get<vector::relayed_event<holder<int>>>().relayed, 8);
}

@end
