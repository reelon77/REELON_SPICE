function plot_transient(csv_file, columns, output_file)
%PLOT_TRANSIENT Plot waveform columns from a TinySpice transient CSV.
%   PLOT_TRANSIENT(CSV_FILE) plots every non-time column interactively.
%   PLOT_TRANSIENT(CSV_FILE, COLUMNS) plots the named string/cell columns.
%   PLOT_TRANSIENT(CSV_FILE, COLUMNS, OUTPUT_FILE) saves the axes to a file.

if nargin < 2
    columns = strings(1, 0);
end
if nargin < 3
    output_file = "";
end

csv_file = normalize_text_scalar(csv_file, "csv_file", false);
output_file = normalize_text_scalar(output_file, "output_file", true);

if ~isfile(csv_file)
    error("TinySpice:MissingCsv", "CSV file does not exist: %s", csv_file);
end

data = readtable(csv_file, "VariableNamingRule", "preserve");
names = string(data.Properties.VariableNames);

if ~any(names == "time")
    error("TinySpice:MissingTime", ...
        "CSV is missing required 'time' column");
end
if height(data) == 0
    error("TinySpice:EmptyCsv", "CSV has no data rows");
end

selected_columns = normalize_columns(columns, names);
time_values = numeric_column(data, "time");
waveform_values = cell(1, numel(selected_columns));
for column_index = 1:numel(selected_columns)
    waveform_values{column_index} = ...
        numeric_column(data, selected_columns(column_index));
end

save_to_file = strlength(output_file) > 0;
if save_to_file
    figure_handle = figure("Visible", "off");
    figure_cleanup = onCleanup(@() close_if_valid(figure_handle)); %#ok<NASGU>
else
    figure_handle = figure();
end

axes_handle = axes(figure_handle);
hold(axes_handle, "on");
for column_index = 1:numel(selected_columns)
    column = selected_columns(column_index);
    plot(axes_handle, time_values, waveform_values{column_index}, ...
        "LineWidth", 1.5, "DisplayName", char(column));
end
hold(axes_handle, "off");

xlabel(axes_handle, "time (s)");
ylabel(axes_handle, "value");
grid(axes_handle, "on");
legend(axes_handle, "show", "Interpreter", "none", "Location", "best");

if save_to_file
    exportgraphics(axes_handle, output_file);
    fprintf("wrote plot: %s\n", output_file);
end
end

function value = normalize_text_scalar(value, argument_name, allow_empty)
if ischar(value)
    if ~(isrow(value) || isempty(value))
        error("TinySpice:InvalidArgument", ...
            "%s must be a character row vector or string scalar", argument_name);
    end
    value = string(value);
elseif ~(isstring(value) && isscalar(value))
    error("TinySpice:InvalidArgument", ...
        "%s must be a character row vector or string scalar", argument_name);
end

if ~allow_empty && strlength(value) == 0
    error("TinySpice:InvalidArgument", "%s cannot be empty", argument_name);
end
end

function selected = normalize_columns(columns, names)
if isempty(columns)
    selected = names(names ~= "time");
elseif ischar(columns)
    if ~isrow(columns)
        error("TinySpice:InvalidColumns", ...
            "columns must be text names in a vector");
    end
    selected = string(columns);
elseif isstring(columns) || iscellstr(columns)
    if ~isvector(columns)
        error("TinySpice:InvalidColumns", ...
            "columns must be text names in a vector");
    end
    selected = string(columns);
else
    error("TinySpice:InvalidColumns", ...
        "columns must be a string vector, character vector, or cell array of text");
end

selected = reshape(selected, 1, []);
if isempty(selected)
    error("TinySpice:NoWaveforms", "CSV has no waveform columns to plot");
end
if any(strlength(selected) == 0)
    error("TinySpice:InvalidColumns", "waveform column names cannot be empty");
end
if numel(unique(selected)) ~= numel(selected)
    error("TinySpice:DuplicateColumns", "requested columns contain duplicates");
end
if any(selected == "time")
    error("TinySpice:TimeAsWaveform", ...
        "'time' is the x-axis and cannot be a waveform column");
end

missing = selected(~ismember(selected, names));
if ~isempty(missing)
    error("TinySpice:MissingColumns", ...
        "CSV does not contain requested column(s): %s", strjoin(missing, ", "));
end
end

function values = numeric_column(data, column)
values = data{:, char(column)};
if ~(isnumeric(values) && isreal(values) && isvector(values))
    error("TinySpice:NonNumericColumn", ...
        "column '%s' must contain one real numeric value per row", column);
end

values = double(values(:));
if any(~isfinite(values))
    error("TinySpice:NonFiniteColumn", ...
        "column '%s' contains a non-finite value", column);
end
end

function close_if_valid(figure_handle)
if isgraphics(figure_handle)
    close(figure_handle);
end
end
