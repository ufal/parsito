Java Parsito Bindings
========================

Installation
============

Binary Java bindings are available in Parsito binary packages.

To compile Java bindings manually, run make in the bindings/java directory,
optionally with the options descriged in Parsito Installation. Java 6 and newer
is supported.

The Java installation specified in the environment variable JAVA_HOME is used.
If the environment variable does not exist, the JAVA_HOME can be specified using
  make JAVA_HOME=path_to_Java_installation


Java Bindings API
=================

Parsito library bindings is available in the cz.cuni.mff.ufal.parsito package.

The bindings is a straightforward conversion of the C++ bindings API. Vectors do
not have native Java interface, see cz.cuni.mff.ufal.parsito.Children class for
reference. Also, class members are accessible and modifiable using using
getField and setField wrappers.

The bindings require native C++ library libparsito_java (called parsito_java on
Windows). If the library is found in the current directory, it is used,
otherwise standard library search process is used.


C++ Bindings API
================


Helper Structures
-----------------

  typedef vector<int> Children;

  class Node {
   public:
    int id;          // 0 is root, >0 is sentence node, <0 is undefined
    string form;    // form
    string lemma;   // lemma
    string upostag; // universal part-of-speech tag
    string xpostag; // language-specific part-of-speech tag
    string feats;   // list of morphological features
    int head;       // head, 0 is root, <0 is without parent
    string deprel;  // dependency relation to the head
    string deps;    // secondary dependencies
    string misc;    // miscellaneous information

    Children children;

    node(int id = -1, string form = string());
  };
  typedef std::vector<node> Nodes;


Main Classes
------------

  class Tree {
   public:
    Tree();

    Nodes nodes;

    bool empty();
    void clear();
    node& addNode(string form);
    void setHead(int id, int head, string deprel);
    void unlinkAllNodes();

    static const std::string root_form;
  }

  class TreeInputFormat {
   public:
    virtual void setText(string text);
    virtual bool nextTree(tree& t) = 0;
    string lastError() const;

    // Static factory methods
    static TreeInputFormat* newInputFormat(string name);
    static TreeInputFormat* newConlluInputFormat();
  };

  class TreeOutputFormat {
   public:

    virtual string writeTree(const tree& t, const tree_input_format* additional_info = nullptr);

    // Static factory methods
    static TreeOutputFormat* newOutputFormat(string name);
    static TreeOutputFormat* newConlluOutputFormat();
  };

  class Parser {
   public:
    virtual void parse(tree& t, unsigned beam_size = 0) const;

    enum { NO_CACHE = 0, FULL_CACHE = 2147483647};
    static Parser* load(string file, unsigned cache = 1000);
  };

  class Version {
   public:
    unsigned major;
    unsigned minor;
    unsigned patch;
    string prerelease;

    static Version current();
  };
