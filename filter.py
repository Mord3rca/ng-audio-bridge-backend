import json
from datetime import date

class song_filter:
    def __init__(self):
        self.minScore = 0
        self.maxScore = 5

        self.minDate = "2003/01/01"
        self.maxDate = date.today().strftime("%Y/%m/%d")

        self.genres = ["Ambient","Bluegrass","Blues","Brit Pop","Classical",
                       "Classical Rock","Country","Dance","Drum N Bass",
                       "Dubstep","Funk","General Rock","Goth","Grunge",
                       "Heavy Metal","Hip Hop - Modern","Hip Hop - Olskool",
                       "House","Indie","Industrial","Jazz","Latin",
                       "Miscellaneous","New Wave","Pop","Punk","R&amp;B","Ska",
                       "Techno","Trance","Video Game","Voice Acting"]

        self.allowAll = True

    def load_from_json(self, js_filter):
        js = json.loads(js_filter)

        self.minScore = js["minScore"]
        self.maxScore = js["maxScore"]

        self.minDate = js["minDate"].replace('-', '/')
        self.maxDate = js["maxDate"].replace('-', '/')

        self.genres = []
        for genre in js["genres"]:
            if genre == "R%26B":
                #Compatibility with my db.
                self.genres.append("R&amp;B")
            else:
                self.genres.append( genre )
            
        self.allowAll = (self.minScore == 0 and self.maxScore == 5)\
                        and (self.minDate == "2003/01/01" and self.maxDate == date.today().strftime("%Y/%m/%d") )\
                        and (len(self.genres) == 32)
        
    def get_sql_condition(self):
        local = ""
        if not self.allowAll:
            local = "WHERE "
            conditions = []
            #Is date condition necessary ?
            if(self.minDate != "2003/01/01" or self.maxDate != date.today().strftime("%Y/%m/%d")):
                conditions.append( """(date >= "%s" and date <= "%s")""" % (self.minDate, self.maxDate) )

            #Is Score condition necessary ?
            if( self.minScore != 0 or self.maxScore != 5 ):
                conditions.append( """(score >= %s and score <= %s)"""   % (self.minScore, self.maxScore) )

            #Is genre condition necessary ?
            if( len(self.genres) != 32 ):
                if len(self.genres) != 1:
                    conditions.append( """genre IN %s""" % str( tuple(self.genres) ) )
                else:
                    conditions.append( """genre=\"%s\"""" % self.genres[0] )

            #Create condition string
            for i in range(0, len(conditions)):
                local += conditions[i]
                #Remember te put AND between them.
                if i != len(conditions) -1:
                    local += " AND "

        return local

if __name__ == "__main__":

    TEST_JSON_FILTER_ALLOWALL="""\
    {"maxScore":5,"minDate":"2003-01-01","genres":["Ambient","Bluegrass","Blues","Brit Pop","Classical","Classical Rock","Country","Dance","Drum N Bass","Dubstep","Funk","General Rock","Goth","Grunge","Heavy Metal","Hip Hop - Modern","Hip Hop - Olskool","House","Indie","Industrial","Jazz","Latin","Miscellaneous","New Wave","Pop","Punk","R%%26B","Ska","Techno","Trance","Video Game","Voice Acting"],"minScore":0,"maxDate":"%s"}\
    """ % date.today().strftime("%Y/%m/%d")

    TEST_JSON_FILTER_SCORE="""\
    {"maxScore":5,"minDate":"2003-01-01","genres":["Ambient","Bluegrass","Blues","Brit Pop","Classical","Classical Rock","Country","Dance","Drum N Bass","Dubstep","Funk","General Rock","Goth","Grunge","Heavy Metal","Hip Hop - Modern","Hip Hop - Olskool","House","Indie","Industrial","Jazz","Latin","Miscellaneous","New Wave","Pop","Punk","R%%26B","Ska","Techno","Trance","Video Game","Voice Acting"],"minScore":4,"maxDate":"%s"}\
    """ % date.today().strftime("%Y/%m/%d")

    TEST_JSON_FILTER_GENRE="""\
    {"maxScore":5,"minDate":"2003-01-01","genres":["R%%26B","Ska","Techno","Trance","Video Game","Voice Acting"],"minScore":0,"maxDate":"%s"}\
    """ % date.today().strftime("%Y/%m/%d")
    
    obj = song_filter()
    obj.load_from_json( TEST_JSON_FILTER_GENRE )

    if obj.allowAll :
        print( "Allowing all, no where condition." )
    else:
        print( obj.get_sql_condition() )
