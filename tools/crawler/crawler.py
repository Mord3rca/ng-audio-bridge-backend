import sqlite3 as lite
import time, sys
from NGAudioParser import NGAudioParser
from http.client import HTTPSConnection

if __name__ == "__main__":
    print("[*] Openning DB...")
    dbcon = lite.connect("./NG-audio-bridge.sqlite3", check_same_thread=False)
    dbcon.cursor().execute("CREATE TABLE IF NOT EXISTS Songs( Id INT PRIMARY KEY not null, composer STRING, title STRING, score FLOAT, genre STRING, date DATE, url STRING, tags STRING);")
    dbcon.commit()

    local = []
    for x in range(1, 250000):
        resp = None
        for v in range(0,3):
            webconn = HTTPSConnection(NGAudioParser.NG_HOSTNAME)
            webconn.request("GET", NGAudioParser.NG_AUDIO_URI + str(x))
            resp = webconn.getresponse()
            if resp.status == 429:
                print("Sleeping...")
                time.sleep(10)
            else:
                break

        if resp.status == 200:
            obj = NGAudioParser()
            if( obj.run( resp ) ):
                local.append( tuple( [x, obj.composer, obj.title, obj.score, obj.genre, obj.date, obj.url, ';'.join(obj.tags)] ) )
                print("[+] ID: " + str(x) + " - Parsed Successfully")
            else:
                print( "ID: " + str(x), "\t" + obj.errorStr, file=sys.stderr )
        else:
            print("[-] ID: " + str(x) + " - Status code error (" + str( resp.status ) + ")")

        if len(local) >= 250 or x == 509:
            print("Inserting data in DB...")
            dbcon.cursor().executemany("INSERT INTO Songs VALUES(?, ?, ?, ?, ?, ?, ?, ?)", tuple(local))
            dbcon.commit()
            local = []

    print("[*] Jobs finished. Good Bye.")
    dbcon.close()
