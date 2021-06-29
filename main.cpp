// Syntax for passing command line arguments- stream speed, epsilon value, minimum support threshold

#include "def.h"
#include "data.h"
#include "anyfptree.hpp"

void anyfp(dataset*, const uint64_t, const uint64_t);

int main(int argc, char *argv[]){

    int i=0;
    int stream_speed = atoi(argv[1]);
    const uint64_t epsilon = atoi(argv[2]);
    const uint64_t sigma = atoi(argv[3]);
    std::vector<std::vector<int>> trans;
    readData(trans);
    std::vector<double> time_allowance;
    readTimeAllowance(time_allowance, stream_speed);


    dataset d;

    //scan each transaction and store its details as one element of dataArray
    for(const auto &x: trans){

        datapoint single_trans;
        single_trans.items = x;
        single_trans.itemsetLength = single_trans.items.size();
        single_trans.time_allowance = time_allowance[i++];
        d.dataArray.emplace_back(single_trans);
        d.count = i;

    }


    //const uint64_t minimum_support_threshold = sigma;
    /*
    const FPTree fptree{ trans, minimum_support_threshold };

    const std::set<Pattern> patterns = fptree_growth( fptree );

    for(auto x: patterns){
        for(auto y: x.first)
            cout<<y<<" ";
        cout<<x.second<<endl;
    } */

    
    anyfp(&d, sigma, epsilon);

    return 0;

    
}

void anyfp(dataset *d, const uint64_t minsup, const uint64_t e){

    uint64_t i= 0;
    FPTree a(minsup,e);
    
    while(i < (d->count)){

        std::cout<<"i: "<<i+1<<std::endl;
        a.FPTree1(d->dataArray[i].items, minsup, d->dataArray[i].time_allowance, i+1);
 
        if((i+1)%(minsup*5)==0){

            std::cout<<std::endl<<"Finding FI"<<std::endl;
            std::set<Pattern> patterns = fptree_growth(a);
            if(!patterns.empty()){
                for(const auto &x: patterns){
                    for(const auto &y: x.first)
                        std::cout<<y<<" ";
                std::cout<<x.second<<std::endl;
                }
            }
            else
                std::cout<<"No Frequent Itemsets found!!"<<std::endl;
        }
        i++;
    }
    
}
