import cellml_api.*;
import cellml_services.*;

public class TestMain
{
    private boolean testJavaBindings()
    {
        System.loadLibrary("java_cellml");
        CellMLBootstrap cb = cellml_bootstrap.CellMLBootstrap.createCellMLBootstrap();
        if (cb == null)
        {
            System.out.println("NULL CellMLBootstrap");
            return false;
        }

        Model m = cb.createModel("1.1");
        m.setName("example1");
        
        //creating units
        Units units = m.createUnits();
        m.addElement(units);
        units.setName("second");
        
        Unit unit = m.createUnit();
        units.addElement(unit);
        unit.setUnits("second");
        
        //creating component 1
        CellMLComponent comp1 = m.createComponent(); 
        m.addElement(comp1);
        comp1.setName("component1");        
        
        //adding a variable to component 1
        CellMLVariable var1 = m.createCellMLVariable();
        comp1.addElement(var1);
        var1.setName("variable1");
        var1.setUnitsElement(units);
        var1.setInitialValue("10");
        var1.setPublicInterface(VariableInterface.INTERFACE_OUT);
        VariableInterface pubi = var1.getPublicInterface();
        VariableInterface privi = var1.getPrivateInterface();
        System.out.println("Private interface " +
                           privi.toString());
        //creating component 2
        CellMLComponent comp2 = m.createComponent();
        m.addElement(comp2);
        comp2.setName("component2");
        
        //adding a variable to component 2
        CellMLVariable var2 = m.createCellMLVariable();
        comp2.addElement(var2);
        var2.setName("variable2");
        var2.setUnitsElement(units);
        var2.setPublicInterface(VariableInterface.INTERFACE_IN);
        
        //connecting the two variables (Creating a connection also creates a MapComponent)
        Connection con = m.createConnection();
        m.addElement(con);
        
        MapComponents mapComp = con.getComponentMapping();
        mapComp.setFirstComponent(comp1);
        mapComp.setSecondComponent(comp2);
        
        MapVariables mapvar = m.createMapVariables();
        con.addElement(mapvar);
        
        mapvar.setFirstVariable(var1);
        mapvar.setSecondVariable(var2);
        
        // Now try an out variable...
        System.loadLibrary("java_vacss");
        VACSService vc = cellml_bootstrap.VACSSBootstrap.createVACSService();
        pjm.Reference<Long> col = new pjm.Reference<Long>();
        long row =
            vc.getPositionInXML(pjm2pcm.cellml_api.CellMLDOMElement.queryInterface((pjm.XPCOMDerived)comp1).getDomElement(), 0, col);
        if (col.get() != 56)
        {
            System.out.println("Read column " + col.get() + ", expected 56 - possible problem with out params.");
            return false;
        }
        return true;
    }

    public static void main(String argv[])
    {
        System.exit((new TestMain()).testJavaBindings() ? 0 : 1);
    }
};
