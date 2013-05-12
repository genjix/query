#include "node_impl.hpp"
#include "echo.hpp"
#include "service.hpp"

int main()
{
    config_map_type config;
    config["output file"] = "debug.log";
    config["error file"] = "error.log";
    config["database"] = "database";
    config["block pub port"] = "5563";
    config["tx pub port"] = "5564";
    node_impl node(config);
    echo() << "Starting node...";
    if (!node.start())
        return 1;
    echo() << "Node started.";
    start_thrift_server(node);
    echo() << "Server stopped.";
    echo() << "Stopping node...";
    if (!node.stop())
        return 1;
    echo() << "Node stopped.";
    return 0;
}

