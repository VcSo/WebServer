#include <Poco/Version.h>
#include <iostream>
#include <string>

int main ()
{
//    std::cout << "POCO version: " << Poco::Version::major () << "." << Poco::Version::minor () << "." << Poco::Version::revision () << " (" << Poco::Version::build () << ")\n";
    std::cout << "POCO version (as string): " << Poco::Version::toString () << "\n";
    return 0;
}

//clang++ -std=c++14 -W -Wall -g poco_version.cpp -o poco_version -lPocoFoundation -I /usr/local/include -I /usr/local/lib
//clang++ -std=c++14 -W -Wall -g poco_version.cpp /usr/local/include/Poco/Version.h -o poco_version  -I /usr/local/include -L /usr/local/lib -lPocoFoundation
