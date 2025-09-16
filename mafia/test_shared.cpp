#include <iostream>
#include "shr_ptr.cpp"


int main(void)
{
    MySharedPtr<int *> ptr1 = new int{1};
    return 0
}