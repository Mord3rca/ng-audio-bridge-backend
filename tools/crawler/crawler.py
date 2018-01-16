#!/usr/bin/env python3
import sqlite3
import time, sys
from NGAudioParser import NGAudioParser
from http.client import HTTPSConnection

count_error_serverside, count_error_nf = [0, 0]
error_file = open("./id_error_list", 'a')
print("[*] Openning DB...")
dbcon = sqlite3.connect("./NG-audio-bridge.sqlite3", check_same_thread=False)
dbcon.cursor().execute("CREATE TABLE IF NOT EXISTS Songs( Id INT PRIMARY KEY not null, composer STRING, title STRING, score FLOAT, genre STRING, date DATE, url STRING, tags STRING);")
dbcon.commit()

local = []
for x in range(1, 999999):
    resp = None
    for v in range(0,5):
        webconn = HTTPSConnection(NGAudioParser.NG_HOSTNAME)
        webconn.request("GET", NGAudioParser.NG_AUDIO_URI + str(x))
        resp = webconn.getresponse()
        if resp.status == 429:
            print("[*] Sleeping...")
            time.sleep(10)
        elif resp.status / 100 == 5:
            print("[*] Server Error Detected ... Long sleep [can solve the problem.]")
            count_error_serverside += 1
            if( count_error_serverside >= 5 ):
                print("[-] Too much error. Exiting")
                exit(-1)
            time.sleep(15 * 60) #Just sleep 15mn and hope.
        else:
            break

    if resp.status == 200:
        count_error_nf = 0
        obj = NGAudioParser()
        if( obj.run( resp ) ):
            local.append( tuple( [x, obj.composer, obj.title, obj.score, obj.genre, obj.date, obj.url, ';'.join(obj.tags)] ) )
            print("[+] ID: " + str(x) + " - Parsed Successfully")
        else:
            print( "ID: " + str(x), "\t" + obj.errorStr, file=sys.stderr )
            error_file.write(str(x) + '\n')
            error_file.flush()
    elif resp.status == 404:
        count_error_nf += 1
        if count_error_nf >= 100:
            print("[*] No songs since 100 ID, assuming end of crawling.")
            exit(0)
    else:
        print("[-] ID: " + str(x) + " - Status code error (" + str( resp.status ) + ")")

    if len(local) >= 250:
        print("Inserting data in DB...")
        dbcon.cursor().executemany("INSERT INTO Songs VALUES(?, ?, ?, ?, ?, ?, ?, ?)", tuple(local))
        dbcon.commit()
        local = []

print("[*] Jobs finished. Good Bye.")
dbcon.close()
error_file.close()
