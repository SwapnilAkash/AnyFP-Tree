#include <algorithm>
#include <cassert>
#include <cstdint>
#include <utility>
#include <chrono>
#include <stdio.h>
#include <iostream>
#include "anyfptree.hpp"


//constructor for FPNode utilised by FPTree1()
FPNode::FPNode(const int &item, const std::shared_ptr<FPNode>& parent, const int& fn) :
    //each pointer is associated with a custom deleter so that memory is deallocated when it goes out of scope
    Item( item ), frequency( 1 ), node_link( nullptr, [](FPNode *p) { delete p;} ), parent( parent), children(),
    Buffer(), first_node(fn)
{
}

//constructor for FPNode utilised by fptree_growth()
FPNode::FPNode(const int &item, const std::shared_ptr<FPNode>& parent) :
    Item( item ), frequency( 1 ), node_link( nullptr, [](FPNode *p) { delete p;} ), parent( parent), children(),
    Buffer(), first_node()
{
}

//constructor for FPTree utilised by FPTree1() 
FPTree::FPTree(const uint64_t minsup, const uint64_t e):
    //root(std::make_shared<FPNode>( -1, nullptr )),
    header_table(),
    minimum_support_threshold(minsup),
    epsilon(e),
    curr_fpnode(nullptr),
    buffer_start_node(), frequency_by_item()
{
    root = std::shared_ptr<FPNode>(new FPNode(-1, nullptr, -1), [](FPNode *p) { delete p;} );
}

