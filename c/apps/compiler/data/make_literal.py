import argparse

def make_literal(array_name: str, data: str) -> str:
    lines = [
        '#include <stddef.h>',
        'const char *' + array_name + '[] = {'
    ]

    for line in data.splitlines():
        line = line.replace('\\', '\\\\')
        lines.append(f'"{line}",')

    lines.append('};')

    lines.append('const size_t ' + array_name + '_LINES = ' + str(len(lines) - 3) + ';')

    return '\n'.join(lines)

parser = argparse.ArgumentParser()
parser.add_argument('--array-name', type=str, help='Name of the generated array.')
parser.add_argument('--source', type=str, help='Name of the file to generate the array from.')
parser.add_argument('--out-name', type=str, help='Name of the file to write the array to.')
args = parser.parse_args()

with open(args.source, 'r') as file:
    data = file.read()

with open(args.out_name, 'w') as file:
    file.write(make_literal(args.array_name, data))
