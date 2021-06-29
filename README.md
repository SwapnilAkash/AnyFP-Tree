# AnyFP-Tree
Anytime algorithm for mining frquent itemsets in data streams.

Frequent Itemset Mining involves uncovering the relationships among the itemsets in a given set of transactions. The trivial way to describe these relationships would be to say 
that the particular sets of items occur together frequently in a given set of transactions. Many algorithms have been proposed to mine these frequent itemsets but the dataset 
under consideration is always static in nature. 

This project aims to achieve the same goal of mining frequent itemsets in case of data streams with the capability to handle inter-arrival rate of transactions and high speed 
streams. It proposes an anytime algorithm called AnyFP-Tree to achieve this functionality. It maintains a FP-Tree structure in such a way that it extracts mining results in 
almost the time available for data streams and also improves upon its accuracy if any spare time is left.