void FPTree::FPTree1(const std::vector<int>& transaction, const uint64_t minimum_support_threshold, double time_allowance, uint64_t no_transactions) {

    // scan the transactions counting the frequency of each item
    for ( const int item : transaction ) {
        ++frequency_by_item[item];
    }

    // order items by decreasing frequency
    struct frequency_comparator
    {
        bool operator()(const std::pair<int, uint64_t> &lhs, const std::pair<int, uint64_t> &rhs) const
        {
             if (lhs.second != rhs.second) {
                return lhs.second < rhs.second;
            }
            return lhs.first < rhs.first;
    
        }
    };
    //maintain a set for all items in descending order according to its frequency
    std::set<std::pair<int, uint64_t>, frequency_comparator> items_ordered_by_frequency(frequency_by_item.cbegin(), frequency_by_item.cend());


    std::vector<int> rem_items;
    std::shared_ptr<FPNode> parent_node = std::shared_ptr<FPNode>();    // initialized by poitning to no object
    auto start_time = std::chrono::high_resolution_clock::now(); //store the start time of transaction
    
    //lambda function to generate the time duration spent since the start
    auto getTime = [&](std::chrono::_V2::system_clock::time_point start_time){
    std::chrono::duration<double, std::milli> time_span = std::chrono::high_resolution_clock::now() - start_time;
    return time_span.count();
    };


    // start tree construction
    auto build_tree = [&](const std::vector<int>& transactions, std::shared_ptr<FPNode>& node, int f){
        auto iter = transactions.cbegin();
        iter++;
        
        curr_fpnode = node;
        int i = 1;
        int flag = 0;
        int fn;
        
        // scan the transactions again
        // select and sort the frequent items in transaction according to the order of items_ordered_by_frequency
        for ( const auto& pair : items_ordered_by_frequency ) {
            const int item = pair.first;
            
            // check if item (of items_ordered_frequency) is contained in the current transaction
            if ( std::find( transactions.cbegin(), transactions.cend(), item ) != transactions.cend() ) {
                
                // insert item in the tree

                // check if curr_fpnode1 has a child curr_fpnode1_child such that curr_fpnode1_child.item = item
                const auto it = std::find_if(
                    curr_fpnode->children.cbegin(), curr_fpnode->children.cend(),  [item](const std::shared_ptr<FPNode>& fpnode) {
                        return fpnode->Item == item;
                } );

                if ( it == curr_fpnode->children.cend() ) {
                    // the child doesn't exist, create a new node
                    auto curr_fpnode_new_child = std::make_shared<FPNode>(item, curr_fpnode, fn);
                    //each node stores the first node that was inserted for the current transaction
                    if(i){
                        fn = item;
                        i--;
                    }
                    
                    // add the new node to the tree
                    curr_fpnode->children.emplace_back( curr_fpnode_new_child );

                    //check if the newly inserted node's item was in the buffer of its parent
                    //if its present then don't change the parent_node 
                     if(!flag)
                    {
                        parent_node = curr_fpnode;
                    }
                    if(parent_node && !(parent_node->Buffer.empty())){
                        if(!(parent_node->Buffer[0].empty())){
                            auto it2 = find(parent_node->Buffer[0].begin(),parent_node->Buffer[0].end(), item);
                            if(it2 != parent_node->Buffer[0].end()){   
                                //newly inserted node is present in its parent's buffer 
                                if(f)
                                    ++curr_fpnode_new_child->frequency; 
                                //delete the item from parent's buffer
                                parent_node->Buffer[0].erase(it2);
                                flag=1;
                                if(parent_node->Buffer[0].empty())
                                    parent_node->Buffer.erase(parent_node->Buffer.begin());
                            }
                        }
                    } 

                    // update the node-link structure
                    if ( header_table.count( curr_fpnode_new_child->Item ) ) {
                        auto prev_fpnode = header_table[curr_fpnode_new_child->Item];
                        while ( prev_fpnode->node_link ) { prev_fpnode = prev_fpnode->node_link; }
                        prev_fpnode->node_link = curr_fpnode_new_child;
                    }
                    else {
                        header_table[curr_fpnode_new_child->Item] = curr_fpnode_new_child;
                    }

                    // advance to the next node of the current transaction
                    curr_fpnode = curr_fpnode_new_child;
                }
                else {
                    // the child exist, increment its frequency
                    auto curr_fpnode1_child = *it;
                    ++curr_fpnode1_child->frequency;

                    // advance to the next node of the current transaction
                    curr_fpnode = curr_fpnode1_child;
                    flag = 0;
                    
                }
                //check for time allowance expiration before the transaction finishes its processing
                if(getTime(start_time) > time_allowance && iter != transactions.cend()){
                    //curr_fpnode1->Buffer.push_back(std::vector<int>());
                    int j=0;
                    for(const auto &x: transactions){
                        if(x == item){
                            j=1;
                            continue;
                        }
                        if(j)
                            rem_items.push_back(x);
                    }
                    //insert the unprocessed items in the current node's buffer
                    curr_fpnode->Buffer.push_back(rem_items);

                    //check if parent's node buffer is non-empty
                    //if non-empty insert it in buffer_start_node to insert in the tree later
                    if(flag && !(parent_node->Buffer.empty())){
                        buffer_start_node.push_back(parent_node);
                        parent_node.reset(); 
                    }
                    return;
                }
            }
        }
        if(flag && !(parent_node->Buffer.empty())){
            buffer_start_node.push_back(parent_node);
            parent_node.reset();
        }
    };

    //call the build_tree lambda function
    build_tree(transaction, root, 1);

    //if time is left and any buffer along the path of current transaction is not empty, insert them in the FP-Tree
    if(getTime(start_time) <= time_allowance && !buffer_start_node.empty() && buffer_start_node[0]->first_node == curr_fpnode->first_node){
        build_tree(buffer_start_node[0]->Buffer[0], buffer_start_node[0], 0);
        buffer_start_node[0]->Buffer.erase(buffer_start_node[0]->Buffer.begin());
        buffer_start_node.erase(buffer_start_node.begin());
    }


    /*
    if((getTime(start_time) <= time_allowance) && (curr_fpnode1->Buffer.find(first_node) != curr_fpnode1->Buffer.end())){
        if(!curr_fpnode1->Buffer[first_node].empty()){
            build_tree(curr_fpnode1->Buffer[first_node][0],2);
           // auto it2 = Buffer[first_node].begin();
            //Buffer[first_node][0].erase((*it2).begin(), (*it2).end());    
            std::vector<int>().swap(curr_fpnode1->Buffer[first_node][0]);

        

        }
        else
            curr_fpnode1->Buffer.erase(first_node);
    }*/
    

    //FP-Tree Pruning starts if time permits
    auto next_it = frequency_by_item.begin();
    if((no_transactions % (minimum_support_threshold*5) == 0) && (getTime(start_time) <= time_allowance)){    
        for ( auto it = frequency_by_item.begin(); it != frequency_by_item.end(); it = next_it) {
            const uint64_t item_frequency = (*it).second;
            ++next_it;
            //check if item is error-frequent or not
            //if it's not then all the node having this item will be deleted using header table
            if ( item_frequency < epsilon ) {

                if(header_table.find((*it).first) != header_table.end()){

                    while(header_table[(*it).first]){
                        
                        auto ptr = header_table[(*it).first]->parent.lock();
                        if(!ptr)
                            break;
                        
                        if(!((header_table[(*it).first]->children).empty())){

                            for(auto &x: (header_table[(*it).first]->children)){

                                x->parent = header_table[(*it).first]->parent;
                                (ptr->children).emplace_back(x);
                            }
                        }
                        auto temp = find(ptr->children.begin(), ptr->children.end(), header_table[(*it).first]);
                        (ptr->children).erase(temp);
                        auto next = header_table[(*it).first]->node_link;
                        header_table[(*it).first].reset();
                        header_table[(*it).first] = next;
                        next.reset();
                        ptr.reset();
                    }
                    header_table.erase((*it).first);
                }
                frequency_by_item.erase( it ); 
            }
            if(getTime(start_time) > time_allowance)
                break;
        }
    }
}

