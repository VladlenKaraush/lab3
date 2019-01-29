#ifndef SET_LS_H
#define SET_LS_H

#include <functional>
#include <pthread.h>
#include <bits/stdc++.h>
#include <syslog.h>

template <class T>
class SetLS{
private:
class Node{
    public:
    T data; 
    size_t key;
    Node* next;
    bool del_mark;
    pthread_mutex_t mutex;
  
    Node(T init_value){
    	data = init_value;
    	key = std::hash<T>()(init_value);
    	next = nullptr;
    	del_mark = false;
    	mutex = PTHREAD_MUTEX_INITIALIZER;
    }
	
	void lock(){
      if(pthread_mutex_lock(&mutex) != 0)
        syslog(LOG_INFO, "E:Mutex lock");   
    }

    void unlock(){
      if(pthread_mutex_unlock(&mutex) != 0)
        syslog(LOG_INFO, "E:Mutex unlock");   
    }
   };

  Node* head; 

public:
  SetLS(){
    head = new Node(0);
    if (!head)
      syslog(LOG_INFO, "E:Creating node");
    head->next = new Node(INT_MAX);
    if (!head->next)
      syslog(LOG_INFO, "E:Creating node");
}

  ~SetLS(){
    for (Node *cur = head, *next = nullptr; cur != nullptr; cur = next){
      next = cur->next;
      delete cur;
    }
  }

  size_t generate_hash(const T& data){
    return std::hash<T>()(data);
  }

  bool validate(Node* prev, Node* cur){
    return !prev->del_mark && !cur->del_mark && (prev->next == cur);
  }

  bool add(const T& data){
    size_t key = generate_hash(data);
    while (true){
      Node* prev = head;
      Node* cur = head->next;
      while (cur->key < key){
        
        prev = cur;
        cur = cur->next;
      }

      prev->lock();
      cur->lock();
     
      if (validate(prev, cur)){
        if (cur->key == key){
          prev->unlock();
          cur->unlock();
          return false;
        }
        else{
          Node* node = new Node(data);
          if (!node)
            syslog(LOG_INFO, "E:Creating node");
          node->next = cur;
          prev->next = node;
          prev->unlock();
          cur->unlock();
          return true;
        }
      }
      
      prev->unlock();
      cur->unlock();
    }
  }

  bool remove(const T& data){
    size_t key = generate_hash(data);
    while (true){
      Node* prev = head;
      Node* cur = head->next;
      while (cur->key < key){
        prev = cur;
        cur = cur->next;
      }

      prev->lock();
      cur->lock();

      if (validate(prev, cur)){
        if (cur->key == key){
          cur->del_mark = true;
          prev->unlock();
          cur->unlock();
          prev->next = cur->next;
          delete cur;
          return true;
        }
        else{
          prev->unlock();
          cur->unlock();
          return false;
        }
      }
      prev->unlock();
      cur->unlock();
    }
  }

  bool contains(const T& data){
    size_t key = generate_hash(data);
    Node* cur = head;
    while(cur->key < key)
    	cur = cur->next;

    return (cur->key == key) && !cur->del_mark;
  }
};

#endif
