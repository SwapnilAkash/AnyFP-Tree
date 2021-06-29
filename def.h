#include<iostream>
#include<vector>

class datapoint{
    public:
    std::vector<int> items;
    int itemsetLength;
    double time_allowance;
};

class dataset:public datapoint{
    public:
    uint64_t count; //total transactions
    std::vector<datapoint> dataArray;
};

