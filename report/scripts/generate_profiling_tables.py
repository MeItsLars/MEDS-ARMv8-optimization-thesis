from typing import List

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

FUNCTION_NAME_CONVERSIONS = {
    "pmod_mat_syst_ct_partial_swap_backsub": "pmod_mat_syst",
    "solve_opt_raw": "solve_opt"
}

KEYGEN_ROW_DATA = [
    "  pmod_mat_mul: 15971433 cycles (69.81%) - tot runs: 70 - median of op: 115895",
    "  pmod_mat_syst_ct_partial_swap_backsub: 2074718 cycles (9.07%) - tot runs: 6 - median of op: 409347",
    "  rnd_sys_mat: 2071868 cycles (9.06%) - tot runs: 1 - median of op: 2071868",
    "  solve_opt_raw: 1414416 cycles (6.18%) - tot runs: 3 - median of op: 168866",
    "  bs_fill: 1062187 cycles (4.64%) - tot runs: 2 - median of op: 531093"
]

SIGN_ROW_DATA = [
    "  pmod_mat_syst_ct_partial_swap_backsub: 391870439 cycles (10.75%) - tot runs: 1040 - median of op: 407264",
    "  solve_opt_raw: 293111545 cycles (8.04%) - tot runs: 624 - median of op: 167469",
    "  bs_fill: 212750854 cycles (5.84%) - tot runs: 208 - median of op: 1022423",
    "  shake256_absorb: 202923383 cycles (5.57%) - tot runs: 212 - median of op: 975852",
    "  pmod_mat_mul: 2523803231 cycles (69.22%) - tot runs: 14635 - median of op: 115857"
]

VERIFY_ROW_DATA = [
    "pmod_mat_syst_ct_partial_swap_backsub: 391579119 cycles (10.73%) - tot runs: 1040 - median of op: 407603",
    "solve_opt_raw: 293176501 cycles (8.04%) - tot runs: 624 - median of op: 167497",
    "bs_fill: 212748765 cycles (5.83%) - tot runs: 208 - median of op: 1022630",
    "shake256_absorb: 202940231 cycles (5.56%) - tot runs: 210 - median of op: 975529",
    "pmod_mat_mul: 2525932899 cycles (69.23%) - tot runs: 14560 - median of op: 115907",
]

def convert_function_name(name: str) -> str:
    if name in FUNCTION_NAME_CONVERSIONS:
        name = FUNCTION_NAME_CONVERSIONS[name]
    name = name.replace("_", "\\_")
    return f"\\texttt{{{name}}}"

def parse_algorithm_data(data: List[str]) -> List[dict]:
    entries = []
    # Parse
    for line in data:
        line = line.replace(" ", "")
        function_name, rest = line.split(":", 1)
        cycles, rest = rest.split("cycles(", 1)
        percentage, rest = rest.split("%)-totruns:", 1)
        calls, _ = rest.split("-medianofop:", 1)
        entry = {
            "function": convert_function_name(function_name),
            "cycles": float(cycles) / 1_000_000,
            "percentage": percentage,
            "calls": calls
            }
        entries.append(entry)
    # Sort based on total cycles
    sorted_entries = sorted(entries, key=lambda x: int(x["cycles"]), reverse=True)
    return sorted_entries

def compute_cumulative(data: List[dict]) -> tuple:
    total_cycles = sum([entry["cycles"] for entry in data])
    total_percentage = sum([float(entry["percentage"]) for entry in data])
    return total_cycles, total_percentage

def compute_remaining(data: List[dict]) -> tuple:
    total_cycles = sum([entry["cycles"] for entry in data])
    total_percentage = sum([float(entry["percentage"]) for entry in data])
    all_cycles = total_cycles / (total_percentage / 100)
    remaining_cycles = all_cycles - total_cycles
    remaining_percentage = 100 - total_percentage
    return remaining_cycles, remaining_percentage

def generate_table(data: List[str]) -> str:
    parsed_data = parse_algorithm_data(data)
    table_data = []
    for entry in parsed_data:
        table_data.append(f"      {entry['function']} & {entry['cycles']:.2f} & {entry['percentage']} & {entry['calls']} \\\\")
    cumulative_data = compute_cumulative(parsed_data)
    cumulative_table_data = f"      Cumulative & {cumulative_data[0]:.2f} & {cumulative_data[1]:.2f} & \\\\"
    remaining_data = compute_remaining(parsed_data)
    remaining_table_data = f"      Remaining & {remaining_data[0]:.2f} & {remaining_data[1]:.2f} & \\\\"
    return TABLE_TEMPLATE % ("\n".join(table_data), cumulative_table_data, remaining_table_data)

print(generate_table(KEYGEN_ROW_DATA))
print(generate_table(SIGN_ROW_DATA))
print(generate_table(VERIFY_ROW_DATA))
