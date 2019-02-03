//
//  yas_alias_tests.mm
//

#import <XCTest/XCTest.h>
#import <chaining/yas_chaining_alias.h>
#import <chaining/yas_chaining_value_holder.h>

using namespace yas;
using namespace yas::chaining;

@interface yas_alias_tests : XCTestCase

@end

@implementation yas_alias_tests

- (void)setUp {
}

- (void)tearDown {
}

- (void)test_chain {
    value::holder<int> holder{0};
    chaining::alias<value::holder<int>> alias{holder};

    std::vector<int> called;

    auto observer = alias.chain().perform([&called](int const &value) { called.push_back(value); }).sync();

    XCTAssertEqual(called.size(), 1);
    XCTAssertEqual(called.at(0), 0);

    holder.set_value(1);

    XCTAssertEqual(called.size(), 2);
    XCTAssertEqual(called.at(1), 1);
}

- (void)test_make_alias {
    value::holder<int> holder{0};
    auto alias = make_alias(holder);

    std::vector<int> called;

    auto observer = alias.chain().perform([&called](int const &value) { called.push_back(value); }).sync();

    XCTAssertEqual(called.size(), 1);
    XCTAssertEqual(called.at(0), 0);
}

- (void)test_raw {
    value::holder<int> holder{0};
    auto alias = make_alias(holder);
    auto const const_alias = make_alias(holder);

    XCTAssertEqual(alias.raw(), 0);
    XCTAssertEqual(const_alias.raw(), 0);

    holder.set_value(1);

    XCTAssertEqual(alias.raw(), 1);
    XCTAssertEqual(const_alias.raw(), 1);
}

@end
