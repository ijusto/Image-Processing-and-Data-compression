import matplotlib.pyplot as plt
import pandas as pd

leftChannelData = pd.read_csv("./src/audio/audioHistograms/leftChannel.csv", sep=',',header=None)
rightChannelData = pd.read_csv("./src/audio/audioHistograms/rightChannel.csv", sep=',',header=None)
monoData = pd.read_csv("./src/audio/audioHistograms/mono.csv", sep=',',header=None)

leftChannelData.hist(bins=500)
plt.title("Histogram of Left channel")
plt.xlabel("Samples")
plt.ylabel("Freq")
plt.savefig("./src/audio/audioHistograms/Left_channel.png")

rightChannelData.hist(bins=500)
plt.title("Histogram of Right channel")
plt.xlabel("Samples")
plt.ylabel("Freq")
plt.savefig("./src/audio/audioHistograms/Right_channel.png")

monoData.hist(bins=500)
plt.title("Histogram of mono")
plt.xlabel("Samples")
plt.ylabel("Freq")
plt.savefig("./src/audio/audioHistograms/Mono.png")