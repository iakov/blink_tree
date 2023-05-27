import matplotlib.pyplot as plt
import pandas as pd
import numpy as np
import sys
import os


def main():
    assert len(sys.argv) == 3
    src_filename = sys.argv[1]
    rand_type = os.getenv('RAND_TYPE', "AUTOINCREAMENT")
    start_tree_size = int(os.getenv('START_TREE_SIZE', 1000000))

    # tree_type start_tree_size threads read_threads key_size read_threads_part rand_type event workload amount avg
    data = pd.read_csv(src_filename, header=None, delimiter=r"\s+", names=["tree_type", "start_tree_size", "threads",
                       "read_threads", "key_size", "read_threads_part", "rand_type", "event",
                       "workload", "amount", "avg"])


    # THREADS = (1 4 12 16 24 32 48 64 128)
    #
    # READ_THREADS_PERC = (0.0 0.2 0.5 0.8)
    #
    # KEY_SIZES = (4 8 16 32 64 128)
    #
    # START_TREE_SIZES = (0 1000000 10000000)
    #
    # RANDOM AUTOINCREAMENT SHAREDAUTOINCREAMENT
    key_sizes = data.key_size.unique()

    data_copy = data

    fig, axes = plt.subplots(nrows=len(key_sizes), ncols=2, figsize=(28, 14))

    fig.subplots_adjust(hspace=0.1)

    current_position = 0
    for key_size in key_sizes:
        data = data_copy
        data = data.loc[(data['key_size'] == key_size) & (data['rand_type'] == rand_type)
                        & (data['start_tree_size'] == start_tree_size)
                        & (data['event'] == "write")]

        data = data.drop_duplicates(subset=['tree_type', 'read_threads', 'threads'])
        data = data.sort_values(['threads', 'read_threads'])

        if data.empty:
            print("ERROR Empty event: " + event)
            continue

        # print(data.head())
        data['full_event'] = data.apply(lambda row: row.event + "_" + row.workload, axis=1)
        data['full_thread'] = data.apply(lambda row: str(row.read_threads) + ":" + str(row.threads), axis=1)

        data['tree_type'] = pd.Categorical(data['tree_type'], categories=['bplus_tree', 'jaluta_tree'])
        data = pd.get_dummies(data, columns=['tree_type'])

        data['bplus'] = data.apply(lambda row: row.tree_type_bplus_tree * row.avg, axis=1)
        data['jaluta'] = data.apply(lambda row: row.tree_type_jaluta_tree * row.avg, axis=1)

        data = data[['full_thread', 'bplus', 'jaluta']]
        data = data.groupby('full_thread', sort=False).sum()

        ax = data.plot.bar(xlabel="", ax=axes[current_position, 0], rot=90,
                             fontsize=9) #xlabel="read threads:total threads"
        current_position += 1
        handles, labels = ax.get_legend_handles_labels()
        ax.get_legend().remove()
        ax.yaxis.set_label_position("right")
        ax.set_ylabel(key_size, rotation=0, fontsize=10, labelpad=65)
        ax.yaxis.get_major_formatter().set_scientific(False)
        if key_size != key_sizes[-1]:
            ax.get_xaxis().set_visible(False)

        fig.legend(handles, labels, loc='upper right')


    current_position = 0
    for key_size in key_sizes:
        data = data_copy
        data = data.loc[(data['key_size'] == key_size) & (data['rand_type'] == rand_type)
                        & (data['start_tree_size'] == start_tree_size)
                        & (data['event'] == "read")]

        data = data.drop_duplicates(subset=['tree_type', 'read_threads', 'threads'])
        data = data.sort_values(['threads', 'read_threads'])

        if data.empty:
            print("ERROR Empty event: " + event)
            continue

        # print(data.head())
        data['full_event'] = data.apply(lambda row: row.event + "_" + row.workload, axis=1)
        data['full_thread'] = data.apply(lambda row: str(row.read_threads) + ":" + str(row.threads), axis=1)

        data['tree_type'] = pd.Categorical(data['tree_type'], categories=['bplus_tree', 'jaluta_tree'])
        data = pd.get_dummies(data, columns=['tree_type'])

        data['bplus'] = data.apply(lambda row: row.tree_type_bplus_tree * row.avg, axis=1)
        data['jaluta'] = data.apply(lambda row: row.tree_type_jaluta_tree * row.avg, axis=1)

        data = data[['full_thread', 'bplus', 'jaluta']]
        data = data.groupby('full_thread', sort=False).sum()

        ax = data.plot.bar(xlabel="", ax=axes[current_position, 1], rot=90,
                             fontsize=9) #xlabel="read threads:total threads"
        current_position += 1
        ax.get_legend().remove()
        ax.yaxis.get_major_formatter().set_scientific(False)

        if key_size != key_sizes[-1]:
            ax.get_xaxis().set_visible(False)


    fig.suptitle("Start tree size: "  + str(start_tree_size) + " rand type: " + rand_type)
    filename = sys.argv[2]
    fig.savefig(filename)

if __name__ == '__main__':
    main()

