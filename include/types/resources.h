/*
  OS Resources
  Copyright (c) 2023, John Ryland
  All rights reserved.
*/

#pragma once

#include "common/integers.hpp"
#include "runtime/memory.h"
#include "types/memory.h"
#include "types/tasks.h"

//
// Goals of resource management:
//
//    iterate and find available features, devices, capabilities
//    account for resource usage
//    be able to apply limits to resource usage by user/process etc
//    be able to reclaim resources (ie process dies, close open handle, reclaim memory etc) - process lifecycle
//    gather statistics
//    fairly share resources
//    be able to gain exclusive access to a resource, or ensure all requirements can be met for running some high assurance task
//

//   Process, has a PID
//      - parent PID
//      - group
//      - cpu usage
//      - user/owner (account)
//      - memory (virtual)
//      - memory (real)
//      - memory (private)
//      - memory (shared)
//      - threads (execution contexts)
//      - ports (sockets)
//

enum ResourceType
{
    ROOT, // COMPUTER

    FREE,

    RESOURCE_GROUP,

    USER,
    GROUP,

    // Ideas like:
    //   MOUNTS, /PROC, /DEVFS etc, device-info,  HARDWARE, SOFTWARE, NETWORK

//    RESOURCE_SET,      // set of resources

    PHYSICAL_MEMORY,   // allocation of physical memory ranges to owners of that memory
                       // initialized from the BIOS memory map - some memory ranges are reserved or for ACPI etc
    ADDRESS_SPACE,     // set of page tables containing mapping of phy -> virt memory (could be different for different processes)
                       // page tables themselves consume physical memory
    VIRTUAL_MEMORY,    // allocation of virtual memory ranges to owners in the context of an address space (owned by an address space?)

    DEVICE,            // physical device sitting connection to a bus such as pci etc, lives in a device tree, there might be
                       // the topological parent to the device which might be different from the current user which temp owns it.
                       // need to think about those concepts

    SOCKET,            // a specific type of I/O device, perhaps instead of socket and file, might add generic i/o device
                       // in OSI model there are layers - we have device for layer-1, ethernet sits at layer-2, and tcp/ip on top of this.
                       // socket here is skipping some layers, so we may need some resources here to cover those. perhaps a driver type

    DRIVER,            // file-system, network protocol etc
    MODULE,            // loadable/unloadable driver

    EXECUTION_CONTEXT, // cpu-state
    TASK,              // cpu-state + constraints + state, usually uses kernel memory space (kernel ring-0 task / high priority user task)
    THREAD,            // task + owned by process, shares memory space of user process  (TSS, page tables)
    PROCESS,           // task + can own threads, has memory space, runs in user space  (TSS, page tables)

    TASK_STATISTICS,

    

    SEMAPHORE,
    MUTEX,
    CRITICAL_SECTION,
    FILE_HANDLE,

    DIRECTORY,
    // ...

    // smaller quantity of these so could be in a sub-resource type
    PROCESSOR,
    INTERRUPT,
    BUS,
};

struct ResourceHandle
{
    uint32_t        folio : 10;        // folio + page + index allows for up to 2^26 entries which is 67 million resource items
    uint32_t        page  : 10;        // up to 1024 folios, each folio entry is 32-bits as a pointer which is 4kb of data for folio list.
    uint32_t        index : 6;         // same with pages, 1024 pages per folio, so a folio is 4kb of data. A page as 2^6 entries that are 64bytes, so 4kb.
    uint32_t        type  : 6;         // lower 6-bits means if resources are 64-byte aligned, then index can be a byte offset by masking out the type
};

struct root_t
{
    // some childern are created - on start up a root user is created, the kernel process is added, the device tree is created
    uint8_t         computer_name[40];
};

struct resource_group_t
{
    uint8_t         name[36];     // continues from generic name
    uint16_t        child_count;  // number of children
    uint8_t         child_flags;  // 0 - uniform children, 1 - non-uniform children, 2 - nameless uniform children
    uint8_t         child_type;   // type of all the children if uniform
};

struct user_t
{
    uint8_t         data[40];
};

struct group_t
{
    uint8_t         data[40];
};

struct address_space_t
{
    uint8_t         data[40];
};

struct physical_memory_t
{
    uint64_t        start;
    uint64_t        end;

    uint8_t         data[24];
};

struct virtual_memory_t
{
    uint8_t         data[40];
};

struct device_t
{
    uint8_t         data[40];
};

struct socket_t
{
    uint8_t         data[40];
};

struct driver_t
{
    uint8_t         data[40];
};

/*
struct module_t
{
    uint8_t         data[40];
};
*/

