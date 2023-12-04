(This is a template. Delete this line and fill in the sections below)
# Threaded Merge Sort Experiments


## Host 1: [Yassine's MacBook]

- CPU: Apple M1 Pro
- Cores: 10 (8 performance and 2 efficiency)
- Cache size (if known): 
- RAM: 32 GB
- Storage (if known): 1TB
- OS: macOS (Ventura 13.0.1)

### Input data

*Briefly describe how large your data set is and how you created it. Also include how long `msort` took to sort it.*

I am using a data set of one-hundred-million created with the command below:

shuf -i1-100000000 > hundred-million.txt

`msort` completed in 17.744760 seconds.

### Experiments

*Replace X, Y, Z with the number of threads used in each experiment set.*

574 Processes running before each experiment
Command used: `ps aux | wc -l`

#### 1 Threads

Command used to run experiment: `MSORT_THREADS=1 ./tmsort 100000000 < hundred-million.txt > /dev/null`

Sorting portion timings:

1. 3.282404 seconds
2. 3.349102 seconds
3. 3.288315 seconds
4. 3.341340 seconds

#### 2 Threads

Command used to run experiment: `MSORT_THREADS=2 ./tmsort 100000000 < hundred-million.txt > /dev/null`

Sorting portion timings:

1. 3.209543 seconds
2. 3.266205 seconds
3. 3.277608 seconds
4. 3.289155 seconds

#### 3 Threads

Command used to run experiment: `MSORT_THREADS=3 ./tmsort 100000000 < hundred-million.txt > /dev/null`

Sorting portion timings:

1. 3.425718 seconds
2. 3.270077 seconds
3. 3.309261 seconds
4. 3.318026 seconds

#### 4 Threads

Command used to run experiment: `MSORT_THREADS=4 ./tmsort 100000000 < hundred-million.txt > /dev/null`

Sorting portion timings:

1. 3.402875 seconds
2. 3.410064 seconds
3. 3.287895 seconds
4. 3.378169 seconds

#### 8 Threads

Command used to run experiment: `MSORT_THREADS=8 ./tmsort 100000000 < hundred-million.txt > /dev/null`

Sorting portion timings:

1. 3.354376 seconds
2. 3.212787 seconds
3. 3.448287 seconds
4. 4.247704 seconds

#### 64 Threads

Command used to run experiment: `MSORT_THREADS=64 ./tmsort 100000000 < hundred-million.txt > /dev/null`

Sorting portion timings:

1. 5.959581 seconds
2. 6.534976 seconds
3. 6.761779 seconds
4. 4.816433 seconds

#### 128 Threads

Command used to run experiment: `MSORT_THREADS=128 ./tmsort 100000000 < hundred-million.txt > /dev/null`

Sorting portion timings:

1. 5.926170 seconds
2. 5.627533 seconds
3. 7.271670 seconds
4. 7.464516 seconds

*repeat sections as needed*

## Host 2: [Sunny's Windows Machine with WSL2]

- CPU: Intel(R) Core(TM) i7-1065G7 CPU @ 1.30GHz
- Cores: 4
- Cache size (if known): 8 MB
- RAM: 16 GB
- Storage (if known): 512 GB 
- OS: Ubuntu Linux

### Input data

*Briefly describe how large your data set is and how you created it. Also include how long `msort` took to sort it.*

I am using a data set of one-hundred-million created with the command below:

shuf -i1-100000000 > hundred-million.txt

`msort` completed in 37.060358 seconds.

### Experiments

*Replace X, Y, Z with the number of threads used in each experiment set.*

21 Processes running before each experiment
Command used: `ps aux | wc -l`

#### 1 Threads

Command used to run experiment: `MSORT_THREADS=1 ./tmsort 100000000 < hundred-million.txt > /dev/null`

Sorting portion timings:

1. 10.059318 seconds
2. 11.301961 seconds
3. 11.901849 seconds
4. 10.298040 seconds

#### 2 Threads

Command used to run experiment: `MSORT_THREADS=2 ./tmsort 100000000 < hundred-million.txt > /dev/null`

Sorting portion timings:

1. 9.909598 seconds
2. 12.614479 seconds
3. 10.211207 seconds
4. 10.797817 seconds

#### 3 Threads

Command used to run experiment: `MSORT_THREADS=3 ./tmsort 100000000 < hundred-million.txt > /dev/null`

Sorting portion timings:

1. 11.873723 seconds
2. 12.424212 seconds
3. 11.988710 seconds
4. 13.408217 seconds

#### 4 Threads

Command used to run experiment: `MSORT_THREADS=4 ./tmsort 100000000 < hundred-million.txt > /dev/null`

Sorting portion timings:

1. 11.044673 seconds
2. 11.335354 seconds
3. 10.585671 seconds
4. 12.228276 seconds

#### 8 Threads

Command used to run experiment: `MSORT_THREADS=8 ./tmsort 100000000 < hundred-million.txt > /dev/null`

Sorting portion timings:

1. 11.179781 seconds
2. 14.754524 seconds
3. 12.642763 seconds
4. 10.827485 seconds

#### 64 Threads

Command used to run experiment: `MSORT_THREADS=64 ./tmsort 100000000 < hundred-million.txt > /dev/null`

Sorting portion timings:

1. 11.483831 seconds
2. 11.410310 seconds
3. 14.593022 seconds
4. 12.125908 seconds

#### 128 Threads

Command used to run experiment: `MSORT_THREADS=128 ./tmsort 100000000 < hundred-million.txt > /dev/null`

Sorting portion timings:

1. 11.502017 seconds
2. 12.407886 seconds
3. 11.331518 seconds
4. 12.783450 seconds


## Observations and Conclusions

*Reflect on the experiment results and the optimal number of threads for your concurrent merge sort implementation on different hosts or platforms. Try to explain why the performance stops improving or even starts deteriorating at certain thread counts.*

For Yassine's experiments we found that the optimal number of threads was between 1-3 threads. We could not test lower than 1 thread, and going higher than 4 threads led to sorting increase of a few seconds (from 3 to 7 seconds). We are not too sure why this behaviour occurs, but we speculate that it may be how Apple M1 technology handles threading and processes. This experiment definitely went against our intuition of what would happen. 

For Sunny's experiments we found that the sorting times were all slower, which can be attributed to differences in machines. However, we also found that all the runs with 1-4 threads, as well as 8, 64, and 128 produced very similar results, usually around 10 - 13 seconds. This is different from Yassine's results, where the sort times increased with higher thread counts. This might be because our laptop uses an Intel CPU with x86 architecture, which handles threading differently from the M1. This also goes against our expectation that sort time would decrease as thread count increased.
