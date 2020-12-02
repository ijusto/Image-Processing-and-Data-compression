import matplotlib.pyplot as plt
import pandas as pd

leftChannelData = pd.read_csv("leftResidualsChannel.csv", sep=',',header=None)
rightChannelData = pd.read_csv("rightResidualsChannel.csv", sep=',',header=None)

leftChannelData.hist(bins=500)
plt.title("Histogram of Left channel residuals")
plt.xlabel("Samples")
plt.ylabel("Freq")
plt.savefig("l_res.png")

rightChannelData.hist(bins=500)
plt.title("Histogram of Right channel residuals")
plt.xlabel("Samples")
plt.ylabel("Freq")
plt.savefig("r_res.png")
