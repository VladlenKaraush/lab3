#ifndef SET_OS_H
#define SET_OS_H

#include <functional>
#include <pthread.h>
#include <bits/stdc++.h>
#include <syslog.h>

template <class T>
class SetOS{
private:
class Node{
    public:
    T data; 
    size_t key;
    Node* next;
    pthread_mutex_t mutex;
    
    Node(T init_value){
    	data = init_value;
    	key = std::hash<T>()(init_value);
    	next = nullptr;
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
  SetOS(){
    head = new Node(0);
    if (!head)
      syslog(LOG_INFO, "E:Creating node");
    head->next = new Node(INT_MAX);
    if (!head->next)
      syslog(LOG_INFO, "E:Creating node");
  }

  ~SetOS(){
    for (Node *cur = head, *next = nullptr; cur != nullptr; cur = next){
      next = cur->next;
      delete cur;
    }
  }

  size_t generate_hash(const T& data){
    return std::hash<T>()(data);
  }

  bool validate(Node* prev, Node* cur){
    Node* node = head;
    while (node->key <= prev->key){
      if (node == prev)
        return prev->next == cur;
      node = node->next;
    }
    return false;
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
          prev->next = cur->next;
          delete cur;
          prev->unlock();
          cur->unlock();
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
        prev->unlock();
        cur->unlock();
        return cur->key == key;
      }
      prev->unlock();
      cur->unlock();
    }
  }
};

#endif
