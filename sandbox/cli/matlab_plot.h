#pragma once

#include <filesystem>

// Launch MATLAB in non-interactive batch mode and ask the existing
// plot_transient.m companion to render a transient CSV to an image.
void run_matlab_plot(
    const std::filesystem::path& csv_path,
    const std::filesystem::path& image_path);
