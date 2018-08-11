#include <iostream>

extern "C"
{
  #include <unistd.h> //daemon()
  #include <syslog.h> //Will be use... Probably in his own object.
}

#include <csignal> // signal() : To properly free all the ressources.

#include "args.hpp"
#include "Audio-Bridge-Server.hpp"
#include "AudioDatabase.hpp"

cArg *argument = nullptr;
AudioServer *Server = nullptr;

static void daemonize(void)
{
  daemon(0, 0);
  chdir("/");
}

//You really have to check your privilege !
static void drop_privilege(const char* user, const char* group)
{
  //Write the chg user stuff here (IF Root)
}

static void signal_handler( int sig )
{
  switch( sig )
  {
    case SIGINT:  
      if( Server != nullptr )
        Server->stop();
      
      std::cout << "Server stopped..." << std::endl;
      break;
    
    default:;
  }
}

int main( int argc, char *argv[] )
{
  AudioDatabase *db = nullptr;
  argument = new cArg(argc, argv);
  
  if( argument->daemonize() )
    daemonize();
  
  signal(SIGINT, signal_handler);
  
  db = new AudioDatabase();
  if( !db->openDBFile(argument->getDBPath()) )
  {
    std::cerr << "Can't open DB File. Exiting..." << std::endl;
    return -1;
  }
  
  std::cout << "Starting up..." << std::endl;
  Server = new AudioServer( argument->getListenIP(),
                            argument->getListeningPort());
  
  Server->setDBController(db);
  
  drop_privilege( argument->getUserName().c_str(),
                  argument->getGroupName().c_str() );
  
  delete argument; //No longer needed.
  
  Server->serve();
  
  delete Server;
  delete db;
  
  
  return 0;
}
