#include <fstream>
#include <iostream>
#include <vector>
#include <random>
#include <string>
#include <map>
#include <queue>
#define Infinity 999999

int N;
float simulationTime;
int currentTime;

using namespace std;

enum Color { RED, BLACK };

class Process
{
private:
    int processId;
    int arrivalTime;
    int burstTime;
    int completionTime;
    int turnAroundTime;
    int waitingTime;
    int responseTime = Infinity;
    int leftBurstTime;
    int virtualRunTime;
    int timeSlice;
    int staticPriority;

public:
    Process()
    {
    }

    ~Process()
    {
    }

    Process(int pid, int aTime, int bTime, int nice)
    {
        processId = pid;
        arrivalTime = aTime;
        burstTime = bTime;
        leftBurstTime = bTime;
        staticPriority = 120 + nice;
        virtualRunTime = 0;
        
        if (staticPriority < 110 && staticPriority >= 100)
        {
            timeSlice = 4;
        }
        else if (staticPriority < 120 && staticPriority >= 110)
        {
            timeSlice = 3;
        }
        else if (staticPriority < 130 && staticPriority >= 120)
        {
            timeSlice = 2;
        }
        else
        {
            timeSlice = 1;
        }
    }

    void setCompletionTime(int cTime)
    {
        completionTime = cTime;
    }

    void setTurnAroundTime()
    {
        turnAroundTime = completionTime - arrivalTime;
    }

    void setWaitingTime()
    {
        waitingTime = turnAroundTime - burstTime;
    }

    void setResponseTime(int resTime)
    {
        responseTime = resTime - arrivalTime;
    }

    void print()
    {
        cout << "\n Process Id : " << processId << endl;
        cout << " Nice : " << staticPriority - 120 << endl;
        cout << " Arrival Time : " << arrivalTime << " ms" << endl;
        cout << " Burst Time : " << burstTime << " ms" << endl;
        cout << " Completion Time : " << completionTime << " ms" << endl;
        cout << " Turn Around Time : " << turnAroundTime << " ms" << endl;
        cout << " Waiting Time : " << waitingTime << " ms" << endl;
        cout << " Response Time : " << responseTime << " ms" << endl;
    }

    friend class Process_Creator;
    friend class Scheduler;
    friend class MinHeap;
    friend bool compare(const Process &p1, const Process &p2);
    friend class Simulator;
    friend class RBTree;
    void friend callAlgo();
};

class RBNode {
public:
    Process data;
    Color color;
    RBNode *left, *right, *parent;

    RBNode(Process data) : data(data), color(RED), left(nullptr), right(nullptr), parent(nullptr) {}
};

class RBTree {
private:
    RBNode *root;

    void rotateLeft(RBNode *&x) {
        RBNode *y = x->right;
        x->right = y->left;
        if (y->left != nullptr)
            y->left->parent = x;
        y->parent = x->parent;
        if (x->parent == nullptr)
            root = y;
        else if (x == x->parent->left)
            x->parent->left = y;
        else
            x->parent->right = y;
        y->left = x;
        x->parent = y;
    }

    void rotateRight(RBNode *&x) {
        RBNode *y = x->left;
        x->left = y->right;
        if (y->right != nullptr)
            y->right->parent = x;
        y->parent = x->parent;
        if (x->parent == nullptr)
            root = y;
        else if (x == x->parent->right)
            x->parent->right = y;
        else
            x->parent->left = y;
        y->right = x;
        x->parent = y;
    }

    void fixViolation(RBNode *&pt) {
        RBNode *parent_pt = nullptr;
        RBNode *grand_parent_pt = nullptr;

        while ((pt != root) && (pt->color != BLACK) && (pt->parent->color == RED)) {
            parent_pt = pt->parent;
            grand_parent_pt = pt->parent->parent;

            if (parent_pt == grand_parent_pt->left) {
                RBNode *uncle_pt = grand_parent_pt->right;
                if (uncle_pt != nullptr && uncle_pt->color == RED) {
                    grand_parent_pt->color = RED;
                    parent_pt->color = BLACK;
                    uncle_pt->color = BLACK;
                    pt = grand_parent_pt;
                } else {
                    if (pt == parent_pt->right) {
                        rotateLeft(parent_pt);
                        pt = parent_pt;
                        parent_pt = pt->parent;
                    }
                    rotateRight(grand_parent_pt);
                    swap(parent_pt->color, grand_parent_pt->color);
                    pt = parent_pt;
                }
            } else {
                RBNode *uncle_pt = grand_parent_pt->left;
                if (uncle_pt != nullptr && uncle_pt->color == RED) {
                    grand_parent_pt->color = RED;
                    parent_pt->color = BLACK;
                    uncle_pt->color = BLACK;
                    pt = grand_parent_pt;
                } else {
                    if (pt == parent_pt->left) {
                        rotateRight(parent_pt);
                        pt = parent_pt;
                        parent_pt = pt->parent;
                    }
                    rotateLeft(grand_parent_pt);
                    swap(parent_pt->color, grand_parent_pt->color);
                    pt = parent_pt;
                }
            }
        }
        root->color = BLACK;
    }

