# Lottery Simulation

## About

The program simulates two lotteries (8 numbers each from 1-49) continuously drawing until both have matching winning numbers.

It is purely an experiment to see how much power I can squeeze out of my system for calculations, serving as a testing grounds for me to apply concepts I've learned about how to optimize compute-intensive applications.

[About](#about)

[Revamps](#revamps)

- [Revamp #1](#september-2025-revamp-1)

- [Revamp #2](#july-2026-revamp-2)

[Development](#development)

- [Build and Run](#build-and-run)

[Reflection](#reflection)

- [What I learned](#what-i-learned)

- [What I would change](#what-i-would-change)

[Final Results](#final-results)

## Revamps

### September 2025, Revamp #1

I made this project just for fun back in November 2024. I picked it up again recently to experiment with the concept of multi-threading to improve performance.

By implementing multi-threading, I improved the performance of my application drastically.

**Results**

- Old, single-threaded: 731,676 computes/second
- New, multi-threaded: 4,896,160 computes/second, a 6.69× speedup

### July 2026, Revamp #2

Now I've come back once more to see what more can I do to squeeze compute out of this project.

**Results:**

- Old: 5-6 million computes/s,
- New: 9-13 million computes/s, basically a 2x speedup
- New with -O3 compiler optmization flag: ~50 million computes/s, a total of 10x speedup from the old code

**What I changed:**

- **Applied cache line alignment** to reduce potential false sharing between threads, especially with a new array called "randomNumbers" which is shared between all threads
- **Changed lottery draws from integer arrays to single unsigned 64 bit integers**, where each bit corresponds to a lotto number (bit 0 represents number 1, bit 5 represents number 6, etc.)
- **Lottery comparisons are done with a bitwise operator**, using the "and" operator, I cut down the comparison algorithm's time from O(N^2) to O(1)
- **Offloaded I/O work to the main thread**, as I realized it was in a spinlock for the bulk of the program's execution; offloading this I/O work reduced the burden on the worker threads, especially the announcer thread

## Development

### Build and Run

**BUILD:**

```bash
g++ ./main.cpp
```

With aggressive compiler optimizations:

```bash
g++ ./main.cpp -O3
```

**RUN:**

```bash
./a.out
```

## Reflection

### What I learned

From the tests I've conducted with this project, I've learned about:

**Muli-threading basics**:

- Multi-threading overhead can backfire drastically.
- It's about _how_ the multi-threading is implemented using: locks for array manipulation (`std::mutex`), variables made for multi-threading (`std::atomic`), batch updates to mitigate lock contention, and avoid constant spawning of threads at runtime; let loops run _within_ threads not _as_ threads.
- There's a sweet spot with batch size and thread count, allowing for massive gains in performance.

**False sharing and workload optimization**:

- Align data structures so that they rest on cache lines, and do not share cache lines with multiple threads (padding a critical, shared array to avoid the significant overhead that comes with false sharing)
- Reducing how many if statements I use, particularly in resource-heavy threads (like removing the spinlock on my main, idle thread so it can instead handle I/O while worker threads focus purely on computing and not constantly checking an if statement related to I/O)

### What I would change

- Removing the `std::mutex` lock and replacing it with a lock-free data structure, avoiding the massive overhead that comes from the OS waking and sleeping threads
- Replacing the O(N^2) checking algorithm inside `void randomizeDist()` with a more efficient one

## Final Results

- V0 to V1: 6.69x speedup
- V1 to V2: 2-2.5x speedup
- V2 vs V2 w/ -O3: 5x speedup