//constructor for FPTree utilised by fptree_growth() for generating conditional FP-Trees
FPTree::FPTree(const std::vector<Transaction>& transactions, uint64_t minimum_support_threshold) :
    root( std::make_shared<FPNode>( -1, nullptr ) ), header_table(),
    minimum_support_threshold( minimum_support_threshold ), epsilon()
{
    // scan the transactions counting the frequency of each item
    std::map<int, uint64_t> frequency_by_item;
    for ( const Transaction& transaction : transactions ) {
        for ( int item : transaction ) {
            ++frequency_by_item[item];
        }
    }

    // keep only items which have a frequency greater or equal than the minimum support threshold
    auto next_it = frequency_by_item.begin();
    for ( auto it = frequency_by_item.cbegin(); it != frequency_by_item.cend(); it = next_it ) {
        ++next_it;
        const uint64_t item_frequency = (*it).second;
        if ( item_frequency < minimum_support_threshold ) { frequency_by_item.erase(it); }
    }

    // order items by decreasing frequency
    struct frequency_comparator
    {
        bool operator()(const std::pair<int, uint64_t> &lhs, const std::pair<int, uint64_t> &rhs) const
        {
            return std::tie(lhs.second, lhs.first) > std::tie(rhs.second, rhs.first);
        }
    };
    std::set<std::pair<int, uint64_t>, frequency_comparator> items_ordered_by_frequency(frequency_by_item.cbegin(), frequency_by_item.cend());

    // start tree construction

    // scan the transactions again
    for ( const Transaction& transaction : transactions ) {
        auto curr_fpnode1 = root;

        // select and sort the frequent items in transaction according to the order of items_ordered_by_frequency
        for ( const auto& pair : items_ordered_by_frequency ) {
            const int item = pair.first;

            // check if item is contained in the current transaction
            if ( std::find( transaction.cbegin(), transaction.cend(), item ) != transaction.cend() ) {
                // insert item in the tree

                // check if curr_fpnode1 has a child curr_fpnode1_child such that curr_fpnode1_child.item = item
                const auto it = std::find_if(
                    curr_fpnode1->children.cbegin(), curr_fpnode1->children.cend(),  [item](const std::shared_ptr<FPNode>& fpnode) {
                        return fpnode->Item == item;
                } );
                if ( it == curr_fpnode1->children.cend() ) {
                    // the child doesn't exist, create a new node
                    auto curr_fpnode1_new_child = std::make_shared<FPNode>( item, curr_fpnode1 );

                    // add the new node to the tree
                    curr_fpnode1->children.push_back( curr_fpnode1_new_child );

                    // update the node-link structure
                    if ( header_table.count( curr_fpnode1_new_child->Item ) ) {
                        auto prev_fpnode = header_table[curr_fpnode1_new_child->Item];
                        while ( prev_fpnode->node_link ) { prev_fpnode = prev_fpnode->node_link; }
                        prev_fpnode->node_link = curr_fpnode1_new_child;
                    }
                    else {
                        header_table[curr_fpnode1_new_child->Item] = curr_fpnode1_new_child;
                    }

                    // advance to the next node of the current transaction
                    curr_fpnode1 = curr_fpnode1_new_child;
                }
                else {
                    // the child exist, increment its frequency
                    auto curr_fpnode1_child = *it;
                    ++curr_fpnode1_child->frequency;

                    // advance to the next node of the current transaction
                    curr_fpnode1 = curr_fpnode1_child;
                }
            }
        }
    }
}


bool FPTree::empty() const
{
    assert( root );
    return root->children.size() == 0;
}


bool contains_single_path(const std::shared_ptr<FPNode>& fpnode)
{
    assert( fpnode );
    if ( fpnode->children.size() == 0 ) { return true; }
    if ( fpnode->children.size() > 1 ) { return false; }
    return contains_single_path( fpnode->children.front() );
}
bool contains_single_path(const FPTree& fptree)
{
    return fptree.empty() || contains_single_path( fptree.root );
}

