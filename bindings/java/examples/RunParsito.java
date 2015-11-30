// This file is part of Parsito <http://github.com/ufal/parsito/>.
//
// Copyright 2015 Institute of Formal and Applied Linguistics, Faculty of
// Mathematics and Physics, Charles University in Prague, Czech Republic.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

import cz.cuni.mff.ufal.parsito.*;
import java.util.Scanner;

class RunParsito {
  public static void main(String[] args) {
    if (args.length == 0) {
      System.err.println("Usage: RunParsito parser_file");
      System.exit(1);
    }

    System.err.print("Loading parser: ");
    Parser parser = Parser.load(args[0]);
    if (parser == null) {
      System.err.println("Cannot load parser from file '" + args[0] + "'");
      System.exit(1);
    }
    System.err.println("done");


    TreeInputFormat conlluInput = TreeInputFormat.newInputFormat("conllu");
    TreeOutputFormat conlluOutput = TreeOutputFormat.newOutputFormat("conllu");
    Scanner reader = new Scanner(System.in);
    Tree tree = new Tree();

    boolean not_eof = true;
    while (not_eof) {
      StringBuilder textBuilder = new StringBuilder();
      String line;

      // Read block
      while ((not_eof = reader.hasNextLine()) && !(line = reader.nextLine()).isEmpty()) {
        textBuilder.append(line);
        textBuilder.append('\n');
      }
      if (not_eof) textBuilder.append('\n');

      // Tokenize and tag
      String text = textBuilder.toString();
      conlluInput.setText(text);
      while (conlluInput.nextTree(tree)) {
        parser.parse(tree);

        String output = conlluOutput.writeTree(tree, conlluInput);
        System.out.print(output);
      }
      if (!conlluInput.lastError().isEmpty()) {
        System.err.println("Cannot read input CoNLL-U: " + conlluInput.lastError());
        System.exit(1);
      }
    }
  }
}
