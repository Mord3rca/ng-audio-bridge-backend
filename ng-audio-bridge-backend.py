from dbManager import db_manager
from filter import song_filter
from http.server import HTTPServer, BaseHTTPRequestHandler
import urllib

#THIS PYTHON FILE IS USED TO COMMUNICATE WITH THE APP (ROOT NEEDED for port 80)
HOSTNAME="localhost"
HOSTPORT=80

FILE_CROSSDOMAIN=bytes("""\
<?xml version="1.0"?>	
<!DOCTYPE cross-domain-policy SYSTEM "http://www.adobe.com/xml/dtds/cross-domain-policy.dtd">
<cross-domain-policy>
    <site-control permitted-cross-domain-policies="master-only"/>
    <allow-access-from domain="*" secure="false"/>
    <allow-http-request-headers-from domain="*" headers="*"/>
</cross-domain-policy>\
""", "utf-8")

DB = db_manager("db/NG-audio-bridge.sqlite3")

class HttpHandler(BaseHTTPRequestHandler):
    def do_GET(self):
        if self.path == "/crossdomain.xml":
            self.send_response(200)
            self.send_header("Content-Type", "text/xml")
            self.end_headers()
            self.wfile.write( FILE_CROSSDOMAIN )
        elif self.path.rfind(".mp3") > 0:
            global DB
            #Extracting song id from URL
            pos1,pos2 = [self.path.rfind('/')+1, self.path.rfind(".mp3")]
            song_id = self.path[pos1:pos2]
            #Send moved HTTP code and let NG do it's job !
            self.send_response(301)
            self.send_header("Location", DB.get_song_url(song_id))
            self.end_headers()
        else:
            self.send404()

    def do_POST(self):
        global conn
        if self.path == "/Radio2/FilterBridge.php":
            length = int( self.headers["Content-Length"] )
            data = str( self.rfile.read(length), "utf-8" )
            #Assuming FilterJSON received.
            #FilterJSON = json.loads( urllib.parse.unquote( data[11:] )
            #print( urllib.parse.unquote( data[11:] ) )
            self.send_response(200)
            self.send_header("Content-Type", "application/json")
            
            global DB
            sfilter = song_filter()
            sfilter.load_from_json( urllib.parse.unquote( data[11:] ) )
            
            tmp_str = "{\"ResultSet\":%s}" % DB.get_song_list( sfilter )
            tmp_str = bytes( tmp_str, "utf-8" )
            self.send_header("Content-Length", str(len(tmp_str)))
            self.end_headers()
            self.wfile.write(tmp_str)
        
    def send404(self):
        self.send_response(404)
        self.send_header("Content-Type", "text/html")
        self.end_headers()
        self.wfile.write( bytes("Content Not Found", "utf-8") )

myServer = HTTPServer((HOSTNAME, HOSTPORT), HttpHandler)

if __name__ == "__main__":
    try:
        myServer.serve_forever()
    except KeyboardInterrupt:
        pass
    DB.close()
    myServer.server_close()
