import matplotlib.pyplot as plt
import pandas as pd
import glob

csv_files = glob.glob('*.csv')

for f in csv_files:
    data = pd.read_csv(f, sep=',',header=None)
    data.hist(bins=500)
    plt.title("Histogram")
    plt.xlabel("Samples")
    plt.ylabel("Freq")
    plt.savefig("{}.png".format(f.split(".")[0]))