    RBNode* minValueNode(RBNode* node) {
        RBNode* current = node;
        while (current->left != nullptr)
            current = current->left;
        return current;
    }

    RBNode* deleteNode(RBNode* root, Process data) {
        if (root == nullptr)
            return root;

        if (compare(data, root->data))
            root->left = deleteNode(root->left, data);
        else if (compare(root->data, data))
            root->right = deleteNode(root->right, data);
        else {
            if (root->left == nullptr || root->right == nullptr) {
                RBNode *temp = root->left ? root->left : root->right;
                if (temp == nullptr) {
                    temp = root;
                    root = nullptr;
                } else {
                    *root = *temp;
                }
                delete temp;
            } else {
                RBNode* temp = minValueNode(root->right);
                root->data = temp->data;
                root->right = deleteNode(root->right, temp->data);
            }
        }

        if (root == nullptr)
            return root;

        // TODO: Fix deletion violations if needed
        return root;
    }

public:
    RBTree() : root(nullptr) {}

    void insert(Process data) {
        RBNode *pt = new RBNode(data);
        root = BSTInsert(root, pt);
        fixViolation(pt);
    }

    RBNode* BSTInsert(RBNode* root, RBNode *pt) {
        if (root == nullptr)
            return pt;

        if (compare(pt->data, root->data)) {
            root->left = BSTInsert(root->left, pt);
            root->left->parent = root;
        } else if (compare(root->data, pt->data)) {
            root->right = BSTInsert(root->right, pt);
            root->right->parent = root;
        }

        return root;
    }

    Process getMin() {
        RBNode* current = root;
        while (current->left != nullptr)
            current = current->left;
        return current->data;
    }

    void deleteMin() {
        Process min = getMin();
        root = deleteNode(root, min);
    }

    bool empty() {
        return root == nullptr;
    }
};

class Process_Creator
{
private:
    vector<Process> *array = new vector<Process>();

public:
    Process_Creator()
    {
        array->resize(N);
        srand(time(0));

        for (int i = 0; i < N; i++)
        {
            random_device rd;
            mt19937 gen(rd());
            uniform_int_distribution<int> distArrTime(0, 9);
            uniform_int_distribution<int> distBurstTime(1, 9);
            uniform_int_distribution<int> distNice(-20, 19);

            int aTime = distArrTime(gen);
            int bTime = distBurstTime(gen);
            int nice = distNice(gen);

            array->at(i) = Process(i, aTime, bTime, nice);
        }
    }

    friend class Scheduler;
    friend class Simulator;
};

class MinHeap
{
private:
    vector<Process> *arr;
    int leftChild(int parent) { return 2 * parent + 1; }
    int rightChild(int parent) { return 2 * parent + 2; }
    int parent(int child) { return (child - 1) / 2; }

    void minHeapify(int parent)
    {
        int left = leftChild(parent);
        int right = rightChild(parent);
        int smallest = parent;

        if (left < arr->size() && arr->at(left).arrivalTime < arr->at(smallest).arrivalTime)
        {
            smallest = left;
        }
        else if (left < arr->size() && arr->at(left).arrivalTime == arr->at(smallest).arrivalTime)
        {
            if (arr->at(left).processId < arr->at(smallest).processId)
            {
                smallest = left;
            }
            else
            {
                smallest = parent;
            }
        }
        else
        {
            smallest = parent;
        }

        if (right < arr->size() && arr->at(right).arrivalTime < arr->at(smallest).arrivalTime)
        {
            smallest = right;
        }

        if (right < arr->size() && arr->at(right).arrivalTime == arr->at(smallest).arrivalTime)
        {
            if (arr->at(right).processId < arr->at(smallest).processId)
            {
                smallest = right;
            }
        }

        if (smallest != parent)
        {
            swap(arr->at(smallest), arr->at(parent));
            minHeapify(smallest);
        }
    }

public:
    MinHeap(vector<Process> *array)
    {
        arr = array;
        buildMinHeap();
    }

