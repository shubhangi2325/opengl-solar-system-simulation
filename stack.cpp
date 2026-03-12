#include"stack.h"

node::node(mat4 new_matrix) : matrix(new_matrix), prev(0), next(0) 
{

}

Stack::Stack()
{
    p_head_node = new node(mat4::identity());
    p_head_node->prev = p_head_node; 
    p_head_node->next = p_head_node;
}

Stack::~Stack()
{
    node* p_run, *p_run_next; 
    for(p_run = this->p_head_node->next; p_run != this->p_head_node; p_run = p_run_next)
        {
            p_run_next = p_run->next; 
            delete p_run; 
        }
    delete this->p_head_node;
    this->p_head_node = 0; 
}

void Stack::push(mat4 new_data)
{
    node* new_matrix = new node(new_data);
    new_matrix->prev = p_head_node->prev;
    new_matrix->next = p_head_node;
    p_head_node->prev->next = new_matrix;
    p_head_node->prev = new_matrix;
}

mat4 Stack::pop()
{
    mat4 matrix = mat4::identity();

    if(this->p_head_node->next != this->p_head_node && this->p_head_node->prev != this->p_head_node)
    {
        node* deleteNode = this->p_head_node->prev ;
        matrix = deleteNode->matrix;
        deleteNode->prev->next = deleteNode->next;
        deleteNode->next->prev = deleteNode->prev;
        delete deleteNode;
        deleteNode = 0;
    }

    return (matrix);
}

mat4 Stack::top()
{
    if(this->p_head_node->next == this->p_head_node && this->p_head_node->prev == this->p_head_node)
    {
        return (mat4::identity());
    }

    return (this->p_head_node->prev->matrix);

}