%module "Ufal::Parsito"

%runtime %{
#ifdef seed
  #undef seed
#endif
%}

%include "../common/parsito.i"
