package example;
import cellml_api.*;

public class TestException {
  public static void main(String[] args) {
     System.loadLibrary("java_cellml");
     CellMLBootstrap bootstrap = cellml_bootstrap.CellMLBootstrap.createCellMLBootstrap();
     try {
       // We provoke an exception by calling createModel with an invalid version...
       bootstrap.createModel("1.3");
     } catch (CellMLException e) {
        System.out.println("Got expected exception, error was: " + e.explanation);
     }
  }
};