struct execution_context_t
{
    uint8_t         data[40];
};

/*
struct task_t
{
    uint8_t         data[40];
};
*/

struct thread_t
{
    uint8_t         data[40];
};

struct process_t
{
    // PID  is the resource handle of this
    // PPID is the parent resource handle

    ResourceHandle  process_group;
    ResourceHandle  user;

    ResourceHandle  virtual_memory;
    ResourceHandle  address_space;  // process owns this - this can be a child resource, same with threads

    uint32_t        cpu_usage;

    uint8_t         data[20];

//      - memory (virtual)
//      - memory (real)
//      - memory (private)
//      - memory (shared)
//      - threads (execution contexts)
//      - ports (sockets)

};

struct semaphore_t
{
    uint8_t         data[40];
};

struct mutex_t
{
    uint8_t         data[40];
};

struct critical_section_t
{
    uint8_t         data[40];
};

struct file_handle_t
{
    uint8_t         data[40];
};

struct directory_t
{
    uint8_t         data[40];
};

struct processor_t
{
    uint8_t         data[40];
};

struct interrupt_t
{
    uint8_t         data[40];
};

struct bus_t
{
    uint8_t         data[40];
};

// Perhaps could have two types, named and/or branch nodes, nameless leaf nodes so that nameless leaf nodes can use the full 64 bytes
struct Resource : public ResourceHandle
{
    ResourceHandle          parent;
    ResourceHandle          first_child;
    ResourceHandle          next_sibling;
    char                    name[8];       // some types might extend the name to more than 8 characters
    union
    {
        root_t              root;
        resource_group_t    resource_group;
        user_t              user;
        group_t             group;
        address_space_t     address_space;
        physical_memory_t   physical_memory;
        virtual_memory_t    virtual_memory;
        // memory_t            memory;
        device_t            device;
        socket_t            socket;
        driver_t            driver;
        // module_t            module;
        execution_context_t execution_context;
        task_properties_t   task;
        task_statistics_t   task_statistics;
        thread_t            thread;
        process_t           process;
        semaphore_t         semaphore;
        mutex_t             mutex;
        critical_section_t  critical_section;
        file_handle_t       file_handle;
        directory_t         directory;
        processor_t         processor;
        interrupt_t         interrupt;
        bus_t               bus;
    } data;
};

static_assert(sizeof(Resource) == 64, "cache line sized");

struct ResourcePage
{
    Resource        resources[64];
};

static_assert(sizeof(ResourcePage) == 4096, "page sized");

struct ResourceFolio
{
    ResourcePage*   pages[1024];
};

static_assert(sizeof(ResourceFolio) == 4096, "page sized");

struct AllResources
{
    ResourceFolio*  folios[1024];
};

static_assert(sizeof(AllResources) == 4096, "page sized");

ResourcePage   page_0;
ResourceFolio  folio_0;
AllResources   resources;
ResourceHandle root = { 0, 0, 0, 0 };
ResourceHandle first_free_resource;  // chained with next_sibling

void initialize_resources()
{
    mem_set(resources.folios, 0, sizeof(resources.folios));
    mem_set(&folio_0, 0, sizeof(folio_0));
    mem_set(&page_0, 0, sizeof(page_0));
    resources.folios[0] = &folio_0;
    resources.folios[0]->pages[0] = &page_0;
    for (unsigned i = 0; i < 64; i++)
    {
        Resource& res = resources.folios[0]->pages[0]->resources[i];
        res.folio = 0;
        res.page = 0;
        res.index = i;
        res.type = FREE;
        res.next_sibling = { 0, 0, i+1, FREE };
    }

    Resource& root_res = resources.folios[0]->pages[0]->resources[0];
    root_res.type = ROOT;
    root_res.first_child = { 0, 0, 0, 0 };
    root_res.next_sibling = { 0, 0, 0, 0 };
    mem_cpy(root_res.name, "root", 5);
    // resources.folios[0]->pages[0]->resources[0] = { 0, 0, 0, 0, "root", .data.root.computer_name = "Computer" };

    Resource& res = resources.folios[0]->pages[0]->resources[63];
    res.next_sibling = { 0, 0, 0, 0 };

    first_free_resource = { 0, 0, 1, FREE };
}

Resource* allocate_resource(ResourceType /*type*/, ResourceHandle /*parent*/)
{
    // will need to allocate memory if run out of resources in pool
    // to allocate memory might mean adding entries in memory resources which is a catch-22.
    // some thought is needed for that.
    return nullptr;
}


