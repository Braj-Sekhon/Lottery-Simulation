# Lottery Simulation

### Description

I made this project just for fun a long time ago. I picked it up recently to experiment with multi-threading to improve performance.

The program simulates two lotteries (8 numbers each from 1-49) continuously drawing until both have matching winning numbers. Probability: 1 in 450,978,066. Through iterative optimization, I improved performance from 731,676 computes/second (single-threaded) to 4,896,160 computes/second (multi-threaded with batching)—a 6.69× speedup.

### How to run

**BUILD:**
`g++ ./main.cpp`
**RUN:**
`./a.out`

### What I learned

This project served me best as a testing grounds for multi-threading. I learned that more threads ≠ better performance.

- Multi-threading overhead can backfire drastically.
- It's about _how_ the multi-threading is implemented using: locks for array manipulation (std::mutex), variables made for multi-threading (std::atomic), batch updates to mitigate lock contention, and avoid making new threads as much as possible — let all the repetitive work be handled _within_ threads, not _as_ threads.
- There's a sweet spot with batch size and thread count, allowing for massive gains in performance.
