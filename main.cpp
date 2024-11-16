#include <iostream>
#include <random>
#include <ctime>

using std::random_device, std::mt19937, std::uniform_int_distribution, std::cout, std::time;

const int maxNumber = 64;
const int numbersInALottery = 5;

int randNum()
{
    static int additive = 0;
    random_device rd;
    mt19937 gen(rd());
    gen.seed(time(0)+additive);
    uniform_int_distribution<> distrib(1, maxNumber);
    ++additive;
    return distrib(gen);
}

struct lotteryDrawing;

bool isNumberInListExcept(const lotteryDrawing &List, int Num, int IgnoreIndex);

struct lotteryDrawing {
    int drawnNumbers[numbersInALottery];

    lotteryDrawing()
    {
        for(int i = 0; i < numbersInALottery; i++)
        {
            int n = 0;
            do
            {
                n=randNum();
            }while(isNumberInListExcept(*this, n, i)==true);
            drawnNumbers[i]=n;
        }
    };
};

bool isNumberInListExcept(const lotteryDrawing &List, int Num, int IgnoreIndex)
{
    for(int i = 0; i < numbersInALottery; i++)
    {
        if(i==IgnoreIndex){
            continue;
        }else{
            if(List.drawnNumbers[i]==Num)
            {
                return true;
            }
        }
    }
    return false;
}

bool isNumberInDrawing(const lotteryDrawing &List, int Num)
{
    for(int n : List.drawnNumbers)
    {
        if(Num==n)
        {
            return true;
        }
    }
    return false;
}

int getMatchingBetween(const lotteryDrawing &L1, const lotteryDrawing &L2)
{
    int matches = 0;
    for(int n : L1.drawnNumbers)
    {
        if(isNumberInDrawing(L2, n))
        {
            ++matches;
        }
    }
    return matches;
}

int main()
{
    int drawings = 0;
    int matchResults[numbersInALottery+1];

    auto listResults = [&matchResults]()
    {
        for(int i = 0; i < numbersInALottery+1; i++)
        {
           cout << "Drawings with " << i << " match: " << matchResults[i] << '\n';
        }
    };

    while(true)
    {
        ++drawings;
        lotteryDrawing lotto1 = lotteryDrawing();
        lotteryDrawing lotto2 = lotteryDrawing();
        int numOfMatches = getMatchingBetween(lotto1, lotto2);
        ++matchResults[numOfMatches];

        if(drawings%10000==0)
        {
            cout << "It has been " << drawings << " drawings.\n";
            listResults();
        }

        if(numOfMatches==numbersInALottery)
        {
            cout << "WINNING LOTTERY:";
            for(int i = 0; i < numbersInALottery; i++)
            {
                cout << " " << lotto1.drawnNumbers[i];
            }
            cout << "!!!\n";
            break;
        }
    }
    cout << "Drawings until win: " << drawings << '\n';
    listResults();
    return 0;
}
