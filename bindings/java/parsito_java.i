%module parsito_java

%include "../common/parsito.i"

%pragma(java) jniclasscode=%{
  static {
    java.io.File libraryFile = new java.io.File(parsito_java.libraryPath);

    if (libraryFile.exists())
      System.load(libraryFile.getAbsolutePath());
    else
      System.loadLibrary("parsito_java");
  }
%}

%pragma(java) modulecode=%{
  static String libraryPath;

  static {
    libraryPath = System.mapLibraryName("parsito_java");
  }

  public static void setLibraryPath(String path) {
    libraryPath = path;
  }
%}