std::set<Pattern> fptree_growth(const FPTree& fptree)
{
    if ( fptree.empty() ) { return {}; }

    if ( contains_single_path( fptree ) ) {
        // generate all possible combinations of the items in the tree

        std::set<Pattern> single_path_patterns;

        // for each node in the tree
        assert( fptree.root->children.size() == 1 );
        auto curr_fpnode1 = fptree.root->children.front();
        while ( curr_fpnode1 ) {
            const int curr_fpnode1_item = curr_fpnode1->Item;
            const uint64_t curr_fpnode1_frequency = curr_fpnode1->frequency;

            // add a pattern formed only by the item of the current node
            Pattern new_pattern{ { curr_fpnode1_item }, curr_fpnode1_frequency };
            single_path_patterns.insert( new_pattern );

            // create a new pattern by adding the item of the current node to each pattern generated until now
            for ( const Pattern& pattern : single_path_patterns ) {
                Pattern new_pattern{ pattern };
                new_pattern.first.insert( curr_fpnode1_item );
                assert( curr_fpnode1_frequency <= pattern.second );
                new_pattern.second = curr_fpnode1_frequency;

                single_path_patterns.insert( new_pattern );
            }

            // advance to the next node until the end of the tree
            assert( curr_fpnode1->children.size() <= 1 );
            if ( curr_fpnode1->children.size() == 1 ) { curr_fpnode1 = curr_fpnode1->children.front(); }
            else { curr_fpnode1 = nullptr; }
        }

        return single_path_patterns;
    }
    else {
        // generate conditional fptrees for each different item in the fptree, then join the results

        std::set<Pattern> multi_path_patterns;

        // for each item in the fptree
        for ( const auto& pair : fptree.header_table ) {
            const int curr_item = pair.first;

            // build the conditional fptree relative to the current item

            // start by generating the conditional pattern base
            std::vector<TransformedPrefixPath> conditional_pattern_base;

            // for each path in the header_table (relative to the current item)
            auto path_starting_fpnode = pair.second;
            while ( path_starting_fpnode ) {
                // construct the transformed prefix path

                // each item in th transformed prefix path has the same frequency (the frequency of path_starting_fpnode)
                const uint64_t path_starting_fpnode_frequency = path_starting_fpnode->frequency;

                auto curr_path_fpnode = path_starting_fpnode->parent.lock();
                // check if curr_path_fpnode is already the root of the fptree
                if ( curr_path_fpnode->parent.lock() ) {
                    // the path has at least one node (excluding the starting node and the root)
                    TransformedPrefixPath transformed_prefix_path{ {}, path_starting_fpnode_frequency };

                    while ( curr_path_fpnode->parent.lock() ) {
                        assert( curr_path_fpnode->frequency >= path_starting_fpnode_frequency );
                        transformed_prefix_path.first.push_back( curr_path_fpnode->Item );

                        // advance to the next node in the path
                        curr_path_fpnode = curr_path_fpnode->parent.lock();
                    }

                    conditional_pattern_base.push_back( transformed_prefix_path );
                }

                // advance to the next path
                path_starting_fpnode = path_starting_fpnode->node_link;
            }

            // generate the transactions that represent the conditional pattern base
            std::vector<Transaction> conditional_fptree_transactions;
            for ( const TransformedPrefixPath& transformed_prefix_path : conditional_pattern_base ) {
                const std::vector<int>& transformed_prefix_path_items = transformed_prefix_path.first;
                const uint64_t transformed_prefix_path_items_frequency = transformed_prefix_path.second;

                Transaction transaction = transformed_prefix_path_items;

                // add the same transaction transformed_prefix_path_items_frequency times
                for ( auto i = 0; i < transformed_prefix_path_items_frequency; ++i ) {
                    conditional_fptree_transactions.push_back( transaction );
                }
            }

            // build the conditional fptree relative to the current item with the transactions just generated
            FPTree conditional_fptree( conditional_fptree_transactions, fptree.minimum_support_threshold);
            // call recursively fptree_growth on the conditional fptree (empty fptree: no patterns)
            std::set<Pattern> conditional_patterns = fptree_growth( conditional_fptree );

            // construct patterns relative to the current item using both the current item and the conditional patterns
            std::set<Pattern> curr_item_patterns;

            // the first pattern is made only by the current item
            // compute the frequency of this pattern by summing the frequency of the nodes which have the same item (follow the node links)
            uint64_t curr_item_frequency = 0;
            auto fpnode = pair.second;
            while ( fpnode ) {
                curr_item_frequency += fpnode->frequency;
                fpnode = fpnode->node_link;
            }
            // add the pattern as a result
            Pattern pattern{ { curr_item }, curr_item_frequency };
            curr_item_patterns.insert( pattern );

            // the next patterns are generated by adding the current item to each conditional pattern
            for ( const Pattern& pattern : conditional_patterns ) {
                Pattern new_pattern{ pattern };
                new_pattern.first.insert( curr_item );
                assert( curr_item_frequency >= pattern.second );
                new_pattern.second = pattern.second;

                curr_item_patterns.insert( { new_pattern } );
            }

            // join the patterns generated by the current item with all the other items of the fptree
            multi_path_patterns.insert( curr_item_patterns.cbegin(), curr_item_patterns.cend() );
        }

        return multi_path_patterns;
    }
}
