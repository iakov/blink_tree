from matplotlib import pyplot as plt
import sys
import re

def find_height_change(l, start_index):
    if start_index >= len(l):
        return -1
    start_value = l[start_index]
    for i in range(start_value + 1, len(l)):
        if l[i] > start_value:
            return i
    return -1

def main():
    assert len(sys.argv) == 2
    tree_types = ["bplus", "jaluta"]
    random_types = ["random", "autoincreament", "sharedautoincreament"]


    fig, axes = plt.subplots(nrows=len(tree_types), ncols=len(random_types), figsize=(28, 14))


    position_row = 0
    for tree_type in tree_types:
        position_col = 0
        for random_type in random_types:
            rates   = []
            heights = []
            file_path = tree_type + "/inserted_rate_" + random_type[0]
            with open(file_path) as file:
                for l in file.readlines():
                    if "inserted" in l:
                        rates.append(int(re.search(r'\d+', l).group()))
                    if "hight" in l:
                        _, _, _, _, _, _, _, _,_, height, _, _ = l.split(" ")
                        heights.append(int(height))

            if position_row == len(tree_types) - 1:
                axes[position_row, position_col].set_xlabel(random_type, rotation=0, fontsize=10, labelpad=65)
            if position_col == len(random_types) - 1:
                axes[position_row, position_col].set_ylabel(tree_type, rotation=0, fontsize=10, labelpad=65)

            x = find_height_change(heights, 0)
            while x > 0:
                axes[position_row, position_col].axvline(x=x)
                x = find_height_change(heights, x + 1)
            axes[position_row, position_col].yaxis.set_label_position("right")
            axes[position_row, position_col].plot(rates, 'ro', markersize=1)
            axes[position_row, position_col].set_ylim(ymin=0)
            position_col += 1
        position_row += 1

    filename = sys.argv[1]
    fig.savefig(filename)

if __name__ == '__main__':
    main()