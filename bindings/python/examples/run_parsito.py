# This file is part of Parsito <http://github.com/ufal/parsito/>.
#
# Copyright 2015 Institute of Formal and Applied Linguistics, Faculty of
# Mathematics and Physics, Charles University in Prague, Czech Republic.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

import sys

from ufal.parsito import *

# In Python2, wrap sys.stdin and sys.stdout to work with unicode.
if sys.version_info[0] < 3:
  import codecs
  import locale
  encoding = locale.getpreferredencoding()
  sys.stdin = codecs.getreader(encoding)(sys.stdin)
  sys.stdout = codecs.getwriter(encoding)(sys.stdout)

if len(sys.argv) == 1:
  sys.stderr.write('Usage: %s parser_file\n' % sys.argv[0])
  sys.exit(1)

sys.stderr.write('Loading parser: ')
parser = Parser.load(sys.argv[1])
if not parser:
  sys.stderr.write("Cannot load parser from file '%s'\n" % sys.argv[1])
  sys.exit(1)
sys.stderr.write('done\n')

conlluInput = TreeInputFormat.newInputFormat("conllu");
conlluOutput = TreeOutputFormat.newOutputFormat("conllu");
tree = Tree()

not_eof = True
while not_eof:
  text = ''

  # Read block
  while True:
    line = sys.stdin.readline()
    not_eof = bool(line)
    if not not_eof: break
    line = line.rstrip('\r\n')
    text += line
    text += '\n';
    if not line: break


  # Parse
  conlluInput.setText(text)
  while conlluInput.nextTree(tree):
    parser.parse(tree)

    output = conlluOutput.writeTree(tree, conlluInput)
    sys.stdout.write(output)
  if conlluInput.lastError():
    sys.stderr.write("Cannot read input CoNLL-U: ")
    sys.stderr.write(conlluInput.lastError())
    sys.stderr.write("\n")
    sys.exit(1)
