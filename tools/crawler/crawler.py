#!/usr/bin/env python3
import sqlite3
import time, sys, signal, getopt

from NGAudioParser import NGAudioParser
from http.client import HTTPSConnection

#globals
count_error_serverside, count_error_nf = [0, 0]
error_file, dbcon = [None, None]
local, iteration = [[], range(1, 999999)]

#Config globals
sqlite_filename = "./NG-audio-bridge.sqlite3"
error_filename  = "id_error_list"

doResume = False

#Some usefull tiny funcs
def init_db(filename):
  db = sqlite3.connect(filename)
  db.cursor().execute("CREATE TABLE IF NOT EXISTS Songs( Id INT PRIMARY KEY not null, composer STRING, title STRING, score FLOAT, genre STRING, date DATE, url STRING, tags STRING);")
  db.commit()
  return db

def insert_in_db(db, array):
  if len(array) > 0:
    db.cursor().executemany("INSERT INTO Songs VALUES(?, ?, ?, ?, ?, ?, ?, ?)", tuple(array))
    db.commit()
    array.clear()

def db_get_max_id(db):
  cur = db.cursor()
  cur.execute("SELECT MAX(id) FROM Songs;")
  result = cur.fetchone()
  if not result[0] == None:
    return result[0]
  else:
    return 1

def crawler_cleanup():
  global error_file, dbcon, local
  insert_in_db(dbcon, local)
  error_file.close() #Will flush it too.
  dbcon.close()

#In case of CTRL + C, cleanup
def signal_handler(signal, frame):
  print("[*] SIGINT received, cleaning up !")
  crawler_cleanup()
  print("[*] All done, goodbye o/")
  sys.exit(0)

def init_iter_from_errfile(filename):
  inputfile = open(filename, 'r')
  rslt = inputfile.read().split('\n')
  inputfile.close()
  return tuple(rslt)

def usage():
  print("""\
Usage: crawler.py [OPTIONS]

Options:
  -h, --help:
    Print this help message
  -r, --resume:
    Resume crawling from last ID in tho DB
  -i, --iter <ERR FILE>:
    Change the iteration method to crawl errors.
  -d, --dbfile <DB FILE>:
    Set the db filename (default: NG-audio-bridge.sqlite3)
  -e, --errfile <ERR FILE>:
    Set the error filename (default: id_error_list)\
""")

#ARG Decode
try:
  opts, args = getopt.getopt(sys.argv[1:], "hri:d:e:", ["help", "resume", "iter=", "dbfile=", "errfile="])
except getopt.GetoptError as err:
  print(err)
  usage()
  sys.exit(-1)

for o, a in opts:
  if o in ("-h", "--help"):
    usage()
    sys.exit(0)
  elif o in ("-r", "--resume"):
    doResume = True
  elif o in ("-i", "--iter"):
    iteration = init_iter_from_errfile(a)
  elif o in ("-d", "--dbfile"):
    sqlite_filename = a
  elif o in ("-e", "--errfile"):
    error_filename = a
  else:
    assert False, "unhandled option"

#MAIN
signal.signal(signal.SIGINT, signal_handler)

print("[*] Openning DB...")
dbcon = init_db(sqlite_filename)
print("[*] Creating error file")
error_file = open(error_filename, 'a')

if doResume:
  print("[*] Resuming previous session...")
  iteration = range(db_get_max_id(dbcon) + 1, 999999)

for k in iteration:
  resp = None
  
  for v in range(0,5):
    webconn = HTTPSConnection(NGAudioParser.NG_HOSTNAME)
    webconn.request("GET", NGAudioParser.NG_AUDIO_URI + str(k))
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
        local.append( tuple( [k, obj.composer, obj.title, obj.score, obj.genre, obj.date, obj.url, ';'.join(obj.tags)] ) )
        print("[+] ID: " + str(k) + " - Parsed Successfully")
    else:
        print( "ID: " + str(k), "\t" + obj.errorStr, file=sys.stderr )
        error_file.write(str(k) + '\n')
        error_file.flush()
  elif resp.status == 404:
    count_error_nf += 1
    if count_error_nf >= 100:
        print("[*] No songs since 100 ID, assuming end of crawling.")
        break
  else:
    print("[-] ID: " + str(k) + " - Status code error (" + str( resp.status ) + ")")

  if len(local) >= 250:
    print("[*] Inserting data in DB...")
    insert_in_db(dbcon, local)

#KTHXBYE !
crawler_cleanup()
print("[*] Jobs finished. Good Bye.")
