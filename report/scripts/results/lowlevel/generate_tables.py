import csv
import os

from typing import List

THIS_FILE_PATH = os.path.dirname(os.path.abspath(__file__))

TABLE_TEMPLATE = """
  \\begin{tabular}{lrrr}
    \\toprule
    \\textbf{Function} & \\textbf{\\# MCycles} ($\\pm$) & \\textbf{\\%% of Total} ($\pm$) & \\textbf{\\# Calls} \\\\
    \\midrule
%s
    \\midrule
%s
%s
    \\bottomrule
  \\end{tabular}
"""

def load_csv(file):
    with open(os.path.join(THIS_FILE_PATH, file), 'r') as file:
        # Read but ignore header
        reader = csv.reader(file)
        data = list(reader)
    return data

def convert_function_name(name: str) -> str:
    name = name.replace("_", "\\_")
    return f"\\texttt{{{name}}}"

def parse_algorithm_data(data: List[str]) -> List[dict]:
    entries = []
    # Parse but ignore header
    for measurement in data[1:]:
        paramset = measurement[0]
        variant = measurement[1]
        total_cycles = float(measurement[2]) / 1_000_000
        function_measurements = []
        
        for i in range(3, len(measurement)):
            cycles = float(measurement[i].split("/")[0]) / 1_000_000
            function_measurement = {
                "function_name": data[0][i],
                "cycles": cycles,
                "percentage": (cycles / total_cycles) * 100,
                "calls": measurement[i].split("/")[1]
            }
            function_measurements.append(function_measurement)
        # Sort based on total cycles
        sorted_function_measurements = sorted(function_measurements, key=lambda x: x["cycles"], reverse=True)
        entries.append({
            "paramset": paramset,
            "variant": variant,
            "cycles": total_cycles,
            "functions": sorted_function_measurements
        })
    return entries

def compute_cumulative(variant: str) -> tuple:
    total_cycles = sum([entry["cycles"] for entry in variant["functions"]])
    total_percentage = sum([float(entry["percentage"]) for entry in variant["functions"]])
    return total_cycles, total_percentage

def compute_remaining(variant: str) -> tuple:
    total_cycles = sum([entry["cycles"] for entry in variant["functions"]])
    total_percentage = sum([float(entry["percentage"]) for entry in variant["functions"]])
    all_cycles = total_cycles / (total_percentage / 100)
    remaining_cycles = all_cycles - total_cycles
    remaining_percentage = 100 - total_percentage
    return remaining_cycles, remaining_percentage

def generate_table(file: str, paramset: str, variant: str) -> str:
    data = load_csv(file)
    parsed_data = parse_algorithm_data(data)
    # Find the JSON object in 'parsed_data' list that has correct 'paramset' and 'variant' values
    relevant_variant = next((entry for entry in parsed_data if entry["paramset"] == paramset and entry["variant"] == variant), None)
    table_data = []
    for function in relevant_variant["functions"]:
        table_data.append(f"      {convert_function_name(function['function_name'])} & {function['cycles']:.2f} & {function['percentage']:.2f} & {function['calls']} \\\\")
    cumulative_data = compute_cumulative(relevant_variant)
    cumulative_table_data = f"      Cumulative & {cumulative_data[0]:.2f} & {cumulative_data[1]:.2f} & \\\\"
    remaining_data = compute_remaining(relevant_variant)
    remaining_table_data = f"      Remaining & {remaining_data[0]:.2f} & {remaining_data[1]:.2f} & \\\\"
    return TABLE_TEMPLATE % ("\n".join(table_data), cumulative_table_data, remaining_table_data)

print(generate_table('results_keygen.csv', 'MEDS-55520', 'Reference'))
print(generate_table('results_sign.csv', 'MEDS-55520', 'Reference'))
print(generate_table('results_verify.csv', 'MEDS-55520', 'Reference'))
