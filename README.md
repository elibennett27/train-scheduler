# P2: Multi-threaded Train Scheduler (mts)

## Implementation Status

### Features
Input file handling - Correctly reads train details with error handling
Thread creation - Creates one thread for each train
Priority scheduling - High-priority trains cross before low
Starvation prevention - Tracks consecutive same direction crossings
Output formatting - Matches specified format 


### Test Input File

input.txt - contains:

E 5 10
W 10 5
W 15 8
e 20 6
E 25 7
W 30 5
W 35 9
e 40 4

Estimated execution time - 5.9 seconds


Correct Scheduling Outputed
00:00:00.5 Train  0 is ready to go East
00:00:00.5 Train  0 is ON the main track going East
00:00:01.0 Train  1 is ready to go West
00:00:01.5 Train  2 is ready to go West
00:00:01.5 Train  0 is OFF the main track after going East
00:00:01.5 Train  1 is ON the main track going West
00:00:02.0 Train  3 is ready to go East
00:00:02.0 Train  1 is OFF the main track after going West
00:00:02.0 Train  2 is ON the main track going West
00:00:02.5 Train  4 is ready to go East
00:00:02.8 Train  2 is OFF the main track after going West
00:00:02.8 Train  4 is ON the main track going East
00:00:03.0 Train  5 is ready to go West
00:00:03.5 Train  6 is ready to go West
00:00:03.5 Train  4 is OFF the main track after going East
00:00:03.5 Train  5 is ON the main track going West
00:00:04.0 Train  7 is ready to go East
00:00:04.0 Train  5 is OFF the main track after going West
00:00:04.0 Train  6 is ON the main track going West
00:00:04.9 Train  6 is OFF the main track after going West
00:00:04.9 Train  3 is ON the main track going East
00:00:05.5 Train  3 is OFF the main track after going East
00:00:05.5 Train  7 is ON the main track going East
00:00:05.9 Train  7 is OFF the main track after going East

## Program Status

Program is complete.
