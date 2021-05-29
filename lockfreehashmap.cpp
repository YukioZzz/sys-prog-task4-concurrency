#include <stddef.h>
#include <vector>
#include <iostream>
#include <atomic>
#include "chashmap.h"
#define PAD 64

using namespace std;
/*
Hashmap: list of buckets
bucket1 -> sentinel -> node1 -> node2 -> NULL
bucket2 -> sentinel -> node3 -> NULL
...
bucketN -> sentinel -> NULL
*/

//define a node in the hashmap
struct Node_HM_t
{
    long m_val; //value of the node
    char padding[PAD];
    atomic<Node_HM_t*> m_next; //pointer to next node in the bucket
    atomic<bool> mark;
    Node_HM_t(long val):m_val(val),padding(),m_next(nullptr),mark(false){};
};

//defining a bucket in the hashmap
struct List_t
{
    Node_HM_t* sentinel; //list of nodes in a bucket
    List_t():sentinel(new Node_HM_t(-1)){};
    ~List_t(){Node_HM_t* tempptr; while(sentinel->m_next!=nullptr){tempptr=sentinel->m_next; delete sentinel; sentinel=tempptr;}delete sentinel;};  
};

//defining the hashmap
class hm_t
{
private:
    long size;
    vector<List_t> buckets; //list of buckets in the hashmap
public:
    hm_t(size_t n_buckets){size=n_buckets; buckets.resize(n_buckets);};
    ~hm_t(){};
    inline long hashalgo(long val){return val%size;}
    bool search(Node_HM_t*& left, long val, Node_HM_t*& right){
        while(right!=nullptr){
            if(right->mark){
                Node_HM_t* next = right->m_next;
                if(!atomic_compare_exchange_strong(&left->m_next, &right, next))
                    if(left->mark)return false;///> just in case
                right = left->m_next;
                continue;
            }//delete marked item
            if(val <= right->m_val)return true;
            left = right;
            right = left->m_next;
        }
        return true;
    }//find a free place or not 
    int insert(long val){
        long index = hashalgo(val);
        Node_HM_t* pre = buckets[index].sentinel;
        Node_HM_t* cur = pre->m_next;
        if(!this->search(pre,val,cur)) return 1; 
        if(cur!=nullptr && val == cur->m_val) return 1;       
        ///< inserting with CAS 
        //cur == nullptr||cur< cur->m_val
        Node_HM_t* new_node = new Node_HM_t(val);
        new_node->m_next = cur;
        if(!atomic_compare_exchange_strong(&pre->m_next, &cur, new_node)){
            delete new_node;
            return 1;
        }
        return 0;
    }
    int remove(long val){
        long index = hashalgo(val);
        Node_HM_t* pre = buckets[index].sentinel;
        Node_HM_t* cur = pre->m_next;
        if(!this->search(pre,val,cur)) return 1;
        if(cur == nullptr) return 1;
        if(val == cur->m_val){
          cur->mark=true;
          return 0;
        }           
        return 1;
    };
    int lookup(long val){
        long index = hashalgo(val);
        Node_HM_t* pre = buckets[index].sentinel;
        Node_HM_t* cur = pre->m_next;
        if(!this->search(pre,val,cur)) return 1;
        if(cur == nullptr) return 1;
        if(val == cur->m_val){
          return 0;
        }
        return 1;
    };
    //Bucket 1 - val1 - val2 - val3 ...
    void display(){
        Node_HM_t* p;
        for(int i=0;i<size;i++){
            std::cout<<"Bucekt "<<i+1;
            p = buckets[i].sentinel;
            while(p->m_next!=nullptr){
                p = p->m_next;
                std::cout<<" - "<< p->m_val;
            }    
           std::cout<<std::endl;
        }
    };
};

//allocate a hashmap with given number of buckets
hm_t* alloc_hashmap(size_t n_buckets){
    return new hm_t(n_buckets);
}

//free a hashamp
void free_hashmap(hm_t* hm){
    if(hm!=nullptr)delete hm;
}

//insert val into the hm and return 0 if successful
//return 1 otherwise, e.g., could not allocate memory
int insert_item(hm_t* hm, long val){
    if(hm!=nullptr)return hm->insert(val);
    return 1;
}

//remove val from the hm, if it exist and return 0 if successful
//return 1 if item is not found
int remove_item(hm_t* hm, long val){
    if(hm!=nullptr)return hm->remove(val);
    return 1;
}

//check if val exists in hm, return 0 if found, return 1 otherwise
int lookup_item(hm_t* hm, long val){
    if(hm!=nullptr)return hm->lookup(val);
    return 1;
}

//print all elements in the hashmap as follows:
//Bucket 1 - val1 - val2 - val3 ...
//Bucket 2 - val4 - val5 - val6 ...
//Bucket N -  ...
void print_hashmap(hm_t* hm){
    if(hm!=nullptr)hm->display();
}
