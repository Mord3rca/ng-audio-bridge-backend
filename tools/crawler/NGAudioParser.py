import http.client, urllib
import json, sys, re

sys.path.append("../../")
from niceFormat import niceuri

class NGAudioParser:
    
    NG_HOSTNAME  = "www.newgrounds.com"
    NG_AUDIO_URI = "/audio/listen/"
    
    RE_DATEFORMAT = re.compile("\d{4}/\d{2}/\d{2}")

    def __init__(self):
        self.errorStr = "Obj not run."

        self.score = -1
        self.title = ""
        self.composer = ""
        self.genre = "Error"
        self.date = "2003/01/01"
        self.url = ""
        self.tags = []

    def run(self, response):
        if response.status != 200:
            print("[-] Status errror. Reason: " + response.reason)
            response.close()
            return

        #data, buff = ["","."*100]

        #while len(buff) >= 100:
        #    buff = response.read(100)
        #    try:
        #        data += buff.decode("utf-8")
        #    except:
        #        pass
        
        data = response.read(64 * 1024).decode("utf-8").split('\n')

        for line in data:
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
            elif line.find("/audio/browse/tag/") > 0:
                pos1,pos2 = [line.find(">", 10), line.find('<', 10)]
                self.tags.append( line[pos1+1:pos2] )
        
        response.close()
        
        return self.__validate()
    
    def __validate(self):
        if not self.RE_DATEFORMAT.match( self.date ):
            self.errorStr = "[NGParser] Date parsing failed."
            return False

        if ( float( self.score ) > 5 or float( self.score ) < 0 ):
            self.errorStr = "[NGParser] Score not in range [0-5]"
            return False

        #TODO: PUT Genre verification here.
        if self.genre == "Error":
            self.errorStr = "[NGParser] Genre parsing error."
            return False
        
        return True

    def print(self):
        #print( "id: " + str(self.id) )
        print( "Title: " + str(self.title))
        print( "Composer: " + str(self.composer) )
        print( "Score: " + str( self.score ) )
        print( "Date: " + self.date )
        print( "Genre: " + self.genre )
        print( "Tags: ", self.tags)

#Just for testing
if __name__ == "__main__":
    from random import randint
    obj, conn = [NGAudioParser(), http.client.HTTPSConnection("www.newgrounds.com")]
    objid = str( randint( 0, 99999 ) )
    #objid = "594742"
    conn.request("GET", obj.NG_AUDIO_URI + objid )
    check = obj.run( conn.getresponse() )

    print( "ID: " + objid )
    if not check:
        print( obj.errorStr )
    else:
        obj.print()

