from cellml_api import CellML_APISPEC
from cellml_api import CIS
import time

canQuit = False

class ResultWatcher:
    def computedConstants(self, values):
        print "Computed Constants:"
        for v in values:
            print ("%g" % v)
        print "End"
    def results(self, values):
        print "Residuals:"
        print values
        print "End"
    def done(self):
        print "Done"
        global canQuit
        canQuit = True
    def failed(self, msg):
        print "Failed: %s" % msg
        global canQuit
        canQuit = True

m = CellML_APISPEC.CellMLBootstrap().getmodelLoader().loadFromURL("./tests/test_xml/newton_raphson_parabola.xml")
cis = CIS.CellMLIntegrationService()
ir = cis.createDAEIntegrationRun(cis.compileModelDAE(m))
ir.setProgressObserver(ResultWatcher())
ir.start()

while not canQuit:
    time.sleep(1)
