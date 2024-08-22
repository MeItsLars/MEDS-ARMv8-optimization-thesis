import csv
import os

FILE_PATH_CORTEX_A72 = os.path.join(os.path.dirname(os.path.abspath(__file__)), "results_cortexa72.csv")
FILE_PATH_APPLE_M2 = os.path.join(os.path.dirname(os.path.abspath(__file__)), "results_m2.csv")

TABLE_TEMPLATE = """
    \\begin{tabular}{l r @{\hskip 5pt} r r @{\hskip 5pt} r r @{\hskip 5pt} r}
        \\toprule
        \\textbf{Variant} & \\multicolumn{2}{c}{\\textbf{Keygen}} & \\multicolumn{2}{c}{\\textbf{Sign}} & \\multicolumn{2}{c}{\\textbf{Verify}} \\\\
        \\midrule
%s
        \\bottomrule
    \\end{tabular}
"""

def generate_table(data):
    table_data = ""
    ref_keygen_cycles = data["Reference"][0]
    ref_sign_cycles = data["Reference"][1]
    ref_verify_cycles = data["Reference"][2]
    for technique, cycles in data.items():
        keygen_cycles = cycles[0]
        sign_cycles = cycles[1]
        verify_cycles = cycles[2]
        keygen_speedup = ref_keygen_cycles / keygen_cycles
        sign_speedup = ref_sign_cycles / sign_cycles
        verify_speedup = ref_verify_cycles / verify_cycles
        table_data += "      %s & %.1f & \\footnotesize$(\\times %.1f)$ & %.1f & \\footnotesize$(\\times %.1f)$ & %.1f & \\footnotesize$(\\times %.1f)$ \\\\\n" % (technique, keygen_cycles, keygen_speedup, sign_cycles, sign_speedup, verify_cycles, verify_speedup)
    return TABLE_TEMPLATE % table_data[:-1]

def parse_data(data):
    parsed_data = {}
    for row in data:
        meds_param = row[0]
        technique = row[1]
        values = row[2:]
        if meds_param not in parsed_data:
            parsed_data[meds_param] = {}
        
        # These values should contain 1 decimal place
        values_to_mcycles = [round(float(value) / 1e6, 1) for value in values]
        parsed_data[meds_param][technique] = values_to_mcycles
    return parsed_data

def load_csv(file):
    with open(file, 'r') as file:
        reader = csv.reader(file)
        data = list(reader)
    return data

def print_tables(file):
    print()
    print("RESULTS FOR %s" % file)
    print()
    data = load_csv(file)
    parsed_data = parse_data(data)
    print(generate_table(parsed_data["MEDS-21595"]))
    print(generate_table(parsed_data["MEDS-55520"]))
    print(generate_table(parsed_data["MEDS-122000"]))

if __name__ == "__main__":
    print_tables(FILE_PATH_CORTEX_A72)
    print_tables(FILE_PATH_APPLE_M2)