    void buildMinHeap()
    {
        for (int start = (arr->size() / 2) - 1; start >= 0; --start)
        {
            minHeapify(start);
        }
    }

    Process getMin() { return arr->at(0); }

    void pop()
    {
        arr->at(0) = arr->back();
        arr->pop_back();
        minHeapify(0);
    }

    bool empty() { return arr->empty(); }
};

bool compare(const Process &p1, const Process &p2)
{
    if(p1.virtualRunTime != p2.virtualRunTime)
    {
        return p1.virtualRunTime < p2.virtualRunTime;
    }
    
    if(p1.staticPriority != p2.staticPriority)
    {
        return p1.staticPriority < p2.staticPriority;
    }

    return p1.arrivalTime < p2.arrivalTime;
}

class Scheduler
{
private:
    vector<Process> *readyQueue = new vector<Process>();
    vector<Process> *array;

public:
    Scheduler(Process_Creator &obj)
    {
        array = obj.array;
    }

    void fcfs(vector<Process> *runningQueue);
    void rr(vector<Process> *runningQueue);
    void cfs(Process p, vector<Process> *runningQueue);

    friend class RBTree;
    friend class MinHeap;
    friend class Simulator;
};

void Scheduler::fcfs(vector<Process> *runningQueue)
{
    runningQueue->push_back(readyQueue->at(0));
    readyQueue->erase(readyQueue->begin());
}

void Scheduler::rr(vector<Process> *runningQueue)
{
    if (!readyQueue->empty())
    {
        runningQueue->push_back(readyQueue->at(0));
        readyQueue->erase(readyQueue->begin());

        if (runningQueue->back().responseTime == Infinity)
        {
            runningQueue->back().setResponseTime(currentTime);
        }
    }
}

void Scheduler::cfs(Process Q, vector<Process> *runningQueue)
{
    runningQueue->push_back(Q);

    if (runningQueue->back().responseTime == Infinity)
    {
        runningQueue->back().setResponseTime(currentTime);
    }
}

class Simulator
{
private:
    vector<Process> *array;
    vector<Process> *runningQueue = new vector<Process>();
    vector<Process> *readyQueue;
    Scheduler *sch;
    int timeQuantum, choice;

    void CaptureValues()
    {
        for (int i = 0; i < array->size(); i++)
        {
            array->at(i).setTurnAroundTime();
            array->at(i).setWaitingTime();
        }
    }

public:
    Simulator(Scheduler &schObj, int a)
    {
        readyQueue = schObj.readyQueue;
        array = schObj.array;
        sch = &schObj;
        choice = a;
    }

    void printProcesses()
    {
        for (int i = 0; i < array->size(); i++)
        {
            array->at(i).print();
        }
    }

    void Run()
    {    
        if (choice == 1)
        {
            runFCFS();
        }
        else if (choice == 2)
        {
            runRR();
        }
        else
        {
            runCFS();
        }

        for (int i = 0; i < runningQueue->size(); i++)
        {
            array->push_back(runningQueue->at(i));
        }

        delete readyQueue;
        delete runningQueue;

        CaptureValues();
        printProcesses();
        cout << "\n";
    }

    void runFCFS();
    void runRR();
    void runCFS();

    void friend callAlgo();
};

