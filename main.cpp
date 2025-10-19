#include <iostream>
#include <random>
#include <thread>
#include <atomic>
#include <mutex>

using std::atomic;
using std::cout;
using std::lock_guard;
using std::mt19937;
using std::mutex;
using std::random_device;
using std::thread;
using std::uniform_int_distribution;

// Parameters for the user
const int maxNumber = 49;
const int numbersInALottery = 8;
const int batching_size = 2500;
const int threadCount = 8;

// Functional variables
atomic<bool> matchReached{false};
atomic<unsigned long long int> drawings{0};
unsigned long long int matchResults[numbersInALottery + 1]{};
mutex resultsMutex;

// Prints out how many drawings have had a match x numbers, y numbers, ... and z numbers
void listResults()
{
    lock_guard<mutex> lock(resultsMutex);
    for (int i = 0; i < numbersInALottery + 1; i++)
    {
        cout << "Drawings with " << i << " match: " << matchResults[i] << '\n';
    }
};

// Gets a random number from 1 to maxNumber
int randNum()
{
    thread_local random_device rd;
    thread_local mt19937 gen(rd());
    thread_local uniform_int_distribution<> distrib(1, maxNumber);
    return distrib(gen);
}

// isNumberInListExcept is defined earlier here just to allow the function to be used within shuffleNumbers; avoids compile-time error

bool isNumberInListExcept(const int *drawnNumbers, int Num, int IgnoreIndex);

struct lotteryDrawing
{
    int drawnNumbers[numbersInALottery];
};

// Takes in an integer array and randomizes each entry from index 0 to [numbersInALottery-1]
void shuffleNumbers(int *drawnNumbers)
{
    for (int i = 0; i < numbersInALottery; i++)
    {
        int n = 0;
        do
        {
            n = randNum();
        } while (isNumberInListExcept(drawnNumbers, n, i) == true);
        drawnNumbers[i] = n;
    }
};

// Checks if a [num] is in the integer array [drawnNumbers], ignoring the index of [Ignoreindex]
bool isNumberInListExcept(const int *drawnNumbers, int Num, int IgnoreIndex)
{
    for (int i = 0; i < numbersInALottery; i++)
    {
        if (i == IgnoreIndex)
        {
            continue;
        }
        else
        {
            if (drawnNumbers[i] == Num)
            {
                return true;
            }
        }
    }
    return false;
}

// Checks if [Num] is in [List]
bool isNumberInDrawing(const lotteryDrawing &List, int Num)
{
    for (int n : List.drawnNumbers)
    {
        if (Num == n)
        {
            return true;
        }
    }
    return false;
}

// Returns the # of matching numbers between [L1] and [L2]
int getMatchingBetween(const lotteryDrawing &L1, const lotteryDrawing &L2)
{
    int matches = 0;
    for (int n : L1.drawnNumbers)
    {
        if (isNumberInDrawing(L2, n))
        {
            ++matches;
        }
    }
    return matches;
}

// This function is to be multi-threaded. This is what runs the actual lottery simulation. [Announcer] just says if this thread is going to be the one who announces updates like how many numbers matched.
void runLotteryWorker(bool announcer = false)
{
    lotteryDrawing lotto1, lotto2;
    unsigned int local_drawings = 0;
    unsigned int local_MatchResults[numbersInALottery + 1]{};

    // Batch updates to reduce lock contention
    auto addToGlobalResult = [&local_MatchResults]()
    {
        lock_guard<mutex> lock(resultsMutex);
        for (int i = 0; i < numbersInALottery + 1; i++)
        {
            matchResults[i] += local_MatchResults[i];
            local_MatchResults[i] = 0;
        }
    };

    // The meat of the simulation
    while (matchReached == false)
    {
        shuffleNumbers(lotto1.drawnNumbers);
        shuffleNumbers(lotto2.drawnNumbers);

        int numOfMatches = getMatchingBetween(lotto1, lotto2);
        ++local_MatchResults[numOfMatches];
        ++local_drawings;
        if (local_drawings == batching_size)
        {
            drawings += batching_size;
            addToGlobalResult();
            local_drawings = 0;
        }

        if (announcer == true && drawings != 0 && drawings % 100000 == 0)
        {
            cout << "It has been " << drawings << " drawings.\n";
            listResults();
        }

        if (numOfMatches == numbersInALottery)
        {
            cout << "WINNING LOTTERY:";
            for (int i = 0; i < numbersInALottery; i++)
            {
                cout << " " << lotto1.drawnNumbers[i];
            }
            cout << "!!!\n";
            matchReached = true;
            break;
        }
    }

    // If a different thread finds the matching thing, then we have to add the remaining local drawings and results because they're batched otherwise so this is an accurate count
    addToGlobalResult();
    drawings += local_drawings;
}

int main()
{
    for (int i = 0; i < threadCount; i++)
    {
        if (i == 0)
        {
            thread worker(runLotteryWorker, true);
            worker.detach();
        }
        else
        {
            thread worker(runLotteryWorker, false);
            worker.detach();
        }
    }

    while (matchReached == false)
    {
        // Busy-wait until match found.
    }
    cout << "Drawings until win: " << drawings << '\n';
    listResults();
    return 0;
}
