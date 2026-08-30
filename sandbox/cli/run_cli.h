#pragma once

#include <filesystem>
#include <functional>
#include <iosfwd>
#include <string>
#include <vector>

using MatlabPlotRunner = std::function<void(
    const std::filesystem::path& csv_path,
    const std::filesystem::path& image_path)>;

int run_cli(
    const std::vector<std::string>& args,
    std::ostream& standard_out,
    std::ostream& standard_err);

int run_cli(
    const std::vector<std::string>& args,
    std::ostream& standard_out,
    std::ostream& standard_err,
    const MatlabPlotRunner& matlab_plot_runner);
