import http.client, urllib
import json, sys

sys.path.append("../../")
from niceFormat import niceuri

class NGAudioParser:
    
    NG_NAME = "www.newgrounds.com"
    NG_AUDIO_URI = "/audio/listen/"
    
    def __init__(self, id):
        self.processed = False

        self.id = id
        self.score = -1
        self.title = ""
        self.composer = ""
        self.genre = "None"
        self.date = "2003/01/01"
        self.url = ""

    def run(self):
        print( "[*] Fetching: " + self.NG_AUDIO_URI + str( self.id ) )
        
        conn = http.client.HTTPSConnection( self.NG_NAME )
        conn.request("GET", self.NG_AUDIO_URI + str( self.id ))

        response = conn.getresponse()
        if response.status != 200:
            print("[-] Status errror. Reason: " + response.reason)
            response.close()
            return

        while True:
            line = str( response.readline() )

            if line != "":
                try:
                    self.__processLine(line)
                except:
                    print( "[-] Error while parsing sond ID: " + str( self.id ) )
                    break
                
                if self.genre != "None":
                    break
        
        response.close()
        self.processed = True

    def __processLine(self, line):
        if line.find("id=\"score_number\"") > 0:
            pos1, pos2 = [ line.find('>'), line.find('<', 15) ]
            self.score = line[pos1+1:pos2]
        elif line.find("\"params\":") > 0:
            #get the embedded player JSON config
            pos1, pos2 = [ line.find("\"params\":") + len( "\"params\":" ),
                           line.find("\"icon\":") - 1 ]
            line = line[pos1:pos2] + '}'
            
            #Just read it and get the data !
            jsonObj = json.loads(line)
            self.composer = urllib.parse.unquote( jsonObj["artist"] )
            self.url = niceuri( jsonObj["filename"] )
            self.title = urllib.parse.unquote( jsonObj["name"] )
        elif line.find("itemprop=\"datePublished\"") > 0:
            #Get date and format it for the SWF app.
            pos = line.find("content=\"") + len("content=\"")
            year, month, day = line[pos:pos+10].split('-')
            self.date = line[pos:pos+10].replace('-', '/')
        elif line.find("/audio/browse/genre/") > 0:
            pos1,pos2 = [line.find(">", 10), line.find('<', 10)]
            self.genre = line[pos1+1:pos2]
            

if __name__ == "__main__":
    from random import randint
    obj = NGAudioParser( 100000 + randint( 0, 99999 ) )
    obj.run()
    if obj.processed:
        print(obj.composer)
        print( obj.url )
