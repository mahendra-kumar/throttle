<b>Description: Refer Throttle_problem.pdf for details</b>
_______
Running a producer and consumer in separate threads.<br>
  Producer produces messages (sequential numbers) with priority.<br>
  Consumer processes messages with TPS (transactions/ second) restrictions using a "sliding window" model - between any two time points 1 second apart only a certain maximum number of messages will be processed.<br>

<b>Build:</b>
_______
Includes Visual C++ 2017 Project File.<br>
Can be built and run on other platforms, supporting C++14 or above directly from source.

<b>Usage:</b>
______

throttle.exe 300 4  10 > data.txt<br>
300: transactions/ second, default:100<br>
4: number of seconds producer will produce data, default: 5<br>
10: indicative producer speed [1 - 100], default: 50<br>
<br>
All params are optional.<br>
data.txt: redirect std out to file<br>

<b>Third Party Source:</b>
_______
date.h used for time point formatting as string<br>

<b>Output:</b>
_______
(to stdout, so can be redirected to file)

23:24:03.399744:	1 (0)

23:24:03.399744: UTC Time at which message "sent";<br>
1: Sequential number used as "message"<br>
(0): Message Priority<br>

<b>Steps:</b>
_____
1) To make the solution generic, maintainable ..., abstract the problem from the domain. The solution does not use any domain specific features/ knowledge - just pure CS.
2) Everything is inline, only for simplicity.
3) Messages are just sequential numbers with those divisible by 7 having higher priority (1 vs 0)
4) For this demo, the size of sliding window or TPS count is assumed not to change at runtime. Should work though ... but not tested.
