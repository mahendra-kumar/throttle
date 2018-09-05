Usage:
______

throttle.exe 300 4  10 > data.txt

300: transactions/ second, default:100

4: number of seconds producer will produce data, default: 5

10: indicative producer speed [1 - 100], default: 50

All params are optional.

data.txt: redirect std out to file

Third Party Source:
_______
date.h used for time point formatting as string

Output:
_______
(to stdout, so can be redirected to file)

23:24:03.399744:	1 (0)

23:24:03.399744: UTC Time at which message sent

1: Sequential number used as "message"

0: Message Priority

Steps:
_____
1) To make the solution generic, maintainable ..., abstract the problem from the domain. The solution does not use any domain specific features/ knowledge - just pure CS.
2) Everything is inline, only for simplicity.
3) Messages are just sequential numbers with those divisible by 7 having higher priority (1 vs 0)
4) For this demo, the size of sliding window or TPC count is assumed not to change at runtime. Should work though ... but not tested.
