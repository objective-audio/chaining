//
//  yas_chaining_joint.h
//

#pragma once

#include <chaining/yas_chaining_handler.h>
#include <chaining/yas_chaining_types.h>

#include <mutex>
#include <vector>

namespace yas::chaining {
template <typename T>
class sender;

struct any_joint {
    virtual ~any_joint();

    virtual void fetch() = 0;
    virtual void invalidate() = 0;
    [[nodiscard]] virtual std::size_t handlers_size() const = 0;
    virtual any_handler_ptr const &any_handler(std::size_t const idx) const = 0;

    uintptr_t identifier() const;

    template <typename P>
    [[nodiscard]] joint_handler_f<P> const &handler(std::size_t const idx) const;

   protected:
    any_joint();

   private:
    any_joint(any_joint const &) = delete;
    any_joint(any_joint &&) = delete;
    any_joint &operator=(any_joint const &) = delete;
    any_joint &operator=(any_joint &&) = delete;
};

using any_joint_ptr = std::shared_ptr<any_joint>;

template <typename T>
struct [[nodiscard]] joint final : any_joint {
    joint(std::weak_ptr<sender<T> const> &&);

    ~joint();

    void call_first(T const &);
    void fetch() override;
    void invalidate() override;

    template <typename P>
    void push_handler(joint_handler_f<P>);

    [[nodiscard]] std::size_t handlers_size() const override;

    void add_sub_joint(any_joint_ptr);

   private:
    std::weak_ptr<sender<T> const> _weak_sender;
    std::vector<any_handler_ptr> _handlers;
    std::vector<any_joint_ptr> _sub_joints;
    std::mutex _send_mutex;
    bool _pushed = false;

    any_handler_ptr const &any_handler(std::size_t const idx) const override;
};

template <typename T>
using joint_ptr = std::shared_ptr<joint<T>>;

template <typename T>
joint_ptr<T> make_joint(std::weak_ptr<sender<T>>);
}  // namespace yas::chaining

#include <chaining/yas_chaining_joint_private.h>
