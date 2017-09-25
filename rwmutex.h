#pragma once
#include<mutex>
#include<condition_variable>

using namespace std;

/* Read-Write Mutex: Allows for multiple readers XOR one writer
 *
 * If a write lock is requested, no further read locks will be granted
 * to prevent write starvation.
 *
 * The RLock and WLock classes can be used to obtain a read or write lock
 * on construction and release their lock on destruction.
 * This behavior is similar to std::unique_lock.
 */
class RWmutex
{
  private:
    /* Possible states:
     * writer = false && readers = 0: no lock held; next lock request succeeds
     * writer = false && readers > 0: at least one read lock held
     * writer = true  && readers = 0: write lock held
     * writer = true  && readers > 0: write lock requested; no new read locks granted
     */
    unsigned int readers;
    bool writer;

    mutex m; /* mutex for class variables */
    condition_variable read_cv; /* Signals when read locks may be granted */
    condition_variable write_cv; /* Signals when a write lock may be granted */

  public:
    RWmutex() : readers(0), writer(false) {}

    RWmutex(const RWmutex& m) = delete; /* no copying allowed */

    RWmutex& r_lock()
    {
      unique_lock<mutex> lck(m); // Lock m; unlock at exit of function
      while(writer) // Wait until there is no write lock
      {
        read_cv.wait(lck);
      }

      readers++; // Obtain read lock
      return *this;
    }

    RWmutex& w_lock()
    {
      unique_lock<mutex> lck(m);

      while(writer) // Wait until there is no write lock
      {
        write_cv.wait(lck);
      }

      writer = true; // Request write lock...

      if(readers > 0) // Wait until there are no read locks
      {
        while(readers > 0)
        {
          write_cv.wait(lck);
        }
      }
      // Write lock obtained!

      return *this;
    }

    void r_unlock()
    {
      unique_lock<mutex> lck(m);
      if(readers > 0) // Hedge against double-unlocks
      {
        readers--;
      }
      if(readers == 0 && writer) // Check if we can grant a write lock
      {
        write_cv.notify_all();
      }
    }

    void w_unlock()
    {
      unique_lock<mutex> lck(m);
      writer = false;
      read_cv.notify_all();
      write_cv.notify_all();
    }
};

/* Obtains a read lock on an RWmutex when constructed
 * and releases the lock when destructed.
 */
class Rlock
{
  private:
    RWmutex& m;

  public:
    Rlock(RWmutex& mtx) : m(mtx.r_lock()) {}

    Rlock(const Rlock&) = delete;

    ~Rlock()
    {
      m.r_unlock();
    }
};

/* Obtains a write lock on an RWmutex when constructed
 * and releases the lock when destructed.
 */
class Wlock
{
  private:
    RWmutex& m;

  public:
    Wlock(RWmutex& mtx) : m(mtx.w_lock()) {}

    Wlock(const Wlock&) = delete;

    ~Wlock()
    {
      m.w_unlock();
    }
};
