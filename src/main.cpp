#include <iostream>
#include <vector>
#include <algorithm>
#include <stdexcept>
#include <memory>
#include <sys/time.h>

using std::cout;
using std::endl;
 
class Timer
{
public:
    Timer(): begin_(getUsec()) {}
    unsigned long long getTime() const { return getUsec() - begin_; }
private:
    static unsigned long long getUsec()
    {
        timeval tv;
        const int res = ::gettimeofday(&tv, 0);
        if(res)
            return 0;
        return tv.tv_usec + 1000000 * tv.tv_sec;
    }
    unsigned long long begin_;
};
 
struct OptionOrder
{
    struct Option
    {
        // each option has the name of platform, the cost of each individual option unit, 
        // the amount of underlying each unit the option is for, and the total quantity of 
        // units available for purchase (in units) the platform has based on liquidity.
        std::string name;
        unsigned c, a, qty;
        Option(): c(), a(), qty() {}
        Option(const std::string& iname, unsigned ic, unsigned ia, unsigned iqty):
            name(iname), c(ic), a(ia), qty(iqty)
        {}
    };
    typedef std::vector<Option> Options;
    struct Solution
    {
        unsigned a, c;
        unsigned long long iterations, usec;
        std::vector<unsigned> n;
        Solution(): a(), c(), iterations(), usec() {}
    };
    //...
    OptionOrder(): maxCost_(), totalCost_() {}
    void add(const Option& option)
    {
        const unsigned totalOptionCost = option.c * option.qty;
        if(const bool invalidOption = !totalOptionCost)
            throw std::logic_error("Invalid option: " + option.name);
        totalCost_ += totalOptionCost;
        options_.push_back(option);
    }
    const Options& getOptions() const { return options_; }
    void setMaxCost(unsigned maxCost) { maxCost_ = maxCost; }
    unsigned getMaxCost() const { return std::min(totalCost_, maxCost_); }
 
private:
    unsigned maxCost_, totalCost_;
    Options options_;
};
 
class OrderRecSolver
{
public:
    typedef OptionOrder Task;
    typedef Task::Option Option;
    typedef Task::Options Options;
    typedef Task::Solution Solution;
 
    void solve(const Task& task)
    {
        Impl(task, solution_).solve();
    }
    const Solution& getSolution() const { return solution_; }
private:
    class Impl
    {
        struct Candidate
        {
            unsigned a, n;
            bool visited;
            Candidate(): a(), n(), visited(false) {}
        };
        typedef std::vector<Candidate> Cache;
    public:
        Impl(const Task& task, Solution& solution):
            options_(task.getOptions()),
            maxCost_(task.getMaxCost()),
            maxColumnIndex_(task.getOptions().size() - 1),
            solution_(solution),
            cache_(task.getMaxCost() * task.getOptions().size()),
            iterations_(0)
        {}
        void solve()
        {
            if(const bool nothingToSolve = !maxCost_ || options_.empty())
                return;
            Timer timer;
            Candidate candidate;
            solve(candidate, maxCost_, options_.size() - 1);
            convertToSolution(candidate);
            solution_.usec = timer.getTime();
        }
    private:
        void solve(Candidate& current, unsigned reminderCost, const unsigned optionIndex)
        {
            ++iterations_;
 
            const Option& option(options_[optionIndex]);
 
            if(const bool firstColumn = !optionIndex)
            {
                const unsigned maxQty = std::min(option.qty, reminderCost/option.c);
                current.a = option.a * maxQty;
                current.n = maxQty;
                current.visited = true;
            }
            else
            {
                const unsigned nextOptionIndex = optionIndex - 1;
                {
                    Candidate& nextOption = cachedOption(reminderCost, nextOptionIndex);
                    if(!nextOption.visited)
                        solve(nextOption, reminderCost, nextOptionIndex);
                    current.visited = true;
                    current.a = nextOption.a;
                    current.n = 0;
                }
                if(reminderCost >= option.c)
                {
                    for (unsigned numberOfOptions = 1; numberOfOptions <= option.qty; ++numberOfOptions)
                    {
                        reminderCost -= option.c;
                        Candidate& nextOption = cachedOption(reminderCost, nextOptionIndex);
                        if(!nextOption.visited)
                            solve(nextOption, reminderCost, nextOptionIndex);
 
                        const unsigned checkAmount = nextOption.a + numberOfOptions * option.a;
                        if ( checkAmount > current.a)
                        {
                            current.a = checkAmount;
                            current.n = numberOfOptions;
                        }
                        if(!(reminderCost >= option.c))
                            break;
                    }
                }
            }
        }
        void convertToSolution(const Candidate& candidate)
        {
            solution_.iterations = iterations_;
            solution_.a = candidate.a;
            solution_.n.resize(options_.size());
 
            const Candidate* iter = &candidate;
            unsigned cost = maxCost_, optionIndex = options_.size() - 1;
            while(true)
            {
                const unsigned currentCost = iter->n * options_[optionIndex].c;
                solution_.n[optionIndex] = iter->n;
                cost -= currentCost;
                if(!optionIndex--)
                    break;
                iter = &cachedOption(cost, optionIndex);
            }
            solution_.c = maxCost_ - cost;
        }
        Candidate& cachedOption(unsigned cost, unsigned optionIndex)
        {
            return cache_[cost * maxColumnIndex_ + optionIndex];
        }
        const Options& options_;
        const unsigned maxCost_;
        const unsigned maxColumnIndex_;
        Solution& solution_;
        Cache cache_;
        unsigned long long iterations_;
    };
    Solution solution_;
};
 
void populateDataset(OptionOrder& task)
{
    typedef OptionOrder::Option Option;
    task.setMaxCost( 400 );
    task.add(Option("fnx",9,2,1));
    task.add(Option("actus",13,10,1));
    task.add(Option("premia",153,1,2));
    task.add(Option("opyn",50,2,2));
    task.add(Option("hegic",15,1,2));
    task.add(Option("hedget",68,1,3));
}
 
int main()
{
    OptionOrder task;
    populateDataset(task);
 
    OrderRecSolver solver;
    solver.solve(task);
    const OptionOrder::Solution& solution = solver.getSolution();
 
    cout << "Iterations to solve: " << solution.iterations << endl;
    cout << "Time to solve: " << solution.usec << " usec" << endl;
    cout << "Solution:" << endl;
    for (unsigned i = 0; i < solution.n.size(); ++i)
    {
        if (const bool optionIsNotInOrder = !solution.n[i])
            continue;
        cout << "  " << solution.n[i] << ' ' << task.getOptions()[i].name << " ( option cost = " << task.getOptions()[i].c << " )" << endl;
    }
 
    cout << "Cost: " << solution.c << " Amount: " << solution.a << endl;
    return 0;
}
