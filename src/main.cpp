#include "node_impl.hpp"
#include "echo.hpp"
#include "service.hpp"

int main(int argc, char** argv)
{
    config_map_type config;
    load_config(config, "query.cfg");
    node_impl node;
    echo() << "Starting node...";
    if (!node.start(config))
        return 1;
    echo() << "Node started.";
    start_thrift_server(config, node);
    echo() << "Server stopped.";
    echo() << "Stopping node...";
    if (!node.stop())
        return 1;
    echo() << "Node stopped.";
    return 0;
}

