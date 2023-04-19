#include <Poco/Net/ServerSocket.h>
#include <Poco/Net/HTTPServer.h>
#include <Poco/Net/HTTPRequestHandler.h>
#include <Poco/Net/HTTPRequestHandlerFactory.h>
#include <Poco/Net/HTTPResponse.h>
#include <Poco/Net/HTTPServerRequest.h>
#include <Poco/Net/HTTPServerResponse.h>
#include <Poco/Util/ServerApplication.h>

#include <iostream>
#include <string>
#include <vector>
#include <Poco/JSON/Parser.h>
#include <Poco/StreamCopier.h>
#include <Poco/Util/ServerApplication.h>
#include <Poco/JSON/Parser.h>
#include <Poco/StreamCopier.h>

using namespace Poco::JSON;
using namespace std;
using namespace Poco::Net;
using namespace Poco::Util;
// g++ -std=c++14 -W -Wall -g server.cpp -o server -lPocoJSON -lPocoFoundation -lPocoNet -lPocoUtil -I /usr/local/include/Poco -I/usr/local/lib

class MyRequestHandler : public HTTPRequestHandler
{
public:
    virtual void handleRequest(HTTPServerRequest &req, HTTPServerResponse &resp)
    {
        Application& app = Application::instance();
        app.logger().information("Request from " + req.clientAddress().toString());

        std::string Json_str;
        Poco::StreamCopier::copyToString(req.stream(), Json_str);
        std::cout<<Json_str<<std::endl;

        Poco::JSON::Parser parser;
        Poco::Dynamic::Var json = parser.parse(Json_str);
        Poco::JSON::Object::Ptr object_ptr = json.extract<Poco::JSON::Object::Ptr>();

        int a = object_ptr->getValue<int>("a");
        int b = object_ptr->getValue<int>("b");

        int d  = a*b;
        //解决跨域问题
        resp.add("Access-Control-Allow-Origin","*");
        resp.add("Access-Control-Allow-Methods","POST,GET,OPTIONS,DELETE");
        resp.add("Access-Control-Max-Age","3600");
        resp.add("Access-Control-Allow-Headers","x-requested-with,content-type");
        resp.add("Access-Control-Allow-Credentials","true");

        resp.setStatus(HTTPResponse::HTTP_OK);
        resp.setContentType("text/json");

        Poco::JSON::Object bodyObj;
        bodyObj.set("d", d);
        std::ostringstream ss;
        bodyObj.stringify(ss);
        std::string body;
        body = ss.str();
        resp.setStatus(HTTPResponse::HTTP_OK);
        ostream &out = resp.send();
        out << body;

        std::cout<<body<<std::endl;
    }
};

class MyRequestHandlerFactory : public HTTPRequestHandlerFactory
{
public:
    virtual HTTPRequestHandler* createRequestHandler(const HTTPServerRequest &)
    {
        return new MyRequestHandler;
    }
};

class MyServerApp :public ServerApplication
{
protected:
    int main(const vector<string> &)
    {
        HTTPServer s(new MyRequestHandlerFactory, ServerSocket(20777), new HTTPServerParams);

        s.start();
        cout << endl << "Server started" << endl;

        waitForTerminationRequest();  // wait for CTRL-C or kill

        cout << endl << "Shutting down..." << endl;

        s.stop();

        return Application::EXIT_OK;
    }
};

int main(int argc, char **argv)
{
    MyServerApp app;

    return app.run(argc, argv);
}