void Simulator::runFCFS()
{
    cout << "\n Hey! FCFS here\n"
         << endl;

    MinHeap heap(array);

    if (heap.getMin().arrivalTime != 0)
    {
        currentTime = heap.getMin().arrivalTime;
    }

    ofstream status_file("status.txt");
    status_file << "\n\tFirst Come First Serve: \n";
    status_file << "\n\tTime Stamp\tProcess Id\tStatus\n\n";

    while (currentTime < simulationTime)
    {
        while (!array->empty())
        {
            if (heap.getMin().arrivalTime <= currentTime)
            {
                readyQueue->push_back(heap.getMin());
                status_file << "\t" << readyQueue->back().arrivalTime << " ms"
                            << "\t\t" << readyQueue->back().processId << "\t\t\tArrived"
                            << "\n";
                heap.pop();
            }
            else
            {
                break;
            }
        }

        if (!runningQueue->empty())
        {
            status_file << "\t" << currentTime << " ms"
                        << "\t\t" << runningQueue->back().processId << "\t\t\tExit"
                        << "\n";
        }

        if (array->empty() && readyQueue->empty())
        {
            break;
        }

        sch->fcfs(runningQueue);

        runningQueue->back().setResponseTime(currentTime);

        status_file << "\t" << currentTime << " ms"
                    << "\t\t" << runningQueue->back().processId << "\t\t\tRunning"
                    << "\n";

        int endTime = currentTime + runningQueue->back().burstTime;
        runningQueue->back().setCompletionTime(endTime);

        currentTime = endTime;
    }

    status_file.close();
}

void Simulator::runRR()
{
    cout << "\n Hey! Round Robin Here\n"
         << endl;
    cout << " Enter the Time Quantum : ";
    cin >> timeQuantum;

    MinHeap heap(array);

    if (heap.getMin().arrivalTime != 0)
    {
        currentTime = heap.getMin().arrivalTime;
    }

    ofstream status_file("status.txt");
    status_file << "\n\tRound Robin: \n";
    status_file << "\n\tTime Stamp\tProcess Id\tStatus\n\n";

    while (currentTime < simulationTime)
    {
        while (!array->empty())
        {
            if (heap.getMin().arrivalTime <= currentTime)
            {
                readyQueue->push_back(heap.getMin());
                status_file << "\t" << readyQueue->back().arrivalTime << " ms"
                            << "\t\t" << readyQueue->back().processId << "\t\t\tArrived"
                            << "\n";
                heap.pop();
            }
            else
            {
                break;
            }
        }

        if (!runningQueue->empty())
        {
            if (runningQueue->back().leftBurstTime != 0)
            {
                readyQueue->push_back(runningQueue->back());

                status_file << "\t" << currentTime << " ms"
                            << "\t\t" << runningQueue->back().processId << "\t\t\tArrived"
                            << "\n";

                runningQueue->pop_back();
            }
            else
            {
                status_file << "\t" << currentTime << " ms"
                            << "\t\t" << runningQueue->back().processId << "\t\t\tExit"
                            << "\n";
                runningQueue->pop_back();
            }
        }

        if (readyQueue->empty() && array->empty())
        {
            break;
        }

        sch->rr(runningQueue);

        status_file << "\t" << currentTime << " ms"
                    << "\t\t" << runningQueue->back().processId << "\t\t\tRunning"
                    << "\n";

        if (!runningQueue->empty())
        {
            if (runningQueue->back().leftBurstTime >= timeQuantum)
            {
                currentTime += timeQuantum;
                runningQueue->back().leftBurstTime -= timeQuantum;
            }
            else if (runningQueue->back().leftBurstTime != 0 && runningQueue->back().leftBurstTime < timeQuantum)
            {
                currentTime += runningQueue->back().leftBurstTime;
                runningQueue->back().leftBurstTime = 0;
            }
            else
            {
                currentTime += 0;
            }

            if (runningQueue->back().leftBurstTime == 0) {
                runningQueue->back().setCompletionTime(currentTime);
            }
        }
    }

    status_file.close();
}

