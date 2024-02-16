#include "..\\headers\\inlcudes.hpp"

int main()
{
    std::cout << "It worked, I was setted up by Premake5.lua file"<<std::endl;

    double a=1.1,b=2.2;
    double c = MyNumericLibrary::add(a,b);
    if (c == 3.3)
        std::cout << "It Worked, My static Numeric Library is working correctly" << std::endl;


}