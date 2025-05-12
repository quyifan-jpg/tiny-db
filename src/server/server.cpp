#include <iostream>
#include <memory>
#include <string>
#include <chrono>
#include <vector>
#include <thread>
#include <atomic>
#include <unordered_map>
#include <functional>
#include <sstream>
#include <muduo/net/EventLoop.h>
#include <muduo/net/InetAddress.h>
#include <muduo/net/TcpConnection.h>
#include <muduo/net/TcpServer.h>
#include <muduo/base/Logging.h>

#include "../db/db.h"
#include "../db/options.h"
#include "../db/status.h"

using namespace muduo;
using namespace muduo::net;
using namespace smallkv;

class LSMServer {
public:
    LSMServer(EventLoop* loop, const InetAddress& listenAddr)
        : server_(loop, listenAddr, "LSMServer") {
        
        // 初始化LSM数据库
        auto opts = MakeOptionsForDebugging();
        db_holder_ = std::make_shared<DB>(opts);
        
        // 设置连接回调
        server_.setConnectionCallback(
            std::bind(&LSMServer::onConnection, this, std::placeholders::_1));
        
        // 设置消息回调
        server_.setMessageCallback(
            std::bind(&LSMServer::onMessage, this, std::placeholders::_1,
                     std::placeholders::_2, std::placeholders::_3));
        
        // 初始化命令处理器
        initCommandHandlers();
    }

    void start() {
        server_.start();
    }

private:
    void onConnection(const TcpConnectionPtr& conn) {
        if (conn->connected()) {
            LOG_INFO << "New connection from " << conn->peerAddress().toIpPort();
        } else {
            LOG_INFO << "Connection closed from " << conn->peerAddress().toIpPort();
        }
    }

    void onMessage(const TcpConnectionPtr& conn, Buffer* buf, Timestamp time) {
        std::string msg(buf->retrieveAllAsString());
        LOG_INFO << "Received message at " << time.toString() << ":\n" << msg;

        // 解析并处理请求
        std::string response = handleRequest(msg);
        conn->send(response);
    }

    void initCommandHandlers() {
        // 注册命令处理器
        command_handlers_["SET"] = [this](const std::vector<std::string>& args) {
            return handleSet(args);
        };
        command_handlers_["GET"] = [this](const std::vector<std::string>& args) {
            return handleGet(args);
        };
        command_handlers_["DEL"] = [this](const std::vector<std::string>& args) {
            return handleDel(args);
        };
        // 可以添加更多命令处理器
    }

    std::string handleRequest(const std::string& request) {
        if (request.empty()) {
            return "-ERR Protocol error: expected '*'\r\n";
        }

        if (request == "PING\r\n") {
            return "+PONG\r\n";
        }

        // 解析RESP协议
        std::vector<std::string> args = parseRESP(request);
        if (args.empty()) {
            return "-ERR Protocol error\r\n";
        }

        // 查找并执行命令处理器
        auto it = command_handlers_.find(args[0]);
        if (it != command_handlers_.end()) {
            return it->second(args);
        }

        return "-ERR unknown command '" + args[0] + "'\r\n";
    }

    std::vector<std::string> parseRESP(const std::string& request) {
        std::vector<std::string> args;
        size_t pos = 0;

        if (request[pos] != '*') {
            return args;
        }

        try {
            int numElements = std::stoi(request.substr(pos + 1));
            pos = request.find('\n', pos) + 1;

            for (int i = 0; i < numElements; ++i) {
                if (pos >= request.size() || request[pos] != '$') {
                    return args;
                }

                int len = std::stoi(request.substr(pos + 1));
                pos = request.find('\n', pos) + 1;
                args.push_back(request.substr(pos, len));
                pos = request.find('\n', pos) + 1;
            }
        } catch (const std::exception&) {
            return args;
        }

        return args;
    }

    // 命令处理器实现
    std::string handleSet(const std::vector<std::string>& args) {
        if (args.size() != 3) {
            return "-ERR wrong number of arguments for 'SET' command\r\n";
        }

        WriteOptions wopts;
        auto status = db_holder_->Put(wopts, args[1], args[2]);
        
        if (status == Status::Success) {
            return "+OK\r\n";
        } else {
            return "-ERR " + std::string(status.err_msg)  + "\r\n";
        }
    }

    std::string handleGet(const std::vector<std::string>& args) {
        if (args.size() != 2) {
            return "-ERR wrong number of arguments for 'GET' command\r\n";
        }

        ReadOptions ropts;
        std::string value;
        auto status = db_holder_->Get(ropts, args[1], &value);
        
        if (status == Status::Success) {
            return "$" + std::to_string(value.size()) + "\r\n" + value + "\r\n";
        } else if (status == Status::NotFound) {
            return "$-1\r\n";
        } else {
            return "-ERR " + std::string(status.err_msg)  + "\r\n";
        }
    }

    std::string handleDel(const std::vector<std::string>& args) {
        if (args.size() < 2) {
            return "-ERR wrong number of arguments for 'DEL' command\r\n";
        }

        WriteOptions wopts;
        int deleted = 0;
        
        for (size_t i = 1; i < args.size(); ++i) {
            // 在LSM中，删除操作通常通过写入一个特殊的删除标记来实现
            auto status = db_holder_->Put(wopts, args[i], ""); // 或者使用特殊的删除标记
            if (status == Status::Success) {
                deleted++;
            }
        }

        return ":" + std::to_string(deleted) + "\r\n";
    }

    // 成员变量
    TcpServer server_;
    std::shared_ptr<DB> db_holder_;
    std::unordered_map<std::string, 
        std::function<std::string(const std::vector<std::string>&)>> command_handlers_;
};

int main() {
    // 设置日志级别
    Logger::setLogLevel(Logger::INFO);
    
    // 创建事件循环
    EventLoop loop;
    
    // 创建服务器实例
    InetAddress listenAddr(6379); // 使用Redis默认端口
    LSMServer server(&loop, listenAddr);
    
    // 启动服务器
    server.start();
    
    // 进入事件循环
    loop.loop();
    
    return 0;
}