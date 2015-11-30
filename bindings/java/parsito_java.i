%module parsito_java

%include "../common/parsito.i"

%pragma(java) jniclasscode=%{
  static {
    java.io.File localParsito = new java.io.File(System.mapLibraryName("parsito_java"));

    if (localParsito.exists())
      System.load(localParsito.getAbsolutePath());
    else
      System.loadLibrary("parsito_java");
  }
%}
