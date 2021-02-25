#pragma once
#include <functional>
#include <string>
namespace dt::df::editor
{
using NodeDisplayDrawFnc = std::function<void(
    int prev_level, int level, bool is_leaf, const std::string &node_key, const std::string &node_name)>;
}
