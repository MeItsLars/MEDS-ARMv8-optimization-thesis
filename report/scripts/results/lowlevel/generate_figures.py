import csv
import os
import matplotlib.pyplot as plt
import seaborn as sns
import numpy as np
from typing import List

sns.set_theme()

THIS_FILE_PATH = os.path.dirname(os.path.abspath(__file__))

def generate_barplot(measurements, filename):
    # Create figure and axis
    fig, ax = plt.subplots(figsize=(12, 6))
    
    subcategories = [function["function_name"] for function in measurements[0]["functions"]]
    x = np.arange(len(subcategories))
    num_variants = len(measurements)
    
    width = 0.09
    subcategory_closeness = 2.5
    
    for j, measurement in enumerate(measurements):
        function_cycle_values = [function["cycles"] for function in measurement["functions"]]
        ax.bar(x + (j - num_variants / 2 + 0.5) * width * subcategory_closeness, function_cycle_values, width * subcategory_closeness)
            
    # Set x-axis labels
    ax.set_ylabel("MCycles")
    ax.set_xticks(x)
    ax.set_xticklabels(subcategories)
        
    # Set layout settings
    fig.subplots_adjust(top=0.85)
    fig.subplots_adjust(left=0.15)
    fig.subplots_adjust(right=0.85)
    fig.subplots_adjust(bottom=0.15)
    # fig.subplots_adjust(wspace=0.2)
    
    # Create a legend
    handles = [plt.Rectangle((0, 0), 1, 1, color=plt.cm.tab10(i)) for i in range(num_variants)]
    fig.legend(handles=handles, labels=[f"{measurement['variant']}" for measurement in measurements], loc='upper center', bbox_to_anchor=(0.5, 0.1), ncol=3)

    # Export images
    plt.savefig(os.path.join(THIS_FILE_PATH, "..", "..", "..", "imgs", "plots", filename), bbox_inches='tight')
    # plt.show()

def load_csv(file):
    with open(os.path.join(THIS_FILE_PATH, file), 'r') as file:
        # Read but ignore header
        reader = csv.reader(file)
        data = list(reader)
    return data

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
        # sorted_function_measurements = sorted(function_measurements, key=lambda x: x["cycles"], reverse=True)
        entries.append({
            "paramset": paramset,
            "variant": variant,
            "cycles": total_cycles,
            "functions": function_measurements
        })
    return entries

def compute_remaining(variant: str) -> tuple:
    total_cycles = sum([entry["cycles"] for entry in variant["functions"]])
    total_percentage = sum([float(entry["percentage"]) for entry in variant["functions"]])
    all_cycles = total_cycles / (total_percentage / 100)
    remaining_cycles = all_cycles - total_cycles
    remaining_percentage = 100 - total_percentage
    return remaining_cycles, remaining_percentage

def generate(file: str, paramset: str, output_file: str) -> str:
    data = load_csv(file)
    parsed_data = parse_algorithm_data(data)
    
    # Find the JSON objects in 'parsed_data' list that has correct 'paramset'
    relevant_variants = [entry for entry in parsed_data if entry["paramset"] == paramset]
    for variant in relevant_variants:
        remaining_cycles, remaining_percentage = compute_remaining(variant)
        variant["functions"].append({
            "function_name": "remaining",
            "cycles": remaining_cycles,
            "percentage": remaining_percentage,
            "calls": 0
        })
    generate_barplot(relevant_variants, output_file)

generate('results_keygen.csv', 'MEDS-55520', 'barplot_MEDS-55520_profile_keygen.png')
generate('results_sign.csv', 'MEDS-55520', 'barplot_MEDS-55520_profile_sign.png')
generate('results_verify.csv', 'MEDS-55520', 'barplot_MEDS-55520_profile_verify.png')
