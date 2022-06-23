import sys
import re

regex = r'\[\d+\] id=\d+ EXIT w=(\d+\.\d+) ta=(\d+\.\d+)'
afiles = ['q50d0', 'q50d5', 'q50d10', 'q50d15', 'q50d20', 'q50d25']
bfiles = ['q100d0', 'q100d5', 'q100d10', 'q100d15', 'q100d20', 'q100d25']
cfiles = ['q250d0', 'q250d5', 'q250d10', 'q250d15', 'q250d20', 'q250d25']
dfiles = ['q500d0', 'q500d5', 'q500d10', 'q500d15', 'q500d20', 'q500d25']
files = [afiles, bfiles, cfiles, dfiles]

if __name__ == '__main__':
    for set in files:
        for filename in set:
            avg_wait = 0
            avg_turn = 0
            num_exits = 0
            for i in range(20):
                file = open( './sim/' + filename + '-' + str(i+1) + '.txt', 'r')
                for line in file:
                    match = re.match(regex, line)
                    if match:
                        num_exits = num_exits + 1
                        avg_wait = avg_wait + float(match.group(1))
                        avg_turn = avg_turn + float(match.group(2))
            avg_wait = round(avg_wait / num_exits, 2)
            avg_turn = round(avg_turn / num_exits, 2)
            print(f'FILE: {filename} avg_wait={avg_wait} avg_turn={avg_turn}')