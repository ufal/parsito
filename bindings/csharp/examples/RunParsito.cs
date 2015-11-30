// This file is part of Parsito <http://github.com/ufal/parsito/>.
//
// Copyright 2015 Institute of Formal and Applied Linguistics, Faculty of
// Mathematics and Physics, Charles University in Prague, Czech Republic.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

using Ufal.Parsito;
using System;
using System.Text;
using System.Xml;

class RunParsito {
    public static int Main(string[] args) {
        if (args.Length < 1) {
            Console.Error.WriteLine("Usage: RunParsito parser_file");
            return 1;
        }

        Console.Error.Write("Loading parser: ");
        Parser parser = Parser.load(args[0]);
        if (parser == null) {
            Console.Error.WriteLine("Cannot load parser from file '{0}'", args[0]);
            return 1;
        }
        Console.Error.WriteLine("done");

        TreeInputFormat conlluInput = TreeInputFormat.newInputFormat("conllu");
        TreeOutputFormat conlluOutput = TreeOutputFormat.newOutputFormat("conllu");

        Tree tree = new Tree();
        for (bool not_eof = true; not_eof; ) {
            string line;
            StringBuilder textBuilder = new StringBuilder();

            // Read block
            while ((not_eof = (line = Console.In.ReadLine()) != null) && line.Length > 0) {
                textBuilder.Append(line).Append('\n');
            }
            if (not_eof) textBuilder.Append('\n');

            // Tokenize and tag
            string text = textBuilder.ToString();
            conlluInput.setText(text);
            while (conlluInput.nextTree(tree)) {
                parser.parse(tree);

                string output = conlluOutput.writeTree(tree, conlluInput);
                Console.Write(output);
            }
            if (conlluInput.lastError().Length > 0) {
                Console.Error.WriteLine("Cannot read input CoNLL-U: {0}", conlluInput.lastError());
                return 1;
            }
        }
        return 0;
    }
}
