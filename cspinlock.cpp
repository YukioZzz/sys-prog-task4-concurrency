#include <atomic>
#include "cspinlock.h"
struct cspinlock {
    std::atomic<bool> lock_ = {false};
    void lock() { while(lock_.exchange(true, std::memory_order_acquire)); }
    bool trylock() noexcept {
        return !lock_.load(std::memory_order_relaxed) &&
               !lock_.exchange(true, std::memory_order_acquire);
    }
    void unlock() noexcept {
        lock_.store(false, std::memory_order_release);
    }
};
typedef struct cspinlock cspinlock_t;

//acquire the lock
int cspin_lock(cspinlock_t *slock){
    slock->lock();
    return 0;
}
//if the lock can not be acquired, return immediately
int cspin_trylock(cspinlock_t *slock){
    slock->trylock();
    return 0;
}
//release the lock
int cspin_unlock(cspinlock_t *slock){
    slock->unlock();
    return 0;
}

//allocate a lock
cspinlock_t* cspin_alloc(){
    cspinlock_t* slock = new cspinlock_t; 
    return slock;
}

//free a lock
void cspin_free(cspinlock_t* slock){
    delete slock;
}
