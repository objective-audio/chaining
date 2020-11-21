//
//  yas_receiver_tests.mm
//

#import <XCTest/XCTest.h>
#import <chaining/chaining.h>
#import <string>

using namespace yas;

@interface yas_receiver_tests : XCTestCase

@end

@implementation yas_receiver_tests

- (void)setUp {
    [super setUp];
}

- (void)tearDown {
    [super tearDown];
}

- (void)test_make_shared {
    auto int_handler = [](int const &) {};
    auto copy_int_receiver = chaining::perform_receiver<int>::make_shared(int_handler);
    auto move_int_receiver = chaining::perform_receiver<int>::make_shared(std::move(int_handler));
    auto empty_handler = [] {};
    auto copy_empty_receiver = chaining::perform_receiver<int>::make_shared(empty_handler);
    auto move_empty_receiver = chaining::perform_receiver<int>::make_shared(std::move(empty_handler));

    XCTAssertTrue(copy_int_receiver);
    XCTAssertTrue(move_int_receiver);
    XCTAssertTrue(copy_empty_receiver);
    XCTAssertTrue(move_empty_receiver);
}

- (void)test_receive {
    std::string received = "";

    auto notifier = chaining::notifier<int>::make_shared();
    auto receiver = chaining::perform_receiver<std::string>::make_shared(
        [&received](std::string const &value) { received = value; });

    auto node = notifier->chain().to([](int const &value) { return std::to_string(value); }).send_to(receiver).end();

    notifier->notify(3);

    XCTAssertEqual(received, "3");
}

- (void)test_receive_tuple {
    auto notifier = chaining::notifier<std::tuple<int, std::string>>::make_shared();

    int int_received = -1;
    std::string string_received = "";

    auto int_receiver =
        chaining::perform_receiver<int>::make_shared([&int_received](int const &value) { int_received = value; });
    auto string_receiver = chaining::perform_receiver<std::string>::make_shared(
        [&string_received](std::string const &value) { string_received = value; });

    auto chain = notifier->chain().send_to<0>(int_receiver).send_to<1>(string_receiver).end();

    notifier->notify(std::make_tuple(int(10), std::string("20")));

    XCTAssertEqual(int_received, 10);
    XCTAssertEqual(string_received, "20");
}

- (void)test_send_null {
    bool received = false;

    auto notifier = chaining::notifier<int>::make_shared();
    auto receiver = chaining::perform_receiver<>::make_shared([&received]() { received = true; });

    auto chain = notifier->chain().send_null_to(receiver).end();

    notifier->notify(4);

    XCTAssertTrue(received);
}

- (void)test_receiver {
    int received = -1;

    auto notifier = chaining::notifier<int>::make_shared();

    auto receiver = chaining::perform_receiver<int>::make_shared([&received](int const &value) { received = value; });

    auto chain = notifier->chain().send_to(receiver).end();

    notifier->notify(100);

    XCTAssertEqual(received, 100);
}

- (void)test_receive_array {
    auto notifier = chaining::notifier<std::array<int, 2>>::make_shared();
    int received0 = -1;
    int received1 = -1;

    auto receiver0 =
        chaining::perform_receiver<int>::make_shared([&received0](int const &value) { received0 = value; });
    auto receiver1 =
        chaining::perform_receiver<int>::make_shared([&received1](int const &value) { received1 = value; });
    std::array<std::shared_ptr<chaining::perform_receiver<int>>, 2> receivers{receiver0, receiver1};

    chaining::any_observer_ptr observer = notifier->chain().send_to(receivers).end();

    notifier->notify(std::array<int, 2>{10, 20});

    XCTAssertEqual(received0, 10);
    XCTAssertEqual(received1, 20);
}

- (void)test_receive_array_individual {
    auto notifier = chaining::notifier<std::array<int, 2>>::make_shared();
    int received0 = -1;
    int received1 = -1;

    auto receiver0 =
        chaining::perform_receiver<int>::make_shared([&received0](int const &value) { received0 = value; });
    auto receiver1 =
        chaining::perform_receiver<int>::make_shared([&received1](int const &value) { received1 = value; });

    chaining::any_observer_ptr observer = notifier->chain().send_to<0>(receiver0).send_to<1>(receiver1).end();

    notifier->notify(std::array<int, 2>{10, 20});

    XCTAssertEqual(received0, 10);
    XCTAssertEqual(received1, 20);
}

- (void)test_receiver_vector {
    auto notifier = chaining::notifier<std::vector<int>>::make_shared();
    int received0 = -1;
    int received1 = -1;

    auto receiver0 =
        chaining::perform_receiver<int>::make_shared([&received0](int const &value) { received0 = value; });
    auto receiver1 =
        chaining::perform_receiver<int>::make_shared([&received1](int const &value) { received1 = value; });
    std::vector<std::shared_ptr<chaining::perform_receiver<int>>> receivers{receiver0, receiver1};

    chaining::any_observer_ptr observer = notifier->chain().send_to(receivers).end();

    notifier->notify(std::vector<int>{30, 40});

    XCTAssertEqual(received0, 30);
    XCTAssertEqual(received1, 40);
}

- (void)test_receiver_initializer_list {
    auto notifier = chaining::notifier<std::vector<int>>::make_shared();
    int received0 = -1;
    int received1 = -1;

    auto receiver0 =
        chaining::perform_receiver<int>::make_shared([&received0](int const &value) { received0 = value; });
    auto receiver1 =
        chaining::perform_receiver<int>::make_shared([&received1](int const &value) { received1 = value; });

    chaining::any_observer_ptr observer = notifier->chain().send_to({receiver0, receiver1}).end();

    notifier->notify(std::vector<int>{50, 60});

    XCTAssertEqual(received0, 50);
    XCTAssertEqual(received1, 60);
}

@end
