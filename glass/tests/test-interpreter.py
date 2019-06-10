import argparse
from subprocess import Popen, PIPE
import sys

parser = argparse.ArgumentParser()
parser.add_argument('--exe', type=str, help='Path to the executable')
parser.add_argument('sources', nargs='+', type=str, help='Path to the glass source file')
parser.add_argument('--expected', type=str, help='Path to the expected output')
args = parser.parse_args()

process = Popen([args.exe] + args.sources, stdout=PIPE, stderr=PIPE)
stdout, stderr = process.communicate()

with open(args.expected, 'rb') as file:
    expected = file.read()
    if stdout != expected:
        print('---Expected Output---')
        print(expected)
        print('---Actual Output---')
        print(stdout)
        sys.exit(1)
