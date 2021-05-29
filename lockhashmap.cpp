#include <stddef.h>
#include <vector>
#include <iostream>
#include <mutex>
#include <shared_mutex>
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
    struct Node_HM_t* m_next; //pointer to next node in the bucket
    Node_HM_t(long val):m_val(val),padding(),m_next(nullptr){};
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
    shared_timed_mutex mtx;
public:
    hm_t(size_t n_buckets){size=n_buckets; buckets.resize(n_buckets);};
    ~hm_t(){};
    inline long hashalgo(long val){return val%size;}
    int insert(long val){
        unique_lock<shared_timed_mutex> lck(mtx);
        long index = hashalgo(val);
        Node_HM_t* p = buckets[index].sentinel;
        while(p->m_next!=nullptr){
            p=p->m_next;
            if(p->m_val==val)return 1;
        }
        p->m_next = new Node_HM_t(val);
        return 0;
    }
    int remove(long val){
        unique_lock<shared_timed_mutex> lck(mtx);
        long index = hashalgo(val);
        Node_HM_t* p, *tempptr;
        p = buckets[index].sentinel;
        while(p->m_next!=nullptr){
            if(p->m_next->m_val == val){tempptr=p->m_next;p->m_next=tempptr->m_next;delete tempptr;return 0;}
            else p=p->m_next;
        }
        return 1;
    };
    int lookup(long val){
        shared_lock<shared_timed_mutex> lck(mtx,defer_lock);
        long index = hashalgo(val);
        Node_HM_t* p = buckets[index].sentinel;
        while(p->m_next!=nullptr){
            if(p->m_next->m_val == val)return 0;
            else p=p->m_next;
        }
        return 1;
    };
//Bucket 1 - val1 - val2 - val3 ...
    void display(){
        shared_lock<shared_timed_mutex> lck(mtx,defer_lock);
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
