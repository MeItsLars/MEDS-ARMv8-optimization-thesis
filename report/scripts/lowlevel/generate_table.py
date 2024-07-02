import csv
import os

THIS_FILE_PATH = os.path.dirname(os.path.abspath(__file__))

TABLE_TEMPLATE = """
  \\begin{tabular}{lllrrr}
    \\toprule
    \\textbf{Function} & \\multicolumn{2}{c}{\\textbf{Input Size}} & \\textbf{Cycles} & \\textbf{Bound} & \\textbf{Ratio}\\\\
    \\toprule
%s
    \\bottomrule
  \\end{tabular}
"""

PARAMSET_MAPPINGS = {
    "n": 35,
    "m": 34,
    "k": 34,
    "mn": 35 * 34,
    "2n": 2 * 35,
    "2m": 2 * 34,
    "2k": 2 * 34,
    "nr1": 35 - 1,
    "mr1": 34 - 1
}

def load_csv(file):
    with open(os.path.join(THIS_FILE_PATH, file), 'r') as file:
        # Read but ignore header
        reader = csv.reader(file)
        data = list(reader)
    return data

def evaluate(value):
    try:
        return int(value)
    except:
        return PARAMSET_MAPPINGS[value]

def replace_minus(value):
    return value.replace('r', '-')

def matmul_cycles(m, o, n):
    return (1 / 64.0) * m * n * (18 * o + 88)

def matsyst_cycles(m, n, r_max, do_swap, do_backsub):
    result = 0
    
    result = result + 115 * r_max
    result = result + (27/8) * (n * r_max - (r_max - 1) * r_max / 2)
    result = result + (1/8) * ((m-1) * r_max - (r_max - 1) * r_max / 2)
    result = result + (40/8) * (m * n * r_max - m * (r_max - 1) * r_max / 2 - n * (r_max - 1) * r_max / 2 + (r_max - 1) * r_max * (2 * r_max - 1) / 6 - n * r_max + (r_max - 1) * r_max / 2)
    
    if do_swap:
        result = result + (2/8) * (m * r_max - (r_max - 1) * r_max / 2)
        result = result + (4/8) * (r_max - 1) * r_max / 2
    
    if do_backsub:
        result = result + (33/8) * (r_max - 1) * r_max / 2
        result = result + (33/8) * (n - r_max) * (r_max - 1) * r_max / 2
    
    return result

def solve_cycles():
    meds_n = evaluate('n')
    meds_m = evaluate('m')
    return (1 / 8) * (meds_m - 3) * (meds_m - 1) * (28 * meds_m + 28 * meds_n + 16)

def parse_matmul(function, cycles):
    function_parts = function.split('_')
    m = function_parts[3]
    o = function_parts[4]
    n = function_parts[5]
    cycle_bound = matmul_cycles(evaluate(m), evaluate(o), evaluate(n))
    return {
        "name": "Matrix Multiplication",
        "input": f"$A$: ${m} \\times {o}$ & $B$: ${o} \\times {n}$",
        "cycles": int(cycles),
        "bound": cycle_bound,
        "ratio": int(cycles) / cycle_bound
    }

def parse_matsyst(function, cycles, first):
    function_parts = function.split('_')
    m = function_parts[3]
    n = function_parts[4]
    max_r = function_parts[5]
    do_swap = int(function_parts[6])
    do_backsub = int(function_parts[7])
    cycle_bound = matsyst_cycles(evaluate(m), evaluate(n), evaluate(max_r), do_swap, do_backsub)
    fun_name = "Matrix Systemization" if first else "Matrix Syst."
    name = f"{fun_name}{' (' + replace_minus(max_r) + '$^{**}$)' if max_r != m else ''}{' (swap$^{***}$)' if do_swap == 1 else ''}{' (bsub$^{*}$)' if do_backsub == 1 else ''}"
    return {
        "name": name,
        "input": f"$A$: ${replace_minus(m)} \\times {replace_minus(n)}$&",
        "cycles": int(cycles),
        "bound": cycle_bound,
        "ratio": int(cycles) / cycle_bound
    }

def parse_solve(function, cycles):
    function_parts = function.split('_')
    m = function_parts[3]
    n = function_parts[4]
    cycle_bound = solve_cycles()
    return {
        "name": "System Solving",
        "input": f"$C$: ${m} \\times {n}$ &",
        "cycles": int(cycles),
        "bound": cycle_bound,
        "ratio": int(cycles) / cycle_bound
    }

def parse_data(data):
    result = []
    new_stage = None
    stage = None
    for row in data:
        function = row[0]
        cycles = row[1]
        if function.startswith('pmod_mat_mul'):
            new_line = parse_matmul(function, cycles)
            new_stage = 'matmul'
        if function.startswith('pmod_mat_syst'):
            new_line = parse_matsyst(function, cycles, stage != 'matsyst')
            new_stage = 'matsyst'
        if function.startswith('solve_opt_part'):
            new_line = parse_solve(function, cycles)
            new_stage = 'solve'
        
        if new_stage != stage:
            if stage is not None:
                result.append('rule')
            stage = new_stage
        result.append(new_line)
    return result

def generate_table(file: str) -> str:
    data = load_csv(file)
    parsed_data = parse_data(data)
    table_data = []
    for row in parsed_data:
        if row == 'rule':
            table_data.append("\\midrule")
        else:
            table_data.append(f"    {row['name']} & {row['input']} & {row['cycles']:.0f} & {row['bound']:.0f} & {row['ratio']:.2f}\\\\")
    return TABLE_TEMPLATE % '\n'.join(table_data)

print(generate_table('results.csv'))