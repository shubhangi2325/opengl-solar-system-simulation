#ifndef _STACK_H 
#define _STACK_H 

#include "vmath.h"
using namespace vmath;

class node
{
    friend class Stack;
    private:
    mat4 matrix;
    node* prev;
    node* next;

    node(mat4 new_matrix);
};

class Stack
{
    private:
    node* p_head_node;

    public:
    Stack();
    ~Stack(); 
    void push(mat4 new_data); 
    mat4 pop(); 
    mat4 top(); 
};

#endif 