void Simulator::runCFS()
{
    cout << "\n Hey! Completely Fair Scheduler Here\n"
         << endl;

    MinHeap heap(array);
    RBTree redBlackTree;

    ofstream status_file("status.txt");
    status_file << "\n\tCompletely Fair Scheduler: \n";
    status_file << "\n\tTime Stamp\tProcess Id\tStatus\n\n";

    if (heap.getMin().arrivalTime != 0)
    {
        currentTime = heap.getMin().arrivalTime;
    }

    while (currentTime < simulationTime)
    {
        while (!heap.empty())
        {
            if (heap.getMin().arrivalTime <= currentTime)
            {
                Process p = heap.getMin();
                redBlackTree.insert(p);
                status_file << "\t" << p.arrivalTime << " ms"
                            << "\t\t" << p.processId << "\t\t\tArrived"
                            << "\n";
                heap.pop();
            }
            else
            {
                break;
            }
        }

        if (!runningQueue->empty())
        {
            if (runningQueue->back().leftBurstTime != 0)
            {
                redBlackTree.insert(runningQueue->back());

                status_file << "\t" << currentTime << " ms"
                            << "\t\t" << runningQueue->back().processId << "\t\t\tArrived"
                            << "\n";

                runningQueue->pop_back();
            }
            else
            {
                status_file << "\t" << currentTime << " ms"
                            << "\t\t" << runningQueue->back().processId << "\t\t\tExit"
                            << "\n";
                runningQueue->pop_back();
            }
        }

        if (redBlackTree.empty() && heap.empty())
        {
            break;
        }

        Process Q = redBlackTree.getMin();
        redBlackTree.deleteMin();

        sch->cfs(Q, runningQueue);

        status_file << "\t" << currentTime << " ms"
                    << "\t\t" << runningQueue->back().processId << "\t\t\tRunning"
                    << "\n";

        if (!runningQueue->empty())
        {
            if (runningQueue->back().leftBurstTime >= runningQueue->back().timeSlice)
            {
                currentTime += runningQueue->back().timeSlice;
                runningQueue->back().leftBurstTime -= runningQueue->back().timeSlice;
            }
            else if (runningQueue->back().leftBurstTime != 0 && runningQueue->back().leftBurstTime < runningQueue->back().timeSlice)
            {
                currentTime += runningQueue->back().leftBurstTime;
                runningQueue->back().leftBurstTime = 0;
            }
            else
            {
                currentTime += 0;
            }
        }

        int cpuTime = runningQueue->back().burstTime - runningQueue->back().leftBurstTime;
        runningQueue->back().virtualRunTime += (cpuTime * (140 - runningQueue->back().staticPriority));

        if (runningQueue->back().leftBurstTime == 0) {
            runningQueue->back().setCompletionTime(currentTime);
        }
    }

    status_file.close();
}

void callAlgo()
{
    int a;
    vector<Process> *f;

    cout << "\n1) First Come First Serve" << endl;
    cout << "2) Round Robin" << endl;
    cout << "3) Completely Fair Scheduler\n"
         << endl;

    cout << " Choose the Scheduling Algorithm (1/2/3): ";
    cin >> a;

    if (!(a == 1 || a == 2 || a == 3))
    {
        cout << "\n Error! invalid input" << endl;
        return;
    }

    Process_Creator obj;
    Scheduler sch(obj);
    Simulator sim(sch, a);
    sim.Run();

    f = sim.array;

    ofstream output_file("processes.txt");

    if (a == 1)
    {
        output_file << "\n\tFirst Come First Serve : \n\n";
    }
    else if (a == 2)
    {
        output_file << "\n\tRound Robin: \n\n";
    }
    else
    {
        output_file << "\n\tCompletely Fair Scheduler : \n\n";
        output_file << "\tID\tAT\tBT\tCT\tTAT\tWT\tRT\t\tNICE\tVRT\n";

        for (int i = 0; i < f->size(); i++)
        {
            output_file << "\t" << f->at(i).processId << "\t"
                        << f->at(i).arrivalTime << "\t" << f->at(i).burstTime << "\t" << f->at(i).completionTime
                        << "\t" << f->at(i).turnAroundTime << "\t" << f->at(i).waitingTime << "\t" << f->at(i).responseTime
                        << "\t\t" << f->at(i).staticPriority - 120 << "\t\t" << f->at(i).virtualRunTime << "\n";
        }
    }

    if (a == 1 || a == 2)
    {
        output_file << "\tID\tAT\tBT\tCT\tTAT\tWT\tRT\n";

        for (int i = 0; i < f->size(); i++)
        {
            output_file << "\t" << f->at(i).processId << "\t"
                        << f->at(i).arrivalTime << "\t" << f->at(i).burstTime << "\t" << f->at(i).completionTime
                        << "\t" << f->at(i).turnAroundTime << "\t" << f->at(i).waitingTime << "\t" << f->at(i).responseTime << "\n";
        }
    }

    output_file.close();

    delete f;
    return;
}

int main()
{
    cout << "\n In, the arrival time and burst time " << endl;
    cout << " will be set in the range of (1-10) milli seconds " << endl;
    cout << "\n Enter the Number of Processes : ";
    cin >> N;

    cout << "\n Enter the Simulation Time (secs) : ";
    cin >> simulationTime;

    // Converting from seconds to milliseconds
    simulationTime *= 1000;

    callAlgo();

    return 0;
}
