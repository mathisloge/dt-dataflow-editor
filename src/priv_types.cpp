#include "priv_types.hpp"

namespace dt::df::editor
{
RefCon::~RefCon()
{
    connection.disconnect();
}

} // namespace dt::df::editor
