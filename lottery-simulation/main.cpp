#include <atomic>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <mutex>
#include <random>
#include <thread>

using std::atomic, std::lock_guard;
using std::mt19937;
using std::mutex;
using std::printf;
using std::random_device;
using std::thread;
using std::uniform_int_distribution;

alignas(64) const uint8_t maxNumber = 49;
const uint8_t numbersInALottery = 8;
const uint16_t batching_size = 2500;
const uint8_t threadCount = 8;
const char padding0[59]{};
alignas(64) atomic<uint64_t> winningBits = 0;
alignas(64) atomic<bool> matchReached{false};
const char padding1[63]{};
alignas(64) atomic<uint64_t> drawings{0};
const char padding2[56]{};
alignas(64) uint64_t matchResults[numbersInALottery + 1]{};
const char padding3[64 - ((numbersInALottery + 1) * 8) % 64]{};
alignas(64) mutex resultsMutex;

// Array that is pre-allocated and shared among all threads when generating
// random numbers; each thread gets 64 bytes to avoid false sharing, meaning
// some bytes are there simply for padding
alignas(64) uint8_t randomNumbers[threadCount * 64];

void listResults() {
  lock_guard<mutex> lock(resultsMutex);
  for (int i = 0; i < numbersInALottery + 1; i++) {
    printf("Drawings with %u match: %lu\n", i, matchResults[i]);
  }
};

void randomizeDist(uint64_t *dist, uint8_t threadNumber) {
  thread_local random_device rd;
  thread_local mt19937 gen(rd());
  thread_local uniform_int_distribution<> distrib(1, maxNumber);
  uint8_t numbersPicked = 0;
  *dist = 0;
  while (numbersPicked < numbersInALottery) {
    uint8_t randomNumber = distrib(gen);
    bool duplicateFound = false;
    for (int i = 0; i < numbersPicked; ++i) {
      if (randomNumbers[threadNumber * 64 + i] == randomNumber) {
        duplicateFound = true;
        break;
      }
    }
    if (duplicateFound) {
      continue;
    }
    randomNumbers[threadNumber * 64 + numbersPicked] = randomNumber;
    uint64_t randomBit = (uint64_t)1 << (randomNumber - 1);
    *dist |= randomBit;
    ++numbersPicked;
  }
}

uint8_t getMatchingBetween(uint64_t lotto1, uint64_t lotto2) {
  return std::__popcount(lotto1 & lotto2);
}

void runLotteryWorker(uint8_t threadNumber = 0) {
  uint64_t lotto1{0}, lotto2{0};
  unsigned int local_drawings = 0;
  unsigned int local_MatchResults[numbersInALottery + 1]{};

  // Batch
  auto addToGlobalResult = [&local_MatchResults]() {
    lock_guard<mutex> lock(resultsMutex);
    for (int i = 0; i < numbersInALottery + 1; i++) {
      matchResults[i] += local_MatchResults[i];
      local_MatchResults[i] = 0;
    }
  };

  while (matchReached == false) {
    randomizeDist(&lotto1, threadNumber);
    randomizeDist(&lotto2, threadNumber);

    int numOfMatches = getMatchingBetween(lotto1, lotto2);
    ++local_MatchResults[numOfMatches];
    ++local_drawings;
    if (local_drawings >= batching_size) {
      drawings += batching_size;
      addToGlobalResult();
      local_drawings -= batching_size;
    }

    if (numOfMatches == numbersInALottery) {
      winningBits = lotto1 | lotto2;
      matchReached = true;
      break;
    }
  }
  // If a different thread finds the matching thing, then we have to add the
  // remaining local drawings and results because they're batched otherwise so
  // this is an accurate count
  addToGlobalResult();
  drawings += local_drawings;
}

int main() {
  for (int i = 0; i < threadCount; i++) {
    thread worker(runLotteryWorker, i);
    worker.detach();
  }

  const auto start = std::chrono::steady_clock::now();
  auto lastPrint = start;
  while (matchReached == false) {
    auto now = std::chrono::steady_clock::now();
    if (now - lastPrint > std::chrono::milliseconds(100)) {
      std::cout << "It has been " << drawings << " drawings.\n";
      listResults();
      lastPrint = now;
    }
  }
  std::cout << "Drawings until win: " << drawings << '\n';
  printf("WINNING LOTTERY:");
  for (uint8_t i = 1; i <= maxNumber; i++) {
    const uint64_t iToNum = uint64_t(1) << (i - 1);
    if (winningBits & iToNum) {
      printf("%u ", i);
    }
  }
  printf("!!!\n");
  listResults();
  return 0;
}
