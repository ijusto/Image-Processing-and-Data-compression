import matplotlib.pyplot as plt
import pandas as pd
import sys

if len(sys.argv) == 1:
    R_fname = 'R.csv'
    G_fname = 'G.csv'
    B_fname = 'B.csv'

    R_data = pd.read_csv(R_fname, header=None)
    G_data = pd.read_csv(G_fname, header=None)
    B_data = pd.read_csv(B_fname, header=None)

    plt.hist(R_data[0], 256, weights=R_data[1], color='r')
    plt.hist(G_data[0], 256, weights=G_data[1], color='g')
    plt.hist(B_data[0][1:-1], 254, weights=B_data[1][1:-1], color='b')
    plt.show()
else:
    Gr_fname = 'Gr.csv'
    Gr_data = pd.read_csv(Gr_fname, header=None)
    plt.hist(Gr_data[0], 254, weights=Gr_data[1], color='gray')
    plt.show()
    
