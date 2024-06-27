import csv
import os

THIS_FILE_PATH = os.path.dirname(os.path.abspath(__file__))

TABLE_TEMPLATE = """
  \\begin{tabular}{llrrr}
    \\toprule
    \\textbf{Function} & \\textbf{Input Size} & \\textbf{Cycles} & \\textbf{Bound} & \\textbf{Ratio}\\\\
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

def matmul_cycles(m, o, n):
    return (1 / 64.0) * m * n * (18 * o + 88)

def parse_matmul(function, cycles):
    function_parts = function.split('_')
    m = evaluate(function_parts[3])
    o = evaluate(function_parts[4])
    n = evaluate(function_parts[5])
    return {
        "name": "Matrix Mult.",
        "input": f"$A$: ${m} \\times {o} \\quad B$: ${o} \\times {n}$",
        "cycles": int(cycles),
        "bound": matmul_cycles(m, o, n),
        "ratio": int(cycles) / matmul_cycles(m, o, n)
    }

def parse_data(data):
    result = []
    new_stage = None
    stage = None
    for row in data:
        function = row[0]
        cycles = row[1]
        if function.startswith('pmod_mat_mul'):
            result.append(parse_matmul(function, cycles))
            new_stage = 'matmul'
        
        if new_stage != stage:
            result.append('rule')
            stage = new_stage
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