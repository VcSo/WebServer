
#include "Poco/JSON/Parser.h"
#include <iostream>
#include <string>


int main(int argc, char **argv) {
    std::string json = "{ \"test\" : { \"property\" : \"value\", \"dog\": \"cat\" } }";
    Poco::JSON::Parser parser;
    Poco::Dynamic::Var result = parser.parse(json);

    Poco::JSON::Object::Ptr object = result.extract<Poco::JSON::Object::Ptr>();
    Poco::Dynamic::Var test = object->get("test");
    std::cout << "---test object to string : \n" << test.toString() << std::endl;

    Poco::JSON::Object::Ptr subObject = test.extract<Poco::JSON::Object::Ptr>();
    test = subObject->get("property");
    std::string val = test.toString();
    std::cout << "---get 'property' : " << val << std::endl;

    Poco::DynamicStruct ds = *object;
    val = ds["test"]["dog"].toString();
    std::cout << "---get 'dog' : " << val << std::endl;

    return 0;
}
//
// g++ -W -Wall -g json.cpp -o json -lPocoNet -lPocoUtil -lPocoJSON -lPocoFoundation -I /usr/local/include/Poco -I /usr/local/lib
