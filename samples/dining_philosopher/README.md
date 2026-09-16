## Dining Philosopher

An implementation of a solution to the Dining Philosophers problem (a classic multi-thread synchronization problem). This particular implementation demonstrates the usage of multiple preemptible tasks of differing priorities, as well as mutexes and its priority inversion

The philosopher always tries to get the lowest fork first (f1 then f2). When done, he will give back the forks in the reverse order (f2 then f1). If he gets two forks, he is EATING. Otherwise, he is THINKING. Transitional states are shown as well, such as HUNGRY when the philosopher is hungry but the forks are not available, and HOLDING ONE FORK when a philosopher is waiting for the second fork to be available.

Each Philosopher will randomly alternate between the EATING and THINKING states based on a random delay generator and availability of the forks.
