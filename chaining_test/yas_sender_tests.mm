//
//  yas_sender_tests.mm
//

#import <XCTest/XCTest.h>
#import <chaining/yas_chaining_umbrella.h>

using namespace yas;

namespace yas::test {
struct wrapped_sender : chaining::sender<int> {
    int value() const {
        return this->_holder->raw();
    }

    void set_value(int const value) {
        this->_holder->set_value(value);
    }

    virtual chaining::chain_unsync_t<int> chain_unsync() const override {
        return this->sender_protocol()->chain_unsync();
    }
    virtual chaining::chain_sync_t<int> chain_sync() const override {
        return this->sender_protocol()->chain_sync();
    }

   private:
    chaining::value::holder_ptr<int> _holder = chaining::value::holder<int>::make_shared(0);

    std::shared_ptr<chaining::sender_protocol<int>> sender_protocol() const {
        return std::dynamic_pointer_cast<chaining::sender_protocol<int>>(this->_holder);
    }

    virtual void broadcast(int const &value) const override {
        this->sender_protocol()->broadcast(value);
    }
    virtual void erase_joint(std::uintptr_t const key) const override {
        this->sender_protocol()->erase_joint(key);
    }
    virtual void send_value_to_target(int const &value, std::uintptr_t const key) const override {
        this->sender_protocol()->send_value_to_target(value, key);
    }

    void fetch_for(chaining::any_joint const &joint) const override {
        this->sender_protocol()->fetch_for(joint);
    }
};
}

@interface yas_sender_derrived_tests : XCTestCase

@end

@implementation yas_sender_derrived_tests

- (void)test_wrap {
    test::wrapped_sender sender;

    sender.set_value(1);

    int received = 0;

    auto observer = sender.chain_sync().perform([&received](int const &value) { received = value; }).sync();

    XCTAssertEqual(received, 1);

    sender.set_value(2);

    XCTAssertEqual(received, 2);
}

@end
