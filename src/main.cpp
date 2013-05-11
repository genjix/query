#include "query_app.hpp"
#include "echo.hpp"

int main()
{
    config_map_type config;
    config["output file"] = "debug.log";
    config["error file"] = "error.log";
    config["database"] = "database";
    query_app app(config);
    echo() << "Starting node...";
    if (!app.start())
        return 1;
    echo() << "Node started.";
    std::cin.get();
    if (!app.stop())
        return 1;
    return 0;
}

