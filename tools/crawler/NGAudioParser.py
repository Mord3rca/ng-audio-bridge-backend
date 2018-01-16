import http.client, urllib
import json, re

def formatURL(e):
    local = e.replace("\\/", "/")
    pos = local.rfind('?')
    if( pos > 0):
        local = local[:pos]
    return local

class NGAudioParser:
    
    NG_HOSTNAME  = "www.newgrounds.com"
    NG_AUDIO_URI = "/audio/listen/"
    
    RE_DATEFORMAT = re.compile("\d{4}/\d{2}/\d{2}")

    GENRE_LIST = ('Techno', 'Funk', 'Dance', 'Trance', 'Classic Rock', 'Video Game', 'Ambient', 'Drum N Bass', 'Miscellaneous', 'Classical',
                  'Blues', 'Grunge', 'House', 'Hip Hop - Olskool', 'Heavy Metal', 'R&amp;B',
                  'New Wave', 'Hip Hop - Modern', 'Industrial', 'Jazz', 'Goth',
                  'General Rock', 'Country', 'Ska', 'Brit Pop', 'Bluegrass', 'Punk',
                  'World', 'Indie', 'Pop', 'Cinematic', 'Voice Demo',
                  'A Capella', 'Spoken Word', 'Dubstep', 'Solo Instrument',
                  'Storytelling', 'Experimental', 'Fusion', 'Drama', 'Comedy', 'Chipstep', 'Synthwave', 'Informational', 'Nerdcore' )

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

        data, buff = ["",b'']

        while True:
          buff += response.read( 1024 )
          if b'</html>' in buff:
            break

        try:
          data = buff.decode("utf-8").split('\n')
        except:
          self.strError = "[NGParser] Couldn't decode to UTF-8"
          return False

        for line in data:
            if line.find("id=\"score_number\"") > 0:
                pos1, pos2 = [ line.find('>'), line.find('<', 15) ]
                self.score = line[pos1+1:pos2]
                if( self.score == "Awaiting votes"):
                  self.score = -1
            elif line.find("\"params\":") > 0:
                #get the embedded player JSON config
                pos1, pos2 = [ line.find("\"params\":") + len( "\"params\":" ),
                               line.find("\"icon\":") - 1 ]
                line = line[pos1:pos2] + '}'
                
                #Just read it and get the data !
                jsonObj = json.loads(line)
                self.composer = urllib.parse.unquote( jsonObj["artist"] )
                self.url = formatURL( jsonObj["filename"] )
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

        if ( ( float( self.score ) > 5 or float( self.score ) < 0 ) and self.score != -1):
            self.errorStr = "[NGParser] Score not in range [0-5]"
            return False

        if not self.genre in self.GENRE_LIST:
            self.errorStr = "[NGParser] Genre parsing error. Unknown genre: " + self.genre
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
    objid = str( randint( 0, 850000 ) )
    #objid = "710"

    print( "ID: " + objid )
    conn.request("GET", obj.NG_AUDIO_URI + objid )
    check = obj.run( conn.getresponse() )

    if not check:
        print( obj.errorStr )
    else:
        obj.print()