// named, owner, unique resource handler per resource (32-bit), parent resource / owner
// owner itself might be represented as a resource
// if devices are resources, then the owner of a resource might not be the parent if
// that represents the topology
// so there are two concepts there, the topology as in a device tree of how things are
// connected together physically, and there is the ownership hierarchy, what is using what.
// perhaps they are the same or there could be two resource objects, one for the
// hardware and one for the driver of it.
// also some resources can be split, like memory, there can be a range, but that range can
// then be divided in to two ranges. Is it just memory that is like this, or other things?


// Create:
//  map phys -> virt memory
//       vm_map_physical_memory(mem_space, name, virt, phys, size, flags)
//  allocate memory
//       vm_create_anonymous_region(mem_space, name, addr_ptr, size, flags)
//  semaphore
//       sem_create(count, name)
//  mutex
//       mutex_create(name)
//  interrupt
//       int_set_handler(irq, name, handler, user_ptr);

// Lookup:
//  get by name
//       sem_get(name)

// Iterate:
//  all
//       res_get_first()
//       res_get_next(it)
//  by type
//       sem_get_first()
//       sem_get_next(it)


// limits

class ResourceTree
{
public:
};


// TODO: For some reason this pulls in the kitchen sink, including stdio on macOS.
//       Are there some defines we can set to limit what it includes to just atomics?
//       Alternative is to implement these ourself like below.
#if 0
#include <stdatomic.h>
#else

// If no library support for atomics, then here's some hints to implementing

using atomic_flag = volatile bool;
using atomic_uint = volatile uint32_t;

/*
https://gcc.gnu.org/onlinedocs/gcc/Extended-Asm.html

asm asm-qualifiers ( AssemblerTemplate 
                      : OutputOperands
                      : InputOperands
                      : Clobbers
                      : GotoLabels)

 ‘=’ overwriting existing value
 ‘+’ reading and writing
 ‘r’ for register
 ‘m’ for memory

*/

static inline
uint32_t atomic_compare_exchange(atomic_uint *mem, uint32_t compare, uint32_t value)
{
    uint32_t out;
    // outputs:     "=a"(out)     means %0 is eax and will be copied to out
    // in+out:      "+m"(*mem)    means %1 is *mem, the "+" means it is input and output, so *mem is modified
    // inputs:      "q"(value)    means %2 is set to value. %2 will be one of the general purpose registers
    // constraints: "0"(compare)  means %0 will be initially set to compare.
    // clobbers:    "memory"      means memory could change from this (requires a memory barrier)
    asm volatile ("lock; cmpxchgl %2, %1" : "=a"(out), "+m"(*mem) : "q"(value), "0"(compare) : "memory");
    return out;
}

static inline
void atomic_store(atomic_uint* mem, const uint32_t value)
{
    // outputs:     "=m"(*mem)    means %0 will be *mem and will be written to
    // inputs:      "q"(value)    means %1 is set to value (using one of the general purpose registers)
    // clobbers:    "memory"      means memory could change from this (requires a memory barrier)
    asm volatile ("lock; movl %1, %0" : "=m" (*mem) : "q" (value) : "memory");
}

// Reference:
//   https://en.wikipedia.org/wiki/Fetch-and-add
static inline
uint32_t atomic_fetch_add(atomic_uint* mem, uint32_t value)
{
    asm volatile ("lock; xaddl %0, %1" : "+r" (value), "+m" (*mem) : : "memory");
    return value;
}

static inline
uint32_t atomic_fetch_sub(atomic_uint* mem, uint32_t value)
{
    return atomic_fetch_add(mem, -value);
}

static inline
uint32_t atomic_load(atomic_uint* mem)
{
    return atomic_fetch_add(mem, 0);
}

static inline
bool atomic_flag_test_and_set(atomic_flag* mem)
{
    return atomic_compare_exchange((atomic_uint*)mem, 0, 1);
}

static inline
void atomic_flag_clear(atomic_flag* mem)
{
    atomic_store((atomic_uint*)mem, 0);
}

#endif

// Reference:
//   https://stackoverflow.com/questions/12894078/what-is-the-purpose-of-the-pause-instruction-in-x86
//   https://stackoverflow.com/questions/4725676/how-does-x86-pause-instruction-work-in-spinlock-and-can-it-be-used-in-other-sc
//   https://wiki.osdev.org/Spinlock
static inline
void pause()
{
    asm volatile ("pause" : : : "memory");
}


// References:
//   https://gist.github.com/mepcotterell/8df8e9c742fa6f926c39667398846048?permalink_comment_id=3861236
class SimpleMutex
{
public:
    SimpleMutex()
    {
        atomic_flag_clear(&flag);
    }

