import csv
import os

FILE_PATH = os.path.join(os.path.dirname(os.path.abspath(__file__)), "results.csv")

TABLE_TEMPLATE = """
  \\begin{table}[]
    \\centering
    \\begin{tabular}{lrrrrrrrrr}
      \\toprule
      & \\multicolumn{3}{c}{\\textbf{MEDS-21595}} & \\multicolumn{3}{c}{\\textbf{MEDS-55520}} & \\multicolumn{3}{c}{\\textbf{MEDS-122000}} \\\\
      & \\textbf{Keygen} & \\textbf{Sign} & \\textbf{Verify} & \\textbf{Keygen} & \\textbf{Sign} & \\textbf{Verify} & \\textbf{Keygen} & \\textbf{Sign} & \\textbf{Verify} \\\\
      \\midrule
%s
      \\bottomrule
    \\end{tabular}
    \\caption{Benchmarking results for all variants of MEDS. The values represent the number of megacycles required to execute that algorithm.}
    \\label{tab:benchmark_results}
  \\end{table}
"""

def generate_table(data):
    table_data = ""
    for algorithm, meds_data in data.items():
        MEDS_21595 = meds_data["MEDS-21595"]
        MEDS_55520 = meds_data["MEDS-55520"]
        MEDS_122000 = meds_data["MEDS-122000"]
        table_data += "      %s & %s & %s & %s & %s & %s & %s & %s & %s & %s \\\\\n" % (algorithm, MEDS_21595[0], MEDS_21595[1], MEDS_21595[2], MEDS_55520[0], MEDS_55520[1], MEDS_55520[2], MEDS_122000[0], MEDS_122000[1], MEDS_122000[2])
    return TABLE_TEMPLATE % table_data

def parse_data(data):
    parsed_data = {}
    for row in data:
        meds_param = row[0]
        technique = row[1]
        values = row[2:]
        if technique not in parsed_data:
            parsed_data[technique] = {}
        
        # These values should contain 1 decimal place
        values_to_mcycles = [round(float(value) / 1e6, 1) for value in values]
        parsed_data[technique][meds_param] = values_to_mcycles
    return parsed_data

def load_csv():
    with open(FILE_PATH, 'r') as file:
        reader = csv.reader(file)
        data = list(reader)
    return data

def main():
    data = load_csv()
    parsed_data = parse_data(data)
    table = generate_table(parsed_data)
    print(table)

if __name__ == "__main__":
    main()
