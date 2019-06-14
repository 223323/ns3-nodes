#ifndef COMMON_H
#define COMMON_H

#include <iostream>
#define TRACE() do{ std::cout << __PRETTY_FUNCTION__ << "@" << __LINE__ << std::endl; }while(0)
#define DEBUG(x) do{ std::cout << #x << " = " << x << std::endl; }while(0)


#endif // COMMON_H