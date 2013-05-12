#ifndef QUERY_CONFIG_HPP
#define QUERY_CONFIG_HPP

#include <map>
#include <string>

typedef std::map<std::string, std::string> config_map_type;
void load_config(config_map_type& config, const std::string& config_path);

#endif

