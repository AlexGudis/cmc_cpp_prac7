#include <iostream>
#include "shr_ptr.cpp"


int main(void)
{
    {
        // Test deafault constructor
        std::cout << "---------- TEST 1 ----------" << std::endl;
        MySharedPtr<int> ptr1(new int{1});
        std::cout << ptr1.get() << " : " << *ptr1.get() << std::endl;
    }

    {
        // Test copy constructor
        std::cout << "---------- TEST 2 ----------" << std::endl;
        MySharedPtr<int> ptr1(new int{1});
        MySharedPtr<int> ptr2(ptr1);
        std::cout << ptr1.get() << " : " << *ptr1.get() << std::endl;
        std::cout << ptr2.get() << " : " << *ptr2.get() << std::endl;
    }

    {
        std::cout << "---------- TEST 3 ----------" << std::endl;
        MySharedPtr<int> ptr1(new int{1});
        MySharedPtr<int> another_ptr1(new int{2});
        MySharedPtr<int> ptr2 = ptr1;
        std::cout << ptr2.get() << " : " << *ptr2.get() << std::endl;
        ptr2 = another_ptr1;
        std::cout << ptr1.get() << " : " << *ptr1.get() << std::endl;
        std::cout << ptr2.get() << " : " << *ptr2.get() << std::endl;
    }

    {
        std::cout << "---------- TEST 4 ----------" << std::endl;
        // Test move constructor
        MySharedPtr<int> ptr1 = MySharedPtr<int>{new int{4}};
        std::cout << ptr1.get() << " : " << *ptr1.get() << std::endl;
    }
    return 0;
}