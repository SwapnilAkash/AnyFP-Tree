#ifndef ANYFPTREE_HPP
#define ANYFPTREE_HPP

#include <cstdint>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <vector>
#include <utility>
#include<unordered_map>
#include<chrono>

using Transaction = std::vector<int>;
using TransformedPrefixPath = std::pair<std::vector<int>, uint64_t>;
using Pattern = std::pair<std::set<int>, uint64_t>;


struct FPNode {
        const int Item;
        uint64_t frequency;
        std::shared_ptr<FPNode> node_link;
        std::weak_ptr<FPNode> parent;
        const int first_node;
        std::vector<std::shared_ptr<FPNode>> children;
        std::vector<std::vector<int>> Buffer;

        FPNode(const int &Item, const std::shared_ptr<FPNode>&, const int&);
        FPNode(const int &Item, const std::shared_ptr<FPNode>&);
};

struct FPTree{
        std::shared_ptr<FPNode> root;
        std::shared_ptr<FPNode> curr_fpnode;
        std::vector<std::shared_ptr<FPNode>> buffer_start_node;
        std::map<int, std::shared_ptr<FPNode>> header_table;

        const uint64_t minimum_support_threshold;
        const uint64_t epsilon;
        std::map<int, uint64_t> frequency_by_item;

        FPTree(const std::vector<Transaction>&, uint64_t);
        FPTree(const uint64_t, const uint64_t);
        void FPTree1(const std::vector<int>&, uint64_t, double, uint64_t);
        bool empty() const;
};


std::set<Pattern> fptree_growth(const FPTree&);


#endif
