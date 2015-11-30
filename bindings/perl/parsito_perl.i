%module "Ufal::Parsito"

%runtime %{
#ifdef seed
  #undef seed
#endif
%}

%include "../common/parsito.i"

%perlcode %{
@EXPORT_OK = qw(*Children:: *Node:: *Nodes:: *Parser:: *Tree::
                *TreeInputFormat:: *TreeOutputFormat:: *Version::);
%EXPORT_TAGS = (all => [@EXPORT_OK]);
%}
