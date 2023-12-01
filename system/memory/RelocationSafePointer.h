#ifndef SYSTEM_MEMORY_RELOCATIONSAFEPOINTER
#define SYSTEM_MEMORY_RELOCATIONSAFEPOINTER

#include <system/stdint.h>
#include <system/syscall_interface.h>

#ifdef STACK_BALANCIMG

extern uint64_t __virtual_stack_end;
extern uint64_t __virtual_stack_begin;
extern uint64_t __current_stack_base_ptr;

template <typename T>
class Intermediate_Pointer_Setter;

template <typename T>
class RelocationSafePointer {
   private:
    /**
     * The local_ptr stores the pointer itself. The ptr_snapshot_relocation
     * stores the current relocation width, which has to be used later to
     * dereference correctly
     */
    uintptr_t local_ptr;
    uint64_t ptr_snapshot_relocation;

   public:
    RelocationSafePointer() {
        local_ptr = 0;
        ptr_snapshot_relocation = 0;
    }
    RelocationSafePointer(T *ptr) {
        uint64_t __shadow_stack_begin = __virtual_stack_begin - REAL_STACK_SIZE;
        // Check if the pointer needs relocation
        if ((uintptr_t)(ptr) >= __shadow_stack_begin &&
            (uintptr_t)(ptr) < __virtual_stack_end) {
            syscall_delay_relocation();

            local_ptr = (uintptr_t)ptr;
            /**
             * Save the internal pointer against relocations by setting the
             * upper bit to one
             */
            local_ptr |= (0b1UL << 63);

            ptr_snapshot_relocation =
                (__virtual_stack_end - __current_stack_base_ptr);

            syscall_continue_relocatuion();
        } else {
            local_ptr = (uintptr_t)ptr;
        }
    }

    Intermediate_Pointer_Setter<T> operator*() {
        return Intermediate_Pointer_Setter<T>(*this, 0);
    }

    Intermediate_Pointer_Setter<T> operator[](uint64_t index) {
        return Intermediate_Pointer_Setter<T>(*this, index);
    }

    T *operator&() { return (T *)local_ptr; }

    RelocationSafePointer<T> operator+(uint64_t offset) {
        RelocationSafePointer<T> ret;
        ret.local_ptr = local_ptr + (sizeof(T) * offset);
        ret.ptr_snapshot_relocation = ptr_snapshot_relocation;
        return ret;
    }

    RelocationSafePointer<T> operator+=(uint64_t offset) {
        return *this + offset;
    }
    RelocationSafePointer<T> operator++(int) {
        RelocationSafePointer<T> copy = *this;
        *this += 1;
        return copy;
    }
    RelocationSafePointer<T> operator++() { return *this + 1; }

    RelocationSafePointer<T> operator-(uint64_t offset) {
        RelocationSafePointer<T> ret;
        ret.local_ptr = local_ptr - (sizeof(T) * offset);
        ret.ptr_snapshot_relocation = ptr_snapshot_relocation;
        return ret;
    }
    RelocationSafePointer<T> operator-=(uint64_t offset) {
        return *this - offset;
    }
    RelocationSafePointer<T> operator--(int) {
        RelocationSafePointer<T> copy = *this;
        *this -= 1;
        return copy;
    }
    RelocationSafePointer<T> operator--() { return *this - 1; }

    T get_value(uint64_t index) {
        // Check if the pointer needs relocation
        if (local_ptr & (0b1UL << 63)) {
            syscall_delay_relocation();

            // Restore the correct pointer
            uintptr_t correct = local_ptr & ~(0b1UL << 63);
            // Determine the current stack relocation
            uint64_t current_relocation =
                (__virtual_stack_end - __current_stack_base_ptr);

            if (current_relocation >= ptr_snapshot_relocation) {
                correct -= (current_relocation - ptr_snapshot_relocation);
            } else {
                // In this case, the relocation wrapped around
                correct += (ptr_snapshot_relocation - current_relocation);
            }

            T copy = ((T *)(correct))[index];

            syscall_continue_relocatuion();

            return copy;
        } else {
            return ((T *)(local_ptr))[index];
        }
    }

    void set_value(T value, uint64_t index) {
        // Check if the pointer needs relocation
        if (local_ptr & (0b1UL << 63)) {
            syscall_delay_relocation();

            // Restore the correct pointer
            uintptr_t correct = local_ptr & ~(0b1UL << 63);
            // Determine the current stack relocation
            uint64_t current_relocation =
                (__virtual_stack_end - __current_stack_base_ptr);

            if (current_relocation >= ptr_snapshot_relocation) {
                correct -= (current_relocation - ptr_snapshot_relocation);
            } else {
                // In this case, the relocation wrapped around
                correct += (ptr_snapshot_relocation - current_relocation);
            }

            ((T *)(correct))[index] = value;

            syscall_continue_relocatuion();
        } else {
            ((T *)(local_ptr))[index] = value;
        }
    }
};

template <typename T>
class Intermediate_Pointer_Setter {
   private:
    RelocationSafePointer<T> ptr;
    uint64_t index;

   public:
    Intermediate_Pointer_Setter(RelocationSafePointer<T> ptr, uint64_t index) {
        this->ptr = ptr;
        this->index = index;
    }
    T operator=(T value) {
        ptr.set_value(value, index);
        return value;
    }
    operator T() { return ptr.get_value(index); }
};

#endif

#endif