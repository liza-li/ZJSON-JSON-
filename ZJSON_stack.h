#include <iostream>
#include <assert.h>
#include <cstddef>
#include "allocator.h"
#include "Swap.h"

#define unlikely(x) __builtin_expect(!!(x), 0) //x很可能为假

namespace internal {
template <typename Allocator>
class Stack {
public:
   
    Stack(Allocator* allocator, size_t stackCapacity) : allocator_(allocator), ownAllocator_(0), stack_(0), stackTop_(0), stackEnd_(0), initialCapacity_(stackCapacity) 
    {
    }

    ~Stack() {
        Destroy();
    }

    void Swap(Stack& rhs) noexcept {
        internal::Swap(allocator_, rhs.allocator_);
        internal::Swap(ownAllocator_, rhs.ownAllocator_);
        internal::Swap(stack_, rhs.stack_);
        internal::Swap(stackTop_, rhs.stackTop_);
        internal::Swap(stackEnd_, rhs.stackEnd_);
        internal::Swap(initialCapacity_, rhs.initialCapacity_);
    }

    void Clear() { stackTop_ = stack_; }

    bool Empty() const { return stackTop_ == stack_; }
    size_t GetSize() const { return static_cast<size_t>(stackTop_ - stack_); }
    size_t GetCapacity() const { return static_cast<size_t>(stackEnd_ - stack_); }

    void ShrinkToFit() {
        if (Empty()) {
            // If the stack is empty, completely deallocate the memory.
            Allocator::Free(stack_); // NOLINT (+clang-analyzer-unix.Malloc)
            stack_ = 0;
            stackTop_ = 0;
            stackEnd_ = 0;
        }
        else
            Resize(GetSize());
    }

    template<typename T>
  __forceinline  void Reserve(size_t count = 1) {
        if (unlikely(static_cast<std::ptrdiff_t>(sizeof(T) * count) > (stackEnd_ - stackTop_)))
            Expand<T>(count);
    }

    template<typename T>
    __forceinline T* Push(size_t count = 1) {
      Reserve<T>(count);
      return PushUnsafe<T>(count);
  }

    template<typename T>
    __forceinline T* PushUnsafe(size_t count = 1) {
        assert(stackTop_);
        assert (static_cast<std::ptrdiff_t>(sizeof(T) * count) <= (stackEnd_ - stackTop_));
        T* ret = reinterpret_cast<T*>(stackTop_);
        stackTop_ += sizeof(T) * count;
        return ret;
    }

    template<typename T>
    T* Pop(size_t count) {
        assert (GetSize() >= count * sizeof(T));
        stackTop_ -= count * sizeof(T);
        return reinterpret_cast<T*>(stackTop_);
    }
    template<typename T>
    T* Top() {
        assert(GetSize() >= sizeof(T));
        return reinterpret_cast<T*>(stackTop_ - sizeof(T));
    }

    template<typename T>
    const T* Top() const {
        assert(GetSize() >= sizeof(T));
        return reinterpret_cast<T*>(stackTop_ - sizeof(T));
    }

    template<typename T>
    T* End() { return reinterpret_cast<T*>(stackTop_); }

    template<typename T>
    const T* End() const { return reinterpret_cast<T*>(stackTop_); }

    template<typename T>
    T* Bottom() { return reinterpret_cast<T*>(stack_); }

    template<typename T>
    const T* Bottom() const { return reinterpret_cast<T*>(stack_); }

    bool HasAllocator() const {
        return allocator_ != 0;
    }

    Allocator& GetAllocator() {
        RAPIDJSON_ASSERT(allocator_);
        return *allocator_;
    }


private:
    template<typename T>
    void Expand(size_t count) {
        // 仅在当前堆栈存在时展开容量。否则，只需创建具有初始容量的堆栈。
        size_t newCapacity;
        if (stack_ == 0) {
            if (!allocator_)
                ownAllocator_ = allocator_ = new (Allocator)();
            newCapacity = initialCapacity_;
        }
        else {
            newCapacity = GetCapacity();
            newCapacity += (newCapacity + 1) / 2;
        }
        size_t newSize = GetSize() + sizeof(T) * count;
        if (newCapacity < newSize)
            newCapacity = newSize;

        Resize(newCapacity);
    }

    void Resize(size_t newCapacity) {
        const size_t size = GetSize();  // 备份当前大小
        stack_ = static_cast<char*>(allocator_->Realloc(stack_, GetCapacity(), newCapacity));
        stackTop_ = stack_ + size;
        stackEnd_ = stack_ + newCapacity;
    }

    void Destroy() {
        Allocator::Free(stack_);
       delete(ownAllocator_); // Only delete if it is owned by the stack
    }

 

    // Prohibit copy constructor & assignment operator.
    Stack(const Stack&);
    Stack& operator=(const Stack&);

    Allocator* allocator_;
    Allocator* ownAllocator_;
    char* stack_;
    char* stackTop_;
    char* stackEnd_;
    size_t initialCapacity_;
};

} // namespace internal



