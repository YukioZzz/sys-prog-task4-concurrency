typedef struct cspinlock cspinlock_t;

//acquire the lock
int cspin_lock(cspinlock_t *slock);

//if the lock can not be acquired, return immediately
int cspin_trylock(cspinlock_t *slock);

//release the lock
int cspin_unlock(cspinlock_t *slock);

//allocate a lock
cspinlock_t* cspin_alloc();

//free a lock
void cspin_free(cspinlock_t* slock);
