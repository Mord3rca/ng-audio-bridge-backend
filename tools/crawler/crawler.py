import sqlite3 as lite
import time
from NGAudioParser import NGAudioParser
from threading import Thread, Lock

mutex = Lock()
con = ""

def work(begin, end):
    local = []
    for i in range( begin, end ):
        obj = NGAudioParser(i)
        obj.run()
        if( obj.processed ):
            local.append( tuple( [obj.id, obj.composer, obj.title, obj.score, obj.genre, obj.date, obj.url] ) )

        if i%50 == 0:
            print("Sleeping a bit to avoid NG overflow...")
            time.sleep(15)

        if len(local) > 50 or i == base+100000-1:
            global con,mutex
            print("Inserting data in DB...")
            mutex.acquire()
            con.cursor().executemany("INSERT INTO Songs VALUES(?, ?, ?, ?, ?, ?, ?)", tuple(local))
            con.commit()
            local = []
            mutex.release()

if __name__ == "__main__":
    print("[*] Openning DB...")
    con = lite.connect("./NG-audio-bridge.sqlite3", check_same_thread=False)

    local = []
    for x in range(532,750001):
        obj = NGAudioParser(x)
        obj.run()
        if( obj.processed ):
            local.append( tuple( [obj.id, obj.composer, obj.title, obj.score, obj.genre, obj.date, obj.url] ) )

        if x%50 == 0:
            print("Sleeping a bit to avoid NG overflow...")
            time.sleep(15)

        if len(local) > 50 or x == 750000:
            print("Inserting data in DB...")
            #mutex.acquire()
            con.cursor().executemany("INSERT INTO Songs VALUES(?, ?, ?, ?, ?, ?, ?)", tuple(local))
            con.commit()
            local = []
            #mutex.release()

    print("[*] Jobs finished. Good Bye.")
    con.close()
