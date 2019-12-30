import numpy as np
import matplotlib.pyplot as plt
import random

def fft(fname):
    matrix = np.genfromtxt(fname, dtype=int, delimiter=";", skip_header=1)
    timestep = 0.01
    n = len(matrix)
    t = matrix.T[0]
    x = matrix.T[1]
    y = matrix.T[2]
    spd = matrix.T[5]
    t = t / 1000
    x = x/4095 * 9.81
    y = y/4095 * 9.81
    #spd = (3.3/2047)*(spd-248)*(32.4/1.6)
    freqs = np.fft.fftfreq(n, d=timestep)
    xf = np.fft.fft(x)
    xf = 2.0 * np.abs(xf/n)
    xf[0] = 0
    yf = np.fft.fft(y)
    yf = 2.0 * np.abs(yf/n)
    yf[0] = 0
    posPeakX = np.argmax(xf)
    peakX = freqs[posPeakX]
    posPeakY = np.argmax(yf)
    peakY = freqs[posPeakY] 
#   print(str(fname)+"\t AvgSpeed "+str(round(np.average(spd),3))+"\t FreqX: "+str(round(peakX,3))+"\t FreqY: "+str(round(peakY,3))    )
    return [freqs, xf, yf, t, x, y,spd,peakX,peakY]

def crossCor(fft1,fft2,n):
    X1scaler = sum(fft1[1])/n
    Y1scaler = sum(fft1[2])/n
    X2scaler = sum(fft2[1])/n
    Y2scaler = sum(fft2[2])/n
    fft1[1] = fft1[1]/X1scaler
    fft2[1] = fft2[1]/X2scaler
    fft1[2] = fft1[2]/Y1scaler
    fft2[2] = fft2[2]/Y2scaler
    Xcor = 0
    Ycor = 0
    for samp in range(4096):
        # Xcor += ((fft1[1])[samp] * (fft2[1])[samp])
        Ycor += ((fft1[2])[samp] * (fft2[2])[samp])
    speedDif = abs(np.average(fft1[6]) - np.average(fft2[6]))
    cor = Ycor
    return [cor,speedDif]

def fullCompare(s1,n):
    corPlot = []
    spdDifPlot = []
    st = []
    added = []
    for inner in range(502):
        setName = str(inner)+".csv"
        setName2 = str(s1)+".csv"
        set_one = fft(setName2)
        set_two = fft(setName)
        [c,s] =  crossCor(set_one,set_two,n)
        corPlot.append(c)
        spdDifPlot.append(s*50)
        st.append(inner)
        added.append(c + s*50)
    #plt.figure()
    #plt.title("Set"+str(s1))
    #plt.plot(st,corPlot,'b')
    #plt.plot(st,spdDifPlot,'g')
    #plt.plot(st,added,'r')

def train():
    xf = [0]
    yf = [0]
    for outer in range(503):
        setName = str(outer)+".csv"
        thisSet = fft(setName)
        xf += (thisSet[1]*(1/502))
        yf += (thisSet[2]*(1/502))
    #plt.figure()
    #plt.plot(thisSet[0],xf,'b')
    #plt.plot(thisSet[0],yf,'orange')
    return[xf,yf]

def trainedCompare(s1,n):
    xf,yf = train()
    setName = str(s1)+".csv"
    fft2 = fft(setName)
    X1scaler = sum(xf)/n
    Y1scaler = sum(yf)/n
    X2scaler = sum(fft2[1])/n
    Y2scaler = sum(fft2[2])/n
    xf = xf/X1scaler
    yf = yf/Y1scaler
    fft2[1] = fft2[1]/X2scaler
    fft2[2] = fft2[2]/Y2scaler
    Xcor = 0
    Ycor = 0
    for samp in range(4096):
        Xcor += (xf[samp] * (fft2[1])[samp])
        Ycor += (yf[samp] * (fft2[2])[samp])
    print(Xcor)
    print(Ycor)
    plt.figure("X-axis")
    plt.plot(fft2[0],fft2[1])
    plt.plot(fft2[0],xf,'k')

    plt.figure("Y-axis")
    plt.plot(fft2[0],fft2[2])  
    plt.plot(fft2[0],yf,'k')

    return [Xcor, Ycor]

n =4096
for outer in range(5):
    sNum = random.randint(0,503)
    trainedCompare(sNum,n)
    print(sNum)
#print(sNum)
#test = fft(str(sNum)+".csv")
#plt.plot(test[0],test[1])


