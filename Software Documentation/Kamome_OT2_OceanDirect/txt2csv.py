#%% Import libraries
import pandas as pd
import numpy as np
import os
import re
import sys
import csv

#%% Open text file
def txt2csv(path_folder, txt_file, csv_file, file_id):
    path_file = f'{path_folder}{txt_file}'
    with open(path_file, 'r') as file:
        data = file.readlines()

    # Process the data
    processed_data = []
    for line in data[14:]:
        # Split each line by whitespace and strip any leading/trailing whitespace
        split_line = [item.strip() for item in re.split(r'\s+', line) if item]
        processed_data.append(split_line)

    # Save the processed data to a CSV file
    output_path = f'{path_folder}{csv_file}'
    with open(output_path, 'w', newline='') as output_file:
        writer = csv.writer(output_file)
        writer.writerows(processed_data)
#%% 
path_folder = 'color96/'
file_names = [file for file in os.listdir(path_folder) if file.endswith('.txt')]
print(file_names)

for file_id in file_names:
    if file_id.endswith('.txt'):
        txt_file = file_id
        csv_file = f'{file_id[:-4]}.csv'
        txt2csv(path_folder, txt_file, csv_file, file_id)

# %%
