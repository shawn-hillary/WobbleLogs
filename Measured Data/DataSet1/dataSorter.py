"""Python Scrpit to Calculate the average wind spped and direction of each data set
   Shawn Hillary -  Project Wobble Logs 2019"""




import numpy as np
n = 4096
out = open("Averages.csv", "w")
out.write("Set Number"+";"+"Average Direction"+";"+"Average Speed"+"\n")
for i in range(237):           #Edit setNum according to sets in folder
    fname = str(i)+".csv"
    matrix = np.genfromtxt(fname, skip_header=1, delimiter=";", dtype=int)
    sums = matrix.sum(axis=0)
    direction = int(sums[4]/n)
    speed = int(sums[5]/n)
    out.write(str(i)+";"+str(direction)+";"+str(speed)+"\n")
out.close() 