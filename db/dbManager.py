import sqlite3

class db_manager:

    JSON_SONG_OBJECT="""\
{"id":%s, "composer":"%s", "title":"%s", "score":%s, "genre":"%s", "date":"%s"}\
"""
    
    def __init__(self, db_path):
        self.conn = sqlite3.connect(db_path)

    def get_song_list(self, query_filter):
        local = '['
        cur = self.conn.cursor()
        cur.execute("SELECT Id,composer,title,score,genre,date FROM Songs " + query_filter.get_sql_condition() + " ORDER BY RANDOM() LIMIT 25" )

        result = cur.fetchall()
        for i in range(0, len(result)):
            local += self.JSON_SONG_OBJECT % result[i]
            if i != len(result) -1:
                local += ", "
        local += ']'
        return local

    def get_song_url(self, Id):
        cur = self.conn.cursor()
        cur.execute( "SELECT url FROM Songs WHERE Id=" + str(Id) )

        url = cur.fetchone()[0]
        return url

    def close(self):
        self.conn.close()

if __name__ == "__main__":
    from filter import song_filter
    
    obj = db_manager("db/NG-audio-bridge.sqlite3")
    select_all = song_filter()

    print( obj.get_song_list( select_all ) )
