import csv
import os

THIS_FILE_PATH = os.path.dirname(os.path.abspath(__file__))

TABLE_TEMPLATE = """
  \\begin{tabular}{lllrrrr}
    \\toprule
    \\textbf{Function} & \\multicolumn{2}{c}{\\textbf{Input size}} & \\textbf{Factor} & \\textbf{Factor}\\\\
    & \\multicolumn{1}{c}{\\textit{A}} & \\multicolumn{1}{c}{\\textit{B}} & \\text{Theoretical} & \\text{Actual} & & \\\\
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

def matmul_factor(m, o, n):
    a = m * o + o * n + 2 * m * n * o + m * o * 5 + m * n
    b = (1/4.0) * m * n * o + (1/8.0) * m * n + (11/8.0) * m * o + (1/8) * n * o
    return a / b

def matsyst_factor(m, n, r_max, do_swap, do_backsub):
    a = 0
    a = a + r_max * n * (1/4)
    a = a + 115 * r_max
    a = a + (13/8) * (n * r_max - (r_max - 1) * r_max / 2)
    a = a + (23/8) * (m * n * r_max - m * (r_max - 1) * r_max / 2 - n * (r_max - 1) * r_max / 2 + (r_max - 1) * r_max * (2 * r_max - 1) / 6 - n * r_max + (r_max - 1) * r_max / 2)
    
    if do_swap:
        a = a + (1/8) * (m * r_max - (r_max - 1) * r_max / 2)
    
    if do_backsub:
        a = a + (18/8) * (r_max - 1) * r_max / 2
        a = a + (18/8) * (n - r_max) * (r_max - 1) * r_max / 2
    
    b = 0
    b = b + r_max * n * 2
    b = b + 115 * r_max
    b = b + 6 * (n * r_max - (r_max - 1) * r_max / 2)
    b = b + 11 * (m * n * r_max - m * (r_max - 1) * r_max / 2 - n * (r_max - 1) * r_max / 2 + (r_max - 1) * r_max * (2 * r_max - 1) / 6 - n * r_max + (r_max - 1) * r_max / 2)
    
    if do_swap:
        b = b + 1 * (m * r_max - (r_max - 1) * r_max / 2)
    
    if do_backsub:
        b = b + 11 * (r_max - 1) * r_max / 2
        b = b + 11 * (n - r_max) * (r_max - 1) * r_max / 2
    
    return b / a

def solve_factor():
    meds_n = evaluate('n')
    meds_m = evaluate('m')
    
    a = 0
    a += 9/4 * pow(meds_m, 3)
    a += 9/4 * pow(meds_m, 2) * meds_n
    a -= 23/4 * pow(meds_m, 2)
    a -= 11/2 * meds_m * meds_n
    a -= 2 * meds_m
    a += 3 * meds_n
    a += 5

    # 13 m^3 + 13 m^2 n - 47 m^2 - 36 m n + 25 m + 21 n + 5
    b = 0
    b += 13 * pow(meds_m, 3)
    b += 13 * pow(meds_m, 2) * meds_n
    b -= 47 * pow(meds_m, 2)
    b -= 36 * meds_m * meds_n
    b += 25 * meds_m
    b += 21 * meds_n
    b += 5

    return b / a

def parse_matmul(function, cycles, opt_cycles):
    function_parts = function.split('_')
    m = function_parts[3]
    o = function_parts[4]
    n = function_parts[5]
    actual_factor = int(cycles) / int(opt_cycles)
    calculated_factor = matmul_factor(evaluate(m), evaluate(o), evaluate(n))
    return {
        "name": "Matrix Multiplication",
        "input": f"${m} \\times {o}$ & ${o} \\times {n}$",
        "actual_factor": actual_factor,
        "calculated_factor": calculated_factor
    }

def parse_matsyst(function, cycles, opt_cycles, first):
    function_parts = function.split('_')
    m = function_parts[3]
    n = function_parts[4]
    max_r = function_parts[5]
    do_swap = int(function_parts[6])
    do_backsub = int(function_parts[7])
    actual_factor = int(cycles) / int(opt_cycles)
    calculated_factor = matsyst_factor(evaluate(m), evaluate(n), evaluate(max_r), do_swap, do_backsub)
    fun_name = "Matrix Systemizer" if first else "Matrix Syst."
    name = f"{fun_name}{' (' + replace_minus(max_r) + '$^{**}$)' if max_r != m else ''}{' (swap$^{***}$)' if do_swap == 1 else ''}{' (bsub$^{*}$)' if do_backsub == 1 else ''}"
    return {
        "name": name,
        "input": f"${replace_minus(m)} \\times {replace_minus(n)}$&",
        "actual_factor": actual_factor,
        "calculated_factor": calculated_factor
    }

def parse_solve(function, cycles, opt_cycles):
    function_parts = function.split('_')
    m = function_parts[3]
    n = function_parts[4]
    actual_factor = int(cycles) / int(opt_cycles)
    calculated_factor = solve_factor()
    return {
        "name": "Isometry Derivation (part)",
        "input": f"${m} \\times {n}$ &",
        "actual_factor": actual_factor,
        "calculated_factor": calculated_factor
    }

def parse_data(data):
    result = []
    new_stage = None
    stage = None
    for row in data:
        function = row[0]
        cycles = row[1]
        opt_cycles = row[2]
        if function.startswith('pmod_mat_mul'):
            new_line = parse_matmul(function, cycles, opt_cycles)
            new_stage = 'matmul'
        if function.startswith('pmod_mat_syst'):
            new_line = parse_matsyst(function, cycles, opt_cycles, stage != 'matsyst')
            new_stage = 'matsyst'
        if function.startswith('solve_opt_part'):
            new_line = parse_solve(function, cycles, opt_cycles)
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
            table_data.append(f"    {row['name']} & {row['input']} & {row['calculated_factor']:.1f} & {row['actual_factor']:.1f}\\\\")
    return TABLE_TEMPLATE % '\n'.join(table_data)

print(generate_table('results.csv'))