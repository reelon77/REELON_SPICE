#pragma once

#include <iosfwd>
#include <string>
#include <vector>

int run_cli(
    const std::vector<std::string>& args,
    std::ostream& standard_out,
    std::ostream& standard_err);
