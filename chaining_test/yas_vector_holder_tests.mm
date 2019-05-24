//
//  yas_vector_holder_tests.mm
//

#import <XCTest/XCTest.h>
#import <chaining/yas_chaining_vector_holder.h>

using namespace yas;
using namespace yas::chaining;

@interface yas_vector_holder_tests : XCTestCase

@end

@implementation yas_vector_holder_tests

- (void)setUp {
    [super setUp];
}

- (void)tearDown {
    [super tearDown];
}

- (void)test_get_raw {
    vector::holder<int> holder{{0, 1, 2}};

    XCTAssertEqual(holder.raw(), (std::vector<int>{0, 1, 2}));
}

- (void)test_size {
    vector::holder<int> holder0;
    vector::holder<int> holder1{{1}};
    vector::holder<int> holder3{{0, 1, 2}};

    XCTAssertEqual(holder0.size(), 0);
    XCTAssertEqual(holder1.size(), 1);
    XCTAssertEqual(holder3.size(), 3);
}

- (void)test_equal {
    vector::holder<int> holder1a{{1, 2}};
    vector::holder<int> holder1b{{1, 2}};
    vector::holder<int> holder2{{2, 3}};

    XCTAssertTrue(holder1a == holder1a);
    XCTAssertTrue(holder1a == holder1b);
    XCTAssertFalse(holder1a == holder2);

    XCTAssertFalse(holder1a != holder1a);
    XCTAssertFalse(holder1a != holder1b);
    XCTAssertTrue(holder1a != holder2);
}

- (void)test_clear {
    vector::holder<int> holder{{0, 1, 2}};

    XCTAssertEqual(holder.size(), 3);

    holder.clear();

    XCTAssertEqual(holder.size(), 0);
}

- (void)test_erase_at {
    vector::holder<int> holder{{0, 1, 2}};

    int erased = holder.erase_at(1);

    XCTAssertEqual(erased, 1);
    XCTAssertEqual(holder.raw(), (std::vector<int>{0, 2}));
}

- (void)test_push_back {
    vector::holder<int> holder{{0, 1}};

    holder.push_back(2);

    XCTAssertEqual(holder.size(), 3);
    XCTAssertEqual(holder.raw(), (std::vector<int>{0, 1, 2}));
}

- (void)test_insert {
    vector::holder<int> holder{{0, 1}};

    holder.insert(2, 1);

    XCTAssertEqual(holder.size(), 3);
    XCTAssertEqual(holder.raw(), (std::vector<int>{0, 2, 1}));
}

- (void)test_replace_all {
    vector::holder<int> holder{{0, 1}};

    holder.replace({2, 3, 4});

    XCTAssertEqual(holder.size(), 3);
    XCTAssertEqual(holder.raw(), (std::vector<int>{2, 3, 4}));
}

- (void)test_replace {
    vector::holder<int> holder{{0, 1, 2}};

    holder.replace(10, 1);

    XCTAssertEqual(holder.size(), 3);
    XCTAssertEqual(holder.raw(), (std::vector<int>{0, 10, 2}));
}

- (void)test_receiver {
    vector::holder<int> holder{{0, 1, 2}};

    holder.receiver().receivable().receive_value(vector::make_fetched_event(std::vector<int>{10}));

    XCTAssertEqual(holder.raw().size(), 1);
    XCTAssertEqual(holder.at(0), 10);

    holder.receiver().receivable().receive_value(vector::make_any_event(std::vector<int>{3, 4}));

    XCTAssertEqual(holder.raw().size(), 2);
    XCTAssertEqual(holder.at(0), 3);
    XCTAssertEqual(holder.at(1), 4);

    holder.receiver().receivable().receive_value(vector::make_inserted_event(5, 2));

    XCTAssertEqual(holder.raw().size(), 3);
    XCTAssertEqual(holder.at(2), 5);

    holder.receiver().receivable().receive_value(vector::make_erased_event<int>(1));

    XCTAssertEqual(holder.raw().size(), 2);
    XCTAssertEqual(holder.at(0), 3);
    XCTAssertEqual(holder.at(1), 5);

    holder.receiver().receivable().receive_value(vector::make_replaced_event(6, 1));

    XCTAssertEqual(holder.raw().size(), 2);
    XCTAssertEqual(holder.at(0), 3);
    XCTAssertEqual(holder.at(1), 6);
}

@end
