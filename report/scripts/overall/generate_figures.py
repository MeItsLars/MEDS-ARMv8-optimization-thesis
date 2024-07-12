import csv
import os
import matplotlib.pyplot as plt
from matplotlib.colors import to_rgb, to_hex
import seaborn as sns
import numpy as np

# Set the style of the plots
sns.set_theme()

def lighten_color(color, amount=0.5):
    try:
        c = np.array(to_rgb(color))
        white = np.array([1.0, 1.0, 1.0])
        return to_hex((1 - amount) * c + amount * white)
    except ValueError:
        return color

# Original "deep" colors
deep_blue = '#4C72B0'
deep_orange = '#DD8452'
deep_green = '#55A868'

# Lightened colors
light_blue = lighten_color(deep_blue, 0.4)
light_orange = lighten_color(deep_orange, 0.4)
light_green = lighten_color(deep_green, 0.4)

# Combine into a new palette
custom_palette = [deep_blue, light_blue, deep_orange, light_orange, deep_green, light_green]

# Set the custom palette in Seaborn
sns.set_palette(custom_palette)

THIS_FILE_PATH = os.path.dirname(os.path.abspath(__file__))
FILE_PATH = os.path.join(THIS_FILE_PATH, "results.csv")

def generate_barplot(parsed_data, filename):
    # Create and configure the figure and the two subplots
    fig, (ax1, ax2) = plt.subplots(1, 2, figsize=(12, 10), gridspec_kw={'width_ratios': [1, 2]})

    width = 0.1

    ### ax1 ###
    # Create the bars for each category
    subcategories1 = ['Keygen']
    num_techniques1 = len(parsed_data)
    x1 = np.arange(len(subcategories1))

    for i, (_, values) in enumerate(parsed_data.items()):
        extra = (i // 2 - 1) * 0.5
        ax1.bar(x1 + (i - num_techniques1 / 2 + 0.5 + extra) * width, values[:1], width)

    # Add labels
    ax1.set_ylabel('MCycles')
    ax1.set_xticks(x1)
    ax1.set_xticklabels(subcategories1)

    ### ax2 ###
    # Create the bars for each category
    subcategories2 = ['Sign', 'Verify']
    num_techniques2 = len(parsed_data)
    x2 = np.arange(len(subcategories2))

    for i, (_, values) in enumerate(parsed_data.items()):
        extra = (i // 2 - 1) * 0.5
        ax2.bar(x2 + (i - num_techniques2 / 2 + 0.5 + extra) * width * 1.3, values[1:], width * 1.3)

    # Add labels
    ax2.set_ylabel('MCycles')
    ax2.set_xticks(x2)
    ax2.set_xticklabels(subcategories2)

    # Set layout settings
    fig.subplots_adjust(top=0.6)
    fig.subplots_adjust(left=0.15)
    fig.subplots_adjust(right=0.85)
    fig.subplots_adjust(bottom=0.15)
    fig.subplots_adjust(wspace=0.2)

    # Create a single legend for both bar plots
    handles = [plt.Rectangle((0, 0), 1, 1, color=custom_palette[i]) for i in range(num_techniques2)]
    fig.legend(handles=handles, labels=parsed_data.keys(), loc='upper center', bbox_to_anchor=(0.5, 0.1), ncol=3)

    # Export images
    plt.savefig(os.path.join(THIS_FILE_PATH, "..", "..", "imgs", "plots", filename), bbox_inches='tight')
    # plt.show()

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

def load_csv():
    with open(FILE_PATH, 'r') as file:
        reader = csv.reader(file)
        data = list(reader)
    return data

data = load_csv()
parsed_data = parse_data(data)
generate_barplot(parsed_data["MEDS-21595"], "barplot_MEDS-21595.png")
generate_barplot(parsed_data["MEDS-55520"], "barplot_MEDS-55520.png")
generate_barplot(parsed_data["MEDS-122000"], "barplot_MEDS-122000.png")