    inline void acquire() { Lock();   }
    inline void release() { Unlock(); }

    void Lock()
    {
        while (atomic_flag_test_and_set(&flag))
            pause(); // spin
    }

    void Unlock()
    {
        atomic_flag_clear(&flag);
    }

private:
    atomic_flag flag;
};

using spinlock_t = SimpleMutex;



// Reference:
//   Implementing a semaphore:
//     - https://gist.github.com/mepcotterell/6f0a779befe388ab822764255e3776ae
//   Note that this doesn't interact with the OS to put a process in to the not-ready state or wake a process on signal.
//   What this provides is mutex like function, but allowing another thread to be able to unlock a different thread.
class Semaphore
{
public:
    Semaphore()
    {
        // atomic_init(&val, 1);
        val = 1;
    }

    int Wait()
    {
        lock.acquire();
        while (atomic_load(&val) <= 0)
            pause();
        atomic_fetch_sub(&val, 1);
        lock.release();
        return 0;
    }

    int Signal()
    {
        return atomic_fetch_add(&val, 1);
    }

private:
    atomic_uint val;
    spinlock_t  lock;
};

using sem_t = Semaphore;



// References:
//   https://en.wikipedia.org/wiki/Ticket_lock
//   https://en.wikipedia.org/wiki/Fetch-and-add
//   https://wiki.osdev.org/Spinlock
// SimpleMutex may have issues with fairness, so this might be a more fair version, yet relatively simple
class TicketLock
{
public:
    TicketLock()
    {
        next_ticket = 0;
        now_serving = 0;
    }

    void Lock()
    {
        uint32_t my_ticket = atomic_fetch_add(&next_ticket, 1);
        while (atomic_load(&now_serving) != my_ticket)
            pause(); // spin
    }

    void UnLock()
    {
        atomic_fetch_add(&now_serving, 1);
    }

private:
    atomic_uint next_ticket;
    atomic_uint now_serving;
};


// Based on psuedo code from:
//      https://en.wikipedia.org/wiki/Eisenberg_%26_McGuire_algorithm
template <size_t MAX_CONCURRENT>
class CriticalSection
{
public:
    enum State
    {
        IDLE,
        ACTIVE,
        WAITING
    };

    CriticalSection()
    {
        for (size_t i = 0; i < MAX_CONCURRENT; ++i)
            flags[i] = WAITING;
        activeIndex = 0;
    }

    // https://en.wikipedia.org/wiki/Peterson%27s_algorithm
    void SimpleEnter(size_t currentIndex)
    {
        flags[currentIndex] = ACTIVE;

        // TODO: have to check all others, not just one other
        size_t other = next(currentIndex);
        activeIndex = other;
        while (flags[other] == ACTIVE && activeIndex == other)
            pause(); // busy wait
    }

    void SimpleLeave(size_t currentIndex)
    {
        flags[currentIndex] = IDLE;
    }

    void Enter(size_t currentIndex)
    {
        for (;;)
        {
            // Announce we want to acquire the CS
            flags[currentIndex] = WAITING;

            // Scan from activeIndex to ourself for IDLE (possibly this breaks the deadlock of two processes calling enter on an unclaimed CS)
            size_t index = activeIndex;
            while (index != currentIndex)
                index = (flags[index] != IDLE) ? activeIndex : next(index);

            // Tentatively claim the CS (tells the active process it can pass the CS to us if it leaves)
            flags[currentIndex] = ACTIVE;

            // Find any other active processes
            for (index = 0; (index < MAX_CONCURRENT) && ((index == currentIndex) || (flags[index] != ACTIVE)); ++index)
                pause();

            // Keep going unless the active process passed the CS to us, or the active index is marked IDLE (it didn't find anyone to pass it to)
            if ((index >= MAX_CONCURRENT) && ((activeIndex == currentIndex) || (flags[activeIndex] == IDLE)))
                break;
        }
        activeIndex = currentIndex;
    }

    void Leave(size_t currentIndex)
    {
        // Search for next waiting / non-idle index (which could wrap back to ourselves if none waiting)
        size_t index = next(activeIndex);
        while (flags[index] == IDLE)
            index = next(index);

        // Pass the active index to next that is non-idle (or ourself if none waiting)
        activeIndex = index;
        flags[currentIndex] = IDLE;
    }

private:
    State   flags[MAX_CONCURRENT];   // Potentially could bit-pack the enums tighter together
    size_t  activeIndex;

    static inline constexpr
    size_t next(size_t i)
    {
        return (i + 1) % MAX_CONCURRENT;
    }
